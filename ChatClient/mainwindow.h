#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

class QTextEdit;
class QListWidget;
class QLineEdit;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void connectServer();
    void login();
    void sendMessage();
    void readyRead();
    void disconnected();

private:
    QTcpSocket *socket;
    QString username;

    QTextEdit *chatTextEdit;
    QListWidget *userListWidget;
    QLineEdit *messageEdit;
    QLineEdit *usernameEdit;
    QPushButton *sendBtn;
    QPushButton *loginBtn;
    QPushButton *connectBtn;
};

#endif // MAINWINDOW_H
