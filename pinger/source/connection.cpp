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
* This header implements the polling server connection API.
***********************************************************************************************************************/

#include <QObject>
#include <QIODevice>
#include <QLocalSocket>
#include <QRegularExpression>

#include <iostream>

#include <oping.h>

#include "pinger.h"
#include "connection.h"

Connection::Connection(QLocalSocket* localSocket, Pinger* parent):QObject(parent) {
    socket = localSocket;
    connect(socket, &QLocalSocket::readyRead, this, &Connection::readyRead);
    connect(socket, &QLocalSocket::readChannelFinished, this, &Connection::readChannelFinished);
}


Connection::~Connection() {
    delete socket;
}


void Connection::sendMessage(const QString& message) {
    QByteArray encoded = message.toUtf8();
    socket->write(encoded);
}


void Connection::readyRead() {
    if (socket->canReadLine()) {
        char line[maximumLineLength + 1];
        qint64 bytesRead = socket->readLine(line, maximumLineLength);
        if (bytesRead >= 0) {
            QString received = QString::fromUtf8(line);
            processCommand(received.trimmed());
        } else {
            std::cerr << "*** Failed to receive content: " << socket->errorString().toLocal8Bit().data() << std::endl;
        }
    }
}


void Connection::readChannelFinished() {
    pinger()->disconnect(this);
}


void Connection::processCommand(const QString& received) {
    QStringList arguments = received.split(QChar(' '), QString::SplitBehavior::SkipEmptyParts);
    if (arguments.size() > 0) {
        const QString& command = arguments.at(0);
        if (command == QString("A") && arguments.size() == 3) {
            bool          success;
            unsigned long hostId = arguments.at(1).toULong(&success);
            if (success && hostId > 0) {
                const QString& serverName = arguments.at(2);
                pinger()->addServer(hostId, serverName, this);
            } else {
                sendMessage("ERROR " + received + "\n");
            }
        } else if (command == QString("R") && arguments.size() == 2) {
            bool          success;
            unsigned long hostId = arguments.at(1).toULong(&success);
            if (success && hostId > 0) {
                pinger()->removeServer(hostId, this);
            } else {
                sendMessage("ERROR " + received + "\n");
            }
        } else if (command == QString("D") && arguments.size() == 2) {
            bool          success;
            unsigned long hostId = arguments.at(1).toULong(&success);
            if (success && hostId > 0) {
                pinger()->markDefunct(hostId, this);
            } else {
                sendMessage("ERROR " + received + "\n");
            }
        } else if (command == QString("Q") && arguments.size() == 1) {
            sendMessage("DISCONNECTING\n");
            socket->waitForBytesWritten();

            pinger()->disconnect(this);
        } else if (command == QString("!SHUTDOWN!") && arguments.size() == 1) {
            sendMessage("SHUTTING DOWN\n");
            socket->waitForBytesWritten();

            QCoreApplication::quit();
        } else {
            sendMessage("ERROR " + received + "\n");
        }
    }
}
