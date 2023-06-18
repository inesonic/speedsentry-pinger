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
* \filef
*
* This file implements the \ref ServerModel class.
***********************************************************************************************************************/

#include <QString>
#include <QVariant>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QModelIndex>
#include <QAbstractTableModel>
#include <QMessageBox>

#include <algorithm>

#include "../pinger/pinger/include/shared_memory.h"
#include "server_model.h"

/***********************************************************************************************************************
* ServerModel
*/

ServerModel::ServerModel(QObject* parent):QAbstractTableModel(parent) {
    sharedMemory = nullptr;
    semaphore    = nullptr;
}


ServerModel::~ServerModel() {}


bool ServerModel::connect(const QString& serverKey) {
    bool result;

    if (sharedMemory == nullptr && semaphore == nullptr) {
        sharedMemory = new QSharedMemory(serverKey + "Memory", this);
        semaphore    = new QSystemSemaphore(serverKey + "Semaphore");

        result = sharedMemory->create(sizeof(SharedMemory), QSharedMemory::AccessMode::ReadWrite);
        if (result) {
            result = (semaphore->error() == QSystemSemaphore::SystemSemaphoreError::NoError);
            if (!result) {
                QMessageBox::critical(
                    nullptr,
                    tr("No Connect"),
                    tr("Could not create semaphore: %1").arg(semaphore->errorString())
                );
            }
        } else {
            if (sharedMemory->error() == QSharedMemory::SharedMemoryError::AlreadyExists) {
                result = sharedMemory->attach(QSharedMemory::AccessMode::ReadWrite);
                if (!result) {
                    QMessageBox::critical(
                        nullptr,
                        tr("No Connect"),
                        tr("Could not create shared memory instance: %1").arg(sharedMemory->errorString())
                    );
                }
            } else {
                QMessageBox::critical(
                    nullptr,
                    tr("No Connect"),
                    tr("Could not create shared memory instance: %1").arg(sharedMemory->errorString())
                );
            }
        }

        if (result) {
            beginResetModel();
            sharedMemory->lock();

            SharedMemory* shared = reinterpret_cast<SharedMemory*>(sharedMemory->data());
            memset(shared, 0, sizeof(SharedMemory));

            shared->command               = Command::COMPLETED;
            shared->numberHosts           = 0;
            shared->reconfigurationNeeded = 0;

            for (unsigned i=0 ; i<maximumNumberServers ; ++i) {
                shared->servers[i].hostStatus = HostStatus::UNTESTED;
            }

            sharedMemory->unlock();
            endResetModel();
        }
    } else {
        result = false;
    }

    return result;
}


QVariant ServerModel::headerData(int section, Qt::Orientation orientation, int role) const {
    QVariant result;

    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Orientation::Horizontal) {
            if (section == 0) {
                result = QString("Server");
            } else {
                result = QString("Status");
            }
        } else {
            result = QString::number(section + 1);
        }
    }

    return result;
}


Qt::ItemFlags ServerModel::flags(const QModelIndex& index) const {
    unsigned long column = static_cast<unsigned long>(index.column());
    Qt::ItemFlags result;

    if (column == 0) {
        result = QAbstractTableModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsEnabled;
    } else {
        result = QAbstractTableModel::flags(index) | Qt::ItemIsEnabled;
    }

    return result;
}


QVariant ServerModel::data(const QModelIndex& index, int role) const {
    QVariant result;

    switch (role) {
        case Qt::EditRole:
        case Qt::DisplayRole: {
            unsigned long row    = static_cast<unsigned long>(index.row());
            unsigned long column = static_cast<unsigned long>(index.column());

            sharedMemory->lock();
            const SharedMemory* shared = reinterpret_cast<SharedMemory*>(sharedMemory->data());

            if (row < shared->numberHosts) {
                const Server& serverEntry = shared->servers[row];
                result =   (column == 0)
                         ? QString::fromUtf8(reinterpret_cast<const char*>(serverEntry.serverName))
                         : toString(serverEntry.hostStatus);
            } else {
                result = QString();
            }

            sharedMemory->unlock();
            break;
        }
    }

    return result;
}


bool ServerModel::setData(const QModelIndex& index, const QVariant& variant, int role) {
    bool result;

    if (role == Qt::EditRole) {
        unsigned long row    = static_cast<unsigned long>(index.row());
        unsigned long column = static_cast<unsigned long>(index.column());

        if (column == 0) {
            sharedMemory->lock();
            SharedMemory* shared = reinterpret_cast<SharedMemory*>(sharedMemory->data());

            if (row <= shared->numberHosts) {
                Server& server  = shared->servers[row];
                QString oldName = QString::fromUtf8(reinterpret_cast<const char*>(server.serverName));
                QString newName = variant.toString().trimmed();

                if (oldName != newName) {
                    beginResetModel();

                    QByteArray v = newName.toUtf8();
                    unsigned   l = static_cast<unsigned>(v.size());

                    memset(server.serverName, 0, maximumServerNameLength + 1);
                    server.pad = 0;

                    memcpy(server.serverName, v.data(), std::min(l, maximumServerNameLength));
                    shared->reconfigurationNeeded = true;

                    if (row >= shared->numberHosts) {
                        shared->numberHosts = static_cast<std::uint32_t>(row + 1);
                    }

                    endResetModel();
                }
            }

            sharedMemory->unlock();
            result = true;
        } else {
            result = false;
        }
    } else {
        result = false;
    }

    return result;
}


int ServerModel::columnCount(const QModelIndex& /* parent */) const {
    return 2;
}


int ServerModel::rowCount(const QModelIndex& /* parent */) const {
    int result;

    if (sharedMemory == nullptr || semaphore == nullptr) {
        result = 0;
    } else {
        result = reinterpret_cast<SharedMemory*>(sharedMemory->data())->numberHosts + 1;
    }

    return result;
}


void ServerModel::shutdown() {
    sharedMemory->lock();
    SharedMemory* shared = reinterpret_cast<SharedMemory*>(sharedMemory->data());
    if (shared->command == Command::COMPLETED) {
        shared->command = Command::SHUTDOWN;
        sharedMemory->unlock();
        semaphore->release();
    } else {
        sharedMemory->unlock();
        QMessageBox::information(nullptr, tr("Pinger Busy"), tr("Command is not \"COMPLETED\""));
    }
}


void ServerModel::runActive() {
    sharedMemory->lock();
    SharedMemory* shared = reinterpret_cast<SharedMemory*>(sharedMemory->data());
    if (shared->command == Command::COMPLETED) {
        shared->command = Command::RUN_ACTIVE;
        sharedMemory->unlock();
        semaphore->release();
    } else {
        sharedMemory->unlock();
        QMessageBox::information(nullptr, tr("Pinger Busy"), tr("Command is not \"COMPLETED\""));
    }
}


void ServerModel::runAll() {
    sharedMemory->lock();
    SharedMemory* shared = reinterpret_cast<SharedMemory*>(sharedMemory->data());
    if (shared->command == Command::COMPLETED) {
        shared->command = Command::RUN_ALL;
        sharedMemory->unlock();
        semaphore->release();
    } else {
        sharedMemory->unlock();
        QMessageBox::information(nullptr, tr("Pinger Busy"), tr("Command is not \"COMPLETED\""));
    }
}


QString ServerModel::toString(HostStatus hostStatus) {
    QString result;

    switch (hostStatus) {
        case HostStatus::UNTESTED:         { result = tr("Untested");           break; }
        case HostStatus::DEFUNCT:          { result = tr("Defunct");            break; }
        case HostStatus::ACTIVE:           { result = tr("Active");             break; }
        case HostStatus::INACTIVE_1:       { result = tr("Inactive 1");         break; }
        case HostStatus::INACTIVE_2:       { result = tr("Inactive 2");         break; }
        case HostStatus::INACTIVE_3:       { result = tr("Inactive 3");         break; }
        case HostStatus::INACTIVE_4:       { result = tr("Inactive 4");         break; }
        case HostStatus::INACTIVE_FLAGGED: { result = tr("Inactive Flagged");   break; }
        default: {
            result = QString::number(static_cast<unsigned>(hostStatus));
            break;
        }
    }

    return result;
}
