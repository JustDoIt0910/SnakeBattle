#ifndef __USER_H__
#define __USER_H__
#include <string>
#include <time.h>
#include <arpa/inet.h>
#include "../objects/snake.h"
using namespace std;


class User
{
public:
	User(string _ip, int _dataPort, string _nickname);
	~User(){}
	static string generateID(string _ip, int _dataPort);
	void heartbeat();
	time_t getLastHB();
	string getNickName();
	string getUserID();
	struct sockaddr_in getUserAddr();
	int getSnakeID();
	void setSnakeID(int id); 

private:
	struct sockaddr_in userAddr;
	string uid;
	time_t lastHeartBeat;
	string nickname;
	int snakeID;
};

#endif