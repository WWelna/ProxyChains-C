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

int ipv4_connect(char *host, char *port) {
	struct addrinfo hints, *servinfo=NULL, *p=NULL;
	SOCKET sock;
	int r, y=1;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if((r=getaddrinfo(host, port, &hints, &servinfo))!=0) {
		return -1;
	}
	for(p = servinfo; p != NULL; p = p->ai_next) { 
		if((sock = WSASocket(p->ai_family, p->ai_socktype, p->ai_protocol, NULL, 0, 0)) == INVALID_SOCKET) {
			continue;
		}
		//setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &y, 4);
		if(connect(sock, p->ai_addr, p->ai_addrlen) == SOCKET_ERROR) {
			ipv4_close(sock);
			continue;
		}
		break;
	}
	if(p == NULL) {
		if(servinfo)
			freeaddrinfo(servinfo);
		return -2;
	}
	if(servinfo)
		freeaddrinfo(servinfo);
	return sock;
}

/* SOCK_STREAM, IPPROTO_TCP, SOCK_DGRAM, IPPROTO_UDP */
int ipv4_listen(char *ip, char *port, int backlog, int family, int proto) {
	struct sockaddr_in si;
	int y=1;
	SOCKET sock = socket(AF_INET, family, proto);
	if(sock == INVALID_SOCKET) {
		return -1;
	}
	ZeroMemory(&si, sizeof(struct sockaddr_in));
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
	si.sin_family = AF_INET;
	if(ip==NULL) {
		si.sin_addr.s_addr = INADDR_ANY;
	} else {
		si.sin_addr.s_addr = inet_addr((ip));
	}
	si.sin_port = htons(atoi(port));
	if(bind(sock, &si, sizeof(struct sockaddr_in))==SOCKET_ERROR) {
		closesocket(sock);
		return -1;
	}
	if(family == SOCK_STREAM) {
		if(listen(sock, backlog)==SOCKET_ERROR) {
			closesocket(sock);
			return -1;
		}
	}
	return sock;
}

int ipv4_send(int sock, char *b, int size, unsigned int timeout) {
	fd_set fds;
	struct timeval tv;
	int r;
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	tv.tv_sec=timeout; tv.tv_usec=0;
	if((r=select(0, NULL, &fds, NULL, &tv))!=SOCKET_ERROR) {
		if(FD_ISSET(sock, &fds)) { // Can Write
			return send(sock, b, size, 0);
		} else if(r==0) // Timeout
			return 0;
		else
			return -2; // wtf?
	} else
		return -1; // Actual Error
}

int ipv4_recv(int sock, char *b, int size, unsigned int timeout) {
	fd_set fds;
	struct timeval tv;
	int r;
	FD_ZERO(&fds);
	FD_SET(sock, &fds);
	tv.tv_sec=timeout; tv.tv_usec=0;
	if((r=select(0, &fds, NULL, NULL, &tv))!=SOCKET_ERROR) {
		if(FD_ISSET(sock, &fds)) { // Can Receive
			return recv(sock, b, size, 0);
		} else if(r==0) // Timeout
			return 0;
		else
			return -2; // wtf?
	} else
		return -1;	
}

int ipv4_close(int sock) {
    closesocket(sock);
    return 0;
}

char *resolve_host(char *ip, char *b) {
	struct in_addr addr;
	struct hostent *host;
	ZeroMemory(b, 255);
	ZeroMemory(&addr, sizeof(struct in_addr));
	addr.s_addr = inet_addr(ip);
	if(addr.s_addr == INADDR_NONE)
		return NULL;
	if((host=gethostbyaddr((char *)&addr, 4, AF_INET))==NULL)
		return NULL;
	strcpy(b, host->h_name);
	return b;
}

int resolve2addr(char *host, uint32_t *addr) {
	PHOSTENT ent = gethostbyname(host);
	if(ent) {
		if(ent->h_addrtype == AF_INET) { // make sure ipv4
			CopyMemory(addr, ent->h_addr, 4);
			return 0;
		} else
			return -1;
	} else
		return -2;
	return -3;
}

int socks5_connect(SOCKET sock, char *host, char *port) {
	char *buff = calloc(256, sizeof(char));
	uint16_t request_port = htons(atoi(port));
	uint8_t s, host_len = strlen(host), t;
	if(!buff)
		return -1;
	if(ipv4_send(sock, "\x05\x01\x00", 3, DEFAULT_TIMEOUT) < 1) // SOCKS 5 no user/pwd
		return -2;
	if(ipv4_recv(sock, buff, 256, DEFAULT_TIMEOUT) < 1)
		return -3; // timeout or error
	if(buff[0] != 0x05 || buff[1] == 0xFF)
		return -4; // No acceptable response
	buff[1] = 0x01; // TCP Connect
	buff[2] = 0x00; // Reserved
	buff[3] = 0x03; // Domain Name / make server do resolve
	buff[4] = host_len; // Put in the host
	CopyMemory((buff+5), host, host_len);
	CopyMemory(buff+(5+host_len), &request_port, 2); // the port
	s = 7 + host_len;
	if(ipv4_send(sock, buff, s, DEFAULT_TIMEOUT) < 1)
		return -5;
	if(ipv4_recv(sock, buff, s, DEFAULT_TIMEOUT) < 1) {
		return -6;
	}
	s = buff[1]; free(buff);
	return s; // return the SOCKS response
}

int socks4a_connect(SOCKET sock, char *host, char *port) {
	char *buff = calloc(256, sizeof(char));
	uint16_t request_port = htons(atoi(port));
	uint32_t addr=0;
	int ret, hostlen=strlen(host), len=hostlen+10;
	if(!buff)
		return -1;
	buff[0] = 0x04; buff[1] = 0x01;
	CopyMemory(buff+2, &request_port, 2);
	buff[7] = 0x01;
	CopyMemory(buff+9, host, hostlen);
	//dump_("socks4a_req", buff, len);
	if(ipv4_send(sock, buff, len, DEFAULT_TIMEOUT) < 1)
		return -2;
	if(ipv4_recv(sock, buff, 256, DEFAULT_TIMEOUT) < 1)
		return -3;
	ret = buff[1]; free(buff);
	return ret;
}

int socks4_connect(SOCKET sock, char *host, char *port) {
	char *buff = calloc(256, sizeof(char));
	uint16_t request_port = htons(atoi(port));
	uint32_t addr=0;
	int ret;
	if(!buff)
		return -1;
	if(resolve2addr(host, &addr)==0) {
		buff[0] = 0x04; buff[1] = 0x01;
		CopyMemory(buff+2, &request_port, 2);
		CopyMemory(buff+4, &addr, 4);
		//dump_("socks4_req", buff, 9);
		if(ipv4_send(sock, buff, 9, DEFAULT_TIMEOUT) < 1)
			return -2;
		if(ipv4_recv(sock, buff, 256, DEFAULT_TIMEOUT) < 1)
			return -3;
		ret = buff[1]; free(buff);
		return ret;
	} else
		return -4;
	return -5; // Should never get to
}

int http_connect(SOCKET sock, char *host, char *port) {
}

/* There is no good way... */
int proxy_detect(char *host, char *port) {
	SOCKET s;
	/* SOCKS 5 */
	if((s = ipv4_connect(host, port)) == INVALID_SOCKET)
		return -1;
	if(socks5_connect(s, "www.google.com", "80") == SOCKS5_GRANTED) {
		ipv4_close(s);
		return TYPE_SOCKS5; 
	} ipv4_close(s);
	/* SOCKS 4a */
	if((s = ipv4_connect(host, port)) == INVALID_SOCKET)
		return -1;
	if(socks4a_connect(s, "www.google.com", "80") == SOCKS4_GRANTED) {
		ipv4_close(s);
		return TYPE_SOCKS4a;
	} ipv4_close(s);
	/* SOCKS 4 */
	if((s = ipv4_connect(host, port)) == INVALID_SOCKET)
		return -1;
	if(socks4_connect(s, "www.google.com", "80") == SOCKS4_GRANTED) {
		ipv4_close(s);
		return TYPE_SOCKS4;
	} ipv4_close(s);
	// No HTTP for now
	return -2;
}
