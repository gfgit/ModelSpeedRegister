/**
 * Code taken from: www.github.com/traintastic/trainstastic
 *
 * This file is part of the traintastic source code.
 *
 * Copyright (C) 2021-2024 Reinder Feenstra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "z21commandstation.h"

#include <QUdpSocket>

#include <QElapsedTimer>
#include <QtEndian>

#include "z21messages.h"

#include <QDebug>

#include <QTimer>

Z21CommandStation::Z21CommandStation(QObject *parent)
    : ICommandStation{parent}
{
    mSocket = new QUdpSocket(this);
    mSocket->bind(QHostAddress("192.168.1.196"), 21105);
    connect(mSocket, &QUdpSocket::readyRead, this, &Z21CommandStation::readPendingDatagram);
    QTimer *t = new QTimer(this);
    connect(t, &QTimer::timeout, this, &Z21CommandStation::readPendingDatagram);
    t->start(500);

    QTimer *keepAlive = new QTimer(this);
    connect(keepAlive, &QTimer::timeout, this,
            [this]()
    {
        send(Z21::LanSetBroadcastFlags(Z21::BroadcastFlags::AllLocoChanges));
    });
    keepAlive->start(10000);

    send(Z21::LanSetBroadcastFlags(Z21::BroadcastFlags::AllLocoChanges));

    QTimer *locoInfo = new QTimer(this);
    connect(locoInfo, &QTimer::timeout, this,
            [this]()
    {
        requestLocoInfo(46, 0, LocomotiveDirection::Forward);
        requestLocoInfo(47, 0, LocomotiveDirection::Forward);
    });
    //locoInfo->start(1000);
}

bool Z21CommandStation::setLocomotiveSpeed(int address, int speedStep, LocomotiveDirection direction)
{
    ReplyQueueItem item;
    item.address = address;
    item.speedStep = speedStep;
    item.direction = direction;
    item.elapsed.start();
    replyQueue.append(item);


    Z21::LanXSetLocoDrive message;
    message.setAddress(address, false);

    Z21::Direction z21Direction = Z21::Direction::Forward;
    if(direction == LocomotiveDirection::Reverse)
        z21Direction = Z21::Direction::Reverse;
    message.setDirection(z21Direction);
    message.setSpeedSteps(126);
    message.setSpeedStep(speedStep);
    message.updateChecksum();

    send(message);
    return false;
}

bool Z21CommandStation::emergencyStop(int address)
{
    Z21::LanXSetLocoDrive message;
    message.setAddress(address, false);
    message.setDirection(Z21::Direction::Forward);
    message.setEmergencyStop();
    message.updateChecksum();

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

            //qDebug() << "Message???";

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
    //qDebug() << "RECEIVE";

    // Clear old reply queue items
    auto it = replyQueue.begin();
    while(it != replyQueue.end())
    {
        if(it->elapsed.elapsed() > 500)
            it = replyQueue.erase(it);
        else
            it++;
    }

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
                    qDebug() << "WRONG SPEED STEP:" << reply.speedSteps();
                    currentSpeedStep = float(currentSpeedStep) / float(reply.speedSteps()) * 126.0;
                }

                auto queued = replyQueue.end();
                for(auto it = replyQueue.begin(); it != replyQueue.end(); it++)
                {
                    if(it->address != reply.address())
                        continue;

                    if(it->direction != direction)
                        continue;

                    if(it->speedStep != currentSpeedStep)
                        continue;

                    queued = it;
                    break;
                }

                bool wasQueued = (queued != replyQueue.end());

                if(wasQueued)
                    replyQueue.erase(queued);

                qDebug() << "LOCO_INFO:" << reply.address()
                         << "Dir:" << (direction == LocomotiveDirection::Forward ? 'F' : 'R')
                         << "Step:" << currentSpeedStep
                         << "Queued:" << wasQueued;

                emit locomotiveSpeedFeedback(reply.address(), currentSpeedStep, direction, wasQueued);
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

void Z21CommandStation::requestLocoInfo(int address, int oldStep, LocomotiveDirection oldDir)
{
    ReplyQueueItem item;
    item.address = address;
    item.speedStep = oldStep;
    item.direction = oldDir;
    item.elapsed.start();
    //replyQueue.append(item);

    Z21::LanXGetLocoInfo message(address, false);
    send(message);
}
