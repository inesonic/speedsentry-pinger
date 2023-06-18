/*-*-c++-*-*************************************************************************************************************
* Copyright 2016 - 2023 Inesonic, LLC.
*
* GNU Public License, Version 3:
*   This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
*   License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
*   version.
*   
*   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
*   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
*   details.
*   
*   You should have received a copy of the GNU General Public License along with this program.  If not, see
*   <https://www.gnu.org/licenses/>.
********************************************************************************************************************//**
* \file
*
* This file contains the main entry point for the application.
***********************************************************************************************************************/

#include <QApplication>
#include <QSettings>

#include "pinger_test_dialog.h"

int main(int argumentCount, char* argumentValues[]) {
    QApplication application(argumentCount, argumentValues);
    PingerTestDialog pingerTestDialog;

    return pingerTestDialog.exec();
}
