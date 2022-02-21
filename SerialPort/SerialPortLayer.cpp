#include "SerialPortLayer.h"

#include <Logger/Logger.h>
#include <DeviceManager/HostDevice/Device.h>

SerialPort::SerialPort(QSerialPortInfo& portInfo):
    QObject(nullptr),
    m_Port(portInfo),
    m_bWorking(false)
{
   m_Port.open(QIODevice::ReadWrite);
   m_Port.clear();
   if(sendInitMessage())
   {
       m_bWorking = true;
       Logger::logInfo("Found connection on a port with name: " + m_Port.portName());
   }
   else
   {
       return;
   }
   m_sPortName = m_Port.portName();
   connect(&m_Port, SIGNAL(readyRead()), this, SLOT(onReadReady()));
   connect(&m_Port, SIGNAL(errorOccurred(QSerialPort::SerialPortError error)), this, SLOT(onError(QSerialPort::SerialPortError error)));
};

SerialPort::~SerialPort()
{
    sendFinalizeMessage();
    m_Port.close();
};

bool SerialPort::send(const Message& message)
{
    return m_Port.write(message.toByteArray()) != -1;
};

bool SerialPort::sendInitMessage()
{
    Message msg;
    msg.type = MessageType::eSTART_DEVICE;
    msg.addresse = DefaultNames::HOST_DEVICE;
    msg.sender = DefaultNames::DESKTOP;

    return send(msg);
};

bool SerialPort::sendFinalizeMessage()
{
    Message msg;
    msg.type = MessageType::eSHUT_DOWN_DEVICE;
    msg.addresse = DefaultNames::BROADCAST;
    msg.sender = DefaultNames::DESKTOP;

    return send(msg);
};

void SerialPort::onReadReady()
{
    QByteArray message(m_Port.readAll());
    Logger::logDbg("On port " + m_Port.portName() + " received message: " + message);
    uint8_t bracketCounter = 0;
    m_sRawMessage += message;
    for(auto i = m_sRawMessage.begin(); i != m_sRawMessage.end(); i++)
    {
        if (*i == '{')
        {
            bracketCounter++;
        }
        else if (*i == '}')
        {
            bracketCounter--;
        }
    }
    if (bracketCounter == 0)
    {
        Message msg(message);
        switch(msg.type)
        {
            case eCREATE_HOST_DEVICE:
            {
                HostDevice* dev = new HostDevice(this, msg);
                connect(this, SIGNAL(messageReceived(Message)), dev, SLOT(messageWasReceived(Message)));
                HostDeviceManager::addHostdevice(dev);
                break;
            }
            default:
            {
                emit messageReceived(msg);
            }
        }
    }
};

void SerialPort::onError(QSerialPort::SerialPortError error)
{
    if(error != QSerialPort::SerialPortError::NoError)
    {
        Logger::logError("Error on port " + m_Port.portName() + ": " + m_Port.errorString());
    }
    switch (error)
    {
    case QSerialPort::SerialPortError::DeviceNotFoundError:
    case QSerialPort::SerialPortError::BreakConditionError:
        m_bWorking = false;
        break;
    default:
        ;
    }
};

const QString& SerialPort::getName()
{
    return m_sPortName;
};

bool SerialPort::isWorking() const
{
    return m_bWorking;
};

SerialPortLayerManager* SerialPortLayerManager::getInstance()
{
    static SerialPortLayerManager* inst = nullptr;
    if(!inst)
    {
        inst = new SerialPortLayerManager();
    }
    return inst;
};

void SerialPortLayerManager::freePort(const QString& portName)
{
    auto inst = SerialPortLayerManager::getInstance();
    inst->m_lTakenPorts.remove(portName);

};


bool SerialPortLayerManager::isPortTaken(const QString& portName)
{
    for(auto i = m_lTakenPorts.begin(); i != m_lTakenPorts.end(); i++)
    {
        if(i->compare(portName) == 0)
        {
            return true;
        }
    }
    return false;
};

void SerialPortLayerManager::updatePortList()
{
    auto inst = SerialPortLayerManager::getInstance();
    for (auto i : QSerialPortInfo::availablePorts())
    {
        if(inst->isPortTaken(i.portName()))
        {
            continue;
        }
        SerialPort* tmpPort = new SerialPort(i);
        if(tmpPort->isWorking())
        {
            SerialPortLayerManager::getInstance()->m_lTakenPorts.push_back(i.portName());
            //yeah, this looks like a memory leak, but it's not
        }
        else
        {
            delete tmpPort;
        }

    }
};
