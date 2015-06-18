/**
 *
 */

#ifndef __SHADOW_X11_H__
#define __SHADOW_X11_H__

#include <X11/Xlib.h>
#include <freerds/backend.h>
#include <freerdp/codec/region.h>

#define TAG FREERDP_TAG("freerds.shadow")

#ifndef WITH_XSHM
#define WITH_XSHM
#define WITH_XFIXES
#define WITH_XTEST
#define WITH_XDAMAGE
#endif

#ifdef WITH_XSHM
#include <X11/extensions/XShm.h>
#endif

#ifdef WITH_XFIXES
#include <X11/extensions/Xfixes.h>
#endif

#ifdef WITH_XTEST
#include <X11/extensions/XTest.h>
#endif

#ifdef WITH_XDAMAGE
#include <X11/extensions/Xdamage.h>
#endif

/** @brief */
struct _shadow_x11 {
	rdsBackendService *service;

	Display *display;
	int x11_fd;
	int default_screen_number;
	int screen_bpp;
	int screen_depth;
	int screen_bytes_per_pixel;
	int screen_width;
	int screen_height;
	int scanline_pad;
	int screen_stride;
	Screen *screen;
	Window root_window;
	XShmSegmentInfo shm_info;
	XImage *ximage;
	Visual* visual;
	Pixmap shm_pixmap;
	GC xshm_gc;

	/* cursor data */
	int cursorMaxWidth, cursorMaxHeight;
	int cursorWidth, cursorHeight;
	int cursorHotX, cursorHotY;
	int pointerX, pointerY;
	int cursorId;
	BYTE *cursorPixels;

	BOOL freerds_sync_signal;
	INT32 freerds_sync_id;
	REGION16 damagedRegion;
	void *dmgbuf;

#ifdef WITH_XFIXES
	int xfixes_cursor_notify_event;
#endif

#ifdef WITH_XDAMAGE
	int xdamage_notify_event;
	Damage xdamage;
	XserverRegion xdamage_region;
#endif
};
typedef struct _shadow_x11 ShadowX11;


ShadowX11 *ShadowX11_new(const char *display);

BOOL ShadowX11_handle_xevent(ShadowX11 *ctx, XEvent *ev);

BOOL ShadowX11_setup(rdsBackendService *service, ShadowX11 *ctx);

#endif /* ___SHADOW_X11_H__ */
