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

#include "proxy.h"

Chain *chain_new() {
	Chain *p = (Chain *)calloc(1, sizeof(Chain));
	if(p)
		p->is_parent = 1;
	return p;
}

int chain_add(Chain *c, char *host, char *port, int type) {
	if(strlen(host)>255 || strlen(port)>7)
		return -1; // Buffer Overflows are bad m'kay
	if(!c->n && (strlen(c->host)==0)) { // first proxy chain!
		CopyMemory(c->host, host, strlen(host));
		CopyMemory(c->port, port, strlen(port));
		c->type = type;
		return 0;
	} else { // Append to Da Proxies!
		Chain *p, *e = calloc(1, sizeof(Chain));
		if(e) {
			CopyMemory(e->host, host, strlen(host));
			CopyMemory(e->port, port, strlen(port));
			e->type = type;
			for(p=c; p->n != NULL; ) p=p->n;
			p->n = e;
		} else
			return -1; // FUK U!
	}
	return 0;
}

int chain_connect(Chain *c, char *dest_host, char *dest_port) {
	Chain *p, *pp;
	SOCKET s=INVALID_SOCKET;
	for(p=c; p != NULL; p=p->n) {
		if(p->is_parent && !p->is_connected) {
			printf("Connecting To %s:%s\n", p->host, p->port);
			if((p->sock=ipv4_connect(p->host, p->port)) == INVALID_SOCKET) {
				return -1; // Initial Connection Failed... Bad!
			} else {
				p->is_connected = 1;
				s=p->sock;
			}
		}
		if(p->type == TYPE_SOCKS5) {
			if(p->n == NULL) { // Last in the chain so.. connect to destination
				printf("%s:%s to Destination %s:%s via SOCKS5\n", p->host, p->port, dest_host, dest_port);
				if(socks5_connect(s, dest_host, dest_port) != SOCKS5_GRANTED) {
					ipv4_close(s);
					return -2;
				} else {
					p->is_connected = 1;
					return 0; // Return here because I can
				}
			} else { pp = p->n;
				printf("SOCKS5 %s:%s -> %s:%s\n", p->host, p->port, pp->host, pp->port);
				if(socks5_connect(s, pp->host, pp->port) != SOCKS5_GRANTED) {
					ipv4_close(s);
					return -3;
				} else
					p->is_connected = 1;
			}
		} else if(p->type == TYPE_SOCKS4a) {
			if(p->n == NULL) { // Last in the chain so.. connect to destination
				printf("%s:%s to Destination %s:%s via SOCKS4a\n", p->host, p->port, dest_host, dest_port);
				if(socks4a_connect(s, dest_host, dest_port) != SOCKS4_GRANTED) {
					ipv4_close(s);
					return -2;
				} else {
					p->is_connected = 1;
					return 0; // Return here because I can
				}
			} else { pp = p->n;
				printf("SOCKS4a %s:%s -> %s:%s\n", p->host, p->port, pp->host, pp->port);
				if(socks4a_connect(s, pp->host, pp->port) != SOCKS4_GRANTED) {
					ipv4_close(s);
					return -3;
				} else
					p->is_connected = 1;
			}
		} else if(p->type == TYPE_SOCKS4) {
			if(p->n == NULL) { // Last in the chain so.. connect to destination
				printf("%s:%s to Destination %s:%s via SOCKS4\n", p->host, p->port, dest_host, dest_port);
				if(socks4_connect(s, dest_host, dest_port) != SOCKS4_GRANTED) {
					ipv4_close(s);
					return -2;
				} else {
					p->is_connected = 1;
					return 0; // Return here because I can
				}
			} else { pp = p->n;
				printf("SOCKS4 %s:%s -> %s:%s\n", p->host, p->port, pp->host, pp->port);
				if(socks4_connect(s, pp->host, pp->port) != SOCKS4_GRANTED) {
					ipv4_close(s);
					return -3;
				} else
					p->is_connected = 1;
			}
		} else if(p->type == TYPE_HTTP) {
			/* Fuck You */
			ipv4_close(s);
			return -666;
		}
	}
}

void chain_destroy(Chain *c) {
	Chain *p;
	void *f;
	if(c->is_connected)
		ipv4_close(c->sock);
	if(c) {
		for(p=c; p != NULL; ) {
			f=p; p=p->n;
			free(f);
		}
	}
}
