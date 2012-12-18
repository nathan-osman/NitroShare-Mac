# NitroShare - A simple network file sharing tool.
# Copyright (C) 2012 Nathan Osman
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

QT += core gui network xml

TARGET = nitroshare
target.path = /opt/extras.ubuntu.com/nitroshare
INSTALLS += target

TEMPLATE = app

SOURCES += \
    src/dialogs/CAboutDialog.cpp \
    src/dialogs/CAcceptPromptDialog.cpp \
    src/dialogs/CFirstStartWizard.cpp \
    src/dialogs/CMachineSelectionDialog.cpp \
    src/dialogs/CSettingsDialog.cpp \
    src/discovery/CBasicBroadcaster.cpp \
    src/discovery/CBasicListener.cpp \
    src/discovery/CBroadcastServer.cpp \
    src/file/CBasicSocket.cpp \
    src/file/CFileHeader.cpp \
    src/file/CFileReceiver.cpp \
    src/file/CFileSender.cpp \
    src/file/CFileServer.cpp \
    src/main/CTrayIcon.cpp \
    src/main/main.cpp \
    src/rpc/CRPCServer.cpp \
    src/sharebox/CShareBox.cpp \
    src/util/defaults.cpp \
    src/util/definitions.cpp \
    src/util/network.cpp \
    src/util/settings.cpp \
    src/widgets/CBroadcastDiscoveryWidget.cpp

HEADERS  += \
    include/dialogs/CAboutDialog.h \
    include/dialogs/CAcceptPromptDialog.h \
    include/dialogs/CFirstStartWizard.h \
    include/dialogs/CMachineSelectionDialog.h \
    include/dialogs/CSettingsDialog.h \
    include/discovery/CBasicBroadcaster.h \
    include/discovery/CBasicListener.h \
    include/discovery/CBroadcastServer.h \
    include/discovery/CMachine.h \
    include/file/CBasicSocket.h \
    include/file/CFileHeader.h \
    include/file/CFileReceiver.h \
    include/file/CFileSender.h \
    include/file/CFileServer.h \
    include/main/CTrayIcon.h \
    include/rpc/CRPCServer.h \
    include/sharebox/CShareBox.h \
    include/util/defaults.h \
    include/util/definitions.h \
    include/util/network.h \
    include/util/settings.h \
    include/widgets/CBroadcastDiscoveryWidget.h

FORMS += \
    ui/CAboutDialog.ui \
    ui/CAcceptPromptDialog.ui \
    ui/CBroadcastDiscoveryWidget.ui \
    ui/CFirstStartWizard.ui \
    ui/CMachineSelectionDialog.ui \
    ui/CSettingsDialog.ui

RESOURCES += \
    resource/main.qrc

OTHER_FILES += \
    resource/resource.rc

# We use only a single translation file here which (after being
# generated) is converted to a POT file and uploaded to Launchpad.
TRANSLATIONS = nitroshare.ts

INCLUDEPATH += include
INCLUDEPATH += ..

LIBS += -lqjson -lqxmlrpc
LIBS += -L..

# On the Windows platform we provide a resource file that defines
# an icon and some internal version information and meta-data.
win32:RC_FILE = resource/resource.rc

ICON = resource/icon.icns
