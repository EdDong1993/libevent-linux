/*
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

#include "event2/event-config.h"
#include "evconfig-private.h"

#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>

#include "event2/util.h"
#include "util-internal.h"
#include "log-internal.h"
#include "mm-internal.h"

#define MAX_SECONDS_IN_MSEC_LONG \
	(((LONG_MAX) - 999) / 1000)

long
evutil_tv_to_msec_(const struct timeval *tv)
{
	if (tv->tv_usec > 1000000 || tv->tv_sec > MAX_SECONDS_IN_MSEC_LONG)
		return -1;

	return (tv->tv_sec * 1000) + ((tv->tv_usec + 999) / 1000);
}

/*
  Replacement for usleep on platforms that don't have one.  Not guaranteed to
  be any more finegrained than 1 msec.
 */
void
evutil_usleep_(const struct timeval *tv)
{
	if (!tv)
		return;
	{
		struct timespec ts;
		ts.tv_sec = tv->tv_sec;
		ts.tv_nsec = tv->tv_usec*1000;
		nanosleep(&ts, NULL);
	}
}

int
evutil_date_rfc1123(char *date, const size_t datelen, const struct tm *tm)
{
	static const char *DAYS[] =
		{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static const char *MONTHS[] =
		{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	time_t t = time(NULL);

	struct tm sys;

	/* If `tm` is null, set system's current time. */
	if (tm == NULL) {
		gmtime_r(&t, &sys);
		tm = &sys;
	}

	return evutil_snprintf(
		date, datelen, "%s, %02d %s %4d %02d:%02d:%02d GMT",
		DAYS[tm->tm_wday], tm->tm_mday, MONTHS[tm->tm_mon],
		1900 + tm->tm_year, tm->tm_hour, tm->tm_min, tm->tm_sec);
}

/*
   This function assumes it's called repeatedly with a
   not-actually-so-monotonic time source whose outputs are in 'tv'. It
   implements a trivial ratcheting mechanism so that the values never go
   backwards.
 */
static void
adjust_monotonic_time(struct evutil_monotonic_timer *base,
    struct timeval *tv)
{
	timeradd(tv, &base->adjust_monotonic_clock, tv);

	if (evutil_timercmp(tv, &base->last_time, <)) {
		/* Guess it wasn't monotonic after all. */
		struct timeval adjust;
		timersub(&base->last_time, tv, &adjust);
		timeradd(&adjust, &base->adjust_monotonic_clock,
		    &base->adjust_monotonic_clock);
		*tv = base->last_time;
	}
	base->last_time = *tv;
}

/*
   Allocate a new struct evutil_monotonic_timer
 */
struct evutil_monotonic_timer *
evutil_monotonic_timer_new(void)
{
  struct evutil_monotonic_timer *p = NULL;

  p = mm_malloc(sizeof(*p));
  if (!p) goto done;

  memset(p, 0, sizeof(*p));

 done:
  return p;
}

/*
   Free a struct evutil_monotonic_timer
 */
void
evutil_monotonic_timer_free(struct evutil_monotonic_timer *timer)
{
  if (timer) {
    mm_free(timer);
  }
}

/*
   Set up a struct evutil_monotonic_timer for initial use
 */
int
evutil_configure_monotonic_time(struct evutil_monotonic_timer *timer,
                                int flags)
{
  return evutil_configure_monotonic_time_(timer, flags);
}

/*
   Query the current monotonic time
 */
int
evutil_gettime_monotonic(struct evutil_monotonic_timer *timer,
                         struct timeval *tp)
{
  return evutil_gettime_monotonic_(timer, tp);
}

/* =====
   The POSIX clock_gettime() interface provides a few ways to get at a
   monotonic clock.  CLOCK_MONOTONIC is most widely supported.  Linux also
   provides a CLOCK_MONOTONIC_COARSE with accuracy of about 1-4 msec.

   On all platforms I'm aware of, CLOCK_MONOTONIC really is monotonic.
   Platforms don't agree about whether it should jump on a sleep/resume.
 */

int
evutil_configure_monotonic_time_(struct evutil_monotonic_timer *base,
    int flags)
{
	/* CLOCK_MONOTONIC exists on FreeBSD, Linux, and Solaris.  You need to
	 * check for it at runtime, because some older kernel versions won't
	 * have it working. */
#ifdef CLOCK_MONOTONIC_COARSE
	const int precise = flags & EV_MONOT_PRECISE;
#endif
	const int fallback = flags & EV_MONOT_FALLBACK;
	struct timespec	ts;

#ifdef CLOCK_MONOTONIC_COARSE
	if (CLOCK_MONOTONIC_COARSE < 0) {
		/* Technically speaking, nothing keeps CLOCK_* from being
		 * negative (as far as I know). This check and the one below
		 * make sure that it's safe for us to use -1 as an "unset"
		 * value. */
		event_errx(1,"I didn't expect CLOCK_MONOTONIC_COARSE to be < 0");
	}
	if (! precise && ! fallback) {
		if (clock_gettime(CLOCK_MONOTONIC_COARSE, &ts) == 0) {
			base->monotonic_clock = CLOCK_MONOTONIC_COARSE;
			return 0;
		}
	}
#endif
	if (!fallback && clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
		base->monotonic_clock = CLOCK_MONOTONIC;
		return 0;
	}

	if (CLOCK_MONOTONIC < 0) {
		event_errx(1,"I didn't expect CLOCK_MONOTONIC to be < 0");
	}

	base->monotonic_clock = -1;
	return 0;
}

int
evutil_gettime_monotonic_(struct evutil_monotonic_timer *base,
    struct timeval *tp)
{
	struct timespec ts;

	if (base->monotonic_clock < 0) {
		if (gettimeofday(tp, NULL) < 0)
			return -1;
		adjust_monotonic_time(base, tp);
		return 0;
	}

	if (clock_gettime(base->monotonic_clock, &ts) == -1)
		return -1;
	tp->tv_sec = ts.tv_sec;
	tp->tv_usec = ts.tv_nsec / 1000;

	return 0;
}
