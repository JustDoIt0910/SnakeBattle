#ifndef __SERVER_H__
#define __SERVER_H__
#include <string>
#include <unordered_map>
#include <queue>
#include <thread>
#include <mutex>
#include "../objects/object.h"
#include "../user/user.h"
#include "listener.h"
#include "../protocol/protocol.h"
#include "../user/user.h"
using namespace std;

struct ObjInfo
{
    int id;
    int size;
};

class Server
{
public:
    Server(int _listenPort, int _dataPort);
    ~Server();  
    void Serve();

    vector<vector<ObjInfo>>& getMap();

    void addUser(string uid, User* user);
    User getUser(string uid);
    void userHeartBeat(string uid);
    vector<User>& getUsers();

    void addObj(int objID, Object* obj);
    void removeObj(int objID);
    void moveObj(int objID);
    void moveAllObjs();
    Object* getObj(int objID);
    vector<Object*> getAllObjs();
    void setSnakeDir(int sid, vector<char> dirVec);
    
    friend void game(Server* s);
    friend void broadcast(Server* s);
    friend void data_receive_cb(evutil_socket_t fd, short what, void* arg);
    friend void snake_move_cb(evutil_socket_t fd, short what, void* arg);

private:  
    int listenPort;
    int dataPort;
    int Receiver;
    int Sender;
    Listener* listener = nullptr;
    struct event_base* base = nullptr;
    struct event* dataInEvent = nullptr;
    struct event* snakeMoveEvent = nullptr;
    unordered_map<string, User*> users;
    unordered_map<int, Object*> objects;
    vector<vector<ObjInfo>> Map;

    thread* gameTh = nullptr;
    thread* broadcastTh = nullptr;
    mutex userLock;
    mutex objLock;
};

void game(Server* s);
void broadcast(Server* s);

void data_receive_cb(evutil_socket_t fd, short what, void* arg);
void snake_move_cb(evutil_socket_t fd, short what, void* arg);

#endif