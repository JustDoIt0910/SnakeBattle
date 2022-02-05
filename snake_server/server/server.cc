#include "server.h"
#include "../error.h"
#include "../protocol/protocol.h"
#include <iostream>
#include <string.h>

vector<User> us;

Server::Server(int _listenPort, int _dataPort):
    listenPort(_listenPort), dataPort(_dataPort), Receiver(-1)
{
    Map = vector<vector<ObjInfo>>(1000, vector<ObjInfo>(1000));

    listener = new Listener(listenPort, 2, this);
    Receiver = socket(PF_INET, SOCK_DGRAM, 0);
    Sender = socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(dataPort);

    if(evutil_make_socket_nonblocking(Receiver) < 0)
        error_handling("evutil_make_socket_nonblocking() error");

    if(::bind(Receiver, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        error_handling("bind() error");
    
    base = event_base_new();
    dataInEvent = event_new(base, Receiver, EV_READ | EV_PERSIST, data_receive_cb, this);
    event_add(dataInEvent, nullptr);

    timeval tv = {0, 100 * 1000};
    snakeMoveEvent = event_new(base, -1, EV_TIMEOUT | EV_PERSIST, snake_move_cb, this);
    event_add(snakeMoveEvent, &tv); 
}

vector<vector<ObjInfo>>& Server::getMap()
{
    return Map;
}

void Server::Serve()
{
    gameTh = new thread(game, this);
    broadcastTh = new thread(broadcast, this);
    listener->Listen();
}

void Server::addUser(string uid, User* user)
{
    lock_guard<mutex> l(userLock);
    if(!users.count(uid))
        users[uid] = user;
}

User Server::getUser(string uid)
{
    lock_guard<mutex> l(userLock);
    if(users.count(uid))
        return *users[uid];
}

void Server::userHeartBeat(string uid)
{
    lock_guard<mutex> l(userLock);
    if(users.count(uid))
        users[uid]->heartbeat();
}

vector<User>& Server::getUsers()
{
    us.clear();
    lock_guard<mutex> l(userLock);
    for(auto it = users.begin(); it != users.end(); it++)
        us.push_back(*it->second);
    return us;
}

void Server::addObj(int objID, Object* obj)
{
    lock_guard<mutex> l(objLock);
    objects[objID] = obj;
}

void Server::removeObj(int objID)
{
    lock_guard<mutex> l(objLock);
    if(objects.count(objID))
    {
        delete objects[objID];
        objects.erase(objID);
    }   
}

void Server::moveObj(int objID)
{
    lock_guard<mutex> l(objLock);
    if(objects.count(objID) && objects[objID]->Type() == SNAKE)
    {
        Snake* snake = (Snake*)objects[objID];
        int colli = snake->move();
        if(colli)
            cout << "snake " << colli << " died" << endl;
    }
}

void Server::moveAllObjs()
{
    lock_guard<mutex> l(objLock);
    for(auto it = objects.begin(); it != objects.end(); it++)
    {
        if(it->second->Type() == SNAKE)
        {
            Snake* snake = (Snake*)(it->second);
            snake->move();
        }
    }
}

vector<Object*> Server::getAllObjs()
{
    vector<Object*> objs;
    lock_guard<mutex> l(objLock);
    for(auto it = objects.begin(); it != objects.end(); it++)
    {
        if(it->second->Type() == SNAKE)
        {
            Snake* snake = new Snake((Snake*)(it->second));
            objs.push_back(snake);
        }
    }
    return objs;
}

Object* Server::getObj(int objID)
{
    lock_guard<mutex> l(objLock);
    if(objects.count(objID))
    {
        Object* obj = objects[objID];
        if(obj->Type() == SNAKE)
        {
            Snake* s = new Snake((Snake*)obj);
            return s;
        }
        else
        {
            //TODO
            //return food
        }
    }
}

void Server::setSnakeDir(int sid, vector<char> dirVec)
{
    lock_guard<mutex> l(objLock);
    if(objects.count(sid) && objects[sid]->Type() == SNAKE)
    {
        Snake* snake = (Snake*)objects[sid];
        snake->setDir(dirVec);
    }
}

void game(Server* s)
{
    event_base_dispatch(s->base);
} 

void broadcast(Server* s)
{
    while(true)
    {
        vector<Object*> objs = s->getAllObjs();
        UpdatePkt update;
        for(Object* obj: objs)
            update.push(ObjectPkt(obj));

        uchar* pkt = update.buildPacket();
        vector<User>& users = s->getUsers();
        for(User& u: users)
        {
            struct sockaddr_in addr = u.getUserAddr();
            sendto(s->Sender, pkt, update.length() + 3, 0, (struct sockaddr*)&addr, sizeof(addr)); 
        }
        delete pkt;
        for(Object* obj: objs)
        {
            delete obj;
            obj = nullptr;
        }
        usleep(1000 * 100);
    }   
}

void data_receive_cb(evutil_socket_t fd, short what, void* arg)
{
    Server* s = (Server*)arg;
    struct sockaddr_in client_addr;
    socklen_t clnt_adr_sz = sizeof(client_addr);
    uchar buf[20];
    recvfrom(s->Receiver, buf, 20, 0, (struct sockaddr*)&client_addr, &clnt_adr_sz);
    if(Packet::getType(buf) == PKT_CONTROL) //控制(方向)数据包
    {
        ControlPkt controlPkt;
        controlPkt.parsePacket(buf);
        vector<char> direction = controlPkt.getDir();
        char cip[30];
        inet_ntop(AF_INET, &client_addr.sin_addr, cip, sizeof(cip));

        string uid = User::generateID(string(cip), htons(client_addr.sin_port));
        User user = s->getUser(uid);
        s->setSnakeDir(user.getSnakeID(), direction);
        printf("(%d, %d)\n", direction[0], direction[1]);
    }
}

void snake_move_cb(evutil_socket_t fd, short what, void* arg)
{
    Server* s = (Server*)arg;
    s->moveAllObjs();
}

Server::~Server()
{
    if(listener)
        delete listener;
    if(dataInEvent)
        event_free(dataInEvent);
    if(snakeMoveEvent)
        event_free(snakeMoveEvent);
    if(base)
        event_base_free(base);
    for(auto it = users.begin(); it != users.end(); it++)
        if(it->second)
            delete it->second;
    for(auto it = objects.begin(); it != objects.end(); it++)
        if(it->second)
            delete it->second;
    if(gameTh)
        delete gameTh;
    if(broadcastTh)
        delete broadcastTh;
}