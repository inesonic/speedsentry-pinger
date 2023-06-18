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
* This header defines the \ref PingerTestDialog class.
***********************************************************************************************************************/

/* .. sphinx-project pinger */

#ifndef PINGER_TEST_DIALOG_H
#define PINGER_TEST_DIALOG_H

#include <QDialog>
#include <QWidget>

class QWidget;
class QVBoxLayout;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QLocalSocket;

/**
 * Dialog used test the pinger server.
 */
class PingerTestDialog:public QDialog {
    Q_OBJECT

    public:
        /**
         * Constructor
         *
         * \param[in] parent Pointer to the parent object.
         */
        PingerTestDialog(QWidget* parent = Q_NULLPTR);

        ~PingerTestDialog() override;

    private slots:
        /**
         * Slot that is triggered when the server key is entered.
         */
        void serverKeyEntered();

        /**
         * Slot that is triggered when a new command is entered.
         */
        void commandEntered();

        /**
         * Slot that is triggered when the local socket has data available.
         */
        void readyRead();

        /**
         * Slot that is triggered when the local socket disconnects.
         */
        void readChannelFinished();

        /**
         * Slot that is triggered when we close the dialog.
         */
        void closeDialog();

    private:
        /**
         * Value holding the maximum allowed line length
         */
        static constexpr unsigned maximumLineLength = 16384;

        /**
         * The line editor where the user can enter the server key.
         */
        QLineEdit* serverKeyLineEdit;

        /**
         * The server connect push button.
         */
        QPushButton* serverKeyPushButton;

        /**
         * The command send push button.
         */
        QPushButton* commandSendPushButton;

        /**
         * The command editor.
         */
        QLineEdit* commandLineEdit;

        /**
         * The response text editor.
         */
        QPlainTextEdit* responseTextEdit;

        /**
         * The local socket connection.
         */
        QLocalSocket* socket;
};

#endif
