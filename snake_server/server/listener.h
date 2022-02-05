#ifndef __LISTENER_H__
#define __LISTENER_H__
#include <sys/socket.h>
#include "../protocol/protocol.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>

class Server;

class Listener
{
public:
	Listener(int port, int timeout, Server* s);
	void Listen();
	void acceptClient();
	void Login(LoginPkt& pkt);
	void checkAlive();
	void heartbeat(string uid);
	~Listener();
private:
    Server* server;
	int listen_fd;
	int listenPort;
	struct sockaddr_in client_addr;
	struct event_base* base = nullptr;
    struct event* login_event = nullptr;
	struct bufferevent* heartbeat_event = nullptr;
	struct event* timerEvent = nullptr;
};

void listener_cb(evutil_socket_t fd, short what, void* arg);
void heartbeat_cb(struct bufferevent* bufev, void* arg);
void timer_cb(evutil_socket_t fd, short what, void* arg);

#endif