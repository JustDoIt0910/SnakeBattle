#include "server.h"
#include "../error.h"
#include "../protocol/protocol.h"
#include "../objects/food.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>

vector<User> us;
int foodNum = 0;

Server::Server(int _listenPort, int _dataPort):
    listenPort(_listenPort), dataPort(_dataPort), Receiver(-1)
{
    Map = vector<vector<ObjInfo>>(MAP_SIZE, vector<ObjInfo>(MAP_SIZE));
    foodNum = FOOD_BASE_NUM;
    getRandomPositions(FOOD_BASE_NUM);
    for(auto pos: foodPoss)
    {
        Food* food = new Food(pos, Map, FOOD_SIZE_DEFAULT, Object::getRandomColor());
        addObj(food->getID(), food);
    } 

    listener = new Listener(listenPort, 5, this);
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

    timeval tv = {0, 20 * 1000};
    snakeMoveEvent = event_new(base, -1, EV_TIMEOUT | EV_PERSIST, snake_move_cb, this);
    event_add(snakeMoveEvent, &tv); 

    tv = {10, 0};
    addFoodEvent = event_new(base, -1, EV_TIMEOUT | EV_PERSIST, add_food_cb, this);
    event_add(addFoodEvent, &tv); 
}

vector<vector<ObjInfo>>& Server::getMap()
{
    return Map;
}

void Server::Serve()
{
    gameTh = new thread(game, this);
    broadcastTh = new thread(broadcast, this);
    generateFoodTh = new thread(generateFoods, this);
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

void Server::moveAllSnakes()
{
    bool bflag = false;
    lock_guard<mutex> l(objLock);
    vector<int> remove;
    vector<Food*> deadBody;
    for(auto it = objects.begin(); it != objects.end(); it++)
    {
        if(it->second->Type() == SNAKE)
        {
            
            Snake* snake = (Snake*)(it->second);
            if(!snake->readyToMove())
                continue;
            vector<int>& collisions = snake->move();
            for(int colli: collisions)
            {
                if(colli == -1 || objects[colli]->Type() == SNAKE)
                {
                    cout << "snake " << snake->getID() << " died" << endl;
                    vector<vector<short>> poss = snake->getPositions();
                    for(auto pos: poss)
                    {
                        Food* food = new Food({pos[0], pos[1]}, Map, 6, snake->Color());
                        deadBody.push_back(food);
                    }
                    delete it->second;
                    it->second = nullptr;
                    objects.erase(it++);
                    if(it == objects.end())
                        bflag = true;
                }
                else
                    remove.push_back(colli);
            }
        }
        if(bflag)
            break;
    }
    for(int food: remove)
    {
        delete objects[food];
        objects.erase(food);
        foodNum--;
    }
    for(Food* food: deadBody)
    {
        objects[food->getID()] = food;
        food = nullptr;
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
        else
        {
            Food* food = new Food((Food*)(it->second));
            objs.push_back(food);
        }
    }
    return objs;
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

void Server::speedUp(int sid, bool flag)
{
    lock_guard<mutex> l(objLock);
    if(objects.count(sid) && objects[sid]->Type() == SNAKE)
    {
        Snake* snake = (Snake*)objects[sid];
        snake->speedUp(flag);
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
        usleep(1000 * 50);
    }   
}

void generateFoods(Server* s)
{
    while(true)
    {
        sleep(2);
        s->getRandomPositions(MAP_SIZE * MAP_SIZE);
    }
}

void data_receive_cb(evutil_socket_t fd, short what, void* arg)
{
    Server* s = (Server*)arg;
    struct sockaddr_in client_addr;
    socklen_t clnt_adr_sz = sizeof(client_addr);
    uchar buf[20];
    recvfrom(s->Receiver, buf, 20, 0, (struct sockaddr*)&client_addr, &clnt_adr_sz);
    char cip[30];
    inet_ntop(AF_INET, &client_addr.sin_addr, cip, sizeof(cip));
    string uid = User::generateID(string(cip), htons(client_addr.sin_port));
    User user = s->getUser(uid);

    if(Packet::getType(buf) == PKT_CONTROL) //控制(方向)数据包
    {
        ControlPkt controlPkt;
        controlPkt.parsePacket(buf);
        vector<char> direction = controlPkt.getDir();
        s->setSnakeDir(user.getSnakeID(), direction);
    }
    else if(Packet::getType(buf) == PKT_SPEED) //加(减)速数据包
    {
        SpeedPkt speedPkt;
        speedPkt.parsePacket(buf);
        s->speedUp(user.getSnakeID(), speedPkt.getFlag());
    }
}

void snake_move_cb(evutil_socket_t fd, short what, void* arg)
{
    Server* s = (Server*)arg;
    s->moveAllSnakes();
}

void add_food_cb(evutil_socket_t fd, short what, void* arg)
{
    Server* s = (Server*)arg;
    int delta = FOOD_BASE_NUM - foodNum;
    if(delta <= 1)
        return;
    int add = rand() % (delta - 1) + 1;
    lock_guard<mutex> l(s->foodLock);
    for(int i = 0; i < add; i++)
    {
        Food* food = new Food(s->foodPoss[i], s->Map, FOOD_SIZE_DEFAULT, Object::getRandomColor());
        s->addObj(food->getID(), food);
    }
    printf("%d foods added\n", add);
    foodNum += add;
    printf("%d foods\n", foodNum);
}

void Server::getRandomPositions(int n)
{
    srand(time(NULL));
    int nums[MAP_SIZE * MAP_SIZE];
    memset(nums, 0, sizeof(nums));
    for(int i = 0; i < MAP_SIZE * MAP_SIZE; i++)
    {
        int index = rand() % (MAP_SIZE * MAP_SIZE);
        while(nums[index] != 0)
            index = rand() % (MAP_SIZE * MAP_SIZE);
        nums[index] = i;
    }
    lock_guard<mutex> l(foodLock);
    foodPoss.clear();
    for(int i = 0; i < n; i++)
    {
        int x = nums[i] % MAP_SIZE;
        int y = nums[i] / MAP_SIZE;
        foodPoss.push_back({x, y});
    }
}

Server::~Server()
{
    if(listener)
        delete listener;
    if(dataInEvent)
        event_free(dataInEvent);
    if(snakeMoveEvent)
        event_free(snakeMoveEvent);
    if(addFoodEvent)
        event_free(addFoodEvent);
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
    if(generateFoodTh)
        delete generateFoodTh;
}