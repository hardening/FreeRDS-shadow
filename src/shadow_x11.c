#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <X11/Xutil.h>

#include <freerds/service_helper.h>
#include <freerds/dmgbuf.h>
#include "shadow_x11.h"


static BOOL init_xfixes(ShadowX11 *ctx) {
#ifdef WITH_XFIXES
	int event;
	int error;
	int major, minor;

	if (!XFixesQueryExtension(ctx->display, &event, &error))
		return FALSE;

	if (!XFixesQueryVersion(ctx->display, &major, &minor))
		return FALSE;

	ctx->xfixes_cursor_notify_event = event + XFixesCursorNotify;

	XFixesSelectCursorInput(ctx->display, ctx->root_window,	XFixesDisplayCursorNotifyMask);
	XSelectInput(ctx->display, ctx->root_window, StructureNotifyMask);

#endif
	return TRUE;
}


BOOL x11_shadow_xdamage_init(ShadowX11* subsystem)
{
#ifdef WITH_XDAMAGE
	int major, minor;
	int event;
	int error;

/*	if (!subsystem->use_xfixes)
		return -1;*/

	if (!XDamageQueryExtension(subsystem->display, &event, &error))
		return FALSE;

	if (!XDamageQueryVersion(subsystem->display, &major, &minor))
		return FALSE;

	if (major < 1)
		return FALSE;

	subsystem->xdamage_notify_event = event + XDamageNotify;
	subsystem->xdamage = XDamageCreate(subsystem->display, subsystem->root_window, XDamageReportNonEmpty);
	if (!subsystem->xdamage)
		return FALSE;

#ifdef WITH_XFIXES
	subsystem->xdamage_region = XFixesCreateRegion(subsystem->display, NULL, 0);
	if (!subsystem->xdamage_region)
		return FALSE;
#endif

	return TRUE;
#else
	return FALSE;
#endif
}

int x11_shadow_xshm_init(ShadowX11* ctx)
{
	Bool pixmaps;
	int major, minor;
	XGCValues values;
	XImage *shmImage;

	if (!XShmQueryExtension(ctx->display))
		return FALSE;

	if (!XShmQueryVersion(ctx->display, &major, &minor, &pixmaps))
		return FALSE;

	if (!pixmaps)
		return FALSE;

	ctx->shm_info.shmid = -1;
	ctx->shm_info.shmaddr = (char*) -1;
	ctx->shm_info.readOnly = False;


	ctx->ximage = XShmCreateImage(ctx->display, ctx->visual, ctx->screen_depth,
			ZPixmap, NULL, &(ctx->shm_info), ctx->screen_width, ctx->screen_height);
	if (!ctx->ximage)	{
		WLog_ERR(TAG, "XShmCreateImage failed");
		return FALSE;
	}

	shmImage = ctx->ximage;
	ctx->shm_info.shmid = shmget(IPC_PRIVATE, shmImage->height * shmImage->bytes_per_line, IPC_CREAT | 0600);
	if (ctx->shm_info.shmid == -1) {
		WLog_ERR(TAG, "shmget failed");
		return FALSE;
	}

	ctx->shm_info.shmaddr = shmat(ctx->shm_info.shmid, 0, 0);
	shmImage->data = ctx->shm_info.shmaddr;

	if (ctx->shm_info.shmaddr == ((char*) -1)) {
		WLog_ERR(TAG, "shmat failed");
		return FALSE;
	}

	if (!XShmAttach(ctx->display, &(ctx->shm_info)))
		return FALSE;

	XSync(ctx->display, False);

	shmctl(ctx->shm_info.shmid, IPC_RMID, 0);

	ctx->shm_pixmap = XShmCreatePixmap(ctx->display, ctx->root_window,
			ctx->shm_info.shmaddr, &(ctx->shm_info),
			ctx->ximage->width, ctx->ximage->height, ctx->ximage->depth);

	XSync(ctx->display, False);

	if (!ctx->shm_pixmap)
		return FALSE;

	values.subwindow_mode = IncludeInferiors;
	values.graphics_exposures = False;

	ctx->xshm_gc = XCreateGC(ctx->display, ctx->root_window, GCSubwindowMode | GCGraphicsExposures, &values);

	//XSetFunction(ctx->display, ctx->xshm_gc, GXcopy);
	XSync(ctx->display, False);

	return TRUE;
}

BOOL x11_shadow_query_cursor(ShadowX11 *ctx, BOOL getImage)
{
	int x, y, n, k;

	if (getImage) {
#ifdef WITH_XFIXES
		UINT32* pDstPixel;
		XFixesCursorImage* ci;

		ci = XFixesGetCursorImage(ctx->display);
		if (!ci)
			return FALSE;

		x = ci->x;
		y = ci->y;

		if (ci->width > ctx->cursorMaxWidth)
			return FALSE;

		if (ci->height > ctx->cursorMaxHeight)
			return FALSE;

		ctx->cursorHotX = ci->xhot;
		ctx->cursorHotY = ci->yhot;
		ctx->cursorWidth = ci->width;
		ctx->cursorHeight = ci->height;
		ctx->cursorId = ci->cursor_serial;

		n = ci->width * ci->height;
		pDstPixel = (UINT32*) ctx->cursorPixels;

		for (k = 0; k < n; k++)
		{
			/* XFixesCursorImage.pixels is in *unsigned long*, which may be 8 bytes */
			*pDstPixel++ = (UINT32) ci->pixels[k];
		}

		XFree(ci);

		/**
		 *  x11_shadow_pointer_alpha_update(ctx);
		 *
		 *POINTER_NEW_UPDATE pointerNew;
		POINTER_COLOR_UPDATE* pointerColor;
		POINTER_CACHED_UPDATE pointerCached;
		SHADOW_MSG_OUT_POINTER_ALPHA_UPDATE* msg = (SHADOW_MSG_OUT_POINTER_ALPHA_UPDATE*) message->wParam;

		ZeroMemory(&pointerNew, sizeof(POINTER_NEW_UPDATE));

		pointerNew.xorBpp = 24;
		pointerColor = &(pointerNew.colorPtrAttr);

		pointerColor->cacheIndex = 0;
		pointerColor->xPos = msg->xHot;
		pointerColor->yPos = msg->yHot;
		pointerColor->width = msg->width;
		pointerColor->height = msg->height;

		pointerCached.cacheIndex = pointerColor->cacheIndex;

		if (client->activated)
		{
			shadow_client_convert_alpha_pointer_data(msg->pixels, msg->premultiplied,
					msg->width, msg->height, pointerColor);

			IFCALL(update->pointer->PointerNew, context, &pointerNew);
			IFCALL(update->pointer->PointerCached, context, &pointerCached);

			free(pointerColor->xorMaskData);
			free(pointerColor->andMaskData);
		}

		free(msg->pixels);
		free(msg);
		 */
#endif
	} else {
		UINT32 mask;
		int win_x, win_y;
		int root_x, root_y;
		Window root, child;

		if (!XQueryPointer(ctx->display, ctx->root_window,
				&root, &child, &root_x, &root_y, &win_x, &win_y, &mask)) {
			return FALSE;
		}

		x = root_x;
		y = root_y;
	}

	if ((x != ctx->pointerX) || (y != ctx->pointerY)) {
		ctx->pointerX = x;
		ctx->pointerY = y;

		/**
		 *
		 * x11_shadow_pointer_position_update(ctx);
		 *
		 *
		 * 		POINTER_POSITION_UPDATE pointerPosition;
		SHADOW_MSG_OUT_POINTER_POSITION_UPDATE* msg = (SHADOW_MSG_OUT_POINTER_POSITION_UPDATE*) message->wParam;

		pointerPosition.xPos = msg->xPos;
		pointerPosition.yPos = msg->yPos;

		if (client->activated)
		{
			if ((msg->xPos != client->pointerX) || (msg->yPos != client->pointerY))
			{
				IFCALL(update->pointer->PointerPosition, context, &pointerPosition);

				client->pointerX = msg->xPos;
				client->pointerY = msg->yPos;
			}
		}

		free(msg);
		 */
	}

	return TRUE;
}



static BOOL shadow_client_capabilities(ShadowX11* backend, RDS_MSG_CAPABILITIES* capabilities) {
	RDS_MSG_FRAMEBUFFER_INFO msg;

	WLog_INFO(TAG, "client resolution %dx%d, local %dx%d", capabilities->DesktopWidth, capabilities->DesktopHeight,
			backend->screen_width, backend->screen_height);
	msg.type = RDS_SERVER_FRAMEBUFFER_INFO;
	msg.version = 1;
	msg.width = backend->screen_width;
	msg.height = backend->screen_height;
	msg.scanline = backend->screen_stride;
	msg.bitsPerPixel = backend->screen_bpp;
	msg.bytesPerPixel = backend->screen_bytes_per_pixel;
	msg.userId = getuid();

	return freerds_service_write_message(backend->service, (RDS_MSG_COMMON *)&msg);
}

static void sync_damage_rect(ShadowX11* ctx, int x, int y, int w, int h, int srcStride, const char* src, char* dst, BOOL srcOffset)
{
        int i, wb, offset;

        wb = w * ctx->screen_bytes_per_pixel;

        offset = (y * ctx->screen_stride) + (x * ctx->screen_bytes_per_pixel);

        if (srcOffset)
        	src += offset;
        dst += offset;

        for (i = 0; i < h; i++)
        {
                memcpy(dst, src, wb);
                src += srcStride;
                dst += ctx->screen_stride;
        }
}


static BOOL sync_fb(ShadowX11* ctx) {
	int nrects, i;
	const RECTANGLE_16 *rects;
	RDP_RECT *rdpRect;
	RDS_MSG_FRAMEBUFFER_SYNC_REPLY reply;

	rects = region16_rects(&ctx->damagedRegion, &nrects);

	if (nrects > (int)freerds_dmgbuf_get_max_rects(ctx->dmgbuf)) {
		rects = region16_extents(&ctx->damagedRegion);
		nrects = 1;
	}

	freerds_dmgbuf_set_num_rects(ctx->dmgbuf, nrects);
	rdpRect = freerds_dmgbuf_get_rects(ctx->dmgbuf, NULL);

	WLog_DBG(TAG, "%s: %d rects", __FUNCTION__, nrects);

	XLockDisplay(ctx->display);

	XShmGetImage(ctx->display, ctx->root_window, ctx->ximage, 0, 0, AllPlanes);

	for (i = 0; i < nrects; i++, rects++, rdpRect++) {
		/*
#ifdef WITH_XSHM
		WLog_DBG(TAG, "%s: XCopyArea(%d, %d, %d, %d)", __FUNCTION__,
				rects->left, rects->top,
				(rects->right - rects->left), (rects->bottom - rects->top)
		);

		XCopyArea(ctx->display, ctx->root_window, ctx->shm_pixmap,
				ctx->xshm_gc, rects->left, rects->top,
				(rects->right - rects->left), (rects->bottom - rects->top),
				rects->left, rects->top);

		XSync(ctx->display, False);
#endif
	*/

		rdpRect->x = rects->left;
		rdpRect->y = rects->top;
		rdpRect->width = (rects->right - rects->left);
		rdpRect->height = (rects->bottom - rects->top);

		XImage *img = XGetImage(ctx->display, ctx->root_window, rdpRect->x, rdpRect->y,
				rdpRect->width, rdpRect->height, AllPlanes, ZPixmap);

		sync_damage_rect(ctx, rdpRect->x, rdpRect->y, rdpRect->width, rdpRect->height,
				/*ctx->shm_info.shmaddr,*/
				img->bytes_per_line, img->data,
				freerds_dmgbuf_get_data(ctx->dmgbuf), FALSE);
	}

	XUnlockDisplay(ctx->display);

	region16_clear(&ctx->damagedRegion);
	ctx->freerds_sync_signal = FALSE;

	reply.type = RDS_SERVER_FRAMEBUFFER_SYNC_REPLY;
	reply.msgFlags = 0;
	reply.bufferId = ctx->freerds_sync_id;

	return freerds_service_write_message(ctx->service, (RDS_MSG_COMMON *)&reply);
}

BOOL ShadowX11_handle_xevent(ShadowX11 *ctx, XEvent *ev) {
	WLog_INFO(TAG, "X event %d", ev->type);

	if (ev->type == MotionNotify) {
	}
#ifdef WITH_XFIXES
	else if (ev->type == ctx->xfixes_cursor_notify_event) {
		return x11_shadow_query_cursor(ctx, TRUE);
	}
#endif
	else if (ev->type == ctx->xdamage_notify_event) {
		int nrects, i;
		XRectangle* rects;
		XRectangle bounds;
		RECTANGLE_16 rect16;
		const XDamageNotifyEvent* damage_event = (const XDamageNotifyEvent *)ev;

		if (damage_event->damage != ctx->xdamage)
			return TRUE;

		XDamageSubtract(ctx->display, ctx->xdamage, None, ctx->xdamage_region);
		rects = XFixesFetchRegionAndBounds(ctx->display, ctx->xdamage_region, &nrects, &bounds);
		for (i = 0; i < nrects; i++) {
			rect16.left = rects[i].x;
			rect16.top = rects[i].y;
			rect16.right = rects[i].x + rects[i].width;
			rect16.bottom = rects[i].y + rects[i].height;

			WLog_DBG(TAG, "rect (%dx%d, %dx%d)", rect16.left, rect16.top, rect16.right, rect16.bottom);

			if (!region16_union_rect(&ctx->damagedRegion, &ctx->damagedRegion, &rect16)) {
				WLog_ERR(TAG, "error adding rectangle");
			}
		}

		XFree(rects);

		if (ctx->freerds_sync_signal)
			sync_fb(ctx);
	} else {
	}

	return TRUE;

}


static BOOL shadow_client_framebuffer_sync_request(ShadowX11* backend, INT32 bufferId) {
	backend->freerds_sync_signal = TRUE;
	if (backend->dmgbuf && (backend->freerds_sync_id != freerds_dmgbuf_get_id(backend->dmgbuf))) {
		freerds_dmgbuf_free(backend->dmgbuf);
		backend->dmgbuf = NULL;
	}

	if (!backend->dmgbuf) {
		backend->dmgbuf = freerds_dmgbuf_connect(bufferId);
		if (!backend->dmgbuf) {
			WLog_ERR(TAG, "unable to create a dmgbuf on bufferId 0x%x", bufferId);
			return FALSE;
		}
	}

	backend->freerds_sync_id = bufferId;

	if (region16_is_empty(&backend->damagedRegion))
		return TRUE;

	return sync_fb(backend);
}

static BOOL shadow_client_synchronize_keyboard_event(ShadowX11* backend, DWORD flags) {
	WLog_INFO(TAG, "%s(backend=%p, flags=%d): code me", __FUNCTION__, backend, flags);
	return TRUE;
}

static BOOL shadow_client_scancode_keyboard_event(ShadowX11* backend, DWORD flags, DWORD code, DWORD keyboardType) {
#ifdef WITH_XTEST
	DWORD vkcode;
	DWORD keycode;
	BOOL extended = FALSE;

	if (flags & KBD_FLAGS_EXTENDED)
		extended = TRUE;

	if (extended)
		code |= KBDEXT;

	vkcode = GetVirtualKeyCodeFromVirtualScanCode(code, keyboardType);
	if (extended)
		vkcode |= KBDEXT;

	keycode = GetKeycodeFromVirtualKeyCode(vkcode, KEYCODE_TYPE_EVDEV);
	if (keycode != 0) {
		XTestGrabControl(backend->display, True);

		if (flags & KBD_FLAGS_DOWN)
			XTestFakeKeyEvent(backend->display, keycode, True, CurrentTime);
		else if (flags & KBD_FLAGS_RELEASE)
			XTestFakeKeyEvent(backend->display, keycode, False, CurrentTime);

		XTestGrabControl(backend->display, False);

		XFlush(backend->display);
	}
#endif

	return TRUE;
}

static BOOL shadow_client_virtual_keyboard_event(ShadowX11* backend, DWORD flags, DWORD vkcode) {
#ifdef WITH_XTEST
	DWORD keycode;

	if (flags & KBD_FLAGS_EXTENDED)
		vkcode |= KBDEXT;

	keycode = GetKeycodeFromVirtualKeyCode(vkcode, KEYCODE_TYPE_EVDEV);
	if (keycode != 0) {
		XTestGrabControl(backend->display, True);

		if (flags & KBD_FLAGS_DOWN)
			XTestFakeKeyEvent(backend->display, keycode, True, CurrentTime);
		else if (flags & KBD_FLAGS_RELEASE)
			XTestFakeKeyEvent(backend->display, keycode, False, CurrentTime);

		XTestGrabControl(backend->display, False);

		XFlush(backend->display);
	}
#endif
	return TRUE;
}

static BOOL shadow_client_unicode_keyboard_event(ShadowX11* backend, DWORD flags, DWORD code) {
	WLog_INFO(TAG, "%s(backend=%p, flags=%d, code=%d): code me", __FUNCTION__, backend, flags, code);
	return TRUE;
}

static BOOL shadow_client_mouse_event(ShadowX11* backend, DWORD flags, DWORD x, DWORD y) {
#ifdef WITH_XTEST
	int button = 0;
	BOOL down = FALSE;

	XTestGrabControl(backend->display, True);

	if (flags & PTR_FLAGS_WHEEL) {
		BOOL negative = FALSE;

		if (flags & PTR_FLAGS_WHEEL_NEGATIVE)
			negative = TRUE;

		button = (negative) ? 5 : 4;

		XTestFakeButtonEvent(backend->display, button, True, CurrentTime);
		XTestFakeButtonEvent(backend->display, button, False, CurrentTime);
	} else {
		if (flags & PTR_FLAGS_MOVE)
			XTestFakeMotionEvent(backend->display, 0, x, y, CurrentTime);

		if (flags & PTR_FLAGS_BUTTON1)
			button = 1;
		else if (flags & PTR_FLAGS_BUTTON2)
			button = 3;
		else if (flags & PTR_FLAGS_BUTTON3)
			button = 2;

		if (flags & PTR_FLAGS_DOWN)
			down = TRUE;

		if (button)
			XTestFakeButtonEvent(backend->display, button, down, CurrentTime);
	}

	XTestGrabControl(backend->display, False);

	XFlush(backend->display);
#endif

	return TRUE;
}

static BOOL shadow_client_extended_mouse_event(ShadowX11* backend, DWORD flags, DWORD x, DWORD y) {
#ifdef WITH_XTEST
	int button = 0;
	BOOL down = FALSE;

	XTestGrabControl(backend->display, True);

	XTestFakeMotionEvent(backend->display, 0, x, y, CurrentTime);

	if (flags & PTR_XFLAGS_BUTTON1)
		button = 8;
	else if (flags & PTR_XFLAGS_BUTTON2)
		button = 9;

	if (flags & PTR_XFLAGS_DOWN)
		down = TRUE;

	if (button)
		XTestFakeButtonEvent(backend->display, button, down, CurrentTime);

	XTestGrabControl(backend->display, False);

	XFlush(backend->display);
#endif
	return TRUE;
}

static BOOL shadow_client_immediate_sync_request(ShadowX11 *backend, INT32 bufferId) {
	backend->freerds_sync_signal = TRUE;
	if (backend->dmgbuf && (backend->freerds_sync_id != freerds_dmgbuf_get_id(backend->dmgbuf))) {
		freerds_dmgbuf_free(backend->dmgbuf);
		backend->dmgbuf = NULL;
	}

	if (!backend->dmgbuf) {
		backend->dmgbuf = freerds_dmgbuf_connect(bufferId);
		if (!backend->dmgbuf) {
			WLog_ERR(TAG, "unable to create a dmgbuf on bufferId 0x%x", bufferId);
			return FALSE;
		}
	}

	backend->freerds_sync_id = bufferId;
	return sync_fb(backend);
}

static rdsClientInterface g_callbacks = {
        (pRdsClientCapabilities)shadow_client_capabilities,
        (pRdsClientSynchronizeKeyboardEvent)shadow_client_synchronize_keyboard_event,
        (pRdsClientScancodeKeyboardEvent)shadow_client_scancode_keyboard_event,
        (pRdsClientVirtualKeyboardEvent)shadow_client_virtual_keyboard_event,
        (pRdsClientUnicodeKeyboardEvent)shadow_client_unicode_keyboard_event,
        (pRdsClientMouseEvent)shadow_client_mouse_event,
        (pRdsClientExtendedMouseEvent)shadow_client_extended_mouse_event,
        (pRdsClientFramebufferSyncRequest)shadow_client_framebuffer_sync_request,
        (pRdsClientIcps)NULL,
        (pRdsClientImmediateSyncRequest)shadow_client_immediate_sync_request,
		(pRdsClientSeatNew)NULL,
		(pRdsClientSeatRemoved)NULL,
        (pRdsClientMessage)NULL,
};

BOOL ShadowX11_setup(rdsBackendService *service, ShadowX11 *ctx) {
	freerds_service_set_callbacks(service, &g_callbacks);
	return TRUE;
}


ShadowX11 *ShadowX11_new(const char *display)
{
	int vi_count;
	int i;
	int pf_count;
	XVisualInfo* vi;
	XVisualInfo* vis;
	XVisualInfo template;
	XPixmapFormatValues* pf;
	XPixmapFormatValues* pfs;
	char **extensions;
	int nextensions;
	RECTANGLE_16 rect16;
	BOOL composite = FALSE;

	ShadowX11 *ret = calloc(1, sizeof(*ret));
	if (!ret)
		return NULL;

	if (!XInitThreads())
		goto error_display;

	ret->display = XOpenDisplay(display);
	if (!ret->display) {
		goto error_display;
	}

	extensions = XListExtensions(ret->display, &nextensions);
	if (!extensions || (nextensions < 0))
		goto out_close_display;

	for (i = 0; i < nextensions; i++) {
		if (strcmp(extensions[i], "Composite") == 0)
			composite = TRUE;
	}
	XFreeExtensionList(extensions);

	ret->x11_fd = ConnectionNumber(ret->display);
	ret->default_screen_number = DefaultScreen(ret->display);
	ret->screen = ScreenOfDisplay(ret->display, ret->default_screen_number);
	ret->screen_depth = DefaultDepthOfScreen(ret->screen);
	ret->screen_width = WidthOfScreen(ret->screen);
	ret->screen_height = HeightOfScreen(ret->screen);
	ret->root_window = RootWindow(ret->display, ret->default_screen_number);

	if (!init_xfixes(ret))
		goto out_close_display;

	ret->cursorMaxWidth = 96;
	ret->cursorMaxHeight = 96;
	ret->cursorPixels = calloc(ret->cursorMaxWidth * ret->cursorMaxHeight, 4);
	if (!ret->cursorPixels)
		goto out_close_display;

	pfs = XListPixmapFormats(ret->display, &pf_count);
	if (!pfs) {
		WLog_ERR(TAG, "XListPixmapFormats failed");
		goto error_list_pixmap_formats;
	}

	for (i = 0; i < pf_count; i++) {
		pf = pfs + i;

		if (pf->depth == ret->screen_depth) {
			ret->screen_bpp = pf->bits_per_pixel;
			ret->scanline_pad = /*pf->scanline_pad*/ 0;
			break;
		}
	}
	XFree(pfs);

	ret->screen_bytes_per_pixel = ret->screen_bpp / 8;
	ret->screen_stride = (ret->screen_width * ret->screen_bytes_per_pixel) + ret->scanline_pad;
	WLog_INFO(TAG, "%s: size=%dx%d bpp=%d depth=%d stride=%d", __FUNCTION__, ret->screen_width,
			ret->screen_height, ret->screen_bpp, ret->screen_depth, ret->screen_stride);


	ZeroMemory(&template, sizeof(template));
	template.class = TrueColor;
	template.screen = ret->default_screen_number;

	vis = XGetVisualInfo(ret->display, VisualClassMask | VisualScreenMask, &template, &vi_count);
	if (!vis) {
		WLog_ERR(TAG, "XGetVisualInfo failed");
		goto error_list_pixmap_formats;
	}

	for (i = 0; i < vi_count; i++) {
		vi = vis + i;

		if (vi->depth == ret->screen_bpp) {
			ret->visual = vi->visual;
			break;
		}
	}

	XFree(vis);

	if (!x11_shadow_xdamage_init(ret)) {
		WLog_ERR(TAG, "XDamage init failed");
		goto error_list_pixmap_formats;
	}

	if (!x11_shadow_xshm_init(ret)) {
		WLog_ERR(TAG, "XSHM init failed");
		goto error_list_pixmap_formats;
	}

	region16_init(&ret->damagedRegion);

	rect16.left = 0;
	rect16.top = 0;
	rect16.right = ret->screen_width;
	rect16.bottom = ret->screen_height;
	if (!region16_union_rect(&ret->damagedRegion, &ret->damagedRegion, &rect16))
		goto error_union_rect;

	ret->freerds_sync_signal = FALSE;
	ret->freerds_sync_id = 0xffffffff;
	return ret;

error_union_rect:
	region16_uninit(&ret->damagedRegion);
error_list_pixmap_formats:
	free(ret->cursorPixels);
out_close_display:
	XCloseDisplay(ret->display);
error_display:
	free(ret);
	return NULL;
}



