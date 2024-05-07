#ifndef Z21COMMANDSTATION_H
#define Z21COMMANDSTATION_H

#include "../icommandstation.h"

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

private slots:
    void readPendingDatagram();

private:
    void receive(const Z21::Message& message);
    void send(const Z21::Message& message);

private:
    QUdpSocket *mSocket;
    bool socketReadScheduled = false;
};

#endif // Z21COMMANDSTATION_H
