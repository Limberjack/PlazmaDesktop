#ifndef UICONTROLLER_H
#define UICONTROLLER_H

#include <QObject>
#include <QList>
#include <QMainWindow>
#include <QChart>
#include <QtCharts/QSplineSeries>

#include "DeviceManager/HostDevice/Device.h"


class UIHostDevice;
class UIDevice;

QT_BEGIN_NAMESPACE
namespace Ui { class UIController; }
QT_END_NAMESPACE

class UIController : public QWidget
{
    Q_OBJECT

    Ui::UIController *ui;
public:
    UIController(QWidget *parent = nullptr);
    ~UIController();

public slots:
    void onHostDeviceInfoUpdate(UIHostDevice* pUIHostDevice);
    void onNewHostDeviceFound(HostDevice* pHostDevice);
    void onHostDeviceSearchOver();

    void onSearchNewHostDeviceBtnClicked();
    void onStopHostDeviceBtnClicked();
    void onRecordSensorBtnClicked();
    void onSettingsSensorBtnClicked();
    void onHostDeviceComboBoxClicked(int hostDeviceId);
    void onSensorsComboBoxClicked(int sensorId);

private:
    QList<UIHostDevice*> m_lHostDevices;
    QList<UIHostDevice*>::iterator m_iCurrentHostDevice;

    HostDeviceManager m_HostDeviceManager;
};

class UIHostDevice : public QObject
{
    Q_OBJECT

public:
    UIHostDevice(HostDevice* hostDevice);
    ~UIHostDevice();

    void putMessageFromUI(const Message& msg);
    HostDevice* getHostDevice(){return m_pHostDevice;}
    UIDevice* getCurrentUISensor(){return m_iCurrentSensor == m_lSensors.end() ? nullptr : *m_iCurrentSensor;}
    UIDevice* getUISensor(const QString& name);
    QStringList getUISensorsNames();
    QString getInfo();

public slots:
    void onInfoUpdate();
    void onSensorInfoUpdate(UIDevice* pUIDevice);

signals:
    void hostDeviceInfoUpdate(UIHostDevice* pUIHostDevice);

private:
    HostDevice* m_pHostDevice = nullptr;

    QList<UIDevice*> m_lSensors;
    QList<UIDevice*>::iterator m_iCurrentSensor;
};


class UIDevice : public QObject
{
    Q_OBJECT

public:
    UIDevice(Device* device);
    ~UIDevice();

    void putMessageFromUI(const Message& msg);
    QtCharts::QChart* getChart(){return m_pChart;};
    void closeChart();
    QString getInfo();
    Device* getDevice(){return m_pDevice;};

public slots:
    void onInfoUpdate();
    void onDataUpdate(std::list<SensorData> newData);

signals:
    void sensorInfoUpdate(UIDevice* pUIDevice);

private:
    Device* m_pDevice = nullptr;
    QtCharts::QChart* m_pChart = nullptr;
    QtCharts::QSplineSeries* m_pSpline = nullptr;

};
#endif // UICONTROLLER_H
