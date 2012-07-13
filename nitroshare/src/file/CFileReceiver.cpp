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
#include <QFile>
#include <QHostAddress>
#include <QRegExp>
#include <qjson/qobjecthelper.h>

#include "dialogs/CAcceptPromptDialog.h"
#include "file/CFileReceiver.h"
#include "util/defaults.h"
#include "util/settings.h"

CFileReceiver::CFileReceiver(QObject * parent)
    : CBasicSocket(parent), m_state(WaitingForHeader), m_files_received(0)
{
    connect(this, SIGNAL(Data(QByteArray)), SLOT(OnData(QByteArray)));
}

CFileReceiver::~CFileReceiver()
{
    qDeleteAll(m_headers);
}

void CFileReceiver::Start(int handle)
{
    setSocketDescriptor(handle);

    qDebug() << "Incoming connection from" << peerAddress().toString();
}

void CFileReceiver::OnData(QByteArray data)
{
    qDebug() << "Received" << data.size() << "bytes from client.";

    /* If we're waiting for the headers then parse them. */
    if(m_state == WaitingForHeader)
        ParseHeaders(DecodeJSON(data));
    else
    {
        /* If there are files remaining to be read, then read the next one. */
        if(m_headers.size())
        {
            CFileHeader * header = m_headers.takeFirst();
            SaveFile(header, data);
            delete header;

            /* Let the client know they can send the next one. */
            SendStatus("continue");
        }

        /* If we have read all of the files, close the connection. */
        if(!m_headers.size())
        {
            /* Let the user know we received everything and close
               the connection. */
            if(Settings::Notify("FilesReceived"))
                emit Information(tr("%1 file(s) were received.").arg(m_files_received));

            disconnectFromHost();
        }
    }
}

void CFileReceiver::ParseHeaders(QVariantMap map)
{
    qDebug() << "Parsing headers received from client...";

    /* Ensure that the JSON map contains the name and list of files. */
    if(!map.contains("name") || !map.contains("files"))
    {
        emit Error(tr("Unable to parse the JSON response sent from the client."));
        disconnectFromHost();
    }

    /* Construct the headers for each of the files. */
    m_client_name = map["name"].toString();
    foreach(QVariant variant, map["files"].toList())
    {
        CFileHeader * header = new CFileHeader;
        QJson::QObjectHelper::qvariant2qobject(variant.toMap(), header);
        m_headers.append(header);
    }

    qDebug() << "Processed headers for" << m_headers.size() << "file(s).";

    /* Test the files against the current policy. */
    if(TestAgainstPolicy())
    {
        qDebug() << "Awaiting transfer of first file.";

        /* We are now waiting for the first file to arrive. */
        m_state = WaitingForTransfer;
        SendStatus("continue");
    }
    else
    {
        qDebug() << "Files rejected by current security policy. Aborting transfer.";

        SendStatus("rejected");
        disconnectFromHost();
    }
}

bool CFileReceiver::TestAgainstPolicy()
{
    int policy = Settings::Get("Security/IncomingPolicy").toInt();

    /* If the current policy is to accept all or none, we're done. */
    if(policy == Defaults::AcceptAll)  return true;
    if(policy == Defaults::AcceptNone) return false;

    /* Perhaps the policy is to prompt the user? If so, do it. */
    if(policy == Defaults::AcceptPrompt)
        return CAcceptPromptDialog(m_client_name, m_headers).exec();

    /* Note: we don't perform the RegEx match until we are actually
       ready to perform the transfer since we still need the headers
       for files we do not wish to transfer. */

    return true;
}

bool CFileReceiver::TestReceivedFilesDirectory(QDir dir)
{
    /* Ensure that the directory for the received files exists. */
    if(!dir.exists())
    {
        qDebug() << dir.absolutePath() << "does not exist. Attempting to create it.";

        if(!dir.mkpath(dir.absolutePath()))
        {
            emit Error(tr("Unable to create the directory for received files."));
            return false;
        }
    }

    return true;
}

bool CFileReceiver::TestChecksum(QByteArray & data)
{
    /* First make sure the checksum was actually included. */
    int index = data.indexOf(TerminatingCharacter);
    if(index == -1)
    {
        emit Error(tr("Unable to extract CRC checksum from received file. Skipping file."));
        return false;
    }

    /* Now verify the checksum that was provided and remove it. Generate our own checksum. */
    quint16 orig_crc = data.left(index).toUShort();
    data.remove(0, index + 1);
    quint16 calc_crc = qChecksum(data.data(), data.size());

    qDebug() << "Comparing original CRC checksum" << orig_crc << "to calculated checksum:" << calc_crc;

    if(orig_crc != calc_crc)
    {
        emit Error(tr("CRC integrity check failed. Skipping file."));
        return false;
    }

    return true;
}

void CFileReceiver::SaveFile(CFileHeader * header, QByteArray data)
{
    qDebug() << "Received file" << header->GetFilename() << ".";

    /* Check the security policy to see if we need to filter
       incoming filenames. */
    if(Settings::Get("Security/IncomingPolicy").toInt() == Defaults::AcceptPattern)
    {
        /* If so, perform the regular expression match. */
        QRegExp pattern(Settings::Get("Security/IncomingPattern").toString());
        if(pattern.indexIn(header->GetFilename()) == -1)
        {
            qDebug() << header->GetFilename() << "rejected based on regular expression pattern.";
            return;
        }
    }

    /* Ensure the directory where this file is supposed to go does indeed exist. */
    QDir storage_dir(Settings::Get("General/ReceivedFilesDirectory").toString());
    if(!TestReceivedFilesDirectory(storage_dir.absoluteFilePath(header->GetFilename().section("/", 0, -2))))
        return;

    QFile file(storage_dir.absoluteFilePath(header->GetFilename()));
    if(!file.open(QIODevice::WriteOnly))
    {
        emit Error(tr("Unable to store received file '%1'. Skipping file.").arg(header->GetFilename()));
        return;
    }

    /* Extract the CRC checksum if provided. */
    if(header->GetChecksum() && !TestChecksum(data))
        return;

    /* If the file is compressed, we need to uncompress it. */
    if(header->GetCompressed())
    {
        qDebug() << "Uncompressing the contents of the file with zlib.";
        data = qUncompress(data);
    }

    /* Write the data to disk. */
    file.write(data);
    ++m_files_received;

    /* ...and we're done with this file. */
    qDebug() << "File stored on the local filesystem.";
}
