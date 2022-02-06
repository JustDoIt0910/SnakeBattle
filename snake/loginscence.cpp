#include "loginscence.h"
#include "protocol.h"
#include <QHostAddress>
#include <QEvent>
#include <QDebug>

LoginScence::LoginScence(QWidget *parent) : QMainWindow(parent)
{
    setFixedSize(400, 300);
    loginBtn = new QPushButton("登录", this);
    nicknameEdit = new QLineEdit("please enter your nickname", this);
    nicknameEdit->move(150, 200);
    loginBtn->move(150, 150);
    connect(loginBtn, &QPushButton::clicked, this, [=](){
        QString nickname = nicknameEdit->text();
        if(nickname == "please enter your nickname")
            return;
        emit login(nickname);
    });
}

bool LoginScence::event(QEvent *event)
{
    if(event->type() == QEvent::Move)
        pos = frameGeometry().topLeft();
    return QMainWindow::event(event);
}

QPoint LoginScence::getPos()
{
    return pos;
}

LoginScence::~LoginScence(){}
