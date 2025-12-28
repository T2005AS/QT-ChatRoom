#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMap>

// 前向声明（不要include具体控件头文件，放到.cpp里）
class QTextEdit;
class QListWidget;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newConnection();
    void readyRead();
    void disconnected();

private:
    void broadcast(const QString &message, QTcpSocket *sender = nullptr);
    void sendUserList();

    QTcpServer *server;
    QList<QTcpSocket*> clients;
    QMap<QTcpSocket*, QString> userMap;

    // 使用指针，不包含具体头文件
    QTextEdit *logTextEdit;
    QListWidget *userListWidget;
    QLabel *userCountLabel;
};

#endif // MAINWINDOW_H
