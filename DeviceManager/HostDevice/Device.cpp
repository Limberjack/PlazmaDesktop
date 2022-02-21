#include "Device.h"
#include "Logger/Logger.h"

HostDeviceManager* HostDeviceManager::m_pSelfPtr = nullptr;

HostDevice::HostDevice(SerialPort* pSerialPort, Message initialMessage):
    m_pSerialPortInstance(pSerialPort)
{
    DeviceInfo* pInfo = (DeviceInfo*)(initialMessage.cart.get());
    m_sDeviceName = pInfo->deviceName;
    m_wNetworkSettings.password = pInfo->password;
    m_wNetworkSettings.ssid = pInfo->SSID;
};

HostDevice::~HostDevice()
{
    SerialPortLayerManager::freePort(m_pSerialPortInstance->getName());
    delete m_pSerialPortInstance;
};

void HostDevice::onMessageWasReceived(Message message)
{
    Logger::logDbg("Received message " + message);

    if(message.type == eCREATE_SENSOR_DEVICE)
    {
        Device *device = new Device(this, message);
        m_lDevices.push_back(device);
        sendConfirmMessage(message);
        return;
    }
    if(message.addresse == m_sDeviceName && message.sender == DefaultNames::UI)
    {
        if(message.type == eSHUT_DOWN_DEVICE)
        {
            message.sender = DefaultNames::DESKTOP;
            sendMessage(message);
            emit deleteMe(this);
        }
        return;
    }
    if(message.addresse != DefaultNames::DESKTOP)
    {
        Device* pDev = getDeviceByMAC(message.addresse);
        if(pDev)
        {
            pDev->putMessage(message);
        }
        else
        {
            Logger::logError("No device with name <" + message.addresse + "> found");
        }
    }
    else
    {
         //TODO ADD messages resolving swithc case
    }
};

void HostDevice::sendConfirmMessage(const Message& requestMessage)
{
    Message response;
    response.requestId = requestMessage.requestId;
    response.addresse = requestMessage.sender;
    response.sender = requestMessage.addresse;
    response.type = eCONFIRM_MESSAGE;
    m_pSerialPortInstance->send(response);
};

const QString& HostDevice::getDeviceName() const
{
    return m_sDeviceName;
};

const QString& HostDevice::getPortName() const
{
    return m_pSerialPortInstance->getName();
};

bool HostDevice::sendMessage(const Message& msg)
{
    Logger::logDbg("Sending message " + QString::number(msg.type) + " to " + msg.addresse);
    return m_pSerialPortInstance->send(msg);
};

std::list<Device*>& HostDevice::getSensors()
{
    return m_lDevices;
};

Device* HostDevice::getDeviceByMAC(const QString& mac)
{
    for(auto i = m_lDevices.begin(); i != m_lDevices.end(); i++)
    {
        if((*i)->getMAC() == mac)
        {
            return (*i);
        }
    }
    return nullptr;
};

Device::Device(HostDevice* pPort, Message initialMessage): m_pHost(pPort), m_vData(1000)
{
    DeviceInfo* pInfo = (DeviceInfo*)initialMessage.cart.get();
    m_sName = pInfo->deviceName;
    m_sMAC = pInfo->MAC;
    m_sDeviceType = pInfo->deviceType;
    m_sSettings.ssid = pInfo->SSID;
    m_sSettings.password = pInfo->password;
};

void Device::sendMessage(const Message& msg)
{
    Logger::logDbg("Device <" + m_sName + "> sending message " + msg);
    m_pHost->sendMessage(msg);
};

void Device::putMessage(const Message& msg)
{
    Logger::logDbg("Device <" + m_sName + "> received message " + msg);
    switch (msg.type)
    {
        case eDATA_MESSAGE:
        {
            updateData(msg.cart);
            break;
        }
        case eCONFIRM_MESSAGE:
        {
            Logger::logDbg("Request " + QString::number(msg.requestId) + "done");
            break;
        }
        default:
        {
            Logger::logError("No such income message type");
        }
    }
};

const QString& Device::getName()
{
    return m_sName;
};


const QString& Device::getMAC()
{
    return m_sMAC;
};

void Device::updateData(std::shared_ptr<Cart> pCart)
{
    DataCart* pDCart = (DataCart*) pCart.get();
    std::vector<SensorData> vNewData = pDCart->toData();
    m_vData.insert(m_vData.end(), vNewData.begin(), vNewData.end());
    Logger::logData(m_sName, vNewData);
    emit dataUpdated();
};


HostDeviceManager::HostDeviceManager()
{
    if(m_pSelfPtr != nullptr)
    {
        std::runtime_error("HostDeviceManager should be initialized only once");
    }
    m_pSelfPtr = this;
}

HostDeviceManager::~HostDeviceManager()
{
    if(m_pSelfPtr == nullptr)
    {
        return;
    }

    for(auto device: m_lHostDevices)
    {
        delete device;
    }

    m_pSelfPtr = nullptr;
}

void HostDeviceManager::initNewHostdeviceSearch()
{
    for (QSerialPortInfo port : QSerialPortInfo::availablePorts())
    {
        bool isPortTaken = false;
        for(auto activeDevices: m_lHostDevices)
        {
            if(activeDevices->getPortName() == port.portName())
            {
                isPortTaken = true;
                break;
            }
        }

        if(isPortTaken)
        {
            continue;
        }

        SerialPort* tmp = new SerialPort(port);
        if(!tmp->isWorking())
        {
            delete tmp;
        }
    }
    emit hostDeviceSearchOver();
}

void HostDeviceManager::addHostdevice(HostDevice* device)
{
    connect(device, SIGNAL(deleteMe(HostDevice*)), m_pSelfPtr, SLOT(onHostDeviceWaitsForDelete(HostDevice*)));
    m_pSelfPtr->m_lHostDevices.append(device);
    emit m_pSelfPtr->newHostDeviceFound(device);
};


void HostDeviceManager::onHostDeviceWaitsForDelete(HostDevice* device)
{
    for(auto i = m_lHostDevices.begin(); i != m_lHostDevices.end(); i++)
    {
        if((*i)->getPortName() == device->getPortName())
        {
            m_lHostDevices.erase(i);
            device->deleteLater();
        }
    }
}
