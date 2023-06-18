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
* This header implements the polling server main class.
***********************************************************************************************************************/

#include <QObject>
#include <QTimer>
#include <QCoreApplication>
#include <QLocalServer>
#include <QLocalSocket>

#include <iostream>

#include <oping.h>

#include "connection.h"
#include "server_data.h"
#include "pinger.h"

Pinger::Pinger(QObject* parent):QObject(parent) {
    untestedPingListNeedsUpdate = false;
    activePingListNeedsUpdate   = false;
    defunctPingListNeedsUpdate  = false;

    untestedPingObject = nullptr;
    activePingObject   = nullptr;
    defunctPingObject  = nullptr;

    localServer = new QLocalServer(this);
    connect(localServer, &QLocalServer::newConnection, this, &Pinger::newConnection);

    untestedPingTimer = new QTimer(this);
    untestedPingTimer->setSingleShot(false);

    activePingTimer = new QTimer(this);
    activePingTimer->setSingleShot(false);

    defunctPingTimer = new QTimer(this);
    defunctPingTimer->setSingleShot(false);

    connect(untestedPingTimer, &QTimer::timeout, this, &Pinger::doUntestedPing);
    connect(activePingTimer, &QTimer::timeout, this, &Pinger::doActivePing);
    connect(defunctPingTimer, &QTimer::timeout, this, &Pinger::doDefunctPing);

    untestedPingTimer->start(untestedPingInterval);
    activePingTimer->start(activePingInterval);
    defunctPingTimer->start(10000); //defunctPingInterval);
}


Pinger::~Pinger() {
    if (activePingObject != nullptr) {
        ping_destroy(activePingObject);
    }

    if (defunctPingObject != nullptr) {
        ping_destroy(defunctPingObject);
    }
}


bool Pinger::start(const QString& newConnection) {
    if (localServer->isListening()) {
        localServer->close();
    }

    localServer->setSocketOptions(QLocalServer::SocketOption::WorldAccessOption);
    bool success = localServer->listen(newConnection);
    return success;
}


QString Pinger::errorString() const {
    return localServer->errorString();
}


void Pinger::newConnection() {
    std::cout << "New connection." << std::endl;
    connections.insert(new Connection(localServer->nextPendingConnection(), this));
}


void Pinger::disconnect(Connection* connection) {
    std::cout << "Lost connection." << std::endl;
    connections.remove(connection);
    connection->deleteLater();
}


void Pinger::addServer(unsigned long hostId, const QString& serverName, Connection* connection) {
    QHash<unsigned long, ServerData>::iterator it = serverData.find(hostId);
    if (it == serverData.end()) {
        QHash<unsigned long, ServerData>::iterator it = serverData.insert(hostId, ServerData(hostId, serverName));
        bool success = addUntestedServer(&(it.value()));
        if (success) {
            std::cout << "Adding server " << serverName.toLocal8Bit().data() << std::endl;
            connection->sendMessage(QString("OK\n"));
        } else {
            serverData.erase(it);
            connection->sendMessage(QString("failed\n"));
            std::cerr << "*** Failed to add server " << serverName.toLocal8Bit().data() << std::endl;
        }
    } else if (it.value().serverName() != serverName) {
        connection->sendMessage(QString("ERROR DUPLICATE ID\n"));
        std::cerr << "*** Failed to add server (duplicate ID)" << serverName.toLocal8Bit().data() << std::endl;
    } else {
        connection->sendMessage(QString("ERROR DUPLICATE REQUEST\n"));
        std::cerr << "*** Failed to add server (duplicate req)" << serverName.toLocal8Bit().data() << std::endl;
    }
}


void Pinger::removeServer(unsigned long hostId, Connection* connection) {
    QHash<unsigned long, ServerData>::iterator it = serverData.find(hostId);
    if (it != serverData.end()) {
        ServerData::Status status = it.value().status();
        if (status == ServerData::Status::UNTESTED) {
            untestedPingListNeedsUpdate = true;
            std::cout << "Removing untested server " << it.value().serverName().toLocal8Bit().data() << std::endl;
        } else if (status == ServerData::Status::DEFUNCT) {
            defunctPingListNeedsUpdate = true;
            std::cout << "Removing defunct server " << it.value().serverName().toLocal8Bit().data() << std::endl;
        } else {
            activePingListNeedsUpdate = true;
            std::cout << "Removing active server " << it.value().serverName().toLocal8Bit().data() << std::endl;
        }

        serverData.erase(it);
    } else {
        connection->sendMessage(QString("ERROR NO SERVER\n"));
        std::cerr << "*** Failed to remove server " << hostId << std::endl;
    }
}


void Pinger::markDefunct(unsigned long hostId, Connection* connection) {
    QHash<unsigned long, ServerData>::iterator it = serverData.find(hostId);
    if (it != serverData.end()) {
        ServerData::Status status = it.value().status();
        if (status == ServerData::Status::UNTESTED) {
            untestedPingListNeedsUpdate = true;
            bool success = addDefunctServer(&(it.value()));

            connection->sendMessage(success ? QString("OK\n") : QString("failed\n"));

            if (success) {
                std::cout << "Marked untested as defunct " << hostId << std::endl;
            } else {
                std::cerr << "*** Failed to mark defunct " << hostId << std::endl;
            }
        } else if (status != ServerData::Status::DEFUNCT) {
            activePingListNeedsUpdate = true;
            bool success = addDefunctServer(&(it.value()));

            connection->sendMessage(success ? QString("OK\n") : QString("failed\n"));

            if (success) {
                std::cout << "Marked active as defunct " << hostId << std::endl;
            } else {
                std::cerr << "*** Failed to mark defunct " << hostId << std::endl;
            }
        } else {
            connection->sendMessage(QString("ERROR ALREADY DEFUNCT\n"));
            std::cerr << "*** Failed to mark defunct " << hostId << " (already defunct)" << std::endl;
        }
    } else {
        connection->sendMessage(QString("ERROR NO SERVER\n"));
        std::cerr << "*** Failed to mark defunct " << hostId << " (bad ID)" << std::endl;
    }
}


void Pinger::doUntestedPing() {
    if (untestedPingListNeedsUpdate) {
        untestedPingListNeedsUpdate = false;
        rebuildUntestedServerList();
    }

    if (untestedPingObject != nullptr) {
        int result = ping_send(untestedPingObject);
        if (result < 0) {
            std::cerr << "*** Failed to send pings: " << ping_get_error(untestedPingObject) << std::endl;
        } else {
            size_t bufferSize            = sizeof(double);
            for (pingobj_iter_t* it=ping_iterator_get(untestedPingObject) ; it!=nullptr ; it=ping_iterator_next(it)) {
                ServerData* server = reinterpret_cast<ServerData*>(ping_iterator_get_context(it));
                if (server != nullptr) {
                    double latency;
                    int status = ping_iterator_get_info(it, PING_INFO_LATENCY, &latency, &bufferSize);
                    if (status == 0) {
                        if (latency >= 0) {
                            server->setStatus(ServerData::Status::ACTIVE);
                            addActiveServer(server);
                            std::cout << "New server active: "
                                      << server->serverName().toLocal8Bit().data() << std::endl;
                        } else {
                            server->setStatus(ServerData::Status::DEFUNCT);
                            addDefunctServer(server);
                            std::cout << "New server does not respond: "
                                     << server->serverName().toLocal8Bit().data() << std::endl;
                        }
                    } else {
                        std::cerr << "*** Failed to get latency "
                                  << server->serverName().toLocal8Bit().data() << std::endl;
                    }
                } else {
                    std::cerr << "*** Ping structure missing server entry" << std::endl;
                }
            }
        }

        ping_destroy(untestedPingObject);
        untestedPingObject = nullptr;
    }
}


void Pinger::doActivePing() {
    if (activePingListNeedsUpdate) {
        activePingListNeedsUpdate = false;
        rebuildActiveServerList();
    }

    if (activePingObject != nullptr) {
        int result = ping_send(activePingObject);
        if (result < 0) {
            std::cerr << "*** Failed to send pings: " << ping_get_error(activePingObject) << std::endl;
        } else {
            size_t bufferSize            = sizeof(double);
            for (pingobj_iter_t* it=ping_iterator_get(activePingObject) ; it!=nullptr ; it=ping_iterator_next(it)) {
                ServerData* server = reinterpret_cast<ServerData*>(ping_iterator_get_context(it));
                if (server != nullptr) {
                    double latency;
                    int status = ping_iterator_get_info(it, PING_INFO_LATENCY, &latency, &bufferSize);
                    if (status == 0) {
                        if (latency >= 0) {
                            server->setStatus(ServerData::Status::ACTIVE);
                        } else {
                            ServerData::Status currentStatus = server->status();
                            ServerData::Status newStatus;
                            switch (currentStatus) {
                                case ServerData::Status::UNTESTED: {
                                    std::cerr << "*** Untested server in active list "
                                              << server->serverName().toLocal8Bit().data() << std::endl;

                                    bool success = addDefunctServer(server);
                                    if (!success) {
                                        std::cerr << "*** Failed to add server to defunct list "
                                                  << server->serverName().toLocal8Bit().data() << std::endl;
                                    }

                                    activePingListNeedsUpdate = true;
                                    newStatus                 = ServerData::Status::DEFUNCT;

                                    break;
                                }

                                case ServerData::Status::DEFUNCT: {
                                    std::cerr << "*** Defunct server in active list "
                                              << server->serverName().toLocal8Bit().data() << std::endl;

                                    bool success = addDefunctServer(server);
                                    if (!success) {
                                        std::cerr << "*** Failed to add server to defunct list "
                                                  << server->serverName().toLocal8Bit().data() << std::endl;
                                    }

                                    activePingListNeedsUpdate = true;
                                    newStatus                 = ServerData::Status::DEFUNCT;

                                    break;
                                }

                                case ServerData::Status::ACTIVE: {
                                    newStatus = ServerData::Status::INACTIVE_1;
                                    break;
                                }

                                case ServerData::Status::INACTIVE_1: {
                                    newStatus = ServerData::Status::INACTIVE_2;
                                    break;
                                }

                                case ServerData::Status::INACTIVE_2: {
                                    newStatus = ServerData::Status::INACTIVE_3;
                                    break;
                                }

                                case ServerData::Status::INACTIVE_3: {
                                    reportFailedServer(server);
                                    newStatus = ServerData::Status::INACTIVE_FLAGGED;
                                    break;
                                }

                                case ServerData::Status::INACTIVE_4: {
                                    reportFailedServer(server);
                                    newStatus = ServerData::Status::INACTIVE_FLAGGED;
                                    break;
                                }

                                case ServerData::Status::INACTIVE_FLAGGED: {
                                    newStatus = ServerData::Status::INACTIVE_FLAGGED;
                                    break;
                                }

                                default: {
                                    std::cerr << "*** Unexpected state "
                                              << static_cast<unsigned>(currentStatus) << std::endl;

                                    newStatus = ServerData::Status::UNTESTED;
                                }
                            }

                            server->setStatus(newStatus);
                        }
                    } else {
                        std::cerr << "*** Failed to get latency "
                                  << server->serverName().toLocal8Bit().data() << std::endl;
                    }
                } else {
                    std::cerr << "*** Ping structure missing server entry" << std::endl;
                }
            }
        }
    }
}


void Pinger::doDefunctPing() {
    if (defunctPingListNeedsUpdate) {
        defunctPingListNeedsUpdate = false;
        rebuildDefunctServerList();
    }

    if (defunctPingObject != nullptr) {
        int result = ping_send(defunctPingObject);
        if (result < 0) {
            std::cerr << "*** Failed to send pings: " << ping_get_error(defunctPingObject) << std::endl;
        } else {
            pingobj_t* newDefunctPingObject = createPingObject();
            bool       deleteNewPingObject  = true;

            size_t bufferSize            = sizeof(double);
            for (pingobj_iter_t* it=ping_iterator_get(defunctPingObject) ; it!=nullptr ; it=ping_iterator_next(it)) {
                ServerData* server = reinterpret_cast<ServerData*>(ping_iterator_get_context(it));
                if (server != nullptr) {
                    double latency;
                    int status = ping_iterator_get_info(it, PING_INFO_LATENCY, &latency, &bufferSize);
                    if (status == 0) {
                        if (latency >= 0) {
                            server->setStatus(ServerData::Status::ACTIVE);
                            addActiveServer(server);
                            std::cout << "Defunct server now active: "
                                      << server->serverName().toLocal8Bit().data() << std::endl;
                        } else {
                            int result = ping_host_add_with_context(
                                newDefunctPingObject,
                                server->serverName().toUtf8().data(),
                                server
                            );
                            if (result != 0) {
                                const char* errorMessage = ping_get_error(newDefunctPingObject);
                                std::cerr << "*** Failed to re-add defunct host "
                                          << server->serverName().toLocal8Bit().data()
                                          << ": " << errorMessage << std::endl;
                            } else {
                                deleteNewPingObject = false;
                            }
                        }
                    } else {
                        std::cerr << "*** Failed to get latency "
                                  << server->serverName().toLocal8Bit().data() << std::endl;
                    }
                } else {
                    std::cerr << "*** Ping structure missing server entry" << std::endl;
                }
            }

            ping_destroy(defunctPingObject);
            if (!deleteNewPingObject) {
                defunctPingObject = newDefunctPingObject;
            } else {
                defunctPingObject = nullptr;
                ping_destroy(newDefunctPingObject);
            }
        }
    }
}


bool Pinger::addUntestedServer(ServerData* serverData) {
    if (untestedPingObject == nullptr) {
        untestedPingObject = createPingObject();
    }

    int result = ping_host_add_with_context(untestedPingObject, serverData->serverName().toUtf8().data(), serverData);
    if (result != 0) {
        const char* errorMessage = ping_get_error(untestedPingObject);
        std::cerr << "*** Failed to add untested host " << serverData->serverName().toLocal8Bit().data()
                  << ": " << errorMessage << std::endl;
    }

    return result == 0;
}


bool Pinger::addActiveServer(ServerData* serverData) {
    if (activePingObject == nullptr) {
        activePingObject = createPingObject();
    }

    int result = ping_host_add_with_context(activePingObject, serverData->serverName().toUtf8().data(), serverData);
    if (result != 0) {
        const char* errorMessage = ping_get_error(activePingObject);
        std::cerr << "*** Failed to add active host " << serverData->serverName().toLocal8Bit().data()
                  << ": " << errorMessage << std::endl;
    }

    return result == 0;
}


bool Pinger::addDefunctServer(ServerData* serverData) {
    if (defunctPingObject == nullptr) {
        defunctPingObject = createPingObject();
    }

    int result = ping_host_add_with_context(defunctPingObject, serverData->serverName().toUtf8().data(), serverData);
    if (result != 0) {
        std::cerr << "*** Failed to add defunct host " << serverData->serverName().toLocal8Bit().data()
                  << ": " << ping_get_error(defunctPingObject) << std::endl;
    }

    return result == 0;
}


bool Pinger::rebuildUntestedServerList() {
    if (untestedPingObject != nullptr) {
        ping_destroy(untestedPingObject);
    }

    untestedPingObject = createPingObject();

    bool deletePingObject = true;
    for (  QHash<unsigned long, ServerData>::const_iterator it = serverData.constBegin(), end = serverData.constEnd()
         ; it != end
         ; ++it
        ) {
        const ServerData* server = &(it.value());
        if (server->status() == ServerData::Status::UNTESTED) {
            int result = ping_host_add_with_context(
                untestedPingObject,
                server->serverName().toUtf8().data(),
                const_cast<ServerData*>(server)
            );

            if (result != 0) {
                std::cerr << "*** Failed to re-add untested host " << server->serverName().toLocal8Bit().data()
                          << ": " << ping_get_error(untestedPingObject) << std::endl;

            } else {
                deletePingObject = false;
            }
        }
    }

    if (deletePingObject) {
        ping_destroy(untestedPingObject);
        untestedPingObject = nullptr;
    }

    return true;
}


bool Pinger::rebuildActiveServerList() {
    if (activePingObject != nullptr) {
        ping_destroy(activePingObject);
    }

    activePingObject = createPingObject();

    bool deletePingObject = true;
    for (  QHash<unsigned long, ServerData>::const_iterator it = serverData.constBegin(), end = serverData.constEnd()
         ; it != end
         ; ++it
        ) {
        const ServerData* server = &(it.value());
        if (server->status() != ServerData::Status::DEFUNCT && server->status() != ServerData::Status::UNTESTED) {
            int result = ping_host_add_with_context(
                activePingObject,
                server->serverName().toUtf8().data(),
                const_cast<ServerData*>(server)
            );

            if (result != 0) {
                std::cerr << "*** Failed to re-add active host " << server->serverName().toLocal8Bit().data()
                          << ": " << ping_get_error(activePingObject) << std::endl;

            } else {
                deletePingObject = false;
            }
        }
    }

    if (deletePingObject) {
        ping_destroy(activePingObject);
        activePingObject = nullptr;
    }

    return true;
}


bool Pinger::rebuildDefunctServerList() {
    if (defunctPingObject != nullptr) {
        ping_destroy(defunctPingObject);
    }

    defunctPingObject = createPingObject();

    bool deletePingObject = true;
    for (  QHash<unsigned long, ServerData>::const_iterator it = serverData.constBegin(), end = serverData.constEnd()
         ; it != end
         ; ++it
        ) {
        const ServerData* server = &(it.value());
        if (server->status() == ServerData::Status::DEFUNCT) {
            int result = ping_host_add_with_context(
                defunctPingObject,
                server->serverName().toUtf8().data(),
                const_cast<ServerData*>(server)
            );

            if (result != 0) {
                std::cerr << "*** Failed to re-add defunct host " << server->serverName().toLocal8Bit().data()
                          << ": " << ping_get_error(defunctPingObject) << std::endl;

            } else {
                deletePingObject = false;
            }
        }
    }

    if (deletePingObject) {
        ping_destroy(defunctPingObject);
        defunctPingObject = nullptr;
    }

    return true;
}


void Pinger::reportFailedServer(ServerData* server) {
    for (QSet<Connection*>::iterator it=connections.begin(),end=connections.end() ; it!=end ; ++it) {
        Connection* connection = *it;
        connection->sendMessage(QString("NOPING %1 %2\n").arg(server->serverId()).arg(server->serverName()));
    }
}


pingobj_t* Pinger::createPingObject() {
    pingobj_t* result = ping_construct();

    double pingTimeout = 0.8 * activePingInterval / 1000.0;
    ping_setopt(result, PING_OPT_TIMEOUT, &pingTimeout);

    return result;
}
