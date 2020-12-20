/*
MIT License

Copyright (c) 2020 Jeroen Oomkes

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Ssh2LocalPortForwarding.h"
#include "Ssh2Client.h"
#include "Ssh2Debug.h"

#include <libssh2.h>

using namespace daggyssh2;

Ssh2LocalPortForwarding::Ssh2LocalPortForwarding(const QHostAddress& localListenIp, qint16 localListenPort,
                                                 const QHostAddress& remoteListenIp, qint16 remoteListenPort,
                                                 Ssh2Client* ssh2_client)
    : Ssh2Channel{ssh2_client}
    , localListenIp_{localListenIp}
    , localListenPort_{localListenPort}
    , remoteListenIp_{remoteListenIp}
    , remoteListenPort_{remoteListenPort}
{
    connect(&tcpServer_, &QTcpServer::newConnection,
            this, &Ssh2LocalPortForwarding::onNewConnection);
}

void Ssh2LocalPortForwarding::checkIncomingData()
{
    std::error_code error_code = ssh2_success;
    switch (channelState()) {
    case ChannelStates::Opening:

        switch (forwardState_) {
        case ForwardState::NotOpen:
            error_code = startListening();
            break;
        case ForwardState::Listening:
            break;
        case ForwardState::OpenChannel:
            error_code = openChannelSession();
            break;
        }

        break;
    case ChannelStates::Opened:
        onReadyReadServer();
        error_code = lastError();
        if (libssh2_channel_eof(ssh2_channel_) == 1) {
            close();
        }
        break;
    case ChannelStates::Closing:
        error_code = closeChannelSession();
        break;
    default:;
    }

    setLastError(error_code);
}

std::error_code Ssh2LocalPortForwarding::startListening()
{
    if (!tcpServer_.listen(QHostAddress{localListenIp_}, localListenPort_)) {
        qCritical() << tcpServer_.errorString();
        return Ssh2Error::UnexpectedError;
    }

    forwardState_ = ForwardState::Listening;
    return ssh2_success;
}

std::error_code Ssh2LocalPortForwarding::openChannelSession()
{
    std::error_code error_code = ssh2_success;

    if (channelState() == ChannelStates::NotOpen) {
        setSsh2ChannelState(ChannelStates::Opening);
        return startListening();
    }

    ssh2_channel_ = libssh2_channel_direct_tcpip_ex(
        ssh2Client()->ssh2Session(),
        remoteListenIp_.toString().toLatin1().data(), remoteListenPort_,
        localListenIp_.toString().toLatin1().data(), forwardSocket_->peerPort());

    int ssh2_method_result = 0;
    if (!ssh2_channel_) {
        ssh2_method_result = libssh2_session_last_error(ssh2Client()->ssh2Session(),
                                                        nullptr,
                                                        nullptr,
                                                        0);
    }

    switch (ssh2_method_result) {
    case LIBSSH2_ERROR_EAGAIN:
        setSsh2ChannelState(Opening);
        error_code = Ssh2Error::TryAgain;
        break;
    case 0:
        setSsh2ChannelState(Opened);
        connect(forwardSocket_, &QTcpSocket::readyRead,
                this, &Ssh2LocalPortForwarding::onReadyReadForward);
        connect(forwardSocket_, &QTcpSocket::disconnected,
                this, &Ssh2LocalPortForwarding::close);
        break;
    default: {
        debugSsh2Error(ssh2_method_result);
        error_code = Ssh2Error::FailedToOpenChannel;
        setSsh2ChannelState(FailedToOpen);
    }
    }

    return error_code;
}

std::error_code Ssh2LocalPortForwarding::closeChannelSession()
{
    return Ssh2Channel::closeChannelSession();
    delete forwardSocket_;
}

void Ssh2LocalPortForwarding::onNewConnection()
{
    forwardSocket_ = tcpServer_.nextPendingConnection();
    if (!forwardSocket_) {
        setLastError(Ssh2Error::UnexpectedError);
    }

    forwardState_ = ForwardState::OpenChannel;
    openChannelSession();
}

void Ssh2LocalPortForwarding::onReadyReadForward()
{
    QByteArray buffer{65536, 0}; // 2^16
    do {
        auto bytesRead = forwardSocket_->read(buffer.data(), buffer.length());
        if (bytesRead <= 0) {
            break;
        }
        auto* bufferPos = buffer.data();
        do {
            auto writtenBytes = writeData(bufferPos, bytesRead);
            if (writtenBytes < 0) {
                if (writtenBytes == LIBSSH2_ERROR_EAGAIN) {
                    setLastError(Ssh2Error::TryAgain);
                    return;
                }
                qCritical() << "Error forwarding data to server";
                setLastError(Ssh2Error::UnexpectedError);
                return;
            } else {
                bufferPos += writtenBytes;
                bytesRead -= writtenBytes;
            }
        } while (bytesRead);
    } while (1);
}

void Ssh2LocalPortForwarding::onReadyReadServer()
{
    QByteArray buffer{65536, 0}; // 2^16
    do {
        auto readBytes = readData(buffer.data(), buffer.length());
        if (readBytes <= 0) {
            break;
        }
        forwardSocket_->write(buffer.data(), readBytes);
    } while (1);
}

Ssh2LocalPortForwarding::~Ssh2LocalPortForwarding()
{
}
