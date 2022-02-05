#include "user.h"
#include <sys/socket.h>
#include <arpa/inet.h>

User::User(string _ip, int _dataPort, string _nickname): 
nickname(_nickname)
{
    heartbeat();
    uid = generateID(_ip, _dataPort);
    userAddr.sin_family = AF_INET;
    userAddr.sin_addr.s_addr = inet_addr(_ip.c_str());
    userAddr.sin_port = htons(_dataPort);
}

string User::generateID(string _ip, int _dataPort)
{
    char port[10];
    sprintf(port, "%d", _dataPort);
    string id = _ip + ":" + string(port);
    return id;
}

string User::getUserID()
{
    return uid;
}

string User::getNickName()
{
    return nickname;
}

struct sockaddr_in User::getUserAddr()
{
    return userAddr;
}

time_t User::getLastHB()
{
    return lastHeartBeat;
}

void User::heartbeat()
{
    lastHeartBeat = time(NULL);
}

void User::setSnakeID(int id)
{
    snakeID = id;
}

int User::getSnakeID()
{
    return snakeID;
}