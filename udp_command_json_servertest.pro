
DEFINES                                += QT_DEPRECATED_WARNINGS

QT                                     += core
QT                                     -= gui
QT                                     += network
QT                                     += xml
CONFIG                                 -= app_bundle

CONFIG                                 += stl
CONFIG                                 += c++11
CONFIG                                 += console
TARGET                                 = udp_command_test

TEMPLATE                               = app
MOC_DIR                                = moc
RCC_DIR                                = rcc
UI_DIR                                 = uitest
DESTDIR                                = out

INCLUDEPATH                            += $${PWD}/src

SOURCES += \
    src/udpcontrolprocess.cpp \
    src/logging/loggingthreshold.cpp \
    src/logging/logger.cpp \
    src/config/singlealertconfig.cpp \
    src/config/config_all.cpp \
    src/config/availabledevices.cpp \
    src/config/appconfigclass.cpp \
    src/config/alertconfig.cpp \
    src/utils/noavailiblesounddeviceexception.cpp \
    src/utils/configfilenotexistexception.cpp \
    test/testmain.cpp

HEADERS += \
    src/udpcontrolprocess.hpp \
    src/logging/loggingthreshold.hpp \
    src/logging/logger.hpp \
    src/config/singlealertconfig.hpp \
    src/config/config_all.hpp \
    src/config/availabledevices.hpp \
    src/config/appconfigclass.hpp \
    src/config/alertconfig.hpp \
    src/utils/noavailiblesounddeviceexception.hpp \
    src/utils/configfilenotexistexception.hpp \

