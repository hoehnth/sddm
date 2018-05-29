#win32: error("Windows build not supported !!!!")

TARGET = PamAuthTest

TEMPLATE = app

#QT += core
QT -= gui

CONFIG += qt console c++11 debug_and_release

# for Backend.cpp
DEFINES += USE_PAM

LIBS += -lpam -lcrypt

#PAM_LIBRARIES = -lpam
#contains(DEFINES, USE_PAM) {
#LIBS += $$PAM_LIBRARIES
#} else {
#LIBS += -lcrypt
#error("password backend not yet supported!")
#}

INCLUDEPATH += auth helper

SOURCES += \
    auth/Auth.cpp \
    auth/AuthPrompt.cpp \
    auth/AuthRequest.cpp \
    helper/Backend.cpp \
    helper/HelperApp.cpp \
    helper/UserSession.cpp \
    helper/backend/PamBackend.cpp \
    helper/backend/PamHandle.cpp \
    helper/backend/PasswdBackend.cpp \
    helper/Utils.cpp


HEADERS += \
    auth/Auth.h \
    auth/AuthBase.h \
    auth/AuthEnums.h \
    auth/AuthPrompt.h \
    auth/AuthRequest.h \
    helper/Backend.h \
    helper/HelperApp.h \
    helper/UserSession.h \
    helper/backend/PamWorkState.h \
    helper/backend/PamBackend.h \
    helper/backend/PamHandle.h \
    helper/backend/PasswdBackend.h \
    helper/Utils.h

DISTFILES += \
    misc/notes.txt \
    # keep gdm3 pam backend source file as
    # reference for pam backend i18n changes,
    # do not compile, just for convenient reading
    misc/gdm-session-worker.c
