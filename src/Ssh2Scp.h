#ifndef SSH2SCP_H
#define SSH2SCP_H

#include "Ssh2Channel.h"

#include <libssh2.h>

#include <QtCore/QElapsedTimer>

#include <memory>

class QFile;

namespace daggyssh2
{

class Ssh2Scp : public Ssh2Channel
{
    Q_OBJECT

public:
    ~Ssh2Scp();

    enum ScpAction {
        Send,
        Receive
    };

signals:
    void progress(qint64 bytes, qint64 bytesTotal);
    void finished(qint64 msecs);

protected:
    Ssh2Scp(const QString& sourceFilePath, const QString& destinationFilePath,
            ScpAction action, Ssh2Client* ssh2_client);
    void checkIncomingData() override;

private:
    std::error_code openChannelSession() override;
    std::error_code closeChannelSession() override;
    std::error_code openSendChannelSession();
    std::error_code openReceiveChannelSession();

    std::error_code writeFileToServer();
    std::error_code receiveFileFromServer();

    friend class Ssh2Client;

    QString sourceFilePath_;
    QString destinationFilePath_;
    ScpAction scpAction_;

    QElapsedTimer timer_;
    std::unique_ptr<QFile> file_;
    QByteArray buffer_{65536, 0}; // 2^16

    // state for sending
    qint64 filePos_{0};
    qint64 fileSize_{0};

    // state for receiving
    libssh2_struct_stat fileInfo_;
    qint64 totalReadBytes_{0};
};

} // namespace daggyssh2

#endif // SSH2SCP_H
