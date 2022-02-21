#include "Message.h"
#include "Logger/Logger.h"
#include <stdio.h>

const QString operator+(const QString& a, const Message& msg)
{
    QString result = "<sender:" + msg.sender;
    result += "|addresse:" + msg.addresse;
    result += "|type:" + QString::number(msg.type);
    result += "|id:" + QString::number(msg.requestId) + ">";
    return a + result;
};

const QString operator+(const Message& msg, const QString& a)
{
    QString result = "" + msg;
    return result + a;
};


DataCart::DataCart(const char* array, int size):m_aRawData(array, size)
{};

QByteArray DataCart::toByteArray() const
{
    Logger::logError("DataCart::toByteArray() has no implementation");
    return QByteArray();
};

std::vector<SensorData> DataCart::toData()
{
    // |____|____|____|
    // |(4)count|(4)value|(4)time|(4)value|(4)time|(4)value|(4)time|(4)value|(4)time|(4)value|(4)time|(4)value|(4)time|
    // ((4)value|(4)time) repeats 'count' times
    const uint8_t tupleStep = 8;
    const uint8_t firstStep = 4;
    const uint8_t sectionSize = 4;
    float value = 0;
    unsigned int time = 0;
    unsigned int count = 0;
    char* bytePointer = (char*) &count;
    memset(bytePointer, m_aRawData.constData()[0], sectionSize);
    std::vector<SensorData> result(count);
    for(unsigned int i = 0; i != count; i++)
    {
        unsigned int pointer = i * tupleStep + firstStep;

        bytePointer = (char*) &value;
        memcpy(bytePointer, m_aRawData.data() + pointer, sectionSize);

        bytePointer = (char*) &time;
        memcpy(bytePointer, m_aRawData.data() + pointer, sectionSize);
        SensorData sd;
        sd.timeStamp = time;
        sd.value = value;
        result.push_back(sd);
    }
    return result;
};

DeviceInfo::DeviceInfo(const char* array, int size)
{
    // |(8)MAC|(64) device name|(64) device type|(64) SSID|(64) password|
    const uint8_t stringMaxLen = 64;
    const uint8_t lenOfMAC = 8;
    int pointer = 0;
    char readBuffer[stringMaxLen + 1];
    readBuffer[lenOfMAC] = 0;
    memcpy(readBuffer,array + pointer,lenOfMAC);
    this->MAC = QString(readBuffer);
    pointer += lenOfMAC;

    memset(readBuffer, 0, stringMaxLen + 1);
    memcpy(readBuffer,array + pointer, stringMaxLen);
    this->deviceName = QString(readBuffer);
    pointer += stringMaxLen;

    memset(readBuffer, 0, stringMaxLen + 1);
    memcpy(readBuffer,array + pointer, stringMaxLen);
    this->deviceType = QString(readBuffer);
    pointer += stringMaxLen;

    memset(readBuffer, 0, stringMaxLen + 1);
    memcpy(readBuffer,array + pointer, stringMaxLen);
    this->SSID = QString(readBuffer);
    pointer += stringMaxLen;

    memset(readBuffer, 0, stringMaxLen + 1);
    memcpy(readBuffer,array + pointer, stringMaxLen);
    this->password = QString(readBuffer);
    pointer += stringMaxLen;
};

QByteArray DeviceInfo::toByteArray() const
{
    const uint8_t stringMaxLen = 64;
    const uint8_t lenOfMAC = 8;
    char mac[lenOfMAC];
    char name[stringMaxLen];
    char type[stringMaxLen];
    char ssid[stringMaxLen];
    char pass[stringMaxLen];
    memset(mac, 0, lenOfMAC);
    memset(name, 0, stringMaxLen);
    memset(type, 0, stringMaxLen);
    memset(ssid, 0, stringMaxLen);
    memset(pass, 0, stringMaxLen);

    memcpy(mac , MAC.toStdString().c_str(), lenOfMAC);
    memcpy(name, deviceName.toStdString().c_str(), stringMaxLen);
    memcpy(type, deviceType.toStdString().c_str(), stringMaxLen);
    memcpy(ssid, SSID.toStdString().c_str(), stringMaxLen);
    memcpy(pass, password.toStdString().c_str(),stringMaxLen);

    QByteArray result(lenOfMAC + stringMaxLen * 4, 0);
    result.append(mac, lenOfMAC);
    result.append(name, stringMaxLen);
    result.append(type, stringMaxLen);
    result.append(ssid, stringMaxLen);
    result.append(pass, stringMaxLen);
    return result;
};

Message::Message(const QByteArray& rawBody)
{
    // |(8)sender MAC|(8) addresse MAC|(1) message type|(4)cart size|
    // |cart........................................................|
    const uint8_t lenOfMAC = 8;
    const uint8_t headerSize = 8 + 8 + 1 + 4;
    char mac[lenOfMAC + 1];
    mac[lenOfMAC] = 0;
    memcpy(mac, rawBody.data(), lenOfMAC);
    this->sender = QString(mac);
    memcpy(mac, rawBody.data() + lenOfMAC, lenOfMAC);
    this->addresse = QString(mac);
    char type = 0;
    int size = 0;
    memcpy(&type, rawBody.data() + lenOfMAC + lenOfMAC, 1);
    memcpy(&size, rawBody.data() + lenOfMAC + lenOfMAC + 1, 4);

    switch (type)
    {
        case MessageType::eCREATE_HOST_DEVICE:
        case MessageType::eCREATE_SENSOR_DEVICE:
        {
            this->cart = std::shared_ptr<Cart> (new DeviceInfo(rawBody.data()+ headerSize, size));
            break;
        }
        case MessageType::eDATA_MESSAGE:
        {
            this->cart = std::shared_ptr<Cart> (new DataCart(rawBody.data()+ headerSize, size));
            break;
        }
        case MessageType::eCONFIRM_MESSAGE:
        {
            break;
        }
        default:
        {
            Logger::logError("Message with unknown type has came: " + QString::number(type));
        }
    }

};

Message::Message(Cart* cart)
{
    this->cart = std::shared_ptr<Cart>(cart);
};

QByteArray Message::toByteArray() const
{
    QByteArray result;
    result.append(sender.toStdString().c_str(), 8);
    result.append(addresse.toStdString().c_str(), 8);
    result.append((const char*)&type, 1);
    QByteArray cartArray = cart->toByteArray();
    int cartSize = cartArray.size();
    result.append((const char*)&cartSize, 4);
    result.append(cartArray);
    return result;
};
