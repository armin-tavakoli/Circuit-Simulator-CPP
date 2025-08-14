#include "client.h"
#include <QHostAddress>

Client::Client(QObject *parent) : QObject(parent)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
    connect(m_socket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &Client::onDisconnected);
}

void Client::connectToServer(const QString &ip, quint16 port)
{
    emit logMessage("Connecting to server " + ip + ":" + QString::number(port));
    m_socket->connectToHost(QHostAddress(ip), port);
}

void Client::onConnected()
{
    emit logMessage("Connected to server!");
}

void Client::onDisconnected()
{
    emit logMessage("Disconnected from server.");
}

void Client::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    QString dataStr(data);
    bool ok;
    double voltage = dataStr.toDouble(&ok);
    if (ok) {
        emit voltageReceived(voltage);
    }
}