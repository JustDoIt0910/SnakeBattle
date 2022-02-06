#include "snakeclient.h"
#include <QNetworkInterface>
#include <QNetworkDatagram>
#include <QList>
#include <QDebug>

char BUF[1024 * 1024];

SnakeClient::SnakeClient(QString _serverAddr, quint16 _serverListenPort, quint16 _serverDataPort, QWidget *parent)
    : QWidget(parent), serverListenPort(_serverListenPort), serverDataPort(_serverDataPort)
{
    serverAddr = new QHostAddress(_serverAddr);
    clientSocket = new QTcpSocket(this);
    dataSocket = new QUdpSocket(this);

    QHostAddress clientAddr = getHostIPAddress();
    dataSocket->bind(clientAddr, 0, QAbstractSocket::DefaultForPlatform);
    dataPort = dataSocket->localPort();
    userID = QString("%1:%2").arg(clientAddr.toString()).arg(dataPort);
    qDebug() << userID;

    heart = new QTimer(this);
    heartbeatPkt = new HeartBeatPkt(31, userID);

    loginScence = new LoginScence(this);
    playScence = new MainWindow(this);
    loginScence->show();

    connect(loginScence, SIGNAL(login(QString)), this, SLOT(onLogin(QString)));
    connect(heart, &QTimer::timeout, this, [=](){
         clientSocket->write(heartbeatPkt->buildPacket());
    });
    connect(clientSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(playScence, SIGNAL(sendDir(QPoint)), this, SLOT(onSendDir(QPoint)));

    connect(dataSocket, SIGNAL(readyRead()), this, SLOT(onUpdate()));
    connect(playScence, SIGNAL(changeSpeed(bool)), this, SLOT(onChangeSpeed(bool)));
}

void SnakeClient::onLogin(QString nickname)
{
    this->nickname = nickname;
    if(clientSocket->state() == QAbstractSocket::UnconnectedState)
        clientSocket->connectToHost(*serverAddr, serverListenPort);
    else
        clientSocket->abort();
}

void SnakeClient::onConnected()
{
    LoginPkt loginPkt(nickname, dataPort);
    QByteArray pkt = loginPkt.buildPacket();
    clientSocket->write(pkt);
    heart->start(1000);
    loginScence->hide();
    playScence->setPos(loginScence->getPos());
    playScence->show();
}

void SnakeClient::onSendDir(QPoint v)
{
    ControlPkt controlPkt(v);
    QByteArray pkt = controlPkt.buildPacket();
    QNetworkDatagram datagram(pkt, *serverAddr, serverDataPort);
    dataSocket->writeDatagram(datagram);
}

void SnakeClient::onUpdate()
{
    memset(BUF, 0, sizeof(BUF));
    int len = dataSocket->readDatagram(BUF, sizeof(BUF));
    QByteArray pkt(BUF, len);
    UpdatePkt update;
    update.parsePacket(pkt);
    objects = update.getObjs();

    playScence->repaint();
}

void SnakeClient::onChangeSpeed(bool flag)
{
    SpeedPkt pkt(flag);
    QNetworkDatagram datagram(pkt.buildPacket(), *serverAddr, serverDataPort);
    dataSocket->writeDatagram(datagram);
}

QString SnakeClient::getUserID()
{
    return userID;
}

QHostAddress SnakeClient::getHostIPAddress()
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface inter, interfaces)
    {
        if(isVirtualOrLoopback(inter.humanReadableName()))
            continue;
        if(inter.flags() & (QNetworkInterface::IsUp | QNetworkInterface::IsRunning))
        {
            foreach(QNetworkAddressEntry entry, inter.addressEntries())
            {
                if(entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                    return entry.ip();
            }
        }
    }
    return QHostAddress::LocalHost;
}

QVector<ObjectPkt>& SnakeClient::getObjects()
{
    return objects;
}

bool isVirtualOrLoopback(QString cardName)
{
    if(cardName.indexOf("Loopback") != -1 ||
       cardName.indexOf("VMware") != -1 ||
       cardName.indexOf("VirtualBox") != -1)
        return true;
    return false;
}

SnakeClient::~SnakeClient()
{
    if(heartbeatPkt)
        delete heartbeatPkt;
    if(serverAddr)
        delete serverAddr;
    clientSocket->disconnectFromHost();
    clientSocket->close();
    dataSocket->close();
}
