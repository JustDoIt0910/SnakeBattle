#ifndef LOGINSCENCE_H
#define LOGINSCENCE_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QTimer>


class LoginScence : public QMainWindow
{
    Q_OBJECT
public:
    explicit LoginScence(QWidget *parent = nullptr);
    ~LoginScence();
    bool event(QEvent *event);
    QPoint getPos();
signals:
    void login(QString nickname);
public slots:

private:
    QPushButton* loginBtn;
    QLineEdit* nicknameEdit;
    QPoint pos;
};

#endif // LOGINSCENCE_H
