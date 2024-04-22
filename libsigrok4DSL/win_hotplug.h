#ifndef _WIN_HOTPLUG_H_
#define _WIN_HOTPLUG_H_

#include "libsigrok-internal.h"

#ifdef _WIN32

int listen_hotplug_ext( struct sr_context *ctx );
int close_hotplug_ext(struct sr_context *ctx);
int hotplug_wait_timeout_ext(struct sr_context *ctx);

#endif

#endif