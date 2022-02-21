#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <SerialPort/SerialPortLayer.h>
#include <DeviceManager/Message/Message.h>

class HostDevice;
class Device;


class Device : public QObject
{
    Q_OBJECT
public:
    Device(HostDevice* pPort, Message initialMessage);
    void messageWasReceived(Message message);
    void sendMessage(const Message& msg);
    void putMessage(const Message& msg);
    const QString& getName();
    const QString& getMAC();

signals:
    void dataUpdated();

private:
    void updateData(std::shared_ptr<Cart> pCart);
    HostDevice* m_pHost;
    QString m_sName;
    QString m_sMAC;
    WiFiSettings m_sSettings;
    QString m_sDeviceType;
    std::vector<SensorData> m_vData;
    std::vector<SensorData>::iterator m_iDataIterator;
};

class HostDevice : public QObject
{
    Q_OBJECT
public:
    HostDevice(SerialPort* pSerialPort, Message initialMessage);
    ~HostDevice();

    const QString& getDeviceName() const;
    const QString& getPortName() const;
    bool sendMessage(const Message& msg);
    std::list<Device*>& getSensors();
    void sendConfirmMessage(const Message& requestMessage);
    WiFiSettings getNetworkSettings(){return m_wNetworkSettings;}
public slots:
    void onMessageWasReceived(Message message);

signals:
    void deleteMe(HostDevice*);

private:
    Device* getDeviceByMAC(const QString& name);

    SerialPort* m_pSerialPortInstance = nullptr;
    QString m_sDeviceName;
    std::list<Device*> m_lDevices;
    WiFiSettings m_wNetworkSettings;
};


class HostDeviceManager: public QObject
{
    Q_OBJECT
public:
    void initNewHostdeviceSearch();
    static void addHostdevice(HostDevice* device);
    HostDeviceManager();
    ~HostDeviceManager();

public slots:
    void onHostDeviceWaitsForDelete(HostDevice* device);

signals:
    void newHostDeviceFound(HostDevice* pHostDevice);
    void hostDeviceSearchOver();

private:
    static HostDeviceManager* m_pSelfPtr;
    QList<HostDevice*> m_lHostDevices;
};
#endif // DEVICE_H
