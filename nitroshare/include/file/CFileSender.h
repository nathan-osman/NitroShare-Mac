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

#ifndef CFILESENDER_H
#define CFILESENDER_H

#include <QFileInfo>
#include <QList>
#include <QStringList>

#include "discovery/CMachine.h"
#include "file/CBasicSocket.h"
#include "file/CFileHeader.h"

class CFileSender : public CBasicSocket
{
    Q_OBJECT

    public:

        /* These special constants can be used as progress values to
           be emitted with the Progress signal, etc. */
        enum {
            ProgressNone     = -1,
            ProgressComplete = -2,
            ProgressError    = -3
        };

        CFileSender(QObject *, QStringList);
        virtual ~CFileSender();

        void Start(CMachine);

    signals:

        void Progress(int);

    private slots:

        void OnBytesWritten(qint64);
        void OnConnected();
        void OnData(QByteArray);

    private:

        enum SendState {
            WaitingForConnection,
            WaitingForHeaderResponse,
            TransferringFiles
        };

        SendState m_state;

        void PrepareFile(QFileInfo, QString);
        void PrepareDirectory(QFileInfo, QString);
        void PrepareHeaders(QStringList);
        void SendFile(CFileHeader *);

        QList<CFileHeader *> m_headers;
        bool m_compress_files,
             m_calculate_checksum,
             m_oversized_prompt;

        /* Yes, I do believe in verbosity - code completion to the rescue! */
        qint64 m_total_uncompressed_bytes,
               m_total_uncompressed_bytes_so_far;
        qint64 m_current_file_uncompressed_bytes,
               m_current_file_transfer_bytes,
               m_current_file_bytes_so_far;

};

#endif // CFILESENDER_H
