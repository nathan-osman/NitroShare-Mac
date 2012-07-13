/* NitroShare - A simple network file sharing tool.
   Copyright (C) 2012 Nathan Osman

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef CFILERECEIVER_H
#define CFILERECEIVER_H

#include <QDir>
#include <QVariantMap>

#include "file/CBasicSocket.h"
#include "file/CFileHeader.h"

class CFileReceiver : public CBasicSocket
{
    Q_OBJECT

    public:

        CFileReceiver(QObject *);
        virtual ~CFileReceiver();

        void Start(int);

    private slots:

        void OnData(QByteArray);

    private:

        enum ReceiveState {
            WaitingForHeader,
            WaitingForTransfer
        };

        ReceiveState m_state;

        void ParseHeaders(QVariantMap);
        bool TestAgainstPolicy();
        bool TestReceivedFilesDirectory(QDir);
        bool TestChecksum(QByteArray &);
        void SaveFile(CFileHeader *, QByteArray);

        QString              m_client_name;
        QList<CFileHeader *> m_headers;

        int m_files_received;
};

#endif // CFILERECEIVER_H
