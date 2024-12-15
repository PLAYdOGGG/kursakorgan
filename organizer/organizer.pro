QT += core gui widgets network networkauth

CONFIG += c++17

TARGET = organizer
TEMPLATE = app

SOURCES += \
    googlecalendarsync.cpp \
    main.cpp \
    mainwindow.cpp \
    event.cpp \
    task.cpp

HEADERS += \
    googlecalendarsync.h \
    mainwindow.h \
    event.h \
    task.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += $$PWD
