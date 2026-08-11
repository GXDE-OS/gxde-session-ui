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
#include <DPlatformWindowHandle>

#include <QGuiApplication>
#include <QWindow>

#include "sessiontype.h"

Container::Container(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_wmHelper(DWindowManagerHelper::instance())
    , m_quitTimer(new QTimer(this))
    , m_supportComposite(m_wmHelper->hasComposite())
{
    setWindowFlags(Qt::ToolTip | Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus);
    setAttribute(Qt::WA_TranslucentBackground);

    m_quitTimer->setSingleShot(true);
    m_quitTimer->setInterval(60 * 1000);

    m_layout = new QHBoxLayout;
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);

    const int radius = getWindowRadius();

    DPlatformWindowHandle handle(this);
    handle.setBorderColor(QColor(0, 0, 0, 0.04 * 255));
    handle.setShadowColor(Qt::transparent);
    handle.setTranslucentBackground(true);
    handle.setWindowRadius(radius);

    setBlurRectXRadius(radius);
    setBlurRectYRadius(radius);
    setBlendMode(DBlurEffectWidget::BehindWindowBlend);
    setMaskColor(DBlurEffectWidget::LightColor);

    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged,
            this, &Container::windowManagerChanged);

    connect(m_quitTimer, &QTimer::timeout, this, &Container::onDelayQuit);
}

void Container::setContent(QWidget *content)
{
    m_layout->addWidget(content);
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

    // Under Wayland a client cannot position its own toplevel windows via
    // move(); route the position through the layer-shell protocol instead.
    if (!updateLayerShellPosition(targetPos))
        move(targetPos);
}

bool Container::updateLayerShellPosition(const QPoint &pos)
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

    // Anchor to the top-left corner, then use the top/left margins as a plain
    // x/y offset (in coordinates local to the target screen). This mirrors the
    // proven approach used by the notification bubbles and dde-launcher.
    QPoint localPos = pos;
    const QScreen *screen = win->screen();
    if (screen)
        localPos -= screen->geometry().topLeft();

    LayerShellQt::Window::Anchors anchors(LayerShellQt::Window::AnchorTop);
    anchors |= LayerShellQt::Window::AnchorLeft;
    lsWin->setAnchors(anchors);
    lsWin->setExclusiveZone(0);
    lsWin->setLayer(LayerShellQt::Window::LayerOverlay);
    lsWin->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
    lsWin->setMargins(QMargins(localPos.x(), localPos.y(), 0, 0));

    return true;
}

void Container::showEvent(QShowEvent *event)
{
    DBlurEffectWidget::showEvent(event);

    m_quitTimer->stop();
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
    return m_supportComposite ? 10 : 0;
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
