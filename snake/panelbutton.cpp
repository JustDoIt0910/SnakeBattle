#include "panelbutton.h"
#include "panel.h"
#include <math.h>
#include <algorithm>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
using namespace std;

PanelButton::PanelButton(QPoint _center, int r, int pr, QWidget *parent) : QWidget(parent),
    center(_center), radius(r), parentRadius(pr)
{
    QPoint pos(this->center.x() - radius,
               this->center.y() - radius);
    this->setFixedSize(QSize(2 * this->radius, 2 * this->radius));
    this->move(pos);
    this->parent = parent;
}

void PanelButton::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QBrush(QColor(100, 100, 100)));
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    painter.drawRoundedRect(rect, rect.width() / 2, rect.height() / 2);
    QWidget::paintEvent(event);
}

void PanelButton::mousePressEvent(QMouseEvent *event)
{
    i = event->pos();
}

double length(QPoint v)
{
    return sqrt(pow(v.x(), 2) + pow(v.y(), 2));
}

void PanelButton::mouseMoveEvent(QMouseEvent *event)
{
    QPoint w = mapTo(parent, event->pos());
    QPoint j = w - i;
    center = QPoint(j.x() + radius, j.y() + radius);
    QPoint r = center - QPoint(parentRadius + radius, parentRadius + radius);
    QPoint n((double)r.x() * 3 / length(r), (double)r.y() * 3 / length(r));
    emit setDirection(n);

    if(length(r) > parentRadius)
    {
        QPoint p;
        if(r.x() != 0)
        {
            double k = (double)r.y() / (double)r.x();
            double x = sqrt(pow(parentRadius, 2) / (pow(k, 2) + 1));
            if(r.x() < 0)
                x = -x;
            double y = k * x;
            p = QPoint(x, y);
        }
        else
        {
            int ry = r.y();
            int y = (ry < 0) ? (max(ry, -parentRadius)) : (min(ry, parentRadius));
            p = QPoint(0, y);
        }
        p = p + QPoint(parentRadius, parentRadius);
        j = p;
    }

    this->move(j);
}

void PanelButton::mouseReleaseEvent(QMouseEvent*)
{
    this->move(parentRadius, parentRadius);
}
