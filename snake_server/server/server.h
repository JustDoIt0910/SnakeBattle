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
#define MAP_SIZE 500
#define FOOD_BASE_NUM 600

struct ObjInfo
{
    int id;
    int size;
    OBJ_TYPE type;
    ObjInfo(): id(0), size(0), type(NONE){}
    ObjInfo(int _id, int _size, OBJ_TYPE _type): id(_id), size(_size), type(_type){}
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
    void moveAllSnakes();
    void speedUp(int objID, bool flag);
    vector<Object*> getAllObjs();
    void setSnakeDir(int sid, vector<char> dirVec);
    
    friend void game(Server* s);
    friend void broadcast(Server* s);
    friend void generateFoods(Server* s);
    friend void data_receive_cb(evutil_socket_t fd, short what, void* arg);
    friend void snake_move_cb(evutil_socket_t fd, short what, void* arg);
    friend void add_food_cb(evutil_socket_t fd, short what, void* arg);

    void getRandomPositions(int n);

private:  
    int listenPort;
    int dataPort;
    int Receiver;
    int Sender;
    Listener* listener = nullptr;
    struct event_base* base = nullptr;
    struct event* dataInEvent = nullptr;
    struct event* snakeMoveEvent = nullptr;
    struct event* addFoodEvent = nullptr;
    unordered_map<string, User*> users;
    unordered_map<int, Object*> objects;
    vector<vector<int>> foodPoss;
    vector<vector<ObjInfo>> Map;

    thread* gameTh = nullptr;
    thread* broadcastTh = nullptr;
    thread* generateFoodTh = nullptr;
    mutex userLock;
    mutex objLock;
    mutex foodLock;
};

void game(Server* s);
void broadcast(Server* s);
void generateFoods(Server* s);

void data_receive_cb(evutil_socket_t fd, short what, void* arg);
void snake_move_cb(evutil_socket_t fd, short what, void* arg);
void add_food_cb(evutil_socket_t fd, short what, void* arg);

#endif