// Definition of the Socket class

#ifndef __astro_net_socket_h
#define __astro_net_socket_h

#include <netinet/in.h>
#include <astro/exceptions.h>

namespace peyton {
/// network related functionality
namespace net {

const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;
const int MAXRECV = 500;

// Socket exception
DERIVE_EXCEPTION(ESocket, peyton::exceptions::EErrno);

/// high(er) level TCP socket class
class Socket
{
private:
	void *buf;
	int bufAllocated;
	int ptr;

	void *ensureBuffer(int size);
	void freeBuffer();
	bool shiftBuffer(void *data, int length);

public:
	void __init();
	Socket();
	Socket(int port);					///< Open a listener on requested port
	Socket(const std::string host, const int port);		///< Open a connection to host:port
	virtual ~Socket();

	// Server initialization
	void create();
	void bind(const int port);
	void listen() const;
	void accept(Socket& listener, int timeout=-1) const;

	// Client initialization
	void connect (const std::string host, const int port);

	// Data Transimission
	int write(const void *data, int len);
	int read(void *data, int maxlen);
	bool tryRead(void *dat, int length);

	// Connection handling
	void close();

	bool setNonBlocking(const bool noBlock);

	bool isValid() const { return m_sock != -1; }
	bool isConnected() const { return connected; }

	// Low level access
	int fd() const { return m_sock; }
private:
	int m_sock;
	bool connected;
	sockaddr_in m_addr;
};

}
}

#define __peyton_net peyton::net

#endif
