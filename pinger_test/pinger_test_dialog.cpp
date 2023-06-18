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
* This file implements the \ref PingerTestDialog class.
***********************************************************************************************************************/

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QLocalSocket>
#include <QMessageBox>

#include "pinger_test_dialog.h"

PingerTestDialog::PingerTestDialog(QWidget* parent):QDialog(parent) {
    setWindowTitle(tr("Pinger Tester"));
    setSizeGripEnabled(true);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QFormLayout* formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    QHBoxLayout* serverKeyHorizontalLayout = new QHBoxLayout;
    formLayout->addRow(tr("Server Key "), serverKeyHorizontalLayout);

    serverKeyLineEdit = new QLineEdit;
    serverKeyHorizontalLayout->addWidget(serverKeyLineEdit);

    serverKeyPushButton = new QPushButton(tr("Connect"));
    serverKeyHorizontalLayout->addWidget(serverKeyPushButton);

    QHBoxLayout* commandHorizontalLayout = new QHBoxLayout;
    formLayout->addRow(tr("Command "), commandHorizontalLayout);

    commandLineEdit = new QLineEdit;
    commandHorizontalLayout->addWidget(commandLineEdit);

    commandSendPushButton = new QPushButton(tr("Send"));
    commandHorizontalLayout->addWidget(commandSendPushButton);

    responseTextEdit = new QPlainTextEdit;
    mainLayout->addWidget(responseTextEdit);

    responseTextEdit->setReadOnly(true);

    QDialogButtonBox* dialogButtonBox = new QDialogButtonBox(Qt::Orientation::Horizontal);
    dialogButtonBox->setStandardButtons(QDialogButtonBox::StandardButton::Close);

    mainLayout->addWidget(dialogButtonBox);

    socket = new QLocalSocket(this);

    connect(serverKeyLineEdit, &QLineEdit::returnPressed, this, &PingerTestDialog::serverKeyEntered);
    connect(serverKeyPushButton, &QPushButton::clicked, this, &PingerTestDialog::serverKeyEntered);

    connect(commandLineEdit, &QLineEdit::returnPressed, this, &PingerTestDialog::commandEntered);
    connect(commandSendPushButton, &QPushButton::clicked, this, &PingerTestDialog::commandEntered);

    connect(socket, &QLocalSocket::readyRead, this, &PingerTestDialog::readyRead);
    connect(socket, &QLocalSocket::readChannelFinished, this, &PingerTestDialog::readChannelFinished);

    QPushButton* closeButton = dialogButtonBox->button(QDialogButtonBox::StandardButton::Close);
    connect(closeButton, &QPushButton::clicked, this, &PingerTestDialog::closeDialog);

    commandLineEdit->setEnabled(false);
    commandSendPushButton->setEnabled(false);
    responseTextEdit->setEnabled(false);
}


PingerTestDialog::~PingerTestDialog() {}


void PingerTestDialog::serverKeyEntered() {
    QString serverKey = serverKeyLineEdit->text().trimmed();
    if (!serverKey.isEmpty()) {
        socket->connectToServer(serverKey);
        QLocalSocket::LocalSocketState socketState = socket->state();
        if (socketState != QLocalSocket::LocalSocketState::UnconnectedState) {
            responseTextEdit->appendPlainText(tr("< CONNECTED TO %1\n").arg(serverKey));

            commandLineEdit->setEnabled(true);
            commandSendPushButton->setEnabled(true);
            responseTextEdit->setEnabled(true);
        } else{
            QString errorString = socket->errorString();
            QMessageBox::information(this, tr("Failed to connect"), errorString);

            commandLineEdit->setEnabled(false);
            commandSendPushButton->setEnabled(false);
            responseTextEdit->setEnabled(false);
        }
    }
}


void PingerTestDialog::commandEntered() {
    QString rawCommand = commandLineEdit->text();
    QString command    = rawCommand + "\n";
    qint64 bytesSent = socket->write(command.toUtf8());
    if (bytesSent < 0) {
        QMessageBox::information(this, tr("Failed to send message"), socket->errorString());
        responseTextEdit->appendPlainText("! " + rawCommand);
    } else {
        responseTextEdit->appendPlainText("> " + rawCommand);
    }
}


void PingerTestDialog::readyRead() {
    if (socket->canReadLine()) {
        char line[maximumLineLength + 1];
        qint64 bytesRead = socket->readLine(line, maximumLineLength);
        if (bytesRead >= 0) {
            QString received = QString::fromUtf8(line).trimmed();
            responseTextEdit->appendPlainText("< " + received);
        } else {
            QMessageBox::information(this, tr("Failed to receive"), socket->errorString());
        }
    }
}


void PingerTestDialog::readChannelFinished() {
    socket->disconnectFromServer();
    responseTextEdit->appendPlainText(tr("> DISCONNECTED\n\n"));

    commandLineEdit->setEnabled(false);
    commandSendPushButton->setEnabled(false);
    responseTextEdit->setEnabled(false);
}


void PingerTestDialog::closeDialog() {
    disconnect(socket, &QLocalSocket::readyRead, this, &PingerTestDialog::readyRead);
    disconnect(socket, &QLocalSocket::readChannelFinished, this, &PingerTestDialog::readChannelFinished);
    accept();
}
