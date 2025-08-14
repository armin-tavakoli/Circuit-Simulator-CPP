#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class Server : public QObject
{
Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    void start(quint16 port);
    void sendToClient(const QString &message);

signals:
    void logMessage(const QString &msg);

private slots:
    void onNewConnection();

private:
    QTcpServer *m_tcpServer = nullptr;
    QTcpSocket *m_clientSocket = nullptr;
};

#endif //SERVER_H