#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>

class Client : public QObject
{
Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    void connectToServer(const QString &ip, quint16 port);

signals:
    void logMessage(const QString &msg);
    void voltageReceived(double voltage);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();

private:
    QTcpSocket *m_socket = nullptr;
};

#endif