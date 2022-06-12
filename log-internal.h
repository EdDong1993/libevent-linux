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
#ifndef LOG_INTERNAL_H_INCLUDED_
#define LOG_INTERNAL_H_INCLUDED_

#include "event2/util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EV_CHECK_FMT(a,b) __attribute__((format(printf, a, b)))
#define EV_NORETURN __attribute__((noreturn))

#define EVENT_ERR_ABORT_ ((int)0xdeaddead)

EVENT2_EXPORT_SYMBOL
void event_err(int eval, const char *fmt, ...) EV_CHECK_FMT(2,3) EV_NORETURN;
EVENT2_EXPORT_SYMBOL
void event_warn(const char *fmt, ...) EV_CHECK_FMT(1,2);
EVENT2_EXPORT_SYMBOL
void event_sock_err(int eval, int sock, const char *fmt, ...) EV_CHECK_FMT(3,4) EV_NORETURN;
EVENT2_EXPORT_SYMBOL
void event_sock_warn(int sock, const char *fmt, ...) EV_CHECK_FMT(2,3);
EVENT2_EXPORT_SYMBOL
void event_errx(int eval, const char *fmt, ...) EV_CHECK_FMT(2,3) EV_NORETURN;
EVENT2_EXPORT_SYMBOL
void event_warnx(const char *fmt, ...) EV_CHECK_FMT(1,2);
EVENT2_EXPORT_SYMBOL
void event_msgx(const char *fmt, ...) EV_CHECK_FMT(1,2);
EVENT2_EXPORT_SYMBOL
void event_debugx_(const char *fmt, ...) EV_CHECK_FMT(1,2);

EVENT2_EXPORT_SYMBOL
void event_logv_(int severity, const char *errstr, const char *fmt, va_list ap)
	EV_CHECK_FMT(3,0);

#define event_debug(x) ((void)0)

#undef EV_CHECK_FMT

#ifdef __cplusplus
}
#endif

#endif /* LOG_INTERNAL_H_INCLUDED_ */
