/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
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

#include "container.h"

#include <QHBoxLayout>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QTimer>
#include <QGSettings>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QResizeEvent>
#include <DPlatformWindowHandle>
#include <KWindowEffects>

#include <QGuiApplication>
#include <QWindow>

#include "sessiontype.h"

Container::Container(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_wmHelper(DWindowManagerHelper::instance())
    , m_quitTimer(new QTimer(this))
    , m_animation(new QParallelAnimationGroup(this))
    , m_opacityEffect(new QGraphicsOpacityEffect(this))
    , m_effectMargins(SessionType::isWayland() ? QMargins(16, 16, 16, 20) : QMargins())
    , m_supportComposite(m_wmHelper->hasComposite())
{
    setWindowFlags(Qt::ToolTip | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground);
    m_opacityEffect->setOpacity(1.0);
    setGraphicsEffect(m_opacityEffect);

    m_quitTimer->setSingleShot(true);
    m_quitTimer->setInterval(60 * 1000);

    m_layout = new QHBoxLayout;
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);

    const int radius = getWindowRadius();

    DPlatformWindowHandle handle(this);
    handle.setBorderColor(QColor(0, 0, 0, 0.04 * 255));
    handle.setShadowRadius(16);
    handle.setShadowOffset(QPoint(0, 4));
    handle.setShadowColor(SessionType::isWayland() ? QColor(0, 0, 0, 70) : Qt::transparent);
    handle.setTranslucentBackground(true);
    handle.setWindowRadius(radius);

    setBlurRectXRadius(radius);
    setBlurRectYRadius(radius);
    setBlendMode(DBlurEffectWidget::BehindWindowBlend);
    setMaskColor(DBlurEffectWidget::LightColor);

    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged,
            this, &Container::windowManagerChanged);

    connect(m_quitTimer, &QTimer::timeout, this, &Container::onDelayQuit);

    connect(m_animation, &QParallelAnimationGroup::finished, this, [this] {
        if (!m_hideAfterAnimation) {
            return;
        }

        m_hideAfterAnimation = false;
        DBlurEffectWidget::hide();
        m_opacityEffect->setOpacity(1.0);
        setWaylandAnimationOffset(0);
    });
}

void Container::setContent(QWidget *content)
{
    m_layout->addWidget(content);
}

void Container::setContentMargins(const QMargins &margins)
{
    m_layout->setContentsMargins(m_effectMargins.left() + margins.left(),
        m_effectMargins.top() + margins.top(),
        m_effectMargins.right() + margins.right(),
        m_effectMargins.bottom() + margins.bottom());
}

void Container::setContentSize(const QSize &size)
{
    setFixedSize(size + QSize(m_effectMargins.left() + m_effectMargins.right(),
        m_effectMargins.top() + m_effectMargins.bottom()));
    updateWaylandMask();
}

void Container::moveToCenter()
{
    QScreen *primary = QGuiApplication::primaryScreen();
    if (!primary)
        return;
    const QRect primaryRect = primary->geometry();

    // 显示在屏幕水平居中、垂直偏下的位置。
    const QPoint targetPos = QPoint(primaryRect.center().x(), primaryRect.bottom() - 180)
                             - QPoint(rect().center().x(), rect().bottom());

    if (SessionType::isWayland())
        setScreen(primary);
    if (!updateLayerShellPosition())
        move(targetPos);
}

bool Container::updateLayerShellPosition()
{
    if (!SessionType::isWayland())
        return false;

    // Make sure the window handle exists, otherwise LayerShellQt::Window::get()
    // has nothing to attach the layer surface to.
    createWinId();

    QWindow *win = windowHandle();
    if (!win)
        win = window() ? window()->windowHandle() : nullptr;
    if (!win)
        return false;

    LayerShellQt::Window *lsWin = LayerShellQt::Window::get(win);
    if (!lsWin)
        return false;

    lsWin->setScreenConfiguration(LayerShellQt::Window::ScreenFromQWindow);
    lsWin->setAnchors(LayerShellQt::Window::AnchorBottom);
    lsWin->setExclusiveZone(0);
    lsWin->setLayer(LayerShellQt::Window::LayerOverlay);
    lsWin->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    setWaylandAnimationOffset(m_waylandAnimationOffset);

    return true;
}

void Container::showAnimated()
{
    if (!SessionType::isWayland()) {
        show();
        return;
    }

    m_animation->stop();
    m_hideAfterAnimation = false;
    if (isVisible()) {
        m_opacityEffect->setOpacity(1.0);
        setWaylandAnimationOffset(0);
        return;
    }

    m_opacityEffect->setOpacity(0.0);
    setWaylandAnimationOffset(-12);
    show();
    startWaylandAnimation(true);
}

void Container::hideAnimated()
{
    if (!SessionType::isWayland() || !isVisible()) {
        hide();
        return;
    }

    startWaylandAnimation(false);
}

void Container::startWaylandAnimation(bool showing)
{
    m_animation->stop();
    m_animation->clear();
    m_hideAfterAnimation = !showing;

    auto *opacity = new QPropertyAnimation(m_opacityEffect, "opacity", m_animation);
    opacity->setDuration(showing ? 160 : 120);
    opacity->setStartValue(showing ? 0.0 : m_opacityEffect->opacity());
    opacity->setEndValue(showing ? 1.0 : 0.0);
    opacity->setEasingCurve(showing ? QEasingCurve::OutCubic : QEasingCurve::InCubic);

    auto *slide = new QPropertyAnimation(this, "waylandAnimationOffset", m_animation);
    slide->setDuration(opacity->duration());
    slide->setStartValue(showing ? -12 : m_waylandAnimationOffset);
    slide->setEndValue(showing ? 0 : -8);
    slide->setEasingCurve(opacity->easingCurve());

    m_animation->start();
}

int Container::waylandAnimationOffset() const
{
    return m_waylandAnimationOffset;
}

void Container::setWaylandAnimationOffset(int offset)
{
    m_waylandAnimationOffset = offset;
    if (!SessionType::isWayland())
        return;

    if (QWindow *win = windowHandle()) {
        if (LayerShellQt::Window *lsWin = LayerShellQt::Window::get(win)) {
            // The visible rounded panel (rather than its shadow padding) stays
            // 180 logical pixels above the output's bottom edge.
            const int shadowBottom = m_effectMargins.bottom();
            lsWin->setMargins(QMargins(0, 0, 0,
                                      qMax(0, m_waylandBottomMargin - shadowBottom + offset)));
        }
    }
}

void Container::updateWaylandMask()
{
    if (!SessionType::isWayland())
        return;

    QPainterPath path;
    path.addRoundedRect(QRectF(rect().marginsRemoved(m_effectMargins)),
                        getWindowRadius(), getWindowRadius());
    setMaskPath(path);

    if (isVisible() && windowHandle()) {
        const QRegion panelRegion(path.toFillPolygon().toPolygon());
        KWindowEffects::enableBlurBehind(windowHandle(), true, panelRegion);
    }
}

void Container::paintEvent(QPaintEvent *event)
{
    if (SessionType::isWayland()) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);

        const QRectF panelRect(rect().marginsRemoved(m_effectMargins));
        for (int spread = 14; spread >= 1; --spread) {
            const qreal strength = qreal(15 - spread) / 14.0;
            painter.setBrush(QColor(0, 0, 0, qRound(2 + 4 * strength)));
            painter.drawRoundedRect(panelRect.adjusted(-spread, -spread + 2,
                    spread, spread + 2),
                getWindowRadius() + spread,
                getWindowRadius() + spread);
        }
    }

    DBlurEffectWidget::paintEvent(event);
}

void Container::resizeEvent(QResizeEvent *event)
{
    DBlurEffectWidget::resizeEvent(event);
    updateWaylandMask();
}

void Container::showEvent(QShowEvent *event)
{
    DBlurEffectWidget::showEvent(event);

    m_quitTimer->stop();
    updateWaylandMask();
}

void Container::hideEvent(QHideEvent *event)
{
    DBlurEffectWidget::hideEvent(event);

    m_quitTimer->start();
}

void Container::windowManagerChanged()
{
    m_supportComposite = m_wmHelper->hasComposite();

    updateWindowRadius();
}

void Container::updateWindowRadius()
{
    const int value = getWindowRadius();

    DPlatformWindowHandle handle(this);
    handle.setWindowRadius(value);

    setBlurRectXRadius(value);
    setBlurRectYRadius(value);
}

int Container::getWindowRadius()
{
    return (SessionType::isWayland() || m_supportComposite) ? 10 : 0;
}

void Container::onDelayQuit()
{
    const QGSettings gsettings("com.deepin.dde.osd", "/com/deepin/dde/osd/");
    if (gsettings.keys().contains("autoExit") && gsettings.get("auto-exit").toBool())
    {
        if (isVisible())
            return m_quitTimer->start();
        qWarning() << "Killer Timeout, now quiiting...";
        qApp->quit();
    }
}
