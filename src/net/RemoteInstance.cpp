#include <astro/net/remoteinstance.h>
#include <astro/net/socket.h>
#include <astro/system/log.h>
#include <astro/util.h>

//#include "socket/SocketException.h"

#include <vector>
#include <algorithm>
#include <fstream>

#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>

using namespace std;
using namespace peyton;
using namespace peyton::exceptions;
using namespace peyton::net;
using namespace peyton::system;

ServerInstance::ServerInstance(const char *l)
: Instance(), port(0)

{
	if(l != NULL) 	{ host = l; } 
	else {
		char h[200];
		if(gethostname(h, 200) == -1)
			THROW(ERI, "Cannot determine localhost name ! [errno=" + util::str(errno) + "]");
		host = h;
	}

	Log::debug(Log::basic, "Local host: %s", host.c_str());
}

ServerInstance::~ServerInstance()
{
	Log::debug(Log::verbose, "Deleting ServerInstance [in destructor]");
	FOREACH2j(clients_t::iterator, cli, clients) {
		(*cli)->server = NULL;
		delete *cli;
	}
}

RemoteInstance::RemoteInstance()
: Instance(), server(NULL)
{
}

RemoteInstance::~RemoteInstance()
{
	Log::debug(Log::verbose, "Deleting RemoteInstance [in destructor]");
	if(server != NULL) {
		ServerInstance::clients_t::iterator i = find(server->clients.begin(), server->clients.end(), this);
		if(i != server->clients.end()) {
			server->clients.erase(i);
		}
	}
}

int ServerInstance::waitFor(std::vector<RemoteInstance *> &signaled, const
	std::list<RemoteInstance *> &clients, int timeout)
{
	fd_set set;
	FD_ZERO(&set);

	timeval t, *tp = NULL;
	if(timeout != -1) {
		t.tv_sec = timeout;
		t.tv_usec = 0;
		tp = &t;
	}

	int fdmax = -1;
	for(clients_t::const_iterator i = clients.begin(); i != clients.end(); i++) {
		if(!(*i)->isConnected()) continue;

		int fd = (*i)->link.fd();
		FD_SET(fd, &set);
		
		fdmax = std::max(fd, fdmax);
	}

	signaled.clear();

	switch(select(fdmax+1, &set, NULL, NULL, tp)) {
	case 0: return 0;
	case -1: THROW(ESocket, "Error in ::select while waiting for clients to communicate");
	}

	for(clients_t::const_iterator i = clients.begin(); i != clients.end(); i++) {
		int fd = (*i)->link.fd();
		if(!FD_ISSET(fd, &set)) continue;

		signaled.push_back(*i);
	}

	return signaled.size();
}

void ServerInstance::startListening()
{
	if(port != 0) return;

	for(port = 5233; port != 5233+20; port++) {
		try {
			link.bind(port);
			link.listen();
			return;
		} catch(ESocket &e) {
			e.print();
		}
	}
	THROW(ERI, "Cannot listen on any of the ports I tried");
}

RemoteInstance *ServerInstance::spawn(const char *c, const char *h)
{
	startListening();
	RemoteInstance *ri = new RemoteInstance(this, c, h);
	callbackInitializeSpawnedInstance(ri);
	return ri;
}

bool ServerInstance::spawnFlock(string flockName, string exe, vector<pair<string, bool> > *success)
{
	if(success != NULL) { THROW(EAny, "sucess != NULL not implemented yet. Check ServerInstance::clients array instead."); }

	// load flock from flock configuration file
	char *HOME = getenv("HOME");
	if(HOME == NULL) { THROW(EAny, "Cannot load RemoteInstance flock file, because you do not have your $HOME environment variable set. Fix it and then call back."); }

	string file = string(HOME) + "/.remoteinstance-" + flockName;
	std::ifstream f(file.c_str());
	if(!f.good()) { THROW(EAny, "Cannot open RemoteInstance flock file [" + file + "] for reading, and thus cannot load flock."); }

	// start up the client on requested hosts
	string line;
	int n = 0;
	std::getline(f, line);
	while(!f.eof()) {
		istringstream ss(line);
		string host; int ninstances = 0;
		ss >> host >> ninstances;

		try {
			FOR(0, ninstances) {
				n++;
				DEBUG(verbose) << "Starting up flock member: " << host << " [" << i << " currently running]";
				spawn(exe.c_str(), host.c_str());
			}
		} catch(EAny &e) {
			// TODO: there should be a parameter in flock file to set if we want to ignore
			// exceptions here, or not
			e.print();
		}

		std::getline(f, line);
	}

	if(n == 0) { THROW(EAny, "Flock file does not contain a single host. WTF ?!"); }

	return clients.size() != 0;
}

RemoteInstance::RemoteInstance(ServerInstance *server, const char *c, const char *h)
{
	if(h != NULL) { host = h; } else { host = "localhost"; }
	if(c != NULL) { command = c; }

	// execute a SSH session to remote host
	char ssh[1000];
	sprintf(ssh, "/usr/bin/ssh %s %s --remote-server %s:%d > /dev/null 2>&1", host.c_str(), command.c_str(), server->host.c_str(), server->port);
//	sprintf(ssh, "/usr/bin/ssh %s %s --remote-server %s:%d"                 , host.c_str(), command.c_str(), server->host.c_str(), server->port);
	Log::debug(Log::verbose, "%s", ssh);

	int pid = fork();
	switch(pid) {
	case 0: {
#if 1
		if(execl("/bin/sh", "/bin/sh", "-c", ssh, NULL) == -1) {
			THROW(EAny, "Cannot execute remote command startup [errno = " + util::str(errno) + " \"" + strerror(errno) + "\"]");
		}
#endif
#if 0
		FILE *f = popen(ssh, "r");
		if(f == NULL) THROW(ESocket, "Error while opening pipe for remote command startup");

		char buf[101]; int r;
		while(r = fread(buf, 1, 100, f)) {
			buf[r] = 0;
			Log::debug(Log::verbose, "%s", buf);
		}
		if(errno = ferror(f)) THROW(ESocket, "Error while reading remote pipe");
		Log::debug(Log::basic, "Client exiting");
		pclose(f);
#endif
		exit(0);
		}
	case -1:
		THROW(ERI, "Error forking child for SSH remote execution [errno=" + util::str(errno) + "]\n");
	}

	Log::debug(Log::verbose, "Forked a remote exec. process, pid=%d", pid);
	server->link.accept(link, 10);
	server->clients.push_back(this);
	this->server = server;
	Log::debug(Log::verbose, "Remote process established connection to master");
}

int RemoteInstance::write(const void *data, int size)
{
	return link.write(data, size);
}

int RemoteInstance::read(void *data, int size)
{
	return link.read(data, size);
}

bool RemoteInstance::tryRead(void *data, int size)
{
	return link.tryRead(data, size);
}

bool RemoteInstance::isConnected()
{
	return link.isConnected();
}

int Instance::isClient(char *svr, int argc, char **argv)
{
	static struct option options[] = {
		{"remote-server", 1, 0, 'r'},
		{0, 0, 0, 0}
	};

	int opt;
	const char *server = NULL;
	while((opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch(opt) {
		case 'r':
			server = optarg;
			break;
		}
	}

	if(server != NULL) {
		strcpy(svr, server);
		return true;
	} else {
		return false;
	}
}

void RemoteInstance::bindToServer(const char *fqdn)
{
	char master[1000] = {0};
	int port;

	if(sscanf(fqdn, "%[^:]:%d", master, &port) != 2)
		THROW(ERI, "Badly formatted --remote-server string - shoud be host.fqdn.edu:port");

	Log::debug(Log::verbose, "Contacting master at %s:%d", master, port);

	link.connect(master, port);
}
