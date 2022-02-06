#ifndef PANEL_H
#define PANEL_H

#include <QWidget>
#include "panelbutton.h"

class Panel : public QWidget
{
    Q_OBJECT
public:
    explicit Panel(QPoint _center, int r, int pad, QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
    int Radius();

signals:
    void dirChanged(QPoint v);
private:
    QPoint dir;
    PanelButton* btn;
    QPoint center;
    int radius;
    int padding;
public slots:
};

#endif // PANEL_H
