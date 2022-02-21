#include "Logger.h"

#include <QDateTime>
#include <QDir>

#include <QThread>

Logger* Logger::m_pInstance = nullptr;

Logger::Logger() :
    m_pDataLogger(new Logger::DataLogger()),
    m_pCommonLogger(new Logger::CommonLogger()),
    m_sFolderName(QDateTime::currentDateTime().toString("dd.MM.yyyy-hh:mm:ss"))
{
    QDir dir;
    dir.mkdir(m_sFolderName);
    m_aFlags[LogType::INFO] = true;
    m_aFlags[LogType::ERR] = true;
    m_aFlags[LogType::DBG] = false;
    m_aFlags[LogType::DBG] = true;
};

Logger* Logger::getInstance()
{
    if(m_pInstance != nullptr)
    {
        return m_pInstance;
    }
    m_pInstance = new Logger();
    return m_pInstance;
};

void Logger::enableFlag(LogType flag)
{
    getInstance()->m_aFlags[flag] = true;
};

void Logger::disableFalg(LogType flag)
{
    getInstance()->m_aFlags[flag] = false;
};

void Logger::logError(const QString& message)
{
    auto instance = getInstance();
    if(instance->m_aFlags[LogType::ERR])
    {
        QString record = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zz") + ",ERR," +
                QString::number((long long int)(QThread::currentThreadId())) + ",\"" + message+"\"\n";
        instance->m_pCommonLogger->insert(record);
    }
};

void Logger::logInfo(const QString& message)
{
    auto instance = getInstance();
    if(instance->m_aFlags[LogType::INFO])
    {
        QString record = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zz") + ",INFO," +
                QString::number((long long int)(QThread::currentThreadId())) + ",\"" + message+"\"\n";
        instance->m_pCommonLogger->insert(record);
    }
};

void Logger::logDbg(const QString& message)
{
    auto instance = getInstance();
    if(instance->m_aFlags[LogType::DBG])
    {
        QString record = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zz") + ",DBG," +
                QString::number((long long int)(QThread::currentThreadId())) + ",\"" + message+"\"\n";
        instance->m_pCommonLogger->insert(record);
    }
};

void Logger::logData(const QString& sensorName, const SensorData& value)
{
    getInstance()->m_pDataLogger->insert(sensorName, value);
};

void Logger::logData(const QString& sensorName, const std::vector<SensorData>& value)
{
    getInstance()->m_pDataLogger->insert(sensorName, value);
};

const QString& Logger::getLogDirName()
{
  return getInstance()->m_sFolderName;
};

void Logger::flush()
{
    Logger* instance = getInstance();
    instance->m_pCommonLogger->flush();
    instance->m_pDataLogger->flush();
};

Logger::DataLogger::~DataLogger()
{
    flush();
};

void Logger::DataLogger::insert(const QString& sensorName, const SensorData& value)
{
    auto mapPointer = m_mDataMap.find(sensorName);
    if(mapPointer == m_mDataMap.end())
    {
        addFile(sensorName);
        mapPointer = m_mDataMap.find(sensorName);
    }
    mapPointer.value().insert(value);
};

void Logger::DataLogger::insert(const QString& sensorName, const std::vector<SensorData>& value)
{
    auto mapPointer = m_mDataMap.find(sensorName);
    if(mapPointer == m_mDataMap.end())
    {
        addFile(sensorName);
        mapPointer = m_mDataMap.find(sensorName);
    }
    mapPointer.value().insert(value);
};

void Logger::DataLogger::flush()
{
    for(auto dataColumnIter = m_mDataMap.begin(); dataColumnIter != m_mDataMap.end(); dataColumnIter++)
    {
        dataColumnIter->flush();
        Logger::logDbg("Flushed sensor with name: " + dataColumnIter.key());
    }
};

void Logger::DataLogger::addFile(const QString& sensorName)
{
    DataLog newLogger(sensorName);
    m_mDataMap.insert(sensorName, newLogger)->enable();
};

Logger::CommonLogger::CommonLogger()
{
    QString fileName = QDateTime::currentDateTime().toString(Logger::getLogDirName() + "/logs.csv");
    m_fOutputFile.setFileName(fileName);
    m_fOutputFile.open(QIODevice::WriteOnly);
};

Logger::CommonLogger::~CommonLogger()
{
    flush();
    m_fOutputFile.close();
};

void Logger::CommonLogger::insert(const QString& row)
{
    m_mMutex.lock();
    m_qLogs.push(row);
    m_mMutex.unlock();

    if(m_qLogs.size() > 500)
    {
        flush();
    }
};

void Logger::CommonLogger::flush()
{
    std::lock_guard<std::mutex>guard(m_mMutex);
    while(!m_qLogs.empty())
    {
        m_fOutputFile.write(m_qLogs.front().toUtf8());
        m_qLogs.pop();
    }
    m_fOutputFile.flush();
};

Logger::DataLogger::DataLog::DataLog(const QString& sensorName):
    m_iMaxLogAmount(500),
    m_pMutex(new std::mutex())
{
    m_fOutputFile->setFileName(Logger::getLogDirName() + "/" + sensorName + ".csv");
};


void Logger::DataLogger::DataLog::enable()
{
    m_fOutputFile->open(QIODevice::WriteOnly);
};

Logger::DataLogger::DataLog::~DataLog()
{
    flush();
    m_fOutputFile->close();
    delete m_fOutputFile;
};

void Logger::DataLogger::DataLog::insert(const std::vector<SensorData>& value)
{
    m_pMutex->lock();
    for(auto i = value.begin(); i != value.end(); i++)
    {
        m_qRecords.push_back(*i);
    }
    m_pMutex->unlock();
    if(m_qRecords.size() > m_iMaxLogAmount)
    {
        flush();
    }
};

void Logger::DataLogger::DataLog::insert(const SensorData& value)
{
    m_pMutex->lock();
    m_qRecords.push_back(value);
    m_pMutex->unlock();

    if(m_qRecords.size() > m_iMaxLogAmount)
    {
        flush();
    }
};

void Logger::DataLogger::DataLog::flush()
{
    std::lock_guard<std::mutex> lock(*m_pMutex);
    for(auto i = 0; i < m_qRecords.size(); i++)
    {
        m_fOutputFile->write((m_qRecords[i].toString() + "\n").toUtf8());
    }
    m_qRecords.clear();
    m_fOutputFile->flush();
};
