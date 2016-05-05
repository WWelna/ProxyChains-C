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

#ifndef __PROXY__
#define __PROXY__

#define _WIN32_WINNT 0x0501

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Winbase.h>
#include <time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pstdint.h>

#include "proxy_net.h"

#define TYPE_HTTP    1
#define TYPE_ROUTER  2
#define TYPE_SOCKS4a 3
#define TYPE_SOCKS4  4
#define TYPE_SOCKS5  5
#define TYPE_DETECT  9001

void dump_(char *name, char *data, int size); // For Debug

/* Structs */

typedef struct {
	char host[255];
	char port[7];
	int type;
	/* Only used for initial connection */
	SOCKET sock;
	/* Misc Info */
	char country_name[255];
	char country_code[2];
	/* * */
	int is_connected;
	int is_parent;
	void *n;
} Chain;

typedef struct {
	char host[255];
	char port[7];
} Proxy_Return;

/* Structs */

/* Make a new random chain */
typedef Proxy_Return *(*Proxy_Handler)(void *); // Returns good proxy (hopefully) on call
Chain *chain_randombuild(int length, Proxy_Handler h);
Chain *chain_new(); // First allocation

int chain_connect(Chain *c, char *dest_host, char *dest_port);
int chain_torbotstrap(Chain *c); // Connect through tor to first chain

/* Add to a chain */
int chain_add(Chain *c, char *host, char *port, int type);

/* Remove Proxy that errored out for replacement */
void chain_fix(Chain *c);

/* Obvious */
void chain_destroy(Chain *c);

/* Main Listener */

int listen_loop(char *host, char *port);

#endif
