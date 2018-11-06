###############################################################################
####                                                                       ####
#### Radiowecker Dämon für BOSe Soundtouch Geräte (2018)                   ####
####                                                                       ####
###############################################################################

MAJOR                                  = 0
MINOR                                  = 1
PATCH                                  = 10
BUILD                                  = 0

LIBSOUNDTOUCHDIR                       = $$PWD/../soundtouchlib_qt

win32:VERSION                          = $${MAJOR}.$${MINOR}.$${PATCH}.$${BUILD} # major.minor.patch.build
else:VERSION                           = $${MAJOR}.$${MINOR}.$${PATCH}    # major.minor.patch

DEFINES                                += SOUNDTOUCH_QT_LIB_LIBRARY
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

TEMPLATE                               = app
MOC_DIR                                = moc
RCC_DIR                                = rcc
UI_DIR                                 = ui
DESTDIR                                = out

# fuer LIBRARY
INCLUDEPATH                            += $${LIBSOUNDTOUCHDIR}/include
DEPENDPATH                             += $${LIBSOUNDTOUCHDIR}/include

target.path                            = /home/pi/qt5pi/alert_daemon
INSTALLS                               += target

CONFIG(release, debug|release) {
  TARGET                               = alert_daemon
  DEFINES                              += QT_NO_DEBUG_OUTPUT
  # raspi oder lokal
  contains(CONFIG, RASPI) {
    message( RASPI BUILD )
    LIBS                               += -L$${LIBSOUNDTOUCHDIR}/lib/QT-5112-RASPIGCC
  } else {
    message( LOCAL BUILD )
    LIBS                               += -L$${LIBSOUNDTOUCHDIR}/lib/QT-5112-GCC-64
  }
  LIBS                                 += -lsoundtouch_qt
  DEFINES                              += QT_NO_DEBUG_OUTPUT
}
CONFIG(debug, debug|release) {
  TARGET                                 = alert_daemon_d
  # raspi oder lokal
  contains(CONFIG, RASPI) {
    message( RASPI BUILD )
    LIBS                               += -L$${LIBSOUNDTOUCHDIR}/lib/QT-5112-RASPIGCC
  } else {
    message( LOCAL BUILD )
    LIBS                               += -L$${LIBSOUNDTOUCHDIR}/lib/QT-5112-GCC-64
  }
  #LIBS                                 += -lsoundtouch_debug_qt
  LIBS                                 += -lsoundtouch_qt
  DEFINES                              += DEBUG
}

message( radio alert daemon version: $$VERSION )

SOURCES += \
    src/maindaemon.cpp \
    src/main.cpp \
    src/logging/loggingthreshold.cpp \
    src/config/singlealertconfig.cpp \
    src/config/alertconfig.cpp \
    src/config/appconfigclass.cpp \
    src/utils/configfilenotexistexception.cpp \
    src/config/config_all.cpp \
    src/config/availabledevices.cpp \
    src/logging/logger.cpp \
    src/utils/noavailiblesounddeviceexception.cpp \
    src/singleradioalert.cpp \
    src/abstract/asyncalertcommand.cpp \
    src/udpcontrolprocess.cpp

HEADERS += \
    src/maindaemon.hpp \
    src/logging/loggingthreshold.hpp \
    src/config/singlealertconfig.hpp \
    src/config/alertconfig.hpp \
    src/config/appconfigclass.hpp \
    src/main.hpp \
    src/utils/configfilenotexistexception.hpp \
    src/global_config.hpp \
    src/config/config_all.hpp \
    src/config/availabledevices.hpp \
    src/logging/logger.hpp \
    src/utils/noavailiblesounddeviceexception.hpp \
    src/singleradioalert.hpp \
    src/abstract/asyncalertcommand.hpp \
    src/udpcontrolprocess.hpp

DISTFILES += \
    alert_daemon.ini \
    available_devices.ini

