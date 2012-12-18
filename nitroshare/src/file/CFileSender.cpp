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

#include <QDebug>
#include <QVariantList>
#include <qjson/qobjecthelper.h>

#include "file/CFileSender.h"
#include "util/settings.h"

CFileSender::CFileSender(QObject * parent)
    : CBasicSocket(parent), m_state(WaitingForConnection)
{
    connect(this, SIGNAL(connected()),          SLOT(OnConnected()));
    connect(this, SIGNAL(Data(QByteArray)),     SLOT(OnData(QByteArray)));
    connect(this, SIGNAL(bytesWritten(qint64)), SLOT(OnBytesWritten(qint64)));
}
void CFileSender::AddFiles(QStringList filenames)
{
    foreach(QString filename, filenames)
        m_headers.AddFile(QFileInfo(filename));
}

bool CFileSender::AddDirectory(QString directory)
{
    return m_headers.AddDirectory(directory);
}

void CFileSender::Start(CMachine machine)
{
    qDebug() << "Attempting connection to" << machine.address.toString() << "...";

    /* Once we know the information for connecting to the remote
       machine, we immediately attempt a connection. */
    connectToHost(machine.address, machine.port);
}

void CFileSender::OnConnected()
{
    qDebug() << "Connection with remote server established.";

    /* Once we have established connection with the remote machine,
       we immediately begin sending headers for all of the files. */
    QVariantList header_list;
    m_headers.GetHeaders(header_list);

    /* Prepare the JSON object we will send over the wire containing
       the header and some other basic information about the transfer. */
    QVariantMap map;
    map["machine"] = Settings::Get("General/MachineName");
    map["files"]   = header_list;

    SendData(EncodeJSON(map));

    /* Our current state is now waiting for the response from the remote machine. */
    qDebug() << "File headers sent, awaiting server response.";
    m_state = WaitingForHeaderResponse;
}

void CFileSender::OnData(QByteArray data)
{
    qDebug() << "Data received from remote server" << data;

    /* We currently only accept one of the following messages:
        - an "acceptall" status, which indicates the entire payload is accepted
        - an "accept" status which indicates that part of the payload
          is accepted and we can begin transferring the files
        - a "reject" status indicating that none of the payload was accepted
          and we should immediately close the file */

    QVariantMap response = DecodeJSON(data);
    if(response["status"] != "acceptall" && response["status"] != "accept" && response["status"] != "reject")
    {
        emit Error(tr("Unrecognized message received from remote server. Aborting transfer."));

        disconnectFromHost();
        return;
    }

    /* Abort the transfer if the remote server rejected the transfer. */
    if(response["status"] == "reject")
    {
        emit Error(tr("The remote machine aborted the transfer."));

        disconnectFromHost();
        return;
    }

    /* The remote server accepted some of our payload. Get the list of file IDs. */
    if(response["status"] == "accept")
    {
        /* Pass the list of files to the header manager. */
        if(response.contains("files") && response["files"].toList().size())
            m_headers.SetAcceptedFiles(response["files"].toList());
        /* We can only assume no files were accepted so... just close the connection. */
        else
        {
            disconnectFromHost();
            return;
        }
    }
    /* Otherwise, everything was accepted and we can begin the transfer. */
    else
        m_headers.AcceptAll();

    /* Now we can begin sending the files. */
    m_state = TransferringFiles;

    /* Transfer the first chunk. */
    OnBytesWritten();
}

void CFileSender::OnBytesWritten(qint64)

{
    /* Ignore all data written before we are transferring files. */
    if(m_state != TransferringFiles)
        return;

    QByteArray chunk;
    QVariantMap sizes;

    /* If there is another chunk, send it. */
    if(m_headers.GetNextChunk(chunk, sizes))

    {
        /* Create the JSON containing size information. */
        QByteArray sizes_json = EncodeJSON(sizes);
        PrependSize(sizes_json);

        /* Prepend the chunk with the file size information and send it. */
        chunk.prepend(sizes_json);
        SendData(chunk);
    }
    /* Otherwise disconnect. */

    else
    {
        disconnectFromHost();
    }
}
