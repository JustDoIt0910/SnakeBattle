#include "protocol.h"
#include <arpa/inet.h>
#include "string.h"

/**************************Packet********************************/

Packet::Packet(PKT_TYPE _type)
{
    head = Head(_type, 0);
}

PKT_TYPE Packet::getType(uchar* pkt)
{
    Head* head = (Head*)pkt;
    return PKT_TYPE(head->type);
}

int Packet::getLength(uchar* pkt)
{
    Head* head = (Head*)pkt;
    head->content_length = ntohs(head->content_length);
    return head->content_length;
}

PKT_TYPE Packet::type()
{
    return (PKT_TYPE)head.type;
}

int Packet::length()
{
    return head.content_length;
}

/**************************HeartBeatPkt********************************/

HeartBeatPkt::HeartBeatPkt() : Packet(PKT_HEARTBEAT){}

uchar HeartBeatPkt::getMagicNumber()
{
    return magicNumber;
}

void HeartBeatPkt::parsePacket(uchar* pkt)
{
    pkt += 3;
    magicNumber = *pkt;
    uid = string((char*)(pkt + 1));
}

string HeartBeatPkt::getUserID()
{
    return uid;
}

/**************************LoginPkt********************************/

LoginPkt::LoginPkt() : Packet(PKT_LOGIN){}

string LoginPkt::getNickName()
{
    return nickname;
}

ushort LoginPkt::getDataPort()
{
    return data_port;
}

void LoginPkt::parsePacket(uchar* pkt)
{
    pkt += 3;
    uchar high = *pkt;
    uchar low = *(pkt + 1);
    data_port = high;
    data_port = ((data_port << 8) | low);
    nickname = string((char*)(pkt + 2));
}

/**************************ControlPkt********************************/

ControlPkt::ControlPkt(): Packet(PKT_CONTROL)
{
    dirVec.resize(2);
    dirVec[0] = dirVec[1] = 0;
}

vector<char>& ControlPkt::getDir()
{
    return dirVec;
}

void ControlPkt::parsePacket(uchar* pkt)
{
    pkt += 3;
    dirVec[0] = (char)(*pkt);
    dirVec[1] = (char)(*(pkt + 1));
}

/**************************ObjectPkt********************************/

ObjectPkt::ObjectPkt(OBJ_TYPE type, int size, vector<vector<short>> pos): Packet(PKT_OBJECT),
object_type(type), obj_size(size), positions(pos)
{
    head.content_length = 2 + 4 * positions.size();
}

ObjectPkt::ObjectPkt(Object* obj): Packet(PKT_OBJECT)
{
    object_type = obj->Type();
    obj_size = obj->Size();
    positions = obj->getPositions();
    head.content_length = 2 + 4 * positions.size();
}

OBJ_TYPE ObjectPkt::getObjType()
{
    return object_type;
}

vector<vector<short>>& ObjectPkt::getPositions()
{
    return positions;
}

uchar ObjectPkt::getSize()
{
    return obj_size;
}

/**************************UpdatePkt********************************/

UpdatePkt::UpdatePkt(): Packet(PKT_UPDATE){}

void UpdatePkt::push(ObjectPkt obj)
{
    objs.push_back(obj);
    head.content_length += obj.length() + 3;
}

uchar* UpdatePkt::buildPacket()
{
    uchar* buf = new uchar[3 + head.content_length];
    buf[0] = getHigh(head.content_length);
    buf[1] = getLow(head.content_length);
    buf[2] = head.type;
    int n = 3;
    for(ObjectPkt obj : objs)
    {
        buf[n++] = getHigh(obj.length());
        buf[n++] = getLow(obj.length());
        buf[n++] = obj.type();
        buf[n++] = obj.getObjType();
        buf[n++] = obj.getSize();
        for(vector<short>& pos : obj.getPositions())
        {
            buf[n++] = getHigh(pos[0]);
            buf[n++] = getLow(pos[0]);
            buf[n++] = getHigh(pos[1]);
            buf[n++] = getLow(pos[1]);
        }
    }
    return buf;
}

uchar getHigh(ushort i)
{
    uchar high = ((i >> 8) & 0xFF);
    return high;
}

uchar getLow(ushort i)
{
    uchar low = (i & 0xFF);
    return low;
}