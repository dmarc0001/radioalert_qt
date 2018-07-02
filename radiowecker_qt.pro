###############################################################################
####                                                                       ####
#### Radiowecker Dämon für BOSe Soundtouch Geräte (2018)                   ####
####                                                                       ####
###############################################################################

MAJOR                                  = 1
MINOR                                  = 0
PATCH                                  = 0
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
  LIBS                                 += -L$$PWD/rlib -lsoundtouch_qt
  DEFINES                              += QT_NO_DEBUG_OUTPUT
}
CONFIG(debug, debug|release) {
  DESTDIR                              = dout
  LIBS                                 += -L$$PWD/dlib -lsoundtouch_qt
}

MOC_DIR                                = moc
RCC_DIR                                = rcc
UI_DIR                                 = ui

message( radio alert daemon version $$VERSION )


SOURCES += \
    src/maindaemon.cpp \
    src/main.cpp \
    src/config/AppConfigClass.cpp \
    src/logging/Logger.cpp \
    src/logging/loggingthreshold.cpp \
    src/config/singlealertconfig.cpp \
    src/config/globalconfig.cpp

HEADERS += \
    src/maindaemon.hpp \
    src/config/AppConfigClass.hpp \
    src/logging/Logger.hpp \
    src/logging/loggingthreshold.hpp \
    src/config/singlealertconfig.hpp \
    src/config/globalconfig.hpp

DISTFILES += \
    alert_daemon.ini
