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

#include "Ssh2Scp.h"
#include "Ssh2Client.h"
#include "Ssh2Debug.h"

#include <libssh2.h>

#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

using namespace daggyssh2;

Ssh2Scp::~Ssh2Scp()
{
}

Ssh2Scp::Ssh2Scp(const QString& sourceFilePath, const QString& destinationFilePath,
                 ScpAction action, Ssh2Client* ssh2_client)
    : Ssh2Channel{ssh2_client}
    , sourceFilePath_{sourceFilePath}
    , destinationFilePath_{destinationFilePath}
    , scpAction_{action}
{
}

void Ssh2Scp::checkIncomingData()
{
    std::error_code error_code = ssh2_success;
    switch (channelState()) {
    case ChannelStates::Opening:
        error_code = openChannelSession();
        break;
    case ChannelStates::Opened:

        switch (scpAction_) {
        case Send:
            error_code = writeFileToServer();
            break;
        case Receive:
            error_code = receiveFileFromServer();
            break;
        }

        if ((error_code && error_code.value() != Ssh2Error::TryAgain)
            || libssh2_channel_eof(ssh2_channel_) == 1) {
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

std::error_code Ssh2Scp::writeFileToServer()
{
    std::error_code error_code = ssh2_success;

    if (!file_)
        return Ssh2Error::UnexpectedError;

    file_->seek(filePos_);

    qint64 totalBytesRead{0};
    do {
        auto bytesRead = file_->read(buffer_.data(), buffer_.size());
        if (bytesRead <= 0) {
            break; // end of file
        }

        totalBytesRead += bytesRead;
        auto* bufferPos = buffer_.data();

        do {
            auto writtenBytes = writeData(bufferPos, bytesRead);
            if (writtenBytes < 0) {
                if (writtenBytes == LIBSSH2_ERROR_EAGAIN) {
                    filePos_ = file_->pos() - bytesRead;
                    return Ssh2Error::TryAgain;
                }
                filePos_ = 0;
                qCritical("ERROR %d total %ld / %d", writtenBytes,
                          static_cast<long>(totalBytesRead),
                          static_cast<long>(bytesRead));
                return lastError();
            } else {
                bufferPos += writtenBytes;
                bytesRead -= writtenBytes;
                ssh2Client()->flush();
            }
        } while (bytesRead);
        emit progress(totalBytesRead, fileSize_);
    } while (1);

    filePos_ = 0;

    close();
    emit finished(timer_.elapsed());
    return error_code;
}

std::error_code Ssh2Scp::receiveFileFromServer()
{
    std::error_code error_code = ssh2_success;

    if (!file_)
        return Ssh2Error::UnexpectedError;

    while (totalReadBytes_ < fileInfo_.st_size) {
        int readBytes{0};
        int amount = buffer_.length();
        do {
            if ((fileInfo_.st_size - totalReadBytes_) < amount) {
                amount = static_cast<int>(fileInfo_.st_size - totalReadBytes_);
            }

            readBytes = readData(buffer_.data(), buffer_.length());
            if (readBytes > 0) {
                file_->write(buffer_.data(), readBytes);
                totalReadBytes_ += readBytes;
            }
        } while (readBytes > 0);

        if ((readBytes == LIBSSH2_ERROR_EAGAIN) && (totalReadBytes_ < fileInfo_.st_size)) {
            return Ssh2Error::TryAgain;
        }
        emit progress(totalReadBytes_, fileInfo_.st_size);
        break;
    }

    totalReadBytes_ = 0;

    close();
    emit finished(timer_.elapsed());
    return error_code;
}

std::error_code Ssh2Scp::openChannelSession()
{
    switch (scpAction_) {
    case Send:
        return openSendChannelSession();
    case Receive:
        return openReceiveChannelSession();
    }

    return Ssh2Error::UnexpectedError;
}

std::error_code Ssh2Scp::closeChannelSession()
{
    if (file_) {
        file_->close();
    }
    return Ssh2Channel::closeChannelSession();
}

std::error_code Ssh2Scp::openSendChannelSession()
{
    std::error_code error_code = ssh2_success;

    QFileInfo sourceFileInfo{sourceFilePath_};
    QFileInfo destinationFileInfo{destinationFilePath_};
    QString destinationFileName;
    if (destinationFileInfo.isDir()) {
        destinationFileName = QString{"%1/%2"}
                                  .arg(destinationFilePath_)
                                  .arg(sourceFileInfo.fileName());
    } else {
        destinationFileName = destinationFileInfo.filePath();
    }

    auto file = std::make_unique<QFile>(sourceFilePath_);
    if (!file->open(QIODevice::ReadOnly)) {
        return Ssh2Error::ScpReadFileError;
    }

    QFileInfo fileInfo{file->fileName()};
    struct stat fInfo;
    stat(fileInfo.absoluteFilePath().toLatin1().data(), &fInfo);
    ssh2_channel_ = libssh2_scp_send64(ssh2Client()->ssh2Session(),
                                       destinationFileName.toLocal8Bit().data(),
                                       fInfo.st_mode & 0777,
                                       static_cast<unsigned long>(fileInfo.size()), 0, 0);

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
        error_code = writeFileToServer();
        break;
    default: {
        debugSsh2Error(ssh2_method_result);
        error_code = Ssh2Error::FailedToOpenChannel;
        setSsh2ChannelState(FailedToOpen);
    }
    }

    timer_.start();

    file_ = std::move(file);
    fileSize_ = file_->size();
    return error_code;
}

std::error_code Ssh2Scp::openReceiveChannelSession()
{
    std::error_code error_code = ssh2_success;

    ssh2_channel_ = libssh2_scp_recv2(ssh2Client()->ssh2Session(),
                                      sourceFilePath_.toLocal8Bit().data(), &fileInfo_);

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
        error_code = receiveFileFromServer();
        break;
    default: {
        debugSsh2Error(ssh2_method_result);
        error_code = Ssh2Error::FailedToOpenChannel;
        setSsh2ChannelState(FailedToOpen);
    }
    }

    timer_.start();

    QFileInfo sourceFileInfo{sourceFilePath_};
    QFileInfo destinationFileInfo{destinationFilePath_};
    QString destinationFileName;
    if (destinationFileInfo.isDir()) {
        destinationFileName = QString{"%1/%2"}
                                  .arg(destinationFilePath_)
                                  .arg(sourceFileInfo.fileName());
    } else {
        destinationFileName = destinationFileInfo.filePath();
    }

    auto file = std::make_unique<QFile>(destinationFileName);
    if (totalReadBytes_ == 0 && file->exists()) {
        file->remove();
    }

    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
        return Ssh2Error::ScpWriteFileError;
    }

    file_ = std::move(file);

    return error_code;
}
