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
* This header defines the ping server main class.
***********************************************************************************************************************/

/* .. sphinx-project pinger */

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QString>

class QLocalSocket;
class Pinger;

/**
 * The connection instance.
 */
class Connection:public QObject {
    Q_OBJECT

    public:
        /**
         * Constructor
         *
         * \param[in] localSocket The socket managing this connection.  This class will take ownership of the socket.
         *
         * \param[in] pinger      The pinger instance we need to control.  The pinger instance is also made the parent
         *                        of this object.
         */
        Connection(QLocalSocket* localSocket, Pinger* parent);

        ~Connection() override;

    public slots:
        /**
         * Slot you can overload to write a response.
         *
         * \param[in] message The message to be sent.
         */
        void sendMessage(const QString& message);

    private slots:
        /**
         * Slot that is triggered when data is available.
         */
        void readyRead();

        /**
         * Slot that is triggered when the local socket is closed remotely.
         */
        void readChannelFinished();

    private:
        /**
         * Value holding the maximum allowed line length
         */
        static constexpr unsigned maximumLineLength = 512;

        /**
         * Method that is called to process a received command.
         *
         * \param[in] received The received command.
         */
        void processCommand(const QString& received);

        /**
         * Method that obtains a pointer to the pinger instance we are controlling.
         */
        inline Pinger* pinger() const {
            return reinterpret_cast<Pinger*>(parent());
        }

        /**
         * The local socket to receive the data.
         */
        QLocalSocket* socket;
};

#endif
