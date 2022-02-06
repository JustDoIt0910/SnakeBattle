#ifndef SNAKECLIENT_H
#define SNAKECLIENT_H

#include <QWidget>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include "loginscence.h"
#include "playscence.h"
#include "protocol.h"

class SnakeClient : public QWidget
{
    Q_OBJECT
public:
    explicit SnakeClient(QString serverAddr, quint16 listenPort, quint16 dataPort, QWidget *parent = nullptr);
    QString getUserID();
    QHostAddress getHostIPAddress();
    QVector<ObjectPkt>& getObjects();
    ~SnakeClient();
signals:

public slots:
    void onLogin(QString nickname);
    void onSendDir(QPoint v);
    void onUpdate();
    void onConnected();
    void onChangeSpeed(bool flag);

private:
    QHostAddress* serverAddr;
    quint16 serverListenPort;
    quint16 serverDataPort;

    QTcpSocket* clientSocket;
    QUdpSocket* dataSocket;
    quint16 dataPort;

    LoginScence* loginScence;
    MainWindow* playScence;

    QTimer* heart;
    HeartBeatPkt* heartbeatPkt;
    QString userID;
    QString nickname;

    QVector<ObjectPkt> objects;
};

bool isVirtualOrLoopback(QString cardName);

#endif // SNAKECLIENT_H
