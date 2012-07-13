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
#include <QDir>
#include <QMessageBox>
#include <QUrl>
#include <QVariantList>
#include <qjson/qobjecthelper.h>

#include "file/CFileSender.h"
#include "util/settings.h"

CFileSender::CFileSender(QObject * parent, QStringList filenames)
    : CBasicSocket(parent), m_state(WaitingForConnection), m_oversized_prompt(false),
      m_total_uncompressed_bytes(0), m_total_uncompressed_bytes_so_far(0)
{
    connect(this, SIGNAL(bytesWritten(qint64)), SLOT(OnBytesWritten(qint64)));
    connect(this, SIGNAL(connected()),          SLOT(OnConnected()));
    connect(this, SIGNAL(Data(QByteArray)),     SLOT(OnData(QByteArray)));

    /* Look up the current settings for compression and checksum generation. */
    m_compress_files     = Settings::Get("General/CompressFiles").toBool();
    m_calculate_checksum = Settings::Get("General/CalculateChecksum").toBool();

    /* Immediately begin preparing the headers for the transfer. */
    PrepareHeaders(filenames);
}

CFileSender::~CFileSender()
{
    qDeleteAll(m_headers);
}

void CFileSender::Start(CMachine machine)
{
    qDebug() << "Attempting connection to" << machine.address.toString() << "...";

    /* Once we know the information for connecting to the remote
       machine, we immediately attempt a connection. */
    connectToHost(machine.address, machine.port);
}

void CFileSender::OnBytesWritten(qint64 amount)
{
    /* This signal is emitted for not only file data being
       written but also our headers, etc. So only do anything
       if we are in the process of transferring files. */
    if(m_state == TransferringFiles)
    {
        m_current_file_bytes_so_far += amount;

        /* Determine what percentage of the file has been transferred so far. */
        double percent_transferred = CalculateProgress(m_current_file_bytes_so_far, m_current_file_transfer_bytes);

        /* Next, determine what percentage of the total transfer this file represents. */
        double percent_of_total = CalculateProgress(m_current_file_uncompressed_bytes, m_total_uncompressed_bytes);

        /* Lastly add that amount to what has been transferred so far for the final number. */
        double progress = CalculateProgress(m_total_uncompressed_bytes_so_far, m_total_uncompressed_bytes);

        emit Progress((progress + percent_transferred * percent_of_total) * 100.0);

        /* If we've reached the end of the current file, then add the _uncompressed_
           size of this file to the total. */
        if(m_current_file_bytes_so_far >= m_current_file_transfer_bytes)
            m_total_uncompressed_bytes_so_far += m_current_file_uncompressed_bytes;
    }
}

void CFileSender::OnConnected()
{
    qDebug() << "Connection with remote server established.";

    /* Once we have established connection with the remote machine,
       we immediately begin sending headers for all of the files. */
    QVariantList header_list;
    foreach(CFileHeader * header, m_headers)
        header_list.append(QJson::QObjectHelper::qobject2qvariant(header));

    /* Prepare the JSON object we will send over the wire containing
       the header and some other basic information about the transfer. */
    QVariantMap map;
    map["name"]  = Settings::Get("General/MachineName");
    map["files"] = header_list;

    SendData(EncodeJSON(map));

    /* Our current state is now waiting for the response from the remote machine. */
    qDebug() << "File headers sent, awaiting server response.";
    m_state = WaitingForHeaderResponse;
}

void CFileSender::OnData(QByteArray data)
{
    qDebug() << "Data received from remote server" << data;

    /* The client sends us two messages during the process.
       - the first is immediately after we send the headers
       - the second occurs after each file transfer
       Both of these responses are in JSON format. */

    QVariantMap response = DecodeJSON(data);
    if(!response.contains("status") || response["status"] != "continue")
    {
        emit Error(tr("The remote machine aborted the transfer."));

        disconnectFromHost();
        return;
    }

    /* Regardless of our current state, we can assume our task here is to begin
       transferring the next file in the list (if there is one). */
    m_state = TransferringFiles;

    if(m_headers.size())
    {
        CFileHeader * header = m_headers.takeFirst();
        SendFile(header);
        delete header;
    }
    /* Otherwise we're done! Emit the signal and disconnect from the server. */
    else
    {
        emit Progress(ProgressComplete);
        disconnectFromHost();
    }
}

void CFileSender::PrepareFile(QFileInfo info, QString prefix)
{
    /* If one of the files is larger than 30 MB and compression is turned on, ask
       the user if they want to disable compression for the current transfer. */
    if(info.size() > 30000000 && m_compress_files && !m_oversized_prompt)
    {
        if(QMessageBox::warning(NULL, tr("Warning:"), tr("Transferring files larger than 30 MB with compression enabled can significantly slow down your computer.\n\nWould you like to temporarily disable compression for these files?"),
                                QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            m_compress_files = false;

        m_oversized_prompt = true;
    }

    CFileHeader * header = new CFileHeader(info.absoluteFilePath());
    header->SetFilename(prefix + info.fileName());
    header->SetUncompressedSize(info.size());
    header->SetCompressed(m_compress_files);
    header->SetChecksum(m_calculate_checksum);

    m_headers.append(header);

    /* Add the uncompressed size to our running total. */
    m_total_uncompressed_bytes += info.size();
}

void CFileSender::PrepareDirectory(QFileInfo info, QString prefix)
{
    QDir directory(info.absoluteFilePath());
    prefix = prefix + directory.dirName() + "/";

    foreach(QFileInfo info, directory.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot))
    {
        if(info.isDir()) PrepareDirectory(info, prefix);
        else             PrepareFile(info, prefix);
    }
}

void CFileSender::PrepareHeaders(QStringList filenames)
{
    foreach(QString filename, filenames)
    {
        /* If the file begins with "file://" remove that part. */
        if(filename.startsWith("file:///"))
            filename = QUrl(filename).toLocalFile();

        QFileInfo info(filename);

        /* Determine if this "file" is really a file or if it is instead
           a directory (in which case we need to travers it recursively). */
        if(info.isDir()) PrepareDirectory(info, "");
        else             PrepareFile(info, "");
    }
}

void CFileSender::SendFile(CFileHeader * header)
{
    qDebug() << "Preparing to transfer" << header->GetFilename() << "...";

    /* Now we actually begin the transmission of the file. */
    QByteArray contents;
    if(header->GetContents(contents, m_current_file_uncompressed_bytes))
    {
        /* Calculate the checksum if requested. */
        if(header->GetChecksum())
        {
            QByteArray crc = QByteArray::number(qChecksum(contents.data(), contents.size()));
            contents.prepend(crc + TerminatingCharacter);

            qDebug() << "Calculated CRC:" << crc;
        }

        /* Reset the transfer statistics. */
        m_current_file_bytes_so_far   = 0;
        m_current_file_transfer_bytes = contents.size();

        /* Send the actual data. */
        SendData(contents);
    }
    else
    {
        emit Error(tr("Unable to open '%1' for reading. Aborting transfer.").arg(header->GetFilename()));
        disconnectFromHost();
    }
}
