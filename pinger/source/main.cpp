/*-*-c++-*-*************************************************************************************************************
* Copyright 2021 - 2023 Inesonic, LLC.
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
* This file contains the main entry point for the Zoran ping server.
***********************************************************************************************************************/

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <iostream>

#include "pinger.h"

int main(int argumentCount, char* argumentValues[]) {
    int exitStatus = 0;

    QCoreApplication application(argumentCount, argumentValues);
    QCoreApplication::setApplicationName("Inesonic Ping Server");
    QCoreApplication::setApplicationVersion("1.0");

    if (argumentCount == 2) {
        QString connectionName = QString::fromLocal8Bit(argumentValues[1]);
        Pinger pinger;

        bool success = pinger.start(connectionName);
        if (success) {
            exitStatus = application.exec();
        } else {
            std::cerr << "*** Failed to connect to socket " << connectionName.toLocal8Bit().data() << std::endl;
            exitStatus = 1;
        }
    } else {
        std::cerr << "*** Invalid command line.  Include shared memory key as parameter." << std::endl;
        exitStatus = 1;
    }

    return exitStatus;
}
