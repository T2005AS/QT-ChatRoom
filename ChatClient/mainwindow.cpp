#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), socket(nullptr)
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *layout = new QVBoxLayout(central);

    // 1. 连接区域
    QHBoxLayout *connLayout = new QHBoxLayout();
    connectBtn = new QPushButton("连接服务器", this);
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("输入用户名");
    loginBtn = new QPushButton("登录", this);

    connLayout->addWidget(connectBtn);
    connLayout->addWidget(usernameEdit);
    connLayout->addWidget(loginBtn);
    layout->addLayout(connLayout);

    // 2. 聊天显示
    chatTextEdit = new QTextEdit(this);
    chatTextEdit->setReadOnly(true);
    layout->addWidget(new QLabel("聊天记录:"));
    layout->addWidget(chatTextEdit);

    // 3. 在线用户
    userListWidget = new QListWidget(this);
    layout->addWidget(new QLabel("在线用户:"));
    layout->addWidget(userListWidget);

    // 4. 消息发送
    QHBoxLayout *sendLayout = new QHBoxLayout();
    messageEdit = new QLineEdit(this);
    messageEdit->setPlaceholderText("输入消息...");
    sendBtn = new QPushButton("发送", this);

    sendLayout->addWidget(messageEdit);
    sendLayout->addWidget(sendBtn);
    layout->addLayout(sendLayout);

    setWindowTitle("聊天客户端");
    resize(400, 500);

    // 连接信号
    connect(connectBtn, &QPushButton::clicked, this, &MainWindow::connectServer);
    connect(loginBtn, &QPushButton::clicked, this, &MainWindow::login);
    connect(sendBtn, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(messageEdit, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);
}

MainWindow::~MainWindow() {}

void MainWindow::connectServer()
{
    if (socket) {
        socket->deleteLater();
    }

    socket = new QTcpSocket(this);
    socket->connectToHost("127.0.0.1", 12345);

    connect(socket, &QTcpSocket::connected, this, [this]() {
        chatTextEdit->append("已连接到服务器");
    });

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::disconnected);
}

void MainWindow::login()
{
    if (!socket || socket->state() != QTcpSocket::ConnectedState) {
        QMessageBox::warning(this, "错误", "请先连接服务器");
        return;
    }

    username = usernameEdit->text().trimmed();
    if (username.isEmpty()) {
        QMessageBox::warning(this, "错误", "用户名不能为空");
        return;
    }

    QJsonObject json;
    json["type"] = "login";
    json["username"] = username;

    socket->write(QJsonDocument(json).toJson());
    chatTextEdit->append("正在登录...");
}

void MainWindow::sendMessage()
{
    if (!socket || socket->state() != QTcpSocket::ConnectedState) {
        QMessageBox::warning(this, "错误", "未连接服务器");
        return;
    }

    QString text = messageEdit->text().trimmed();
    if (text.isEmpty()) return;

    QJsonObject json;
    json["type"] = "message";
    json["content"] = text;

    socket->write(QJsonDocument(json).toJson());
    messageEdit->clear();
}

void MainWindow::readyRead()
{
    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    QJsonObject json = doc.object();
    QString type = json.value("type").toString();

    if (type == "message") {
        QString content = json.value("content").toString();
        chatTextEdit->append(content);
    } else if (type == "userlist") {
        userListWidget->clear();
        QJsonArray users = json.value("users").toArray();
        for (const QJsonValue &val : users) {
            userListWidget->addItem(val.toString());
        }
    }
}

void MainWindow::disconnected()
{
    chatTextEdit->append("与服务器断开连接");
}

// 阶段1：UI布局代码 - 标记此行为分阶段提交的标识
// 阶段2：添加QTcpSocket连接服务器逻辑（标记分阶段提交）
