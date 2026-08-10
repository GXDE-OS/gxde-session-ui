#include "sessiontype.h"

#include <QByteArray>
#include <QGuiApplication>
#include <QString>

#include <X11/Xlib.h>

namespace {

bool detectWayland()
{
    // The platform plugin name is authoritative, but it is only available
    // after QGuiApplication has been instantiated.
    if (qApp) {
        const QString platform = QGuiApplication::platformName().toLower();
        if (!platform.isEmpty())
            return platform.contains(QStringLiteral("wayland"));
    }

    const QByteArray sessionType = qgetenv("XDG_SESSION_TYPE").toLower();
    if (!sessionType.isEmpty())
        return sessionType.contains("wayland");

    return !qgetenv("WAYLAND_DISPLAY").isEmpty();
}

bool detectX11Available()
{
    // Even under Wayland an XWayland server may be reachable; conversely a
    // X11 session without DISPLAY has no usable connection. Probe for real.
    Display *display = XOpenDisplay(nullptr);
    if (!display)
        return false;

    XCloseDisplay(display);
    return true;
}

}  // namespace

namespace SessionType {

bool isWayland()
{
    static const bool wayland = detectWayland();
    return wayland;
}

bool isX11Available()
{
    static const bool available = detectX11Available();
    return available;
}

}  // namespace SessionType
