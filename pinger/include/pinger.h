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

#ifndef PINGER_H
#define PINGER_H

#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QSet>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QHash>

#include <oping.h>

#include "server_data.h"

class QLocalServer;
class QTimer;
class Connection;

/**
 * The pinger server application class.
 */
class Pinger:public QObject {
    friend class Connection;

    Q_OBJECT

    public:
        /**
         * Constructor
         *
         * \param[in] parent Pointer to the parent object.
         */
        Pinger(QObject* parent = nullptr);

        ~Pinger() override;

        /**
         * Method you can use to instantiate and start the ping server.
         *
         * \param[in] connectionName The name of the connection.
         *
         * \return Returns true if the connection was established.  Returns false on error.
         */
        bool start(const QString& connectionName);

        /**
         * Method you can use to obtain any reported errors.
         *
         * \return Returns a string describing that last reported error.
         */
        QString errorString() const;

    private slots:
        /**
         * Slot that is triggered whenever a new connection is established.
         */
        void newConnection();

        /**
         * Slot that is triggered to remove a connection.
         *
         * \param[in] connection The connection to be removed.  The connection will be deleted by this call after
         *                       processing.
         */
        void disconnect(Connection* connection);

        /**
         * Slot that is triggered when a new server is to be added.
         *
         * \param[in] hostId     The ID of the server to be added.
         *
         * \param[in] serverName The name of the newly added server.
         *
         * \param[in] connection The connection that issued the request.
         */
        void addServer(unsigned long hostId, const QString& serverName, Connection* connection);

        /**
         * Slot that is added when a server is to be removed.
         *
         * \param[in] hostId     The ID of the server to be removed.
         *
         * \param[in] connection The connection that issued the request.
         */
        void removeServer(unsigned long hostId, Connection* connection);

        /**
         * Slot that is added when a server should be marked as defunct for ping operations.
         *
         * \param[in] hostId     The ID of the server to be marked as defunct.
         *
         * \param[in] connection The connection that issued the request.
         */
        void markDefunct(unsigned long hostId, Connection* connection);

        /**
         * Slot that is triggered to perform a single ping on untested servers.
         */
        void doUntestedPing();

        /**
         * Slot that is triggered to perform a single ping on active servers.
         */
        void doActivePing();

        /**
         * Slot that is triggered to perform a single ping of our defunct servers.
         */
        void doDefunctPing();

    private:
        /**
         * The untested ping interval, in milliseconds.  Value is the closest prime value above 30 seconds.
         */
        static constexpr unsigned untestedPingInterval = 30011;

        /**
         * The ping interval, in milliseconds.  Value is the closest prime value above 5 seconds.
         */
        static constexpr unsigned activePingInterval = 5003;

        /**
         * The defunct ping interval, in milliseconds.  The value is the closest prime value pushing us above 5 hours.
         */
        static constexpr unsigned defunctPingInterval = 18000041;

        /**
         * Method that adds a new untested server.
         *
         * \param[in] serverData The server data for the server to be added.
         *
         * \return Returns true on success.  Returns false on error.
         */
        bool addUntestedServer(ServerData* serverData);

        /**
         * Method that adds a new active server.
         *
         * \param[in] serverData The server data for the server to be added.
         *
         * \return Returns true on success.  Returns false on error.
         */
        bool addActiveServer(ServerData* serverData);

        /**
         * Method that adds a new defunct server.
         *
         * \param[in] serverData The server data for the server to be added.
         *
         * \return Returns true on success.  Returns false on error.
         */
        bool addDefunctServer(ServerData* serverData);

        /**
         * Method that rebuilds the untested server list.
         *
         * \return Returns true on success.  Returns false on error.
         */
        bool rebuildUntestedServerList();

        /**
         * Method that rebuilds the active server list.
         *
         * \return Returns true on success.  Returns false on error.
         */
        bool rebuildActiveServerList();

        /**
         * Method that rebuilds the defunct server list.
         *
         * \return Returns true on success.  Returns false on error.
         */
        bool rebuildDefunctServerList();

        /**
         * Method that is called to report a server failed ping.
         *
         * \param[in] server The server that failed the ping operation.
         */
        void reportFailedServer(ServerData* server);

        /**
         * Method that creates a ping object.
         *
         * \return Returns a newly created and configured ping object.
         */
        static pingobj_t* createPingObject();

        /**
         * The local socket server instance.
         */
        QLocalServer* localServer;

        /**
         * The untested ping timer.
         */
        QTimer* untestedPingTimer;

        /**
         * The active ping timer.
         */
        QTimer* activePingTimer;

        /**
         * The defunct ping timer.
         */
        QTimer* defunctPingTimer;

        /**
         * The list of active connections.
         */
        QSet<Connection*> connections;

        /**
         * Ping object holding just our untested servers.
         */
        pingobj_t* untestedPingObject;

        /**
         * Ping object holding only the active server.
         */
        pingobj_t* activePingObject;

        /**
         * Ping object holding just our defunct servers.
         */
        pingobj_t* defunctPingObject;

        /**
         * Hash used to track servers/
         */
        QHash<unsigned long, ServerData> serverData;

        /**
         * Flag indicating that the untested polling list needs to be updated.
         */
        bool untestedPingListNeedsUpdate;

        /**
         * Flag indicating that the active polling list needs to be updated.
         */
        bool activePingListNeedsUpdate;

        /**
         * Flag indicating that the defunct ping list needs to be updated.
         */
        bool defunctPingListNeedsUpdate;
};

#endif
