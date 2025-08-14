#include "server.h"
#include <QDebug>

Server::Server(QObject *parent) : QObject(parent)
{
    m_tcpServer = new QTcpServer(this);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &Server::onNewConnection);
}

void Server::start(quint16 port)
{
    if (!m_tcpServer->listen(QHostAddress::Any, port)) {
        emit logMessage("Server could not start!");
    } else {
        emit logMessage("Server started, listening on port " + QString::number(port));
    }
}

void Server::onNewConnection()
{
    m_clientSocket = m_tcpServer->nextPendingConnection();
    if(m_clientSocket) {
        emit logMessage("Client connected!");
    }
}

void Server::sendToClient(const QString &message)
{
    if (m_clientSocket && m_clientSocket->isOpen()) {
        m_clientSocket->write(message.toUtf8());
        m_clientSocket->flush();
    }
}