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
* This header defines the \ref ServerData class.
***********************************************************************************************************************/

/* .. sphinx-project pinger */

#ifndef SERVER_DATA_H
#define SERVER_DATA_H

#include <QString>
#include <QHash>

#include <cstdint>

class QLocalServer;
class Connection;

/**
 * Class that stores information about a specific server.
 */
class ServerData {
    public:
        /**
         * Enumeration indicating the status of a given host system.
         */
        enum class Status : std::uint8_t {
            /**
             * Indicates the host has not been tested.
             */
            UNTESTED = 0,

            /**
             * Indicates the host has been tested and did not respond.
             */
            DEFUNCT = 1,

            /**
             * Indicates the host is active.
             */
            ACTIVE = 2,

            /**
             * Indicates the host is inactive once.
             */
            INACTIVE_1 = 3,

            /**
             * Indicates the host is inactive twice.
             */
            INACTIVE_2 = 4,

            /**
             * Indicates the host is inactive three times.
             */
            INACTIVE_3 = 5,

            /**
             * Indicates the host is inactive four times.
             */
            INACTIVE_4 = 6,

            /**
             * Indicates the host is inactive more than three times and has now been flagged by the polling server.
             */
            INACTIVE_FLAGGED = 7,

            /**
             * Value indicating the number of host/status values.
             */
            NUMBER_VALUES = 8
        };

        inline ServerData():
            currentId(0),
            currentStatus(Status::UNTESTED) {}

        /**
         * Constructor
         *
         * \param[in] serverId   The internal ID of this server.
         *
         * \param[in] serverName The name of this server.
         *
         * \param[in] status     The server's current status.
         */
        inline ServerData(
                unsigned long  serverId,
                const QString& serverName,
                Status         status = Status::UNTESTED
            ):currentId(
                serverId
            ),currentStatus(
                status
            ),currentName(
                serverName
            ) {}

        /**
         * Copy constructor
         *
         * \param[in] other The instance to assign to this instance.
         */
        inline ServerData(
                const ServerData& other
            ):currentId(
                other.currentId
            ),currentStatus(
                other.currentStatus
            ),currentName(
                other.currentName
            ) {}

        /**
         * Move constructor
         *
         * \param[in] other The instance to assign to this instance.
         */
        inline ServerData(
                ServerData&& other
            ):currentId(
                other.currentId
            ),currentStatus(
                other.currentStatus
            ),currentName(
                other.currentName
            ) {}

        /**
         * Method you can use to obtain the server ID.
         *
         * \return Returns the server Id.
         */
        inline unsigned long serverId() const {
            return currentId;
        }

        /**
         * Method you can use to obtain the server name.
         *
         * \return returns the server name.
         */
        inline const QString& serverName() const {
            return currentName;
        }

        /**
         * Method you can use to obtain the server status.
         *
         * \return Returns the current server status.
         */
        inline Status status() const {
            return currentStatus;
        }

        /**
         * Method you can use to update the server status.
         *
         * \param[in] newStatus The new server status.
         */
        inline void setStatus(Status newStatus) {
            currentStatus = newStatus;
        }

        /**
         * Assignment operator.
         *
         * \param[in] other The instance to assign to this instance.
         *
         * \return Returns a reference to this instance.
         */
        inline ServerData& operator=(const ServerData& other) {
            currentId     = other.currentId;
            currentStatus = other.currentStatus;
            currentName   = other.currentName;

            return *this;
        }

        /**
         * Assignment operator (move semantics).
         *
         * \param[in] other The instance to assign to this instance.
         *
         * \return Returns a reference to this instance.
         */
        inline ServerData& operator=(ServerData&& other) {
            currentId     = other.currentId;
            currentStatus = other.currentStatus;
            currentName   = other.currentName;

            return *this;
        }

        /**
         * Static method you can use to convert the status to a string.
         *
         * \param[in] status The status to be converted.
         *
         * \return Returns the status value converted to a string.
         */
        static QString toString(Status status);

        /**
         * Static method you can use to convert a string to a status value.
         *
         * \param[in]  str The string to be converted.
         *
         * \param[out] ok  A pointer to a boolean value that will be populated with true on success or false on error.
         *
         * \return Returns the status value associated with the string.
         */
        static Status toStatus(const QString& str, bool* ok = nullptr);

    private:
        /**
         * Table of server status strings.
         */
        static const QString serverStatusStrings[static_cast<unsigned>(Status::NUMBER_VALUES)];

        /**
         * Hash table used to convert server status strings to values.
         */
        static const QHash<QString, Status> serverStatusValues;

        /**
         * The server ID.
         */
        unsigned long currentId;

        /**
         * The server status.
         */
        Status currentStatus;

        /**
         * The server name.
         */
        QString currentName;
};

#endif
