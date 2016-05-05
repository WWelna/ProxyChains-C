/*
* Copyright (C) 2012 William H. Welna All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY William H. Welna ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL William H. Welna BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __PROXYNET__
#define __PROXYNET__

/* Basic Socks API with timeouts and good stuff */
int ipv4_connect(char *host, char *port);
int ipv4_listen(char *ip, char *port, int backlog, int family, int proto);
int ipv4_send(int sock, char *b, int size, unsigned int timeout);
int ipv4_recv(int sock, char *b, int size, unsigned int timeout);
int ipv4_close(int sock);
char *resolve_host(char *ip, char *b);

/* Requires initial connect via ipv4_connect or existing for "tunneling" */
int socks5_connect(SOCKET sock, char *host, char *port);
int socks4a_connect(SOCKET sock, char *host, char *port);
int socks4_connect(SOCKET sock, char *host, char *port);
int http_connect(SOCKET sock, char *host, char *port);

/* High Level Detection ;) */
int proxy_detect(char *host, char *port);

#define DEFAULT_TIMEOUT 360

/* SOCKS 5 */
#define SOCKS5_GRANTED                0
#define SOCKS5_FAILURE                1
#define SOCKS5_RULESET                2
#define SOCKS5_NET_UNREACHABLE        3
#define SOCKS5_HOST_UNREACHABLE       4
#define SOCKS5_REFUSED                5
#define SOCKS5_TTL_EXPIRED            6
#define SOCKS5_PROTO_ERROR            7
#define SOCKS5_ADDRESS_NOT_SUPPORTED  8
/* SOCKS 5 */

/* SOCKS 4 (and 4a) */
#define SOCKS4_GRANTED           0x5a
#define SOCKS4_REJECTED          0x5b
#define SOCKS4_IDENT_NOTRUNNING  0x5c
#define SOCKS4_IDENT_IDFAIL      0x5d
/* SOCKS 4 (and 4a) */

#endif
