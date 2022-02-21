QT       += core gui charts serialport
#QT       += serialport
#QT       += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DeviceManager/HostDevice/Device.cpp \
    DeviceManager/Message/Message.cpp \
    Logger/Logger.cpp \
    SerialPort/SerialPortLayer.cpp \
    UI/UIController.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    DeviceManager/HostDevice/Device.h \
    DeviceManager/Message/Message.h \
    Logger/Logger.h \
    SerialPort/SerialPortLayer.h \
    UI/UIController.h \
    mainwindow.h \

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


#DEFINES += INIT_MESSAGE_TO_HOST=        '\\"init\\"'    \
#    RESET_MESSAGE_TO_HOST=              '\\"rst\\"'     \
#    FINALIZE_MESSAGE_TO_HOST=           '\\"fnls\\"'    \
#    REQUEST_SENSORS_MESSAGE_TO_HOST=    '\\"rqsen\\"'
