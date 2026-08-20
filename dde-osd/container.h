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

#ifndef CONTAINER_H
#define CONTAINER_H

#include <DBlurEffectWidget>
#include <DWindowManagerHelper>

#include <LayerShellQt/Window>

#include <QMargins>

DWIDGET_USE_NAMESPACE

class QHBoxLayout;
class QParallelAnimationGroup;
class QGraphicsOpacityEffect;
class Container : public DBlurEffectWidget
{
    Q_OBJECT
    Q_PROPERTY(int waylandAnimationOffset READ waylandAnimationOffset WRITE setWaylandAnimationOffset)
public:
    explicit Container(QWidget *parent = 0);

    void setContent(QWidget *content);
    void setContentMargins(const QMargins &margins);
    void setContentSize(const QSize &size);
    void moveToCenter();
    void showAnimated();
    void hideAnimated();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;

private slots:
    void windowManagerChanged();
    void updateWindowRadius();
    int getWindowRadius();
    void onDelayQuit();

private:
    // Under Wayland a client cannot position its own toplevel windows via
    // move(); configure the requested placement through layer-shell instead.
    bool updateLayerShellPosition();
    void updateWaylandMask();
    void startWaylandAnimation(bool showing);
    int waylandAnimationOffset() const;
    void setWaylandAnimationOffset(int offset);

    QHBoxLayout *m_layout;
    DWindowManagerHelper *m_wmHelper;
    QTimer *m_quitTimer;
    QParallelAnimationGroup *m_animation;
    QGraphicsOpacityEffect *m_opacityEffect;
    QMargins m_effectMargins;
    int m_waylandAnimationOffset = 0;
    int m_waylandBottomMargin = 180;
    bool m_hideAfterAnimation = false;
    bool m_supportComposite;
};

#endif // CONTAINER_H
