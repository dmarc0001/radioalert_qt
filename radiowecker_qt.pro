###############################################################################
####                                                                       ####
#### Radiowecker Dämon für BOSe Soundtouch Geräte (2018)                   ####
####                                                                       ####
###############################################################################

MAJOR                                  = 0
MINOR                                  = 1
PATCH                                  = 7
BUILD                                  = 0

win32:VERSION                          = $${MAJOR}.$${MINOR}.$${PATCH}.$${BUILD} # major.minor.patch.build
else:VERSION                           = $${MAJOR}.$${MINOR}.$${PATCH}    # major.minor.patch

DEFINES                                += QT_DEPRECATED_WARNINGS
DEFINES                                += $$DEBUG
DEFINES                                += VMAJOR=$$MAJOR
DEFINES                                += VMINOR=$$MINOR
DEFINES                                += VPATCH=$$PATCH

QT                                     += core
QT                                     -= gui
QT                                     += network
QT                                     += websockets
QT                                     += xml

CONFIG                                 += stl
CONFIG                                 += c++11
CONFIG                                 += console

TARGET                                 = alert_daemon
TEMPLATE                               = app

target.path                            = /home/pi/qt5pi/alert_daemon
INSTALLS                               += target

CONFIG(release, debug|release) {
  DEFINES                              += QT_NO_DEBUG_OUTPUT
  DESTDIR                              = rout
  contains(CONFIG, RASPI) {
    message( RASPI BUILD )
    LIBS                               += -L$$PWD/rlib/RASPI -lsoundtouch_qt
  } else {
    message( LOCAL BUILD )
    LIBS                               += -L$$PWD/rlib/X86_64 -lsoundtouch_qt
  }
  DEFINES                              += QT_NO_DEBUG_OUTPUT
}
CONFIG(debug, debug|release) {
  DESTDIR                              = dout
  contains(CONFIG, RASPI) {
    message( RASPI BUILD )
    LIBS                               += -L$$PWD/dlib/RASPI -lsoundtouch_qt
  } else {
    message( LOCAL BUILD )
    LIBS                               += -L$$PWD/dlib/X86_64 -lsoundtouch_qt
  }
}


MOC_DIR                                = moc
RCC_DIR                                = rcc
UI_DIR                                 = ui

message( radio alert daemon version: $$VERSION )

SOURCES += \
    src/maindaemon.cpp \
    src/main.cpp \
    src/logging/Logger.cpp \
    src/logging/loggingthreshold.cpp \
    src/config/singlealertconfig.cpp \
    src/config/alertconfig.cpp \
    src/config/appconfigclass.cpp \
    src/utils/configfilenotexistexception.cpp \
    src/radioalertthread.cpp \
    src/config/config_all.cpp \
    src/config/availabledevices.cpp

HEADERS += \
    src/maindaemon.hpp \
    src/logging/Logger.hpp \
    src/logging/loggingthreshold.hpp \
    src/config/singlealertconfig.hpp \
    src/config/alertconfig.hpp \
    src/config/appconfigclass.hpp \
    src/main.hpp \
    src/utils/configfilenotexistexception.hpp \
    src/radioalertthread.hpp \
    src/global_config.hpp \
    src/config/config_all.hpp \
    src/config/availabledevices.hpp

DISTFILES += \
    alert_daemon.ini \
    available_devices.ini
