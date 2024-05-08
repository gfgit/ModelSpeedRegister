#include "z21commandstation.h"

#include <QUdpSocket>

#include <QElapsedTimer>
#include <QtEndian>

#include "z21messages.h"

Z21CommandStation::Z21CommandStation(QObject *parent)
    : ICommandStation{parent}
{
    mSocket = new QUdpSocket(this);
    mSocket->bind(QHostAddress::LocalHost, 21105);
    connect(mSocket, &QUdpSocket::readyRead, this, &Z21CommandStation::readPendingDatagram);
}

bool Z21CommandStation::setLocomotiveSpeed(int address, int speedStep, LocomotiveDirection direction)
{
    Z21::LanXSetLocoDrive message;
    message.setAddress(address, false);

    Z21::Direction z21Direction = Z21::Direction::Forward;
    if(direction == LocomotiveDirection::Reverse)
        z21Direction = Z21::Direction::Reverse;
    message.setDirection(z21Direction);
    message.setSpeedSteps(126);
    message.setSpeedStep(speedStep);
    send(message);
    return false;
}

bool Z21CommandStation::emergencyStop(int address)
{
    Z21::LanXSetLocoDrive message;
    message.setAddress(address, false);
    message.setDirection(Z21::Direction::Forward);
    message.setEmergencyStop();
    send(message);
    return true;
}

void Z21CommandStation::readPendingDatagram()
{
    constexpr int MAX_BUF_SIZE = 4096;

    QElapsedTimer timer;
    timer.start();

    while (mSocket->hasPendingDatagrams() && timer.elapsed() < 500)
    {
        // Allocate buffer
        qint64 sz = mSocket->pendingDatagramSize();
        if (sz > MAX_BUF_SIZE)
            sz = MAX_BUF_SIZE;

        std::unique_ptr<uint8_t[]> buf(new uint8_t[sz]);
        uint8_t *ptr = buf.get();

        // Receive datagram
        mSocket->readDatagram(reinterpret_cast<char *>(ptr), sz);

        // NOTE: a single datagram can contain multimple independent Z21 message
        while (sz > 2)
        {
            // Check message is fully read
            uint16_t msgSize = *reinterpret_cast<uint16_t *>(ptr);
            msgSize = qFromLittleEndian(msgSize);

            if (msgSize > sz || !msgSize)
                break;

            // Handle message
            const Z21::Message& message = *reinterpret_cast<const Z21::Message*>(ptr);
            receive(message);

            // Go to next message
            ptr += msgSize;
            sz -= msgSize;
        }
    }

    if (mSocket->hasPendingDatagrams() && !socketReadScheduled)
    {
        socketReadScheduled = true;
        // Manually schedule next read
        QMetaObject::invokeMethod(
            this,
            [this]() {
                socketReadScheduled = false;
                readPendingDatagram();
            },
            Qt::QueuedConnection);
    }
}

void Z21CommandStation::receive(const Z21::Message &message)
{
    switch(message.header())
    {
    case Z21::LAN_X:
    {
        const auto& lanX = static_cast<const Z21::LanX&>(message);

        if(!Z21::LanX::isChecksumValid(lanX))
            break;

        switch(lanX.xheader)
        {
        case Z21::LAN_X_LOCO_INFO:
        {
            if(message.dataLen() >= Z21::LanXLocoInfo::minMessageSize && message.dataLen() <= Z21::LanXLocoInfo::maxMessageSize)
            {
                const auto& reply = static_cast<const Z21::LanXLocoInfo&>(message);

                LocomotiveDirection direction = LocomotiveDirection::Forward;
                if(reply.direction() == Z21::Direction::Reverse)
                    direction = LocomotiveDirection::Reverse;

                //Rescale everything to 126 steps
                int currentSpeedStep = reply.speedStep();
                if(reply.speedSteps() != 126)
                {
                    currentSpeedStep = float(currentSpeedStep) / float(reply.speedSteps()) * 126.0;
                }

                emit locomotiveSpeedFeedback(reply.address(), currentSpeedStep, direction);
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

void Z21CommandStation::send(const Z21::Message &message)
{
    mSocket->writeDatagram(reinterpret_cast<const char*>(&message),
                           message.dataLen(),
                           QHostAddress::Broadcast, 21105);
}