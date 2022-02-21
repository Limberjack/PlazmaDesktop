#ifndef MESSAGE_H
#define MESSAGE_H
#include <QString>
#include <list>
#include <vector>
#include <QPair>
#include <memory>
using ParamsMap = std::list<std::pair<QString,QString>>;

enum MessageType
{
    //outcomming
    eRENAME_DEVICE,
    eSHUT_DOWN_DEVICE,
    eSTOP_DEVICE,
    eSET_TIME_ON_A_DEVICE,
    eSTART_DEVICE,
    eCHANGE_NETWORK_SETTINGS,

    //incomming
    eCREATE_HOST_DEVICE,
    eCREATE_SENSOR_DEVICE,
    eDATA_MESSAGE,
    eCONFIRM_MESSAGE
};

struct SensorData
{
    float value;
    unsigned int timeStamp;
    const QString toString(){return QString::number(value)+","+QString::number(timeStamp);}

};

struct WiFiSettings
{
    QString ssid;
    QString password;
};

class Cart
{
public:
    virtual ~Cart() = default;
    virtual QByteArray toByteArray() const = 0;
};

class DataCart : public Cart
{
    // |(4)count|(4)value|(4)time|(4)value|(4)time|(4)value|(4)time|(4)value|(4)time|(4)value|(4)time|(4)value|(4)time|
    QByteArray m_aRawData;
public:
    DataCart(const char* array, int size);
    virtual QByteArray toByteArray() const;
    std::vector<SensorData> toData();
};

class SetTime;
class NetworkSettings;

class DeviceInfo : public Cart
{
    // |(8)MAC|(64) device name|(64) device type|(64) SSID|(64) password|
public:
    QString deviceType;
    QString deviceName;
    QString SSID;
    QString password;
    QString MAC;

    DeviceInfo(const char* array, int size);
    virtual QByteArray toByteArray() const;
};

namespace DefaultNames
{
    const QString UI = "UI";
    const QString DESKTOP = "PC";
    const QString HOST_DEVICE = "HD";
    const QString BROADCAST = "BR";
};

struct Message
{
    // |(8)sender MAC|(8) addresse MAC|(1) message type|
    // |cart...........................................|
    Message(const QByteArray& rawBody);
    Message(Cart* cart);
    Message() = default;
    QByteArray toByteArray() const;

    unsigned int requestId;
    QString sender;
    QString addresse;
    MessageType type;
    std::shared_ptr<Cart> cart = nullptr;
};

const QString operator+(const QString& a, const Message& msg);
const QString operator+(const Message& msg, const QString& a);

#endif // MESSAGE_H
