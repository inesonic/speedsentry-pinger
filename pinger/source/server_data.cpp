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
* This header implements the \ref ServerStatus class.
***********************************************************************************************************************/

#include <QString>

#include <cstdint>
#include <initializer_list>

#include "server_data.h"

const QString ServerData::serverStatusStrings[static_cast<unsigned>(Status::NUMBER_VALUES)] = {
    QString("UNTESTED"),         // 0
    QString("DEFUNCT"),          // 1
    QString("ACTIVE"),           // 2
    QString("INACTIVE_1"),       // 3
    QString("INACTIVE_2"),       // 4
    QString("INACTIVE_3"),       // 5
    QString("INACTIVE_4"),       // 6
    QString("INACTIVE_FLAGGED"), // 7
};


const QHash<QString, ServerData::Status> ServerData::serverStatusValues(
    {
        std::pair<QString, ServerData::Status>(QString("UNTESTED"), ServerData::Status::UNTESTED),
        std::pair<QString, ServerData::Status>(QString("DEFUNCT"), ServerData::Status::DEFUNCT),
        std::pair<QString, ServerData::Status>(QString("ACTIVE"), ServerData::Status::ACTIVE),
        std::pair<QString, ServerData::Status>(QString("INACTIVE_1"), ServerData::Status::INACTIVE_1),
        std::pair<QString, ServerData::Status>(QString("INACTIVE_2"), ServerData::Status::INACTIVE_2),
        std::pair<QString, ServerData::Status>(QString("INACTIVE_3"), ServerData::Status::INACTIVE_3),
        std::pair<QString, ServerData::Status>(QString("INACTIVE_4"), ServerData::Status::INACTIVE_4),
        std::pair<QString, ServerData::Status>(QString("INACTIVE_FLAGGED"), ServerData::Status::INACTIVE_FLAGGED)
    }
);


QString ServerData::toString(ServerData::Status status) {
    QString  result;
    unsigned statusValue = static_cast<unsigned>(status);
    if (statusValue < static_cast<unsigned>(Status::NUMBER_VALUES)) {
        result = serverStatusStrings[statusValue];
    }

    return result;
}


ServerData::Status ServerData::toStatus(const QString& str, bool* ok) {
    Status result = serverStatusValues.value(str, Status::NUMBER_VALUES);
    if (ok != nullptr) {
        *ok = (result != Status::NUMBER_VALUES );
    }

    return result;
}
