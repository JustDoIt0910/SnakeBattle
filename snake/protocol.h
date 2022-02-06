#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <QByteArray>
#include <QString>
#include <QVector>
#include <QPoint>

enum PKT_TYPE
{
    PKT_LOGIN = 0,
    PKT_HEARTBEAT,
    PKT_SPEED,
    PKT_CONTROL,

    PKT_DIE,
    PKT_UPDATE,
    PKT_OBJECT
};

enum OBJ_TYPE
{
    SNAKE = 0,
    FOOD
};

enum OBJ_COLOR
{
    RED = 0,
    YELLOW,
    GREEN,
    BLUE,
    BLACK,
    GRAY
};

struct Head
{
    ushort content_length;
    uchar type;
    Head(): content_length(0), type(PKT_HEARTBEAT){}
    Head(uchar t, ushort cl): content_length(cl), type(t){}
};


class Packet
{
public:
    Packet();
    Packet(PKT_TYPE _type);
    virtual ~Packet(){}
    virtual QByteArray buildPacket() = 0;
    virtual void parsePacket(QByteArray pkt) = 0;
    static PKT_TYPE getType(QByteArray pkt);
    static ushort getLength(QByteArray pkt);

protected:
    Head head;
};

class LoginPkt : public Packet
{
public:
    LoginPkt(QString _nickname, quint16 _data_port);
    ~LoginPkt(){}
    QByteArray buildPacket();
    void parsePacket(QByteArray pkt){}

private:
    QString nickname;
    quint16 data_port;
};

class HeartBeatPkt : public Packet
{
public:
    HeartBeatPkt();
    HeartBeatPkt(char mn, QString _uid);
    ~HeartBeatPkt(){}
    void setMagic(char mn);
    void setUID(QString uid);
    QByteArray buildPacket();
    void parsePacket(QByteArray pkt){}

private:
    char magicNumber;
    QString uid;
};

class ControlPkt : public Packet
{
public:
    ControlPkt();
    ControlPkt(QPoint v);
    QByteArray buildPacket();
    void parsePacket(QByteArray pkt){}
private:
    QPoint dirVec;
};

class ObjectPkt : public Packet
{
public:
    ObjectPkt();
    QByteArray buildPacket(){ return QByteArray(); }
    void parsePacket(QByteArray pkt);
    int length();
    QVector<QPoint>& getPositions();
    OBJ_TYPE getObjType();
    int getSize();
    OBJ_COLOR getColor();
private:
    OBJ_TYPE object_type;
    uchar object_size;
    OBJ_COLOR object_color;
    QVector<QPoint> positions;
};

class UpdatePkt : public Packet
{
public:
    UpdatePkt();
    ~UpdatePkt();
    QByteArray buildPacket(){ return QByteArray(); }
    void parsePacket(QByteArray pkt);
    QVector<ObjectPkt>& getObjs();
private:
    QVector<ObjectPkt> objs;
};

class SpeedPkt : public Packet
{
public:
    SpeedPkt(bool _flag);
    ~SpeedPkt(){}
    QByteArray buildPacket();
    void parsePacket(QByteArray pkt){}
private:
    bool flag;
};


QByteArray from16Bit(ushort u);
short get16Bit(uchar high, uchar low);


#endif // PROTOCOL_H
