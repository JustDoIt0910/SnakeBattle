#include "playscence.h"
#include "protocol.h"
#include "panel.h"
#include "ui_mainwindow.h"
#include "snakeclient.h"
#include <QPainter>
#include <QMap>

QMap<OBJ_COLOR, Qt::GlobalColor> m;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), Parent(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(500, 500);
    panel = new Panel(QPoint(400, 400), 30, 10, this);


    connect(panel, &Panel::dirChanged, this, [=](QPoint v){
        emit sendDir(v);
    });
    m.insert(RED, Qt::red);
    m.insert(YELLOW, Qt::yellow);
    m.insert(GREEN, Qt::green);
    m.insert(BLUE, Qt::blue);
    m.insert(BLACK, Qt::black);
    m.insert(GRAY, Qt::gray);
}

void MainWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setPen(Qt::transparent);
    SnakeClient* client = (SnakeClient*)Parent;
    for(ObjectPkt obj: client->getObjects())
    {
        int size = obj.getSize();
        int r = size / 2;
        if(obj.getObjType() == SNAKE)
        {
            painter.setBrush(QBrush(m[obj.getColor()]));
            QVector<QPoint>& poss = obj.getPositions();
            for(int i = poss.size() - 1; i >= 0; i--)
                painter.drawEllipse(poss[i].x() - r, poss[i].y() - r, size, size);
            QPoint v = poss[0] - poss[1];
            v.setX(v.x() / 2);
            v.setY(v.y() / 2);
            QPoint v2(v.y(), -v.x());
            QPoint v3(-v.y(), v.x());
            QPoint e1 = poss[0] + v + v2;
            QPoint e2 = poss[0] + v + v3;
            painter.setBrush(QBrush(Qt::black));
            painter.drawEllipse(e1.x() - 3, e1.y() - 3, 6, 6);
            painter.drawEllipse(e2.x() - 3, e2.y() - 3, 6, 6);
            painter.setBrush(QBrush(Qt::white));
            painter.drawEllipse(e1.x() - 1, e1.y() - 1, 2, 2);
            painter.drawEllipse(e2.x() - 1, e2.y() - 1, 2, 2);
        }

        else
        {
            painter.setBrush(QBrush(m[obj.getColor()]));
            QPoint pos = obj.getPositions()[0];
            painter.drawEllipse(pos.x() - r, pos.y() - r, size, size);
        }
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Space)
        emit changeSpeed(true);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Space)
        emit changeSpeed(false);
}

void MainWindow::setPos(QPoint pos)
{
    move(pos);
}

MainWindow::~MainWindow()
{
    delete ui;
}
