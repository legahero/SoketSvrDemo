QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    httpbase/http_parser.c \
    httpbase/httphandler.cpp \
    httpbase/qasynhttpsocket.cpp \
    httpbase/qasyntcpserver.cpp \
    httpbase/qasyntcpsocket.cpp \
    httpbase/qcfgmanager.cpp \
    httpbase/qconnectpool.cpp \
    httpbase/qhttprequest.cpp \
    httpbase/qhttpresponse.cpp \
    httpbase/qhttpserver.cpp \
    httpbase/staticfilecontroller.cpp \
    httpbase/threadhandle.cpp \
    main.cpp \
    qmoniserver.cpp \
    soketsvrmainwindow.cpp

HEADERS += \
    httpbase/http_parser.h \
    httpbase/httphandler.h \
    httpbase/qasynhttpsocket.h \
    httpbase/qasyntcpserver.h \
    httpbase/qasyntcpsocket.h \
    httpbase/qcfgmanager.h \
    httpbase/qconnectpool.h \
    httpbase/qhttprequest.h \
    httpbase/qhttpresponse.h \
    httpbase/qhttpserver.h \
    httpbase/qhttpserverfwd.h \
    httpbase/staticfilecontroller.h \
    httpbase/threadhandle.h \
    qmoniserver.h \
    soketsvrmainwindow.h

FORMS += \
    soketsvrmainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
