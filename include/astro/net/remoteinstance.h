#ifndef __remoteinstance_h
#define __remoteinstance_h

#include <astro/net/socket.h>
#include <astro/exceptions.h>
#include <astro/system/log.h>

#include <stdio.h>

#include <list>
#include <vector>

namespace peyton {

namespace exceptions {
	/// RemoteInstance exception
	SIMPLE_EXCEPTION(ERI);
}

namespace net {

class RemoteInstance;

class InstanceData {
public:
	virtual ~InstanceData() {}
};

class Instance {
protected:
	char **argv;
	int argc;
	InstanceData *udata;

public:
	Socket link;
	std::string host;
	
	static int isClient(char *server, int argc, char **argv);
	void setArguments(int ac, char **av) { argc = ac; argv = av; }
	InstanceData *&userData() { return udata; }
	void deleteUserData() { delete udata; udata = NULL; }

	virtual int run() { return -1; }
	
	Instance() { udata = NULL; }
	virtual ~Instance() { deleteUserData(); }
};

template<typename T, typename cont = std::vector<T *> >
class ptr_bag : public cont {
public:
	~ptr_bag() { for(typename cont::iterator i = begin(); i != end(); i++) { delete *i; } }
};

class ServerInstance : public Instance {
protected:
	void startListening();
public:
	int port;
	std::list<RemoteInstance *> clients;
public:
	ServerInstance(const char *localhost = NULL);
	virtual ~ServerInstance();

	RemoteInstance *spawn(const char *command, const char *host);
	int waitFor(std::vector<RemoteInstance *> &signaled, const std::list<RemoteInstance *> &clients, int timeout = -1);

	bool spawnFlock(std::string flockName, std::string exe, std::vector<std::pair<std::string, bool> > *success = NULL);

	//---- callbacks:

	// overload this for initializing newly spawned remote instances - eg., for setting user data
	// it's called from ServerInstance:spawn()
	virtual void callbackInitializeSpawnedInstance(RemoteInstance *ri) {}

	friend class RemoteInstance;
};

class RemoteInstance : public Instance {
public:
	int read(void *data, int size);
	bool tryRead(void *data, int size);
	int write(const void *data, int size);
	int shutdown();
	bool isConnected();

public:
	RemoteInstance();
	RemoteInstance(ServerInstance *server, const char *command, const char *host);
	virtual ~RemoteInstance();

	void bindToServer(const char *master);

	template<typename T> bool tryRead(T &v) { return tryRead(&v, sizeof(T)); }
	template<typename T> int read(T &v) { return read(&v, sizeof(T)); }
private:
	ServerInstance *server;
	std::string command;
	
	friend class ServerInstance;
};

template<typename Server, typename Client>
int main_aux(int argc, char **argv) {
	try {
		Instance *ins; char server[1000];

		if(Instance::isClient(server, argc, argv)) {
			DEBUG(basic, "Starting client");
			Client *cli = new Client;
			cli->bindToServer(server);
			ins = cli;
		} else {
			DEBUG(basic, "Starting server");
			ins = new Server;
		}
		ins->setArguments(argc, argv);

		int ret = ins->run();
		delete ins;

		return ret;
	} catch (peyton::exceptions::EAny &e) {
		e.print();
		return -1;
	}
}

#define MAIN(Server, Client) int main(int argc, char **argv) { return main_aux<Server, Client>(argc, argv); }

#define INSTANCE(ins) ins::ins(int argc = 0, char **argv = NULL, const char *localhost = NULL) : RemoteInstance(argc, argv, localhost) {}

}
}

//
//	I'm keeping these in global namespace so that they can be specialized by client classes more easily
//

template<class T> inline peyton::net::RemoteInstance &operator <<(peyton::net::RemoteInstance &r, T &v) { v.write(r); return r; }
template<class T> inline peyton::net::RemoteInstance &operator >>(peyton::net::RemoteInstance &r, T &v) { v.read(r); return r; }

// primitive type specializations

#define WGEN(T) inline peyton::net::RemoteInstance & operator<<(peyton::net::RemoteInstance &r, T v) { r.write(&v, sizeof(T)); return r; }
#define RGEN(T) template< > inline peyton::net::RemoteInstance & operator>>(peyton::net::RemoteInstance &r, T &v) { r.read(&v, sizeof(T)); return r; }
#define GEN(T) WGEN(T); RGEN(T);

GEN(double); GEN(int); GEN(long); GEN(float); GEN(char); GEN(unsigned int);

#undef GEN
#undef WGEN
#undef RGEN

#define __peyton_net peyton::net

#endif
