#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>      // 在这里包含
#include <QListWidget>    // 在这里包含
#include <QPushButton>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QGroupBox>
#include <QFrame>
#include <QHostAddress>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), server(nullptr), logTextEdit(nullptr), userListWidget(nullptr), userCountLabel(nullptr)
{
    // 创建界面（最简单版本）
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *layout = new QVBoxLayout(central);

    // 1. 启动按钮
    QPushButton *startBtn = new QPushButton("启动服务器 (端口:12345)", this);
    layout->addWidget(startBtn);

    // 2. 日志显示
    logTextEdit = new QTextEdit(this);
    logTextEdit->setReadOnly(true);
    layout->addWidget(new QLabel("系统日志:"));
    layout->addWidget(logTextEdit);

    // 3. 在线用户
    userListWidget = new QListWidget(this);
    userCountLabel = new QLabel("在线: 0人", this);

    layout->addWidget(new QLabel("在线用户:"));
    layout->addWidget(userListWidget);
    layout->addWidget(userCountLabel);

    setWindowTitle("聊天服务器");
    resize(400, 400);

    // 连接信号
    connect(startBtn, &QPushButton::clicked, this, [this]() {
        if (!server) {
            server = new QTcpServer(this);
            if (server->listen(QHostAddress::Any, 12345)) {
                logTextEdit->append("✓ 服务器启动成功");
                logTextEdit->append("监听端口: 12345");
                connect(server, &QTcpServer::newConnection, this, &MainWindow::newConnection);
            } else {
                logTextEdit->append("✗ 启动失败: " + server->errorString());
            }
        }
    });
}

MainWindow::~MainWindow() {}

void MainWindow::newConnection()
{
    QTcpSocket *socket = server->nextPendingConnection();
    clients.append(socket);

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::disconnected);

    logTextEdit->append("新客户端连接");
}

void MainWindow::readyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    QJsonObject json = doc.object();
    QString type = json.value("type").toString();

    if (type == "login") {
        QString username = json.value("username").toString();
        userMap[socket] = username;
        logTextEdit->append(username + " 加入聊天室");
        sendUserList();
        broadcast(username + " 进入了聊天室");
    } else if (type == "message") {
        QString username = userMap.value(socket, "匿名");
        QString content = json.value("content").toString();
        broadcast(username + ": " + content);
    }
}

void MainWindow::disconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QString username = userMap.value(socket, "未知用户");
    logTextEdit->append(username + " 离开");

    clients.removeAll(socket);
    userMap.remove(socket);
    sendUserList();
}

void MainWindow::broadcast(const QString &message, QTcpSocket *sender)
{
    logTextEdit->append(message);

    QJsonObject json;
    json["type"] = "message";
    json["content"] = message;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    for (QTcpSocket *client : clients) {
        if (client != sender && client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
        }
    }
}

void MainWindow::sendUserList()
{
    QJsonArray users;
    for (const QString &user : userMap.values()) {
        users.append(user);
    }

    QJsonObject json;
    json["type"] = "userlist";
    json["users"] = users;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    for (QTcpSocket *client : clients) {
        client->write(data);
    }

    // 更新界面
    userListWidget->clear();
    for (const QString &user : userMap.values()) {
        userListWidget->addItem(user);
    }
    userCountLabel->setText(QString("在线: %1人").arg(userMap.size()));
}
// 阶段2：添加QTcpServer监听客户端逻辑（标记分阶段提交）
