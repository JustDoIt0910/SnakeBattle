#include "server.h"
#include "../error.h"
#include "../user/user.h"
#include "../objects/snake.h"
#include <string.h>


Listener::Listener(int port, int timeout, Server* s): server(s)
{
    listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
    listenPort = port;
    server_addr.sin_port = htons(port);

    if(evutil_make_socket_nonblocking(listen_fd) < 0)
        error_handling("evutil_make_socket_nonblocking() error");

    if(::bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        error_handling("bind() error");
    if(listen(listen_fd, 5) == -1)
        error_handling("listen() error");

    base = event_base_new();
    login_event = event_new(base, listen_fd, EV_READ | EV_PERSIST, listener_cb, this);
    event_add(login_event, nullptr);

    timeval tv = {timeout, 0};
    timerEvent = event_new(base, -1, EV_TIMEOUT | EV_PERSIST, timer_cb, this);
    event_add(timerEvent, &tv);
}

void Listener::Listen()
{
    printf("Listening on port %d......\n", listenPort);
    event_base_dispatch(base);
}

void Listener::acceptClient()
{
    int client_fd;
    socklen_t client_adr_sz = sizeof(client_addr);
    client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_adr_sz);
    if(client_fd == -1)
        error_handling("accept() error");
    if(evutil_make_socket_nonblocking(client_fd) < 0)
    {
        close(client_fd);
        error_handling("evutil_make_socket_nonblocking() error");
    }

    heartbeat_event = bufferevent_socket_new(base, client_fd, 0);
    bufferevent_setcb(heartbeat_event, heartbeat_cb, nullptr, nullptr, this);
    bufferevent_enable(heartbeat_event, EV_READ);
    char buf[20];
    printf("new client[%s::%d] login\n", 
    inet_ntop(AF_INET, &client_addr.sin_addr, buf, 20), ntohs(client_addr.sin_port));
}


void listener_cb(evutil_socket_t fd, short what, void* arg)
{
    Listener* l = (Listener*)arg;
    if(what & EV_READ)
    {
        l->acceptClient();
    }
}

void heartbeat_cb(struct bufferevent* bufev, void* arg)
{
    Listener* lis = (Listener*)arg;
    uchar buf[100];
    memset(buf, 0, sizeof(buf));
    size_t n;
    n = bufferevent_read(bufev, buf, sizeof(buf));
    PKT_TYPE type = Packet::getType(buf);
    if(type == PKT_LOGIN) //注册数据包
    {
        LoginPkt loginpkt;
        loginpkt.parsePacket(buf);
        //拿到数据端口和昵称，创建一个user
        lis->Login(loginpkt);
    }
    else if(type == PKT_HEARTBEAT) //心跳包
    {
        HeartBeatPkt heartbeatpkt;
        heartbeatpkt.parsePacket(buf);
        if(heartbeatpkt.getMagicNumber() == 31)
            lis->heartbeat(heartbeatpkt.getUserID());
    }
}

void timer_cb(evutil_socket_t fd, short what, void* arg)
{
    Listener* lis = (Listener*)arg;
    lis->checkAlive();
}

void Listener::checkAlive()
{
    vector<User> users = server->getUsers();
    for(auto it = users.begin(); it != users.end(); it++)
    {
        time_t now = time(NULL);
        if(now - it->getLastHB() >= 2) //2秒无心跳，判断客户端离线
        {
            cout << it->getUserID() << " out" << endl;
            server->removeObj(it->getSnakeID());
        }
    }
}

void Listener::Login(LoginPkt& pkt)
{
    char ip[20];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
    User* user = new User(string(ip), pkt.getDataPort(), pkt.getNickName());
    string uid = user->getUserID();

    vector<int> pos = {200, 200};
     Snake* snake = new Snake(pos, 4, server->getMap(), SNAKE_SIZE_DEFAULT, Object::getRandomColor());
     user->setSnakeID(snake->getID());

    printf("userid: %s, nickname: %s, snakeID: %d\n", user->getUserID().c_str(), 
            user->getNickName().c_str(), user->getSnakeID());

    server->addObj(snake->getID(), snake);
    server->addUser(uid, user); 
}

void Listener::heartbeat(string uid)
{
    server->userHeartBeat(uid);
}

Listener::~Listener()
{
    if(heartbeat_event)
        bufferevent_free(heartbeat_event);
    if(login_event)
        event_free(login_event);
    if(timerEvent)
        event_free(timerEvent);
    if(base)
        event_base_free(base);
    close(listen_fd);
}