#ifndef SSH2LOCALPORTFORWARDING_H
#define SSH2LOCALPORTFORWARDING_H

#include "Ssh2Channel.h"

#include <QtNetwork/QTcpServer>
#include <QtCore/QPointer>

class QTcpSocket;

namespace qlibssh2
{

class Ssh2LocalPortForwarding : public Ssh2Channel
{
    Q_OBJECT

public:
    ~Ssh2LocalPortForwarding();

    enum class ForwardState {
        NotOpen,
        Listening,
        OpenChannel,
    };

protected:
    Ssh2LocalPortForwarding(const QHostAddress& localListenIp, qint16 localListenPort,
                            const QHostAddress& remoteListenIp, qint16 remoteListenPort,
                            Ssh2Client* ssh2_client);
    void checkIncomingData() override;
    void close() override;

private:
    friend class Ssh2Client;

    std::error_code startListening();
    std::error_code openChannelSession() override;
    std::error_code closeChannelSession() override;
    void onNewConnection();
    void onReadyReadForward();
    void onReadyReadServer();

    QHostAddress localListenIp_;
    qint16 localListenPort_{0};
    QHostAddress remoteListenIp_;
    qint16 remoteListenPort_{0};
    QTcpServer tcpServer_;
    ForwardState forwardState_{NotOpen};
    QPointer<QTcpSocket> forwardSocket_;
};

} // namespace qlibssh2

#endif // SSH2LOCALPORTFORWARDING_H
