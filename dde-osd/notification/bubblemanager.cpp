/*
 * Copyright (C) 2014 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     kirigaya <kirigaya@mkacg.com>
 *             listenerri <listenerri@gmail.com>
 *
 * Maintainer: listenerri <listenerri@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bubblemanager.h"
#include <QStringList>
#include <QVariantMap>
#include <QTimer>
#include "bubble.h"
#include "dbuscontrol.h"
#include "dbus_daemon_interface.h"
#include "dbuslogin1manager.h"
#include "notificationentity.h"

#include "persistence.h"

#include <QTimer>
#include <QDebug>
#include <QXmlStreamReader>
#include <QGuiApplication>
#include <QScreen>
#include <DDesktopServices>
#include "sessiontype.h"

DWIDGET_USE_NAMESPACE

static QString removeHTML(const QString &source) {
    QXmlStreamReader xml(source);
    QString textString;
    while (!xml.atEnd()) {
        if ( xml.readNext() == QXmlStreamReader::Characters ) {
            textString += xml.text();
        }
    }

    return textString.isEmpty() ? source : textString;
}

BubbleManager::BubbleManager(QObject *parent)
    : QObject(parent)
{
    m_bubble = new Bubble;
    m_persistence = new Persistence;
    m_dockPosition = DockPosition::Bottom;

    m_dbusDaemonInterface = new DBusDaemonInterface(DBusDaemonDBusService, DBusDaemonDBusPath,
                                                    QDBusConnection::sessionBus(), this);
    m_dbusdockinterface = new DBusDockInterface(DBbsDockDBusServer, DBusDockDBusPath,
                                                QDBusConnection::sessionBus(), this);
    m_dockDeamonInter = new DockDaemonInter(DockDaemonDBusServie, DockDaemonDBusPath,
                                            QDBusConnection::sessionBus(), this);
    m_dbusControlCenter = new DBusControlCenter(ControlCenterDBusService, ControlCenterDBusPath,
                                                    QDBusConnection::sessionBus(), this);
    m_login1ManagerInterface = new Login1ManagerInterface(Login1DBusService, Login1DBusPath,
                                                          QDBusConnection::systemBus(), this);

    connect(m_bubble, SIGNAL(expired(int)), this, SLOT(bubbleExpired(int)));
    connect(m_bubble, SIGNAL(dismissed(int)), this, SLOT(bubbleDismissed(int)));
    connect(m_bubble, SIGNAL(replacedByOther(int)), this, SLOT(bubbleReplacedByOther(int)));
    connect(m_bubble, SIGNAL(actionInvoked(uint, QString)), this, SLOT(bubbleActionInvoked(uint, QString)));

    connect(m_persistence, &Persistence::RecordAdded, this, &BubbleManager::onRecordAdded);
    connect(m_login1ManagerInterface, SIGNAL(PrepareForSleep(bool)),
            this, SLOT(onPrepareForSleep(bool)));

    connect(m_dbusDaemonInterface, SIGNAL(NameOwnerChanged(QString, QString, QString)),
            this, SLOT(onDbusNameOwnerChanged(QString, QString, QString)));
    connect(m_dbusdockinterface, &DBusDockInterface::geometryChanged, this, &BubbleManager::onDockRectChanged);

    // The gxde dock (top.gxde.daemon.dock) does not expose typed property-change
    // signals on its generated proxy, and Qt6's QDBusAbstractInterface does not
    // relay org.freedesktop.DBus.Properties.PropertiesChanged into a proxy signal.
    // Connect to the raw D-Bus PropertiesChanged signal of the gxde dock so the
    // notification window actually reacts to taskbar size/position changes.
    QDBusConnection::sessionBus().connect(
        DockDaemonDBusServie, DockDaemonDBusPath,
        "org.freedesktop.DBus.Properties", "PropertiesChanged",
        this, SLOT(onDockPropertiesChanged(QString, QVariantMap, QStringList)));
    connect(m_dbusControlCenter, &DBusControlCenter::destRectChanged,
            this, &BubbleManager::onCCDestRectChanged);
    connect(m_dbusControlCenter, &DBusControlCenter::rectChanged,
            this, &BubbleManager::onCCRectChanged);

    // get correct value for m_dockGeometry, m_dockPosition, m_ccGeometry
    // NOTE: com.deepin.dde.daemon.Dock is broken under Wayland, so its geometry
    // is ignored here; the gxde dock is the authoritative source.
    if (!SessionType::isWayland() && m_dbusControlCenter->isValid())
        onCCRectChanged(m_dbusControlCenter->rect());

    // The gxde dock (top.gxde.daemon.dock) is the authoritative source for the
    // window geometry. Its property may not be cached synchronously at startup,
    // so defer the read to the event loop.
    if (m_dockDeamonInter->isValid())
        QTimer::singleShot(0, this, [this] {
            onDockPositionChanged(m_dockDeamonInter->position());
            onDockFrontendRectChanged(m_dockDeamonInter->frontendWindowRect());
        });

    registerAsService();
}

BubbleManager::~BubbleManager()
{

}

void BubbleManager::CloseNotification(uint id)
{
    bubbleDismissed(id);

    return;
}

QStringList BubbleManager::GetCapabilities()
{
    QStringList result;
    result << "action-icons" << "actions" << "body" << "body-hyperlinks" << "body-markup";

    return result;
}

QString BubbleManager::GetServerInformation(QString &name, QString &vender, QString &version)
{
    name = QString("DeepinNotifications");
    vender = QString("Deepin");
    version = QString("2.0");

    return QString("1.2");
}

uint BubbleManager::Notify(const QString &appName, uint replacesId,
                           const QString &appIcon, const QString &summary,
                           const QString &body, const QStringList &actions,
                           const QVariantMap hints, int expireTimeout)
{
#ifdef QT_DEBUG
    qDebug() << "a new Notify:" << "appName:" + appName << "replaceID:" + QString::number(replacesId)
             << "appIcon:" + appIcon << "summary:" + summary << "body:" + body
             << "actions:" << actions << "hints:" << hints << "expireTimeout:" << expireTimeout;
#endif

    // The generated NotificationsDBusAdaptor used to play this effect, but the
    // adaptor is never instantiated: registerAsService() exports this object
    // directly, so Notify() is invoked here instead. Play the effect at the
    // real entry point, honouring the freedesktop "suppress-sound" hint.
    if (!hints.value("suppress-sound").toBool())
        playNotifySound();

    NotificationEntity *notification = new NotificationEntity(appName, QString(), appIcon,
                                                              summary, removeHTML(body), actions, hints,
                                                              QString::number(QDateTime::currentMSecsSinceEpoch()),
                                                              QString::number(replacesId),
                                                              QString::number(expireTimeout),
                                                              this);

    if (!m_currentNotify.isNull() && replacesId != 0 && (m_currentNotify->id() == replacesId
                                                         || m_currentNotify->replacesId() == QString::number(replacesId))) {
        m_bubble->setEntity(notification);

        m_currentNotify->deleteLater();
        m_currentNotify = notification;
    } else {
        m_entities.enqueue(notification);
    }

    m_persistence->addOne(notification);

    if (!m_bubble->isVisible()) { consumeEntities(); }

    // If replaces_id is 0, the return value is a UINT32 that represent the notification.
    // If replaces_id is not 0, the returned value is the same value as replaces_id.
    return replacesId == 0 ? notification->id() : replacesId;
}

QString BubbleManager::GetAllRecords()
{
    return m_persistence->getAll();
}

QString BubbleManager::GetRecordById(const QString &id)
{
    return m_persistence->getById(id);
}

QString BubbleManager::GetRecordsFromId(int rowCount, const QString &offsetId)
{
    return m_persistence->getFrom(rowCount, offsetId);
}

void BubbleManager::RemoveRecord(const QString &id)
{
    m_persistence->removeOne(id);

    QFile file(CachePath + id + ".png");
    file.remove();
}

void BubbleManager::ClearRecords()
{
    m_persistence->removeAll();

    QDir dir(CachePath);
    dir.removeRecursively();
}

void BubbleManager::onRecordAdded(NotificationEntity *entity)
{
    QJsonObject notifyJson
    {
        {"name", entity->appName()},
        {"icon", entity->appIcon()},
        {"summary", entity->summary()},
        {"body", entity->body()},
        {"id", QString::number(entity->id())},
        {"time", entity->ctime()}
    };
    QJsonDocument doc(notifyJson);
    QString notify(doc.toJson(QJsonDocument::Compact));

    Q_EMIT RecordAdded(notify);
}

void BubbleManager::registerAsService()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.interface()->registerService(NotificationsDBusService,
                                                  QDBusConnectionInterface::ReplaceExistingService,
                                                  QDBusConnectionInterface::AllowReplacement);
    connection.registerObject(NotificationsDBusPath, this);

    QDBusConnection ddenotifyConnect = QDBusConnection::sessionBus();
    ddenotifyConnect.interface()->registerService(DDENotifyDBusServer,
                                                  QDBusConnectionInterface::ReplaceExistingService,
                                                  QDBusConnectionInterface::AllowReplacement);
    ddenotifyConnect.registerObject(DDENotifyDBusPath, this);
}

void BubbleManager::onCCDestRectChanged(const QRect &destRect)
{
    if (SessionType::isWayland()) {
        QScreen *primaryScreen = QGuiApplication::primaryScreen();
        const QRect screenRect = primaryScreen ? primaryScreen->geometry() : QRect();
        const int screenEnd = screenRect.right() + 1;

        if (destRect.x() < screenEnd && destRect.width() > 0) {
            // An explicitly on-screen destination means the panel is opening.
            m_controlCenterClosing = false;
            m_controlCenterVisible = true;
            m_ccGeometry = destRect;
        } else if (m_controlCenterVisible || destRect.x() > screenEnd) {
            m_controlCenterClosing = true;
            m_controlCenterVisible = false;
            m_ccGeometry = QRect();
        } else if (destRect.x() == screenEnd) {
            m_controlCenterClosing = false;
        }

        m_bubble->setBasePosition(getX(), getBottom());
        return;
    }

    // get the current rect of control-center
    m_ccGeometry = m_dbusControlCenter->rect();
    // use the current rect of control-center to setup position of bubble
    // to avoid a move-anim bug
    m_bubble->setBasePosition(getX(), getBottom());

    // use destination rect of control-center to setup move-anim
    if (destRect.width() == 0) { // closing the control-center
        if (m_dockPosition == DockPosition::Right) {
            const QRect &screenRect = screensInfo(QCursor::pos()).first;
            if ((screenRect.height() - m_dockGeometry.height()) / 2.0 < m_bubble->height()) {
                QRect mRect = destRect;
                mRect.setX((screenRect.right()) - m_dockGeometry.width());
                m_bubble->resetMoveAnim(mRect);
                return;
            }
        }
    }
    m_bubble->resetMoveAnim(destRect);
}

void BubbleManager::onCCRectChanged(const QRect &rect)
{
    if (SessionType::isWayland()) {
        QScreen *primaryScreen = QGuiApplication::primaryScreen();
        const QRect screenRect = primaryScreen ? primaryScreen->geometry() : QRect();
        const int screenEnd = screenRect.right() + 1;

        if (!m_controlCenterClosing
                && m_lastControlCenterX != std::numeric_limits<int>::max()
                && rect.x() < m_lastControlCenterX
                && rect.x() < screenEnd && rect.width() > 0) {
            // The panel is sliding in from the right.  Reserve its fully-open
            // width immediately so a notification never sits underneath it.
            m_controlCenterVisible = true;
            m_ccGeometry = QRect(screenEnd - rect.width(), screenRect.y(),
                rect.width(), rect.height());
            m_bubble->setBasePosition(getX(), getBottom());
        }

        m_lastControlCenterX = rect.x();
        return;
    }

    m_ccGeometry = rect;
    // do NOT call setBasePosition here
}

void BubbleManager::bubbleExpired(int id)
{
    m_bubble->setVisible(false);
    Q_EMIT NotificationClosed(id, BubbleManager::Expired);

    consumeEntities();
}

void BubbleManager::bubbleDismissed(int id)
{
    m_bubble->setVisible(false);
    Q_EMIT NotificationClosed(id, BubbleManager::Dismissed);

    consumeEntities();
}

void BubbleManager::bubbleReplacedByOther(int id)
{
    Q_EMIT NotificationClosed(id, BubbleManager::Unknown);
}

void BubbleManager::bubbleActionInvoked(uint id, QString actionId)
{
    m_bubble->setVisible(false);
    Q_EMIT ActionInvoked(id, actionId);
    Q_EMIT NotificationClosed(id, BubbleManager::Closed);
    consumeEntities();
}

void BubbleManager::onPrepareForSleep(bool sleep)
{
    // workaround to avoid the "About to suspend..." notifications still
    // hanging there on restoring from sleep confusing users.
    if (!sleep) {
        qDebug() << "Quit on restoring from sleep.";
        qApp->quit();
    }
}

bool BubbleManager::checkDockExistence()
{
    return m_dbusDaemonInterface->NameHasOwner(DBbsDockDBusServer).value();
}

bool BubbleManager::checkControlCenterExistence()
{
    return m_dbusDaemonInterface->NameHasOwner(ControlCenterDBusService).value();
}

int BubbleManager::getX()
{
    QPair<QRect, bool> pair = screensInfo(QCursor::pos());
    const QRect &rect = pair.first;
    const int maxX = rect.x() + rect.width();

    // directly show the notify on the screen containing mouse,
    // because dock and control-centor will only be displayed on the primary screen.
    if (!pair.second)
        return  maxX;

    if (SessionType::isWayland()) {
        if (m_controlCenterVisible && !m_ccGeometry.isEmpty()
                && m_ccGeometry.x() > rect.x() && m_ccGeometry.x() < rect.right())
            return m_ccGeometry.x();
        return maxX;
    }

    const bool isCCDbusValid = m_dbusControlCenter->isValid();
    const bool isDockDbusValid = m_dbusdockinterface->isValid() || m_dockDeamonInter->isValid();

    // DBus object is invalid, return screen right
    if (!isCCDbusValid && !isDockDbusValid)
        return maxX;

    // Derive the dock side from its actual window geometry so this works for
    // both the deepin dock and the gxde dock (their position enums differ). A
    // dock is "on a side" when it is a tall, narrow vertical bar.
    const bool dockValid = isDockDbusValid && !m_dockGeometry.isEmpty();
    const bool dockAtRight = dockValid
                             && m_dockGeometry.height() >= m_dockGeometry.width()
                             && m_dockGeometry.x() > rect.x();
    const bool dockAtLeft = dockValid
                            && m_dockGeometry.height() >= m_dockGeometry.width()
                            && m_dockGeometry.right() < rect.right();

    // A right-hand dock occupies the right edge: keep the bubble to its left.
    if (dockAtRight) {
        // check dde-control-center is valid
        if (isCCDbusValid) {
            if (m_ccGeometry.x() >  m_dockGeometry.x()) {
                if (((rect.height() - m_dockGeometry.height()) / 2) > (BubbleHeight + Padding)) {
                    return maxX;
                } else {
                    return maxX - m_dockGeometry.width();
                }
            } else {
                return m_ccGeometry.x();
            }
        }
        // dde-control-center is invalid, return dock' x
        return maxX - m_dockGeometry.width();
    }

    // A left-hand dock, a top/bottom dock, or no dock does not affect the
    // right-aligned bubble horizontally. Only avoid an open control-center that
    // slides in from the right (a non-empty rect whose left edge is on-screen
    // and to the right of the dock/primary origin). When the control-center is
    // not actually open its rect is empty/at the origin, so fall back to the
    // screen's right edge instead of pinning the bubble to the left (which was
    // the previous behaviour and produced a wrong bottom-left placement).
    if (isCCDbusValid && !m_ccGeometry.isEmpty()
            && m_ccGeometry.x() > rect.x() && m_ccGeometry.x() < rect.right()
            && (!dockAtLeft || m_ccGeometry.x() > m_dockGeometry.right()))
        return m_ccGeometry.x();

    return maxX;
}

int BubbleManager::getY()
{
    QPair<QRect, bool> pair = screensInfo(QCursor::pos());
    const QRect &rect = pair.first;

    if (!pair.second)
        return  rect.y();

    if (!m_dbusdockinterface->isValid() && !m_dockDeamonInter->isValid())
        return rect.y();

    if (m_dockPosition == DockPosition::Top)
        return m_dockGeometry.bottom();

    return rect.y();
}

int BubbleManager::getBottom()
{
    // Pin the bubble to the bottom-right corner of the screen, but leave room
    // for the dock when it sits at the bottom so the notification does not cover
    // it. The dock position is derived from the dock window geometry itself so it
    // works regardless of which dock service / enum is active.
    QPair<QRect, bool> pair = screensInfo(QCursor::pos());
    const QRect &rect = pair.first;

    const int bottom_default_padding = BubbleHeight + Padding + Padding;

    if (SessionType::isWayland())
        return rect.y() + rect.height() - bottom_default_padding;

    const bool dockAvailable = m_dbusdockinterface->isValid() || m_dockDeamonInter->isValid();

    // A horizontal panel is not necessarily a bottom dock. In particular, an
    // X11 screen may have a non-zero global origin, making a top panel's y
    // coordinate positive. Only reserve space for a panel which the dock
    // service identifies as Bottom and which actually touches this screen's
    // bottom edge.
    const bool dockAtBottom = dockAvailable
                              && m_dockPosition == DockPosition::Bottom
                              && !m_dockGeometry.isEmpty()
                              && m_dockGeometry.width() >= m_dockGeometry.height()
                              && m_dockGeometry.right() >= rect.left()
                              && m_dockGeometry.left() <= rect.right()
                              && qAbs(m_dockGeometry.bottom() - rect.bottom()) <= 1;

    if (!pair.second || !dockAtBottom)
        goto rect_bottom_of_bubble_top;

    return m_dockGeometry.top() - bottom_default_padding;

rect_bottom_of_bubble_top:
    return rect.y() + rect.height() - bottom_default_padding;
}

QPair<QRect, bool> BubbleManager::screensInfo(const QPoint &point) const
{
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    QScreen *pointScreen = QGuiApplication::screenAt(point);
    if (!pointScreen)
        pointScreen = primaryScreen;

    if (!pointScreen)
        return QPair<QRect, bool>(QRect(), false);

    return QPair<QRect, bool>(pointScreen->geometry(), pointScreen == primaryScreen);
}

void BubbleManager::playNotifySound()
{
    // DTK's DDesktopServices::playSystemSoundEffect() is fixed for .wav files
    // (it now calls QSoundEffect::play() and manages the object lifetime), so
    // use it directly. It honours the "com.deepin.dde.sound-effect" GSettings
    // toggle on its own.
    DDesktopServices::playSystemSoundEffect(DDesktopServices::SSE_Notifications);
}

QRect BubbleManager::toLogicalGeometry(const QRect &deviceRect) const
{
    if (deviceRect.isEmpty())
        return deviceRect;

    // Wayland already hands out logical coordinates, no conversion needed.
    if (SessionType::isWayland())
        return deviceRect;

    QScreen *screen = nullptr;
    for (QScreen *candidate : QGuiApplication::screens()) {
        QRect rawGeometry(candidate->geometry().topLeft(),
                          candidate->geometry().size() * candidate->devicePixelRatio());
        if (rawGeometry.contains(deviceRect.topLeft())) {
            screen = candidate;
            break;
        }
    }
    if (!screen)
        screen = QGuiApplication::primaryScreen();
    if (!screen)
        return deviceRect;

    const qreal ratio = screen->devicePixelRatio();
    if (qFuzzyCompare(ratio, 1.0) || ratio <= 0)
        return deviceRect;

    const QPoint origin = screen->geometry().topLeft();
    const QPoint logicalTopLeft = origin
                                  + (deviceRect.topLeft() - origin) / ratio;
    return QRect(logicalTopLeft,
                 QSize(qRound(deviceRect.width() / ratio),
                       qRound(deviceRect.height() / ratio)));
}

void BubbleManager::onDockRectChanged(const QRect &geometry)
{
    // When the gxde dock is available it is the authoritative source for the
    // geometry, so ignore the (possibly wrong) deepin dock geometry.
    if (m_dockDeamonInter->isValid())
        return;

    m_dockGeometry = toLogicalGeometry(geometry);

    m_bubble->setBasePosition(getX(), getBottom());
}

void BubbleManager::onDockPositionChanged(int position)
{
    m_dockPosition = static_cast<DockPosition>(position);
}

void BubbleManager::onDockFrontendRectChanged(const QRect &rect)
{
    // The gxde dock (top.gxde.daemon.dock) exposes its actual visible window
    // rectangle via FrontendWindowRect, which already reflects the hide state.
    // Use it as the dock geometry so notifications can avoid covering it.
    m_dockGeometry = toLogicalGeometry(rect);

    m_bubble->setBasePosition(getX(), getBottom());
}

void BubbleManager::onDockPropertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    Q_UNUSED(interface);

    // The gxde dock does not emit a typed FrontendWindowRectChanged signal; react
    // to the generic property change notification instead. Process Position
    // first: the rectangle and side are often emitted together, and positioning
    // from the old side briefly puts a top panel notification at the screen top.
    if (changed.contains("Position") || invalidated.contains("Position"))
        onDockPositionChanged(m_dockDeamonInter->position());
    if (changed.contains("FrontendWindowRect") || invalidated.contains("FrontendWindowRect"))
        onDockFrontendRectChanged(m_dockDeamonInter->frontendWindowRect());
}

void BubbleManager::onDbusNameOwnerChanged(QString name, QString, QString newName)
{
    if (name == ControlCenterDBusService && screensInfo(m_bubble->pos()).second && !newName.isEmpty()) {
        if (!SessionType::isWayland())
            onCCRectChanged(m_dbusControlCenter->rect());
    } else if (name == DBbsDockDBusServer && !newName.isEmpty() && !m_dockDeamonInter->isValid()) {
        onDockRectChanged(m_dbusdockinterface->geometry());
    } else if (name == DockDaemonDBusServie && !newName.isEmpty()) {
        onDockPositionChanged(m_dockDeamonInter->position());
        onDockFrontendRectChanged(m_dockDeamonInter->frontendWindowRect());
    }
}

void BubbleManager::consumeEntities()
{
    if (!m_currentNotify.isNull()) {
        m_currentNotify->deleteLater();
        m_currentNotify = nullptr;
    }

    if (m_entities.isEmpty()) {
        m_currentNotify = nullptr;
        return;
    }

    m_currentNotify = m_entities.dequeue();

    QScreen *targetScreen = QGuiApplication::screenAt(QCursor::pos());
    if (!targetScreen)
        targetScreen = QGuiApplication::primaryScreen();

    const QRect screenGeometry = targetScreen ? targetScreen->geometry() : QRect();

    if (SessionType::isWayland() && targetScreen)
        m_bubble->setScreen(targetScreen);

    m_bubble->setBasePosition(getX(), getBottom(), screenGeometry);
    m_bubble->setEntity(m_currentNotify);
}
