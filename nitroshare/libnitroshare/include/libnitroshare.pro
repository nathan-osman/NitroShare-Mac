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

QT += network xml
QT -= gui

TARGET = nitroshare
TEMPLATE = lib

DEFINES += LIBNITROSHARE_LIBRARY

SOURCES += src/CNitroShare.cpp

HEADERS += include/CNitroShare.h\
        include/libnitroshare_global.h

INCLUDEPATH += include
