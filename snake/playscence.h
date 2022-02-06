#ifndef PLAYSCENCE_H
#define PLAYSCENCE_H

#include <QMainWindow>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QUdpSocket>
#include "panel.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void paintEvent(QPaintEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void setPos(QPoint pos);

signals:
    void sendDir(QPoint v);
    void changeSpeed(bool flag);

private:
    Ui::MainWindow *ui;
    Panel* panel;
    QWidget* Parent;
};

#endif // PLAYSCENCE_H
