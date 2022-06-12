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
#ifndef EVENT2_UTIL_H_INCLUDED_
#define EVENT2_UTIL_H_INCLUDED_

/** @file event2/util.h

  Common convenience functions for cross-platform portability and
  related socket manipulations.

 */
#include <event2/visibility.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <event2/event-config.h>
#include <sys/time.h>
#include <stdint.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdarg.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <time.h>

/**
   @name Limits for SIZE_T and SSIZE_T

   @{
*/
#define EV_SIZE_MAX UINT64_MAX
#define EV_SSIZE_MAX INT64_MAX
#define EV_SSIZE_MIN ((-EV_SSIZE_MAX) - 1)
/**@}*/


/**
 * Structure to hold information about a monotonic timer
 *
 * Use this with evutil_configure_monotonic_time() and
 * evutil_gettime_monotonic().
 *
 * This is an opaque structure; you can allocate one using
 * evutil_monotonic_timer_new().
 *
 * @see evutil_monotonic_timer_new(), evutil_monotonic_timer_free(),
 * evutil_configure_monotonic_time(), evutil_gettime_monotonic()
 */
struct evutil_monotonic_timer;

#define EV_MONOT_PRECISE  1
#define EV_MONOT_FALLBACK 2

/** Format a date string using RFC 1123 format (used in HTTP).
 * If `tm` is NULL, current system's time will be used.
 * The number of characters written will be returned.
 * One should check if the return value is smaller than `datelen` to check if
 * the result is truncated or not.
 */
EVENT2_EXPORT_SYMBOL int
evutil_date_rfc1123(char *date, const size_t datelen, const struct tm *tm);

/** Allocate a new struct evutil_monotonic_timer for use with the
 * evutil_configure_monotonic_time() and evutil_gettime_monotonic()
 * functions.  You must configure the timer with
 * evutil_configure_monotonic_time() before using it.
 */
EVENT2_EXPORT_SYMBOL
struct evutil_monotonic_timer * evutil_monotonic_timer_new(void);

/** Free a struct evutil_monotonic_timer that was allocated using
 * evutil_monotonic_timer_new().
 */
EVENT2_EXPORT_SYMBOL
void evutil_monotonic_timer_free(struct evutil_monotonic_timer *timer);

/** Set up a struct evutil_monotonic_timer; flags can include
 * EV_MONOT_PRECISE and EV_MONOT_FALLBACK.
 */
EVENT2_EXPORT_SYMBOL
int evutil_configure_monotonic_time(struct evutil_monotonic_timer *timer,
                                    int flags);

/** Query the current monotonic time from a struct evutil_monotonic_timer
 * previously configured with evutil_configure_monotonic_time().  Monotonic
 * time is guaranteed never to run in reverse, but is not necessarily epoch-
 * based, or relative to any other definite point.  Use it to make reliable
 * measurements of elapsed time between events even when the system time
 * may be changed.
 *
 * It is not safe to use this funtion on the same timer from multiple
 * threads.
 */
EVENT2_EXPORT_SYMBOL
int evutil_gettime_monotonic(struct evutil_monotonic_timer *timer,
                             struct timeval *tp);

/** Create two new sockets that are connected to each other.

    On Unix, this simply calls socketpair().  On Windows, it uses the
    loopback network interface on 127.0.0.1, and only
    AF_INET,SOCK_STREAM are supported.

    (This may fail on some Windows hosts where firewall software has cleverly
    decided to keep 127.0.0.1 from talking to itself.)

    Parameters and return values are as for socketpair()
*/
EVENT2_EXPORT_SYMBOL
int evutil_socketpair(int d, int type, int protocol, int sv[2]);
/** Do platform-specific operations as needed to make a socket nonblocking.

    @param sock The socket to make nonblocking
    @return 0 on success, -1 on failure
 */
EVENT2_EXPORT_SYMBOL
int evutil_make_socket_nonblocking(int sock);

/** Do platform-specific operations to make a listener socket reusable.

    Specifically, we want to make sure that another program will be able
    to bind this address right after we've closed the listener.

    This differs from Windows's interpretation of "reusable", which
    allows multiple listeners to bind the same address at the same time.

    @param sock The socket to make reusable
    @return 0 on success, -1 on failure
 */
EVENT2_EXPORT_SYMBOL
int evutil_make_listen_socket_reuseable(int sock);

/** Do platform-specific operations to make a listener port reusable.

    Specifically, we want to make sure that multiple programs which also
    set the same socket option will be able to bind, listen at the same time.

    This is a feature available only to Linux 3.9+

    @param sock The socket to make reusable
    @return 0 on success, -1 on failure
 */
EVENT2_EXPORT_SYMBOL
int evutil_make_listen_socket_reuseable_port(int sock);

/** Set ipv6 only bind socket option to make listener work only in ipv6 sockets.

    According to RFC3493 and most Linux distributions, default value for the
    sockets is to work in IPv4-mapped mode. In IPv4-mapped mode, it is not possible
    to bind same port from different IPv4 and IPv6 handlers.

    @param sock The socket to make in ipv6only working mode
    @return 0 on success, -1 on failure
 */
EVENT2_EXPORT_SYMBOL
int evutil_make_listen_socket_ipv6only(int sock);

/** Do platform-specific operations as needed to close a socket upon a
    successful execution of one of the exec*() functions.

    @param sock The socket to be closed
    @return 0 on success, -1 on failure
 */
EVENT2_EXPORT_SYMBOL
int evutil_make_socket_closeonexec(int sock);

/** Do the platform-specific call needed to close a socket returned from
    socket() or accept().

    @param sock The socket to be closed
    @return 0 on success (whether the operation is supported or not),
            -1 on failure
 */
EVENT2_EXPORT_SYMBOL
int evutil_closesocket(int sock);
#define EVUTIL_CLOSESOCKET(s) evutil_closesocket(s)

/** Do platform-specific operations, if possible, to make a tcp listener
 *  socket defer accept()s until there is data to read.
 *  
 *  Not all platforms support this.  You don't want to do this for every
 *  listener socket: only the ones that implement a protocol where the
 *  client transmits before the server needs to respond.
 *
 *  @param sock The listening socket to to make deferred
 *  @return 0 on success (whether the operation is supported or not),
 *       -1 on failure
*/ 
EVENT2_EXPORT_SYMBOL
int evutil_make_tcp_listen_socket_deferred(int sock);

#define EVUTIL_INVALID_SOCKET -1
/**@}*/

/** Return true iff the tvp is related to uvp according to the relational
 * operator cmp.  Recognized values for cmp are ==, <=, <, >=, and >. */
#define	evutil_timercmp(tvp, uvp, cmp)					\
	(((tvp)->tv_sec == (uvp)->tv_sec) ?				\
	 ((tvp)->tv_usec cmp (uvp)->tv_usec) :				\
	 ((tvp)->tv_sec cmp (uvp)->tv_sec))

/* big-int related functions */
/** Parse a 64-bit value from a string.  Arguments are as for strtol. */
EVENT2_EXPORT_SYMBOL
int64_t evutil_strtoll(const char *s, char **endptr, int base);

/** Replacement for snprintf to get consistent behavior on platforms for
    which the return value of snprintf does not conform to C99.
 */
EVENT2_EXPORT_SYMBOL
int evutil_snprintf(char *buf, size_t buflen, const char *format, ...)
#ifdef __GNUC__
	__attribute__((format(printf, 3, 4)))
#endif
;
/** Replacement for vsnprintf to get consistent behavior on platforms for
    which the return value of snprintf does not conform to C99.
 */
EVENT2_EXPORT_SYMBOL
int evutil_vsnprintf(char *buf, size_t buflen, const char *format, va_list ap)
#ifdef __GNUC__
	__attribute__((format(printf, 3, 0)))
#endif
;

/** Replacement for inet_ntop for platforms which lack it. */
EVENT2_EXPORT_SYMBOL
const char *evutil_inet_ntop(int af, const void *src, char *dst, size_t len);
/** Variation of inet_pton that also parses IPv6 scopes. Public for
    unit tests. No reason to call this directly.
 */
EVENT2_EXPORT_SYMBOL
int evutil_inet_pton_scope(int af, const char *src, void *dst,
	unsigned *indexp);
/** Replacement for inet_pton for platforms which lack it. */
EVENT2_EXPORT_SYMBOL
int evutil_inet_pton(int af, const char *src, void *dst);

/** Parse an IPv4 or IPv6 address, with optional port, from a string.

    Recognized formats are:
    - [IPv6Address]:port
    - [IPv6Address]
    - IPv6Address
    - IPv4Address:port
    - IPv4Address

    If no port is specified, the port in the output is set to 0.

    @param str The string to parse.
    @param out A struct sockaddr to hold the result.  This should probably be
       a struct sockaddr_storage.
    @param outlen A pointer to the number of bytes that that 'out' can safely
       hold.  Set to the number of bytes used in 'out' on success.
    @return -1 if the address is not well-formed, if the port is out of range,
       or if out is not large enough to hold the result.  Otherwise returns
       0 on success.
*/
EVENT2_EXPORT_SYMBOL
int evutil_parse_sockaddr_port(const char *str, struct sockaddr *out, int *outlen);

/** Compare two sockaddrs; return 0 if they are equal, or less than 0 if sa1
 * preceeds sa2, or greater than 0 if sa1 follows sa2.  If include_port is
 * true, consider the port as well as the address.  Only implemented for
 * AF_INET and AF_INET6 addresses. The ordering is not guaranteed to remain
 * the same between Libevent versions. */
EVENT2_EXPORT_SYMBOL
int evutil_sockaddr_cmp(const struct sockaddr *sa1, const struct sockaddr *sa2,
    int include_port);

/** As strcasecmp, but always compares the characters in locale-independent
    ASCII.  That's useful if you're handling data in ASCII-based protocols.
 */
EVENT2_EXPORT_SYMBOL
int evutil_ascii_strcasecmp(const char *str1, const char *str2);
/** As strncasecmp, but always compares the characters in locale-independent
    ASCII.  That's useful if you're handling data in ASCII-based protocols.
 */
EVENT2_EXPORT_SYMBOL
int evutil_ascii_strncasecmp(const char *str1, const char *str2, size_t n);

/** @name evutil_getaddrinfo() error codes

    These values are possible error codes for evutil_getaddrinfo() and
    related functions.

    @{
*/

/* This test is a bit complicated, since some MS SDKs decide to
 * remove NODATA or redefine it to be the same as NONAME, in a
 * fun interpretation of RFC 2553 and RFC 3493. */
#define EVUTIL_EAI_CANCEL -90001
/**@}*/

/**
 * This function clones getaddrinfo for systems that don't have it.  For full
 * details, see RFC 3493, section 6.1.
 *
 * Limitations:
 * - When the system has no getaddrinfo, we fall back to gethostbyname_r or
 *   gethostbyname, with their attendant issues.
 * - The AI_V4MAPPED and AI_ALL flags are not currently implemented.
 *
 * For a nonblocking variant, see evdns_getaddrinfo.
 */
EVENT2_EXPORT_SYMBOL
int evutil_getaddrinfo(const char *nodename, const char *servname,
    const struct addrinfo *hints_in, struct addrinfo **res);

/** Release storage allocated by evutil_getaddrinfo or evdns_getaddrinfo. */
EVENT2_EXPORT_SYMBOL
void evutil_freeaddrinfo(struct addrinfo *ai);

EVENT2_EXPORT_SYMBOL
const char *evutil_gai_strerror(int err);

/** Generate n bytes of secure pseudorandom data, and store them in buf.
 *
 * Current versions of Libevent use an ARC4-based random number generator,
 * seeded using the platform's entropy source (/dev/urandom on Unix-like
 * systems; CryptGenRandom on Windows).  This is not actually as secure as it
 * should be: ARC4 is a pretty lousy cipher, and the current implementation
 * provides only rudimentary prediction- and backtracking-resistance.  Don't
 * use this for serious cryptographic applications.
 */
EVENT2_EXPORT_SYMBOL
void evutil_secure_rng_get_bytes(void *buf, size_t n);

/**
 * Seed the secure random number generator if needed, and return 0 on
 * success or -1 on failure.
 *
 * It is okay to call this function more than once; it will still return
 * 0 if the RNG has been successfully seeded and -1 if it can't be
 * seeded.
 *
 * Ordinarily you don't need to call this function from your own code;
 * Libevent will seed the RNG itself the first time it needs good random
 * numbers.  You only need to call it if (a) you want to double-check
 * that one of the seeding methods did succeed, or (b) you plan to drop
 * the capability to seed (by chrooting, or dropping capabilities, or
 * whatever), and you want to make sure that seeding happens before your
 * program loses the ability to do it.
 */
EVENT2_EXPORT_SYMBOL
int evutil_secure_rng_init(void);

/**
 * Set a filename to use in place of /dev/urandom for seeding the secure
 * PRNG. Return 0 on success, -1 on failure.
 *
 * Call this function BEFORE calling any other initialization or RNG
 * functions.
 *
 * (This string will _NOT_ be copied internally. Do not free it while any
 * user of the secure RNG might be running. Don't pass anything other than a
 * real /dev/...random device file here, or you might lose security.)
 *
 * This API is unstable, and might change in a future libevent version.
 */
EVENT2_EXPORT_SYMBOL
int evutil_secure_rng_set_urandom_device_file(char *fname);

/** Seed the random number generator with extra random bytes.

    You should almost never need to call this function; it should be
    sufficient to invoke evutil_secure_rng_init(), or let Libevent take
    care of calling evutil_secure_rng_init() on its own.

    If you call this function as a _replacement_ for the regular
    entropy sources, then you need to be sure that your input
    contains a fairly large amount of strong entropy.  Doing so is
    notoriously hard: most people who try get it wrong.  Watch out!

    @param dat a buffer full of a strong source of random numbers
    @param datlen the number of bytes to read from datlen
 */
EVENT2_EXPORT_SYMBOL
void evutil_secure_rng_add_bytes(const char *dat, size_t datlen);

#ifdef __cplusplus
}
#endif

#endif /* EVENT1_EVUTIL_H_INCLUDED_ */
