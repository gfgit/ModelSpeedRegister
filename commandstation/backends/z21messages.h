#ifndef Z21MESSAGES_H
#define Z21MESSAGES_H

#include <cstdint>
#include <cassert>
#include "endian.h"

#ifdef _MSC_VER
#define ATTRIBUTE_PACKED
#define PRAGMA_PACK_PUSH_1 __pragma(pack(push, 1))
#define PRAGMA_PACK_POP __pragma(pack(pop))
#else
#define ATTRIBUTE_PACKED __attribute__((packed))
#define PRAGMA_PACK_PUSH_1
#define PRAGMA_PACK_POP
#endif


namespace XpressNet {

struct Message
{
    uint8_t header;

    Message()
    {
    }

    Message(uint8_t _header) :
        header{_header}
    {
    }

    constexpr uint8_t identification() const
    {
        return header & 0xF0;
    }

    constexpr uint8_t dataSize() const
    {
        return header & 0x0F;
    }

    constexpr uint8_t size() const
    {
        return 2 + dataSize();
    }
};
static_assert(sizeof(Message) == 1);

uint8_t calcChecksum(const Message& msg, const int dataSize)
{
    const uint8_t* p = reinterpret_cast<const uint8_t*>(&msg);
    uint8_t checksum = p[0];
    for(int i = 1; i <= dataSize; i++)
        checksum ^= p[i];
    return checksum;
}

bool isChecksumValid(const Message& msg, const int dataSize)
{
    return calcChecksum(msg, dataSize) == *(reinterpret_cast<const uint8_t*>(&msg) + dataSize + 1);
}

}

namespace Z21 {

enum class Direction : uint8_t
{
    Forward = 0,
    Reverse = 1,
    Unknown = 255
};

static constexpr uint8_t LAN_X_TURNOUT_INFO = 0x43;
static constexpr uint8_t LAN_X_SET_TURNOUT = 0x53;
static constexpr uint8_t LAN_X_SET_EXT_ACCESSORY = 0x54;

static constexpr uint8_t LAN_X_BC = 0x61;

static constexpr uint8_t LAN_X_STATUS_CHANGED = 0x62;
static constexpr uint8_t LAN_X_GET_VERSION_REPLY = 0x63;

//static constexpr uint8_t LAN_X_CV_NACK_SC = 0x12;
//static constexpr uint8_t LAN_X_CV_NACK = 0x13;
//static constexpr uint8_t LAN_X_UNKNOWN_COMMAND = 0x82;

static constexpr uint8_t LAN_X_SET_STOP = 0x80;
static constexpr uint8_t LAN_X_BC_STOPPED = 0x81;

static constexpr uint8_t LAN_X_GET_LOCO_INFO = 0xE3;
static constexpr uint8_t LAN_X_SET_LOCO = 0xE4;
static constexpr uint8_t LAN_X_LOCO_INFO = 0xEF;

static constexpr uint8_t LAN_X_GET_FIRMWARE_VERSION = 0xF1;
static constexpr uint8_t LAN_X_GET_FIRMWARE_VERSION_REPLY = 0xF3;

// db0 for xHeader 0x21
static constexpr uint8_t LAN_X_SET_TRACK_POWER_OFF = 0x80;
static constexpr uint8_t LAN_X_SET_TRACK_POWER_ON = 0x81;

// db0 for xHeader LAN_X_BC
static constexpr uint8_t LAN_X_BC_TRACK_POWER_OFF = 0x00;
static constexpr uint8_t LAN_X_BC_TRACK_POWER_ON = 0x01;
//static constexpr uint8_t LAN_X_BC_PROGRAMMING_MODE = 0x02;
static constexpr uint8_t LAN_X_BC_TRACK_SHORT_CIRCUIT = 0x08;
static constexpr uint8_t LAN_X_UNKNOWN_COMMAND = 0x82;

enum Header : uint16_t
{
    LAN_GET_SERIAL_NUMBER = 0x10,
    LAN_GET_CODE = 0x18,
    LAN_GET_HWINFO = 0x1A,
    LAN_LOGOFF = 0x30,
    LAN_X = 0x40,
    LAN_SET_BROADCASTFLAGS = 0x50,
    LAN_GET_BROADCASTFLAGS = 0x51,
    LAN_GET_LOCO_MODE = 0x60,
    LAN_SET_LOCO_MODE = 0x61,
    LAN_GET_TURNOUTMODE = 0x70,
    LAN_SET_TURNOUTMODE = 0x71,
    LAN_RMBUS_DATACHANGED = 0x80,
    LAN_RMBUS_GETDATA = 0x81,
    LAN_RMBUS_PROGRAMMODULE = 0x82,
    LAN_SYSTEMSTATE_DATACHANGED = 0x84,
    LAN_SYSTEMSTATE_GETDATA = 0x85,
    LAN_RAILCOM_DATACHANGED = 0x88,
    LAN_RAILCOM_GETDATA = 0x89,
    LAN_LOCONET_Z21_RX = 0xA0,
    LAN_LOCONET_Z21_TX = 0xA1,
    LAN_LOCONET_FROM_LAN = 0xA2,
    LAN_LOCONET_DISPATCH_ADDR = 0xA3,
    LAN_LOCONET_DETECTOR = 0xA4,
    LAN_CAN_DETECTOR = 0xC4,
};

enum class BroadcastFlags : uint32_t
{
    None = 0,

    /// Broadcasts and info messages concerning driving and switching are delivered to the registered clients automatically.
    /// The following messages are concerned:
    /// 2.7 LAN_X_BC_TRACK_POWER_OFF
    /// 2.8 LAN_X_BC_TRACK_POWER_ON
    /// 2.9 LAN_X_BC_PROGRAMMING_MODE
    /// 2.10 LAN_X_BC_TRACK_SHORT_CIRCUIT
    /// 2.14 LAN_X_BC_STOPPED
    /// 4.4 LAN_X_LOCO_INFO (loco address must be subscribed too)
    /// 5.3 LAN_X_TURNOUT_INFO
    PowerLocoTurnoutChanges = 0x00000001,

    /// Changes of the feedback devices on the R-Bus are sent automatically.
    /// Z21 Broadcast messages see 7.1 LAN_RMBUS_DATACHANGED
    RBusChanges = 0x00000002,

    /// Changes of RailCom data of subscribed locomotives are sent automatically.
    /// Z21 Broadcast messages see 8.1 LAN_RAILCOM_DATACHANGED
    RailCOMChanges = 0x00000004,

    /// Changes of the Z21 system status are sent automatically.
    /// Z21 Broadcast messages see 2.18 LAN_SYSTEMSTATE_DATACHANGED
    SystemStatusChanges = 0x00000100,

    /// Extends flag 0x00000001; client now gets LAN_X_LOCO_INFO LAN_X_LOCO_INFO
    /// without having to subscribe to the corresponding locomotive addresses, i.e. for all
    /// controlled locomotives!
    /// Due to the high network traffic, this flag may only be used by adequate PC railroad
    /// automation software and is NOT intended for mobile hand controllers under any
    /// circumstances.
    /// From FW V1.20 bis V1.23: LAN_X_LOCO_INFO is sent for all locomotives.
    /// From FW V1.24: LAN_X_LOCO_INFO is sent for all modified locomotives.
    AllLocoChanges = 0x00010000,

    /// Forwarding messages from LocoNet bus to LAN client without locos and switches.
    LocoNetWithoutLocoAndSwitches = 0x01000000,

    /// Forwarding locomotive-specific LocoNet messages to LAN Client:
    /// OPC_LOCO_SPD, OPC_LOCO_DIRF, OPC_LOCO_SND, OPC_LOCO_F912, OPC_EXP_CMD
    LocoNetLoco = 0x02000000,

    /// Forwarding switch-specific LocoNet messages to LAN client:
    /// OPC_SW_REQ, OPC_SW_REP, OPC_SW_ACK, OPC_SW_STATE
    LocoNetSwitch = 0x04000000,

    /// Sending status changes of LocoNet track occupancy detectors to the LAN client.
    /// See 9.5 LAN_LOCONET_DETECTOR
    LocoNetDetector = 0x08000000,

    ///
    LocoNet = LocoNetWithoutLocoAndSwitches | LocoNetLoco | LocoNetSwitch | LocoNetDetector,

    /// Version 1.29:
    /// Sending changes of RailCom data to the LAN Client.
    /// Client gets LAN_RAILCOM_DATACHANGED without having to subscribe to the
    /// corresponding locomotive addresses, i.e. for all controlled locomotives! Due to the high
    /// network traffic, this flag may only be used by adequate PC railroad automation software
    /// and is NOT intended for mobile hand controllers under any circumstances.
    /// Z21 Broadcast messages see 8.1 LAN_RAILCOM_DATACHANGED
    RailComDataChanged = 0x00040000,

    /// Sending status changes of CAN-Bus track occupancy detectors to the LAN client.
    /// See 10.1 LAN_CAN_DETECTOR
    CANDetector = 0x00080000,
};

namespace Utils {

inline constexpr uint8_t directionFlag = 0x80;

constexpr Direction getDirection(uint8_t db)
{
    return (db & directionFlag) ? Direction::Forward : Direction::Reverse;
}

constexpr void setDirection(uint8_t& db, Direction direction)
{
    if(direction == Direction::Forward)
        db |= directionFlag;
    else
        db &= ~directionFlag;
}

constexpr bool isEmergencyStop(uint8_t db, uint8_t speedSteps)
{
    switch(speedSteps)
    {
    case 126:
        return (db & 0x7F) == 0x01;

    case 28:
        return (db & 0x1F) == 0x01 || (db & 0x1F) == 0x11;

    case 14:
        return (db & 0x0F) == 0x01;
    }
    return true;
}

constexpr void setEmergencyStop(uint8_t& db)
{
    db = (db & directionFlag) | 0x01; // preserve direction flag
}

constexpr uint8_t getSpeedStep(uint8_t db, uint8_t speedSteps)
{
    if(isEmergencyStop(db, speedSteps))
        return 0;

    switch(speedSteps)
    {
    case 126:
        db &= 0x7F;
        break;

    case 28:
        db = ((db & 0x0F) << 1) | ((db & 0x10) >> 4); //! @todo check
        if(db >= 3)
            db -= 2;
        break;

    case 14:
        db &= 0x0F;
        break;

    default:
        return 0;
    }
    return db >= 1 ? db - 1 : 0; // step 1 = EStop
}

constexpr void setSpeedStep(uint8_t& db, uint8_t speedSteps, uint8_t speedStep)
{
    db &= directionFlag; // preserve direction flag
    if(++speedStep > 1)
        switch(speedSteps)
        {
        case 126:
            db |= speedStep & 0x7F;
            break;

        case 28:
            speedStep += 2;
            db |= ((speedStep >> 1) & 0x0F) | ((speedStep & 0x01) << 4);
            break;

        case 14:
            db |= speedStep & 0x0F;
            break;
        }
}

constexpr uint8_t toBCD(uint8_t value)
{
    return ((value / 10) << 4) | (value % 10);
}

constexpr uint8_t fromBCD(uint8_t value)
{
    return ((value >> 4) * 10) + (value & 0x0F);
}

} // namespace Utils

PRAGMA_PACK_PUSH_1

    struct Message
{
    uint16_t dataLenLE; //!< DataLen (little endian): Total length over the entire data set including DataLen, Header and Data, i.e. DataLen = 2+2+n.
    Header headerLE;  //!< Header (little endian): Describes the Command and the Protocol’s group. \see Header

    Message(uint16_t _dataLen, Header _header) :
        dataLenLE{host_to_le(_dataLen)},
        headerLE{host_to_le(_header)}
    {
    }

    inline uint16_t dataLen() const
    {
        return le_to_host(dataLenLE);
    }

    inline Header header() const
    {
        return static_cast<Header>(le_to_host(headerLE));
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(Message) == 4);

struct LanX : Message
{
    uint8_t xheader;

    LanX(uint16_t _dataLen, uint8_t _xheader) :
        Message(_dataLen, LAN_X),
        xheader{_xheader}
    {
    }

    void updateChecksum(uint8_t len);

    inline void updateChecksum()
    {
        updateChecksum(xheader & 0x0F);
    }

    static bool isChecksumValid(const LanX& lanX);

} ATTRIBUTE_PACKED;
static_assert(sizeof(LanX) == 5);

// LAN_SET_BROADCASTFLAGS
struct LanSetBroadcastFlags : Message
{
    BroadcastFlags broadcastFlagsLE; // LE

    LanSetBroadcastFlags(BroadcastFlags _broadcastFlags = BroadcastFlags::None) :
        Message(sizeof(LanSetBroadcastFlags), LAN_SET_BROADCASTFLAGS),
        broadcastFlagsLE{host_to_le(_broadcastFlags)}
    {
    }

    inline BroadcastFlags broadcastFlags() const
    {
        return le_to_host(broadcastFlagsLE);
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanSetBroadcastFlags) == 8);

PRAGMA_PACK_POP

    // LAN_X_SET_LOCO_DRIVE
    struct LanXSetLocoDrive : LanX
{
    uint8_t db0;
    uint8_t addressHigh;
    uint8_t addressLow;
    uint8_t speedAndDirection = 0;
    uint8_t checksum;

    LanXSetLocoDrive() :
        LanX(sizeof(LanXSetLocoDrive), LAN_X_SET_LOCO)
    {
    }

    inline uint16_t address() const
    {
        return (static_cast<uint16_t>(addressHigh & 0x3F) << 8) | addressLow;
    }

    inline bool isLongAddress() const
    {
        return (addressHigh & 0xC0) == 0xC0;
    }

    inline void setAddress(uint16_t address, bool longAddress)
    {
        addressHigh = longAddress ? (0xC0 | (address >> 8)) : 0x00;
        addressLow = longAddress ? address & 0xFF : address & 0x7F;
    }

    inline void setSpeedSteps(uint8_t steps)
    {
        switch(steps)
        {
        case 14:
            db0 = 0x10;
            break;
        case 28:
            db0 = 0x12;
            break;
        case 126:
        case 128:
        default:
            db0 = 0x13;
            break;
        }
    }

    inline uint8_t speedSteps() const
    {
        switch(db0 & 0x0F)
        {
        case 0: return 14;
        case 2: return 28;
        case 3: return 126;
        default: return 0;
        }
    }

    inline Direction direction() const
    {
        return Utils::getDirection(speedAndDirection);
    }

    inline void setDirection(Direction value)
    {
        assert(value != Direction::Unknown);
        Utils::setDirection(speedAndDirection, value);
    }

    inline bool isEmergencyStop() const
    {
        return Utils::isEmergencyStop(speedAndDirection, speedSteps());
    }

    inline void setEmergencyStop()
    {
        Utils::setEmergencyStop(speedAndDirection);
    }

    inline uint8_t speedStep() const
    {
        return Utils::getSpeedStep(speedAndDirection, speedSteps());
    }

    inline void setSpeedStep(uint8_t value)
    {
        Utils::setSpeedStep(speedAndDirection, speedSteps(), value);
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXSetLocoDrive) == 10);

// LAN_X_LOCO_INFO
struct LanXLocoInfo : LanX
{
    static constexpr uint8_t db2_busy_flag = 0x08;
    static constexpr uint8_t db2_speed_steps_14 = 0x00;
    static constexpr uint8_t db2_speed_steps_28 = 0x02;
    static constexpr uint8_t db2_speed_steps_128 = 0x04;
    static constexpr uint8_t db2_speed_steps_mask = 0x07;
    static constexpr uint8_t directionFlag = 0x80;
    static constexpr uint8_t speedStepMask = 0x7F;
    static constexpr uint8_t flagF0 = 0x10;
    static constexpr uint8_t supportedFunctionIndexMax = 31; ///< \sa functionIndexMax

    static constexpr uint8_t minMessageSize = 7 + 7;
    static constexpr uint8_t maxMessageSize = 7 + 14;

    uint8_t addressHigh = 0; //db0
    uint8_t addressLow = 0;  //db1
    uint8_t db2 = 0;
    uint8_t speedAndDirection = 0; //db3
    uint8_t db4 = 0;
    uint8_t f5f12 = 0;  //db5
    uint8_t f13f20 = 0; //db6
    uint8_t f21f28 = 0; //db7
    uint8_t db8 = 0;    //db8 is f29f31 (firmware >= 1.42) otherwise checksum
    uint8_t db9 = 0;    //checksum (firmware >= 1.42)

    LanXLocoInfo() :
        LanX(sizeof(LanXLocoInfo), LAN_X_LOCO_INFO)
    {
    }

    inline uint16_t address() const
    {
        return (static_cast<uint16_t>(addressHigh & 0x3F) << 8) | addressLow;
    }

    inline bool isLongAddress() const
    {
        return (addressHigh & 0xC0) == 0xC0;
    }

    inline void setAddress(uint16_t address, bool longAddress)
    {
        addressHigh = longAddress ? (0xC0 | (address >> 8)) : 0x00;
        addressLow = longAddress ? address & 0xFF : address & 0x7F;
    }

    inline bool isBusy() const
    {
        return db2 & db2_busy_flag;
    }

    inline void setBusy(bool value)
    {
        if(value)
            db2 |= db2_busy_flag;
        else
            db2 &= ~db2_busy_flag;
    }

    inline uint8_t speedSteps() const
    {
        switch(db2 & db2_speed_steps_mask)
        {
        case db2_speed_steps_14:  return 14;
        case db2_speed_steps_28:  return 28;
        case db2_speed_steps_128: return 126;
        }
        return 0;
    }

    inline void setSpeedSteps(uint8_t value)
    {
        db2 &= ~db2_speed_steps_mask;
        switch(value)
        {
        case 14:  db2 |= db2_speed_steps_14;  break;
        case 28:  db2 |= db2_speed_steps_28;  break;
        case 126:
        case 128:
        default:  db2 |= db2_speed_steps_128; break;
        }
    }

    inline Direction direction() const
    {
        return Z21::Utils::getDirection(speedAndDirection);
    }

    inline void setDirection(Direction value)
    {
        Z21::Utils::setDirection(speedAndDirection, value);
    }

    inline bool isEmergencyStop() const
    {
        return Z21::Utils::isEmergencyStop(speedAndDirection, speedSteps());
    }

    inline void setEmergencyStop()
    {
        Z21::Utils::setEmergencyStop(speedAndDirection);
    }

    inline uint8_t speedStep() const
    {
        return Z21::Utils::getSpeedStep(speedAndDirection, speedSteps());
    }

    inline void setSpeedStep(uint8_t value)
    {
        Z21::Utils::setSpeedStep(speedAndDirection, speedSteps(), value);
    }

    inline bool supportsF29F31() const
    {
        //Firmware >= 1.42 adds db8 to store F29-F31 so dataLen increases to 15
        return dataLen() >= 15;
    }

    /*!
   * \brief Get maximum fuction index stored in this message
   * \return Maximum index
   *
   * \note There is also a function at index 0 so count it
   *
   * Maximum function index depends on Z21 firmware protocol version
   * versions <  1.42 support up to F28
   * versions >= 1.42 support up to F31
   *
   * Messages are backward compatible but older version will only read
   * up to their maximum function number
   *
   * We currently support up to F31 in trasmission and reception \sa supportedFunctionIndexMax
   */
    inline uint8_t functionIndexMax() const
    {
        if(supportsF29F31())
            return 31;
        return 28;
    }

    bool getFunction(uint8_t index) const
    {
        if(index == 0)
            return db4 & flagF0;
        else if(index <= 4)
            return db4 & (1 << (index - 1));
        else if(index <= 12)
            return f5f12 & (1 << (index - 5));
        else if(index <= 20)
            return f13f20 & (1 << (index - 13));
        else if(index <= 28)
            return f21f28 & (1 << (index - 21));
        else if(index <= 31 && supportsF29F31())
            return db8 & (1 << (index - 29));
        else
            return false;
    }

    void setFunction(uint8_t index, bool value)
    {
        if(index == 0)
        {
            if(value)
                db4 |= flagF0;
            else
                db4 &= ~flagF0;
        }
        else if(index <= 4)
        {
            const uint8_t flag = (1 << (index - 1));
            if(value)
                db4 |= flag;
            else
                db4 &= ~flag;
        }
        else if(index <= 12)
        {
            const uint8_t flag = (1 << (index - 5));
            if(value)
                f5f12 |= flag;
            else
                f5f12 &= ~flag;
        }
        else if(index <= 20)
        {
            const uint8_t flag = (1 << (index - 13));
            if(value)
                f13f20 |= flag;
            else
                f13f20 &= ~flag;
        }
        else if(index <= 28)
        {
            const uint8_t flag = (1 << (index - 21));
            if(value)
                f21f28 |= flag;
            else
                f21f28 &= ~flag;
        }
        else if(index <= 31 && supportsF29F31())
        {
            const uint8_t flag = (1 << (index - 29));
            if(value)
                db8 |= flag;
            else
                db8 &= ~flag;
        }
    }

    inline void updateChecksum()
    {
        //Data length - 7 Z21 header bytes + 1 byte for db0
        LanX::updateChecksum(dataLen() - 6);
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXLocoInfo) >= LanXLocoInfo::minMessageSize &&
              sizeof(LanXLocoInfo) <= LanXLocoInfo::maxMessageSize);

// LAN_X_SET_STOP
struct LanXSetStop : LanX
{
    uint8_t checksum = 0x80;

    LanXSetStop() :
        LanX(sizeof(LanXSetStop), LAN_X_SET_STOP)
    {
    }
} ATTRIBUTE_PACKED;
static_assert(sizeof(LanXSetStop) == 6);

void LanX::updateChecksum(uint8_t len)
{
    *(reinterpret_cast<uint8_t*>(this) + sizeof(LanX) + len) = XpressNet::calcChecksum(*reinterpret_cast<const XpressNet::Message*>(&xheader), len);
}

bool LanX::isChecksumValid(const LanX &lanX)
{
    const XpressNet::Message& msg = *reinterpret_cast<const XpressNet::Message*>(&lanX.xheader);
    int dataSize = msg.dataSize();
    if(lanX.xheader == LAN_X_LOCO_INFO)
    {
        //Special case for variable length message
        dataSize = lanX.dataLen() - 6;
    }

    return XpressNet::isChecksumValid(msg, dataSize);
}

}; // namespace Z21

#endif // Z21MESSAGES_H
