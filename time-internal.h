/*
 * Copyright (c) 2000-2007 Niels Provos <provos@citi.umich.edu>
 * Copyright (c) 2007-2012 Niels Provos and Nick Mathewson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef TIME_INTERNAL_H_INCLUDED_
#define TIME_INTERNAL_H_INCLUDED_

#include "event2/event-config.h"
#include "evconfig-private.h"

#include <time.h>

#include "event2/util.h"

#ifdef __cplusplus
extern "C" {
#endif

long evutil_tv_to_msec_(const struct timeval *tv);
EVENT2_EXPORT_SYMBOL
void evutil_usleep_(const struct timeval *tv);

struct evutil_monotonic_timer {
	int monotonic_clock;

	struct timeval adjust_monotonic_clock;
	struct timeval last_time;
};

EVENT2_EXPORT_SYMBOL
int evutil_configure_monotonic_time_(struct evutil_monotonic_timer *mt,
    int flags);
EVENT2_EXPORT_SYMBOL
int evutil_gettime_monotonic_(struct evutil_monotonic_timer *mt, struct timeval *tv);


#ifdef __cplusplus
}
#endif

#endif /* EVENT_INTERNAL_H_INCLUDED_ */
