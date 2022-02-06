#include "panel.h"
#include <QPainter>
#include <QDebug>


Panel::Panel(QPoint _center, int r, int pad, QWidget *parent) : QWidget(parent),
    center(_center), radius(r), padding(pad)
{
    int btnRadius = radius - padding;
    QPoint pos(this->center.x() - radius - btnRadius,
               this->center.y() - radius - btnRadius);
    this->setFixedSize(QSize(2 * (this->radius +  btnRadius), 2 * (this->radius +  btnRadius)));
    this->move(pos);
    btn = new PanelButton(QPoint(radius + btnRadius, radius + btnRadius), btnRadius, radius, this);
    connect(btn, &PanelButton::setDirection, this, [=](QPoint v){
        if(dir != v)
        {
            dir = v;
            emit dirChanged(v);
        }
    });
}

void Panel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setBrush(QBrush(QColor(200, 200, 200)));
    painter.setPen(Qt::transparent);
    int btnRadius = radius - padding;
    QRect rect = QRect(btnRadius, btnRadius,
                       this->rect().width() - 2 * btnRadius,
                       this->rect().height() - 2 * btnRadius);
    painter.drawRoundedRect(rect, rect.width() / 2, rect.height() / 2);
    QWidget::paintEvent(event);
}

int Panel::Radius()
{
    return radius;
}
