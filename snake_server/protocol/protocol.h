#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
#include <string>
#include <vector>
#include "../objects/object.h"
using namespace std;

typedef unsigned short ushort;
typedef unsigned char uchar;

enum PKT_TYPE
{
    PKT_LOGIN = 0,
    PKT_HEARTBEAT,
    PKT_SPEEDUP,
    PKT_SLOWDOWN,
    PKT_CONTROL,

    PKT_DIE,
    PKT_UPDATE,
    PKT_OBJECT
};

struct Head
{
    ushort content_length;
    uchar type;
    Head() : type(PKT_HEARTBEAT), content_length(0){}
    Head(uchar t, ushort cl) : type(t), content_length(cl){}
};

class Packet
{
public:
    Packet(){}
    Packet(PKT_TYPE _type);
    static PKT_TYPE getType(uchar* pkt);
    static int getLength(uchar* pkt);
    virtual ~Packet(){}
    PKT_TYPE type();
    int length();
    virtual uchar* buildPacket() = 0;
    virtual void parsePacket(uchar* pkt) = 0;

protected:
    Head head;
};

class LoginPkt : public Packet
{
public:
    LoginPkt();
    ~LoginPkt(){}
    string getNickName();
    ushort getDataPort();
    uchar* buildPacket(){};
    void parsePacket(uchar* pkt);

private:
    string nickname;
    ushort data_port;
};

class HeartBeatPkt : public Packet
{
public:
    HeartBeatPkt();
    uchar getMagicNumber();
    string getUserID();
    ~HeartBeatPkt(){}
    uchar* buildPacket(){};
    void parsePacket(uchar* pkt);

private:
    uchar magicNumber;
    string uid;
};

class ControlPkt : public Packet
{
public:
    ControlPkt();
    ~ControlPkt(){}
    vector<char>& getDir();
    uchar* buildPacket(){};
    void parsePacket(uchar* pkt);

private:
    vector<char> dirVec;
};

class ObjectPkt : public Packet
{
public:
    ObjectPkt(OBJ_TYPE type, int size, vector<vector<short>> pos);
    ObjectPkt(Object* obj);
    ~ObjectPkt(){}
    uchar* buildPacket(){}
    void parsePacket(uchar* pkt){}
    OBJ_TYPE getObjType();
    uchar getSize();
    vector<vector<short>>& getPositions();

private:
    OBJ_TYPE object_type;
    uchar obj_size;
    vector<vector<short>> positions;
};

class UpdatePkt : public Packet
{
public:
    UpdatePkt();
    ~UpdatePkt(){}
    void push(ObjectPkt obj);
    uchar* buildPacket();
    void parsePacket(uchar* pkt){}

private:
    vector<ObjectPkt> objs;
};

uchar getHigh(ushort i);
uchar getLow(ushort i);

#endif