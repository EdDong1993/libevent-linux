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

/* This file has our secure PRNG code.  On platforms that have arc4random(),
 * we just use that.  Otherwise, we include arc4random.c as a bunch of static
 * functions, and wrap it lightly.  We don't expose the arc4random*() APIs
 * because A) they aren't in our namespace, and B) it's not nice to name your
 * APIs after their implementations.  We keep them in a separate file
 * so that other people can rip it out and use it for whatever.
 */

#include "event2/event-config.h"
#include "evconfig-private.h"

#include <limits.h>

#include "util-internal.h"
#include "evthread-internal.h"

#define ARC4_LOCK_() EVLOCK_LOCK(arc4rand_lock, 0)
#define ARC4_UNLOCK_() EVLOCK_UNLOCK(arc4rand_lock, 0)
#ifndef EVENT__DISABLE_THREAD_SUPPORT
static void *arc4rand_lock;
#endif

#include "./arc4random.c"

#ifndef EVENT__DISABLE_THREAD_SUPPORT
int
evutil_secure_rng_global_setup_locks_(const int enable_locks)
{
	EVTHREAD_SETUP_GLOBAL_LOCK(arc4rand_lock, 0);
	return 0;
}
#endif

static void
evutil_free_secure_rng_globals_locks(void)
{
#ifndef EVENT__DISABLE_THREAD_SUPPORT
	if (arc4rand_lock != NULL) {
		EVTHREAD_FREE_LOCK(arc4rand_lock, 0);
		arc4rand_lock = NULL;
	}
#endif
	return;
}

int
evutil_secure_rng_set_urandom_device_file(char *fname)
{
	ARC4_LOCK_();
	arc4random_urandom_filename = fname;
	ARC4_UNLOCK_();
	return 0;
}

int
evutil_secure_rng_init(void)
{
	int val;

	ARC4_LOCK_();
	val = (!arc4_stir()) ? 0 : -1;
	ARC4_UNLOCK_();
	return val;
}

static void
ev_arc4random_buf(void *buf, size_t n)
{
	arc4random_buf(buf, n);
}


void
evutil_secure_rng_get_bytes(void *buf, size_t n)
{
	ev_arc4random_buf(buf, n);
}

void
evutil_secure_rng_add_bytes(const char *buf, size_t n)
{
	arc4random_addrandom((unsigned char*)buf,
	    n>(size_t)INT_MAX ? INT_MAX : (int)n);
}

void
evutil_free_secure_rng_globals_(void)
{
    evutil_free_secure_rng_globals_locks();
}
