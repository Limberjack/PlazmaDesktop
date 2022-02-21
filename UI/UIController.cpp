#include "UIController.h"
#include "ui_mainwindow.h"
#include <QChartView>

#include <Logger/Logger.h>

UIController::UIController(QWidget *parent):
    QWidget(parent),
    ui(new Ui::UIController)
{
    LOGDBG("Enter");
    ui->setupUi(this);
    connect(&m_HostDeviceManager, SIGNAL(newHostDeviceFound(HostDevice*)), this, SLOT(onNewHostDeviceFound(HostDevice*)));
    connect(&m_HostDeviceManager, SIGNAL(hostDeviceSearchOver()), this, SLOT(onHostDeviceSearchOver()));

    connect(ui->findeNewSystemsBtn, SIGNAL(clicked(bool)), this, SLOT(onSearchNewHostDeviceBtnClicked()));
    connect(ui->stopSelectedSystemBtn, SIGNAL(clicked(bool)), this, SLOT(onStopHostDeviceBtnClicked()));

    connect(ui->recordSensorToFileBnt, SIGNAL(clicked(bool)), this, SLOT(onRecordSensorBtnClicked()));
    connect(ui->settingBtn, SIGNAL(clicked(bool)), this, SLOT(onSettingsSensorBtnClicked()));

    connect(ui->systemsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onHostDeviceComboBoxClicked(int)));
    connect(ui->sensorsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onSensorsComboBoxClicked(int)));
}

UIController::~UIController()
{
    for(auto i: m_lHostDevices)
    {
        delete i;
    }
};

void UIController::onHostDeviceInfoUpdate(UIHostDevice* pUIHostDevice)
{
    LOGDBG("Enter");
    if(pUIHostDevice == *m_iCurrentHostDevice)
    {
        LOGDBG("Updating " + pUIHostDevice->getHostDevice()->getDeviceName());
        ui->systemDescriptionTextBrowser->setText((*m_iCurrentHostDevice)->getInfo());
    }
};

void UIController::onNewHostDeviceFound(HostDevice* pHostDevice)
{
    LOGDBG("Enter");
    UIHostDevice* newHost = new UIHostDevice(pHostDevice);
    m_lHostDevices.append(newHost);
};

void UIController::onHostDeviceSearchOver()
{
    LOGDBG("Enter");
    ui->systemDescriptionTextBrowser->setText("Systems search over");
};

void UIController::onSearchNewHostDeviceBtnClicked()
{
    LOGDBG("Enter");
    ui->systemDescriptionTextBrowser->setText("Starting for systems searching");
};

void UIController::onStopHostDeviceBtnClicked()
{
    Message msg;
    HostDevice* hostDevice = (*m_iCurrentHostDevice)->getHostDevice();
    msg.addresse = hostDevice->getDeviceName();
    msg.sender = DefaultNames::UI;
    msg.type = MessageType::eSHUT_DOWN_DEVICE;

    LOGDBG("Sending shutdown to " + msg.addresse);
    (*m_iCurrentHostDevice)->putMessageFromUI(msg);

    ui->sensorsComboBox->clear();
    ui->sensorDescriptionTextBrowser->clear();
    ui->scrollArea->widget()->deleteLater();

    ui->systemsComboBox->removeItem(ui->systemsComboBox->currentIndex());

    if(ui->systemsComboBox->children().empty())
    {
        LOGINF("No systems are available any more");
        m_iCurrentHostDevice = m_lHostDevices.end();
        ui->systemDescriptionTextBrowser->setText("No systems are running");
    }
};

void UIController::onRecordSensorBtnClicked()
{
    LOGDBG("Enter");
};

void UIController::onSettingsSensorBtnClicked()
{
    LOGDBG("Enter");
};

void UIController::onHostDeviceComboBoxClicked(int hostDeviceId)
{
    QString deviceName = ui->systemsComboBox->itemText(hostDeviceId);
    LOGINF("New host device is " + QString::number(hostDeviceId) + " - " + deviceName);

    for(auto i = m_lHostDevices.begin(); i != m_lHostDevices.end(); i++)
    {
        if((*i)->getHostDevice()->getDeviceName() == deviceName)
        {
            m_iCurrentHostDevice = i;
            ui->systemDescriptionTextBrowser->setText((*i)->getInfo());
            ui->sensorsComboBox->clear();
            auto newSensors = (*i)->getUISensorsNames();
            ui->sensorsComboBox->addItems(newSensors);
            if(!newSensors.empty())
            {
                ui->sensorsComboBox->setCurrentIndex(0);
            }
            return;
        }
    }
    LOGERR("No such host device in system")
};

void UIController::onSensorsComboBoxClicked(int sensorId)
{
    auto currentDevice = *m_iCurrentHostDevice;
    QString sensorName = ui->sensorsComboBox->itemText(sensorId);

    LOGINF("New sensor is " + QString::number(sensorId) + " - " + sensorName);

    auto newSensor = currentDevice->getUISensor(sensorName);

    if(newSensor == nullptr)
    {
        LOGERR("No such sensor in system");
        ui->sensorDescriptionTextBrowser->setText("No such sensor in system");
        return;
    }
    ui->scrollArea->widget()->deleteLater();
    ui->scrollArea->setWidget(new QtCharts::QChartView(newSensor->getChart()));
    ui->sensorDescriptionTextBrowser->setText(newSensor->getInfo());
};


UIHostDevice::UIHostDevice(HostDevice* hostDevice):
    m_pHostDevice(hostDevice)
{
    LOGDBG("Creating new UI host device with name " + hostDevice->getDeviceName())
    for(auto i : m_pHostDevice->getSensors())
    {
        m_lSensors.append(new UIDevice(i));
    }
}

UIHostDevice::~UIHostDevice()
{
    for(auto i : m_pHostDevice->getSensors())
    {
        delete i;
    }
};

void UIHostDevice::putMessageFromUI(const Message& msg)
{
    m_pHostDevice->onMessageWasReceived(msg);
};

UIDevice* UIHostDevice::getUISensor(const QString& name)
{
    for(auto i: m_lSensors)
    {
        if(i->getDevice()->getName() == name)
        {
            return i;
        }
    }
    LOGERR("No such sensor in " + m_pHostDevice->getDeviceName());
    return nullptr;
};

QStringList UIHostDevice::getUISensorsNames()
{
    QStringList result;
    for(auto i: m_lSensors)
    {
        result.append(i->getDevice()->getName());
    }
    return result;
};

QString UIHostDevice::getInfo()
{
    QString info("Device name: " + m_pHostDevice->getDeviceName() + "\nPort name: " +  m_pHostDevice->getPortName());
    WiFiSettings settings = m_pHostDevice->getNetworkSettings();

    info += "\nSSID: " + settings.ssid + "\n";
    info += "PWD: " + settings.password;
    return info;
};

void UIHostDevice::onInfoUpdate()
{
    LOGDBG("Host deivce info update " + m_pHostDevice->getDeviceName())
    emit hostDeviceInfoUpdate(this);
};

void UIHostDevice::onSensorInfoUpdate(UIDevice* pUIDevice)
{
    LOGDBG("Sensor deivce info update in " + m_pHostDevice->getDeviceName());
};


UIDevice::UIDevice(Device* device):
    m_pDevice(device)
{

};
UIDevice::~UIDevice();

void UIDevice::putMessageFromUI(const Message& msg);
QtCharts::QChart* UIDevice::getChart(){return m_pChart;};
void UIDevice::closeChart();
QString UIDevice::getInfo();
Device* UIDevice::getDevice(){return m_pDevice;};

void UIDevice::onInfoUpdate();
void UIDevice::onDataUpdate(std::list<SensorData> newData);
