/** \file
    \brief Implementation of the Socket class.
*/

#include <astro/net/socket.h>
#include <astro/util.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

#include "string.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <stdio.h>
#include <signal.h>

using namespace peyton;
using namespace peyton::exceptions;
using namespace peyton::net;
using util::str;

#define STHROW(inf) THROW(ESocket, inf)
#define CHKVALID if(!isValid()) STHROW("Socket not initialized - call create first");

/**
	Internal singleton class whose only purpose is to set appropriate
	signal handlers in the constructor, on startup of the library.
*/
class SignalHandlers {
public:
	SignalHandlers()
	{
		// Ignore sigpipe, or else shit will happen with ::select et al.
		// when a client drops a connection

	/*	struct sigaction sa;
		sa.sa_handler = SIG_IGN;
		sa.sa_flags = 0;
		sigemptyset(&sa.sa_mask);

		if(sigaction(SIGPIPE, &sa, (struct sigaction *)NULL) != 0)*/

		if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			STHROW("Cannot setup SIGPIPE-ignoring handler!");
	}
} handlers;

void Socket::__init()
{
	m_sock = -1;
	connected = false;
	buf = NULL; bufAllocated = 0; ptr = 0;
	memset(&m_addr, 0, sizeof(m_addr));

}

Socket::Socket() { __init(); }

Socket::Socket ( int port )
{
	__init();

	bind(port);
	listen();
}

Socket::Socket(std::string host, int port)
{
	__init();

	connect(host, port);
}

void Socket::close()
{
	if(isValid()) {
		::close (m_sock);
		m_sock = -1;
		connected = false;
	}
}

Socket::~Socket()
{
	close();
	freeBuffer();
}

void Socket::create()
{
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (!isValid()) THROW(ESocket, "Could not create socket");

	// TIME_WAIT - argh
	int on = 1;
	if(setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) == -1) {
		close();
		THROW(ESocket, "Could not set socket options");
	}
}

void Socket::bind(const int port)
{
	close(); create();

	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = INADDR_ANY;
	m_addr.sin_port = htons ( port );

	if(::bind(m_sock, (sockaddr *)&m_addr, sizeof(m_addr)))
		STHROW("Could not bind socket to port " + str(port));
}

void Socket::listen() const
{
	CHKVALID;

	if(::listen(m_sock, MAXCONNECTIONS))
		STHROW("Could not start listening");
}


void Socket::accept(Socket& link, int timeout) const
{
	CHKVALID;

	int addrLen = sizeof(m_addr);
	
	fd_set set; FD_ZERO(&set); FD_SET(m_sock, &set);
	timeval tv, *tvp = &tv; tv.tv_sec = timeout; tv.tv_usec = 0;
	if(timeout < 0) tvp = NULL;

	switch(select(m_sock+1, &set, NULL, NULL, tvp)) {
	case 0: STHROW("Timed out after " + str(timeout) + " sec. while waiting for client connection");
	case -1: STHROW("Error in ::select while waiting for incoming connection");
	}

	link.close();

	if((link.m_sock = ::accept(m_sock, (sockaddr *)&m_addr, (socklen_t *) &addrLen)) == -1)
		STHROW("Failed accepting connection");

	link.connected = 1;
}

int Socket::write(const void *data, int length)
{
	CHKVALID;

	fd_set set;
	FD_ZERO(&set);
	FD_SET(m_sock, &set);

	int written = 0;
	while(written < length) {
		if(::select(m_sock+1, NULL, &set, NULL, NULL) != 1)
			STHROW("Error while doing ::select");

		int n = ::write(m_sock, (char *)data + written, length - written);

		if(n == -1 && errno == EPIPE) {
			close();
			THROW(EEOF, "Connection closed while writing to socket");
		}
		if(n == -1) STHROW("Error while doing ::write");

		written += n;
	}

	return written;
}

void *Socket::ensureBuffer(int length)
{
	if(bufAllocated >= length) return buf;
	
	buf = realloc(buf, length);
	bufAllocated = length;
	return buf;
}

bool Socket::shiftBuffer(void *data, int length)
{
	if(ptr < length) return false;

	memcpy(data, buf, length);
	memmove(buf, (char *)buf + length, ptr - length);
	ptr -= length;

	return true;
}

void Socket::freeBuffer()
{
	free(buf); ptr = bufAllocated = 0;
}

bool Socket::tryRead(void *dat, int length)
{
	CHKVALID;

	// check what we already have in the buffer
	if(ptr >= length) { return shiftBuffer(dat, length); }

	// check if there's data available
	fd_set set; FD_ZERO(&set); FD_SET(m_sock, &set);
	timeval tv; tv.tv_sec = 0; tv.tv_usec = 0;

	switch(select(m_sock+1, &set, NULL, NULL, &tv)) {
	case 0: return false;
	case -1: STHROW("Error in ::select");
	}

	// read the data
	if(ensureBuffer(length) == NULL) { return false; }

	int n = ::read(m_sock, (char *)buf + ptr, length - ptr);
	if(n == 0) {
		close();
		THROW(EEOF, "Connection closed while reading on socket");
	}
	if(n == -1) STHROW("Error while reading socket");

	ptr += n;

	return shiftBuffer(dat, length);
}

int Socket::read(void *data, int length)
{
	CHKVALID;

	fd_set set;
	FD_ZERO(&set);
	FD_SET(m_sock, &set);

	int read = 0, n;
	while(read < length) {
		if(select(m_sock+1, &set, NULL, NULL, NULL) != 1)
			STHROW("Error while doing ::select");

		switch(n = ::read(m_sock, (char *)data + read, length - read)) {
		case 0:
			close();
			THROW(EEOF, "Connection closed while reading on socket");
		case -1: STHROW("Error while reading socket");
		}
//		cout << length << " | " << read << "\n"; cout.flush();

		read += n;
	}

	return read;
}

void Socket::connect(const std::string host, const int port )
{
	close(); create();

	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons ( port );

	if(inet_pton(AF_INET, host.c_str(), &m_addr.sin_addr) == 0) {
		hostent *h = gethostbyname(host.c_str());
		if(h == NULL)
			STHROW("Error while connecting : cannot resolve host " + host);
		memcpy(&m_addr.sin_addr, *(h->h_addr_list),sizeof(in_addr));
	} else
		STHROW("Error doing ::inet_pton for " + host + ":" + str(port));

	if(::connect(m_sock, (sockaddr *)&m_addr, sizeof(m_addr)) != 0) {
		STHROW("Error connecting to " + host + ":" + str(port));
	}
}

bool Socket::setNonBlocking(const bool noBlock)
{
	int opts = fcntl (m_sock, F_GETFL);

	if(opts < 0) { return false; }

	opts = noBlock ? (opts | O_NONBLOCK) : (opts & ~O_NONBLOCK);

	return fcntl(m_sock, F_SETFL, opts) == 0;
}
