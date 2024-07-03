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

#ifndef Z21COMMANDSTATION_H
#define Z21COMMANDSTATION_H

#include "../icommandstation.h"

#include <QElapsedTimer>

class QUdpSocket;

namespace Z21 {
class Message;
}

class Z21CommandStation : public ICommandStation
{
    Q_OBJECT
public:
    explicit Z21CommandStation(QObject *parent = nullptr);

    virtual bool setLocomotiveSpeed(int address, int speedStep, LocomotiveDirection direction) override;

    virtual bool emergencyStop(int address) override;

    void requestLocoInfo(int address, int oldStep, LocomotiveDirection oldDir);

private slots:
    void readPendingDatagram();

private:
    void receive(const Z21::Message& message);
    void send(const Z21::Message& message);

private:
    QUdpSocket *mSocket;
    bool socketReadScheduled = false;

    struct ReplyQueueItem
    {
        int address;
        int speedStep;
        LocomotiveDirection direction;
        QElapsedTimer elapsed;
    };

    QVector<ReplyQueueItem> replyQueue;
};

#endif // Z21COMMANDSTATION_H
