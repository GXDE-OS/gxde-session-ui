/*
 * Session type (X11 / Wayland) detection helpers.
 *
 * Under a Wayland session the X server is either absent or only reachable
 * through XWayland. Calling X11 specific APIs (QX11Info::display(),
 * XOpenDisplay(), xcb_ewmh_*, ...) may return a null handle, and the legacy
 * code base dereferences those handles unconditionally, which crashes.
 *
 * Use these helpers to guard every X11 only code path.
 */

#ifndef SESSIONTYPE_H
#define SESSIONTYPE_H

namespace SessionType {

/**
 * @brief Returns true when the process runs on a Wayland session.
 *
 * The check is based on the Qt platform plugin name first (the most reliable
 * source once QGuiApplication exists) and falls back to the XDG_SESSION_TYPE /
 * WAYLAND_DISPLAY environment variables so it is also usable before the
 * application object has been constructed.
 *
 * The result is computed once and cached.
 */
bool isWayland();

/**
 * @brief Returns true when X11 calls are safe to perform.
 *
 * This is not simply !isWayland(): a Wayland session may still provide
 * XWayland, and a X11 session may fail to expose a display (e.g. no DISPLAY
 * set). It reports whether an usable X connection is actually available.
 */
bool isX11Available();

}  // namespace SessionType

#endif  // SESSIONTYPE_H
