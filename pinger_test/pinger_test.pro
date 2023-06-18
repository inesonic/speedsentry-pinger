##-*-makefile-*-########################################################################################################
# Copyright 2021 - 2023 Inesonic, LLC
#
# GNU Public License, Version 3:
#   This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
#   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
#   version.
#   
#   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
#   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
#   details.
#   
#   You should have received a copy of the GNU General Public License along with this program.  If not, see
#   <https://www.gnu.org/licenses/>.
########################################################################################################################

########################################################################################################################
# Basic build characteristics
#

TEMPLATE = app
QT += core network gui widgets
CONFIG += c++14

########################################################################################################################
# Headers
#

HEADERS = pinger_test_dialog.h

########################################################################################################################
# Source files
#

SOURCES = pinger_test.cpp \
          pinger_test_dialog.cpp

########################################################################################################################
# Locate build intermediate and output products
#

TARGET = pinger_test

CONFIG(debug, debug|release) {
    unix:DESTDIR = build/debug
    win32:DESTDIR = build/Debug
} else {
    unix:DESTDIR = build/release
    win32:DESTDIR = build/Release
}

OBJECTS_DIR = $${DESTDIR}/objects
MOC_DIR = $${DESTDIR}/moc
RCC_DIR = $${DESTDIR}/rcc
UI_DIR = $${DESTDIR}/ui
