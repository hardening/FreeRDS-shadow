#include <freerds/backend.h>
#include <freerds/service_helper.h>

#include "shadow_x11.h"


int main(int argc, char *argv[]) {
	ShadowX11 *ctx;
	HANDLE events[2];
	HANDLE listenHandle;
	rdsBackendService *rdsService;
	int sessionId;

	if (!getenv("FREERDS_SID")) {
		WLog_ERR(TAG, "no FREERDS_SID env var");
		return 1;
	}

	sessionId = atoi(getenv("FREERDS_SID"));
	if (sessionId < 1) {
		WLog_ERR(TAG, "invalid FREERDS_SID env var");
		return 1;
	}

	if (!getenv("DISPLAY"))
		setenv("DISPLAY", strdup(":0.0"), 0);

	ctx = ShadowX11_new( getenv("DISPLAY") );
	if (!ctx)
		return 1;

	rdsService = freerds_service_new(sessionId, "Shadow");
	if (!rdsService) {
		WLog_ERR(TAG, "unable to create the FreeRDS service");
		return 1;
	}

	ctx->service = rdsService;
	WLog_INFO(TAG, "listening on pipe for session %d", sessionId);
	listenHandle = freerds_service_bind_endpoint(rdsService);
	if (!listenHandle || listenHandle == INVALID_HANDLE_VALUE) {
		WLog_ERR(TAG, "unable to bind the named pipe");
		return 1;
	}

	events[1] = CreateFileDescriptorEvent(NULL, FALSE, FALSE, ctx->x11_fd);

	while (TRUE) {
		int status;
		BOOL runClient = TRUE;
		HANDLE incomingConn;

		/* accept an incoming connection */
		incomingConn = freerds_service_accept(rdsService);
		if (!incomingConn || incomingConn == INVALID_HANDLE_VALUE) {
			WLog_ERR(TAG, "unable to accept a connection");
			return 1;
		}

		WLog_INFO(TAG, "incoming connection on the backend pipe");
		if (!ShadowX11_setup(rdsService, ctx)) {
			WLog_ERR(TAG, "error setting up the shadow context and service");
			freerds_service_kill_client(rdsService);
			continue;
		}

		events[0] = incomingConn;

		while (runClient) {
			status = WaitForMultipleObjects(2, events, FALSE, INFINITE);
			if (status == WAIT_OBJECT_0) {
				switch (freerds_service_incoming_bytes(rdsService, ctx)) {
				case INCOMING_BYTES_BROKEN_PIPE:
					WLog_ERR(TAG, "broken pipe");
					runClient = FALSE;
					continue;
				case INCOMING_BYTES_INVALID_MESSAGE:
					WLog_ERR(TAG, "error when treating backend message, invalid message");
					runClient = FALSE;
					continue;
				}
			}

			if ((status != WAIT_ABANDONED) && (WaitForSingleObject(events[1], 0) == WAIT_OBJECT_0)) {
				if (XEventsQueued(ctx->display, /*QueuedAlready*/QueuedAfterReading)) {
					while (XPending(ctx->display)) {
						XEvent xevent;

						XNextEvent(ctx->display, &xevent);
						ShadowX11_handle_xevent(ctx, &xevent);
					}
				}
			}
		}

		freerds_service_kill_client(rdsService);
	}

}
