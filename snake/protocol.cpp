#include "protocol.h"
#include <QtEndian>
#include <QDebug>

/****************Packet*****************/

Packet::Packet(PKT_TYPE _type)
{
    head = Head(_type, 0);
}

PKT_TYPE Packet::getType(QByteArray pkt)
{
    Head* h = (Head*)pkt.data();
    return PKT_TYPE(h->type);
}

ushort Packet::getLength(QByteArray pkt)
{
    Head* h = (Head*)pkt.data();
    h->content_length = qToBigEndian(h->content_length);
    return h->content_length;
}

/****************HeartBeatPkt*****************/

HeartBeatPkt::HeartBeatPkt() : Packet(PKT_HEARTBEAT){}


HeartBeatPkt::HeartBeatPkt(char mn, QString _uid):Packet(PKT_HEARTBEAT),
    magicNumber(mn), uid(_uid)
{
    int uid_size = uid.size();
    head.content_length = uid_size + 1;
}

void HeartBeatPkt::setMagic(char mn)
{
    magicNumber = mn;
}

void HeartBeatPkt::setUID(QString uid)
{
    this->uid = uid;
}

QByteArray HeartBeatPkt::buildPacket()
{
    QByteArray pkt;
    pkt.append(from16Bit(head.content_length));
    pkt.append(static_cast<char>(head.type));
    pkt.append(magicNumber);
    pkt.append(uid.toUtf8());
    return pkt;
}

/****************LoginPkt*****************/

LoginPkt::LoginPkt(QString _nickname, quint16 _data_port): Packet(PKT_LOGIN),
    nickname(_nickname), data_port(_data_port)
{
    int name_size = nickname.size();
    head.content_length = name_size + 2;
}

QByteArray LoginPkt::buildPacket()
{
    QByteArray pkt;
    pkt.append(from16Bit(head.content_length));
    pkt.append(static_cast<char>(head.type));
    char high = static_cast<char>(((data_port >> 8) & 0xFF));
    char low = static_cast<char>(data_port & 0xFF);
    pkt.append(high);
    pkt.append(low);
    pkt.append(nickname.toUtf8());

    return pkt;
}

/****************ControlPkt*****************/

ControlPkt::ControlPkt(): Packet(PKT_CONTROL){}

ControlPkt::ControlPkt(QPoint v): Packet(PKT_CONTROL), dirVec(v)
{
    head.content_length = 2;
}

QByteArray ControlPkt::buildPacket()
{
    QByteArray pkt;
    pkt.append(from16Bit(head.content_length));
    pkt.append(static_cast<char>(head.type));
    pkt.append(static_cast<char>(dirVec.x()));
    pkt.append(static_cast<char>(dirVec.y()));
    return pkt;
}

/****************ObjectPkt*****************/

ObjectPkt::ObjectPkt(): Packet(PKT_OBJECT){}

void ObjectPkt::parsePacket(QByteArray pkt)
{
    head.content_length = Packet::getLength(pkt);
    char objType = pkt[3];
    object_type = (OBJ_TYPE)objType;
    object_size = (uchar)pkt[4];
    char objColor = pkt[5];
    object_color = (OBJ_COLOR)objColor;
    int num = (head.content_length - 3) / 4;
    for(int n = 0; n < num; n++)
    {
       int x = get16Bit(pkt[4 * n + 6], pkt[4 * n + 7]);
       int y = get16Bit(pkt[4 * n + 8], pkt[4 * n + 9]);
       positions.push_back(QPoint(x, y));
    }
}

int ObjectPkt::length()
{
    return head.content_length;
}

QVector<QPoint>& ObjectPkt::getPositions()
{
    return positions;
}

OBJ_TYPE ObjectPkt::getObjType()
{
    return object_type;
}

int ObjectPkt::getSize()
{
    return object_size;
}

OBJ_COLOR ObjectPkt::getColor()
{
    return object_color;
}

/****************UpdatePkt*****************/

UpdatePkt::UpdatePkt(): Packet(PKT_UPDATE){}

UpdatePkt::~UpdatePkt()
{
}

void UpdatePkt::parsePacket(QByteArray pkt)
{
    head.content_length = Packet::getLength(pkt);
    pkt = pkt.right(head.content_length);
    int payload_len = head.content_length;
    while(pkt.size())
    {
        ObjectPkt obj;
        obj.parsePacket(pkt);
        int len = payload_len - (obj.length() + 3);
        pkt = pkt.right(len);
        payload_len = len;
        objs.push_back(obj);
    }
}

QVector<ObjectPkt>& UpdatePkt::getObjs()
{
    return objs;
}

/****************SpeedPkt*****************/

SpeedPkt::SpeedPkt(bool _flag): Packet(PKT_SPEED), flag(_flag)
{
    head.content_length = 1;
}

QByteArray SpeedPkt::buildPacket()
{
    QByteArray pkt;
    pkt.append(from16Bit(head.content_length));
    pkt.append(static_cast<char>(head.type));
    pkt.append(static_cast<char>(flag));
    return pkt;
}

/********************************************/

QByteArray from16Bit(ushort u)
{
    char high = static_cast<char>((u >> 8) & 0xFF);
    char low = static_cast<char>(u & 0xFF);
    QByteArray arr;
    arr.append(high);
    arr.append(low);
    return arr;
}

short get16Bit(uchar high, uchar low)
{
    short res = high;
    res = (res << 8) | low;
    return res;
}
