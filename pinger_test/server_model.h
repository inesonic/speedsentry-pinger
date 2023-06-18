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
* This header defines the \ref ServerModel class.
***********************************************************************************************************************/

/* .. sphinx-project pinger */

#ifndef SERVER_MODEL_H
#define SERVER_MODEL_H

#include <QString>
#include <QVariant>
#include <QModelIndex>
#include <QList>
#include <QAbstractTableModel>

#include "../pinger/pinger/include/shared_memory.h"

class QSharedMemory;
class QSystemSemaphore;

/**
 * Class that provides a model for the pinger test.
 */
class ServerModel:public QAbstractTableModel {
    Q_OBJECT

    public:
        /**
         * Constructor.
         *
         * \param[in] parent The parent object for this menu.
         */
        ServerModel(QObject* parent = Q_NULLPTR);

        ~ServerModel() override;

        /**
         * Method you can use to connect to a remote ping server.
         *
         * \param[in] serverKey The server key used to connect to the server.
         *
         * \return Returns true on success.  Returns false on error.
         */
        bool connect(const QString& serverKey);

        /**
         * Method used by the view to obtain the header data to be displayed.
         *
         * \param[in] section     The column or row number.
         *
         * \param[in] orientation The orientation requiring header data.
         *
         * \param[in] role        The desired display role.
         *
         * \return Returns a variant type holding the data to beinde displayed.
         */
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        /**
         * Method used by the view to obtain the item flags.
         *
         * \param[in] index The index of the cell being queried.
         *
         * \return Returns the flags tied to the cell.
         */
        Qt::ItemFlags flags(const QModelIndex& index) const override;

        /**
         * Method used by the view to obtain the data to be displayed.
         *
         * \param[in] index The index into the model used to locate the data.
         *
         * \param[in] role  The display role for the data.
         *
         * \return Returns a variant type holding the data to be displayed.
         */
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        /**
         * Method used by the model to update the contents of a cell.
         *
         * \param[in] index The index indicating the row/column being updated.
         *
         * \param[in] value A variant holding the new value for the cell.
         *
         * \param[in] role  The role for the data being updated.
         *
         * \return Returns true on success.  Returns false on error.
         */
        bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

        /**
         * Method used by the view to obtain the number of columns to be displayed.
         *
         * \return Returns the number of displayable columns.
         */
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;

        /**
         * Method used by the view to obtain the number of rows to be displayed.
         *
         * \return Returns the number of displayable rows.
         */
        int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    public slots:
        /**
         * Slot you can trigger to shutdown the pinger server.
         */
        void shutdown();

        /**
         * Slot you can trigger to run only the active servers.
         */
        void runActive();

        /**
         * Slot you can trigger to run all servers.
         */
        void runAll();

    private:
        /**
         * Method you can use to obtain the host status as a string.
         *
         * \param[in] hostStatus The host status to be converted.
         *
         * \return Returns the host status encoded as a string.
         */
        static QString toString(HostStatus hostStatus);

        /**
         * The shared memory object.
         */
        QSharedMemory* sharedMemory;

        /**
         * The semaphore used to gate access.
         */
        QSystemSemaphore* semaphore;
};

#endif
