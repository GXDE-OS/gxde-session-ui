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

DWIDGET_USE_NAMESPACE

class QHBoxLayout;
class Container : public DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit Container(QWidget *parent = 0);

    void setContent(QWidget *content);
    void moveToCenter();

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;

private slots:
    void windowManagerChanged();
    void updateWindowRadius();
    int getWindowRadius();
    void onDelayQuit();

private:
    // Under Wayland a client cannot position its own toplevel windows via
    // move(); the compositor would place it arbitrarily (usually top-left).
    // This routes the requested position through the layer-shell protocol
    // instead. Returns true when the position was applied via layer-shell.
    bool updateLayerShellPosition(const QPoint &pos);

    QHBoxLayout *m_layout;
    DWindowManagerHelper *m_wmHelper;
    QTimer *m_quitTimer;
    bool m_supportComposite;
};

#endif // CONTAINER_H
