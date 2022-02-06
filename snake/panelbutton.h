#ifndef PANELBUTTON_H
#define PANELBUTTON_H

#include <QWidget>
#include <QPoint>

class PanelButton : public QWidget
{
    Q_OBJECT
public:
    explicit PanelButton(QPoint _center, int r, int pr, QWidget *parent = nullptr);
    void paintEvent(QPaintEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
private:
    QPoint center;
    int radius;
    int parentRadius;
    QPoint i;
    QWidget* parent;

signals:
    void setDirection(QPoint v);
public slots:
};

#endif // PANELBUTTON_H
