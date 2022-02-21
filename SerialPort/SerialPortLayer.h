#ifndef SERIALPORTLAYER_H
#define SERIALPORTLAYER_H

#include <QtSerialPort/QtSerialPort>

#include <QObject>
#include <QString>
#include <QThread>
#include <QMutex>
#include <DeviceManager/Message/Message.h>

#include "memory"

class SerialPort;
class SerialPortLayerManager;

class SerialPort : public QObject
{
    Q_OBJECT
public:
    SerialPort(const SerialPort&) = delete;
    SerialPort& operator = (const SerialPort&) = delete;
    SerialPort(QSerialPortInfo& portInfo);
    ~SerialPort();

    bool send(const Message& message);
    bool sendInitMessage();
    bool sendFinalizeMessage();
    bool isWorking() const;
    const QString &getName();
signals:
    void messageReceived(Message msg);

private slots:
    void onReadReady();
    void onError(QSerialPort::SerialPortError error);

private:

    QByteArray m_sRawMessage;
    QSerialPort m_Port;
    bool m_bWorking;
    QString m_sPortName;
    const QString m_sRestartMessage;
    const QString m_sInitMessage;
    const QString m_sFinalizeMessage;
    const QString m_sRequestSensorsList;
};

class SerialPortLayerManager
{
    SerialPortLayerManager() = default;
    static SerialPortLayerManager* getInstance();
    bool isPortTaken(const QString& portName);

    std::list<QString> m_lTakenPorts;
public:

    static void updatePortList();
    static void freePort(const QString& portName);
};

#endif // SERIALPORTLAYER_H
