#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QMap>
#include <list>
#include <queue>
#include <mutex>
#include <memory>
#include "DeviceManager/Message/Message.h"


#ifndef __FUNCTION_NAME__
    #ifdef WIN32   //WINDOWS
        #define __FUNCTION_NAME__   __FUNCTION__
    #else          //*NIX
        #define __FUNCTION_NAME__   __func__
    #endif
#endif


#define LOGDBG(str) Logger::logDbg(QString(__FUNCTION_NAME__) + ": " + str);
#define LOGERR(str) Logger::logError(QString(__FUNCTION_NAME__) + ": " + str);
#define LOGINF(str) Logger::logInfo(QString(__FUNCTION_NAME__) + ": " + str);

enum LogType{
    INFO,
    DBG,
    ERR,
    DATA,

    _LogType_Len
};

class Logger
{
    class DataLogger;
    class CommonLogger;

    Logger();
    static Logger* m_pInstance;
    DataLogger* m_pDataLogger;
    CommonLogger* m_pCommonLogger;
    const QString m_sFolderName;
    bool m_aFlags[LogType::_LogType_Len];

    static Logger* getInstance();
public:
    static void enableFlag(LogType flag);
    static void disableFalg(LogType flag);

    static void logError(const QString& message);
    static void logInfo(const QString& message);
    static void logDbg(const QString& message);
    static void logData(const QString& sensorName, const SensorData& value);
    static void logData(const QString& sensorName, const std::list<QString>& value);
    static void logData(const QString& sensorName, const std::vector<SensorData>& values);

    static void flush();

    static const QString& getLogDirName();
};

class Logger::DataLogger
{
public:
    class DataLog
    {
        int m_iMaxLogAmount;
        std::shared_ptr<std::mutex> m_pMutex;
        QFile *m_fOutputFile;
        std::vector<SensorData> m_qRecords;
    public:
        DataLog(const QString& sensorName);
        DataLog() = default;
        ~DataLog();
        void enable();
        void insert(const std::vector<SensorData>& value);
        void insert(const SensorData& value);
        void flush();
    };

private:
    void addFile(const QString& sensorName);
    QMap<QString, DataLog> m_mDataMap;
public:
    DataLogger() = default;
    ~DataLogger();

    void insert(const QString& sensorName, const SensorData& value);
    void insert(const QString& sensorName, const std::vector<SensorData>& value);
    void flush();
};

class Logger::CommonLogger
{
    QFile m_fOutputFile;
    std::queue<QString> m_qLogs;
    std::mutex m_mMutex;
public:
    CommonLogger();
    ~CommonLogger();
    void insert(const QString& row);
    void flush();
};

#endif // LOGGER_H
