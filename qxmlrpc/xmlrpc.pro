TEMPLATE = lib
TARGET   = ../../qxmlrpc

CONFIG += warn_on
CONFIG += staticlib
CONFIG += precompile_header
CONFIG += ppc

QT += xml network

INCLUDEPATH += ..

HEADERS += stable.h
PRECOMPILED_HEADER = stable.h

# Input
HEADERS += \
    client.h \
    server.h \
    server_private.h \
    serverintrospection.h \
    request.h \
    response.h \
    variant.h

SOURCES += \
    client.cpp \
    server.cpp \
    serverintrospection.cpp \
    request.cpp \
    response.cpp \
    variant.cpp
