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
    : CBasicSocket(parent), m_state(WaitingForHeader)
{
    connect(this, SIGNAL(Data(QByteArray)), SLOT(OnData(QByteArray)));

    m_directory.setPath(Settings::Get("General/ReceivedFilesDirectory").toString());
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
        if(!data.contains(TerminatingCharacter))
        {
            emit Error(tr("Invalid file chunk received. Transfer aborted."));


            disconnectFromHost();
            return;
        }

        /* Break the data we received into the JSON map of sizes and the actual
           file data. */
        QList<QByteArray> parts = data.split(TerminatingCharacter);

        int map_size = parts[0].toInt();
        QVariantMap sizes_json = DecodeJSON(parts[1].left(map_size));

        /* Write the chunks to the file. */
        parts[1].chop(map_size);
        m_headers.WriteNextChunk(m_directory, parts[1], sizes_json);
   
    }
}

void CFileReceiver::ParseHeaders(QVariantMap map)
{
    qDebug() << "Parsing headers received from client...";

    /* Ensure that the JSON map contains the name and list of files. */
    if(!map.contains("machine") || !map.contains("files"))

    {
        emit Error(tr("Unable to parse the JSON response sent from the client."));
        disconnectFromHost();
    }

    /* Parse the headers for each of the files we've received. */
    m_headers.ParseHeaders(map["files"].toList());

    /* Test the files against the current policy. Note that we may
       receive a list of all the accepted files (a list of IDs). */
    QString status = "acceptall";
    QVariantList accepted_ids;

    if(TestAgainstPolicy(map["machine"].toString(), status, accepted_ids))
    {
        qDebug() << "Awaiting transfer of first file.";

        /* We are now waiting for the first file to arrive. */
        m_state = WaitingForTransfer;
        SendStatus(status);
    }
    else
    {
        qDebug() << "Files rejected by current security policy. Aborting transfer.";

        /* TODO: figure out what to do here, since the socket can be
           closed and this object deleted before we even get to this point. */

        //SendStatus("reject");
        //disconnectFromHost();
    }
}

bool CFileReceiver::TestAgainstPolicy(QString machine, QString & status, QVariantList & accepted_ids)
{
    int policy = Settings::Get("Security/IncomingPolicy").toInt();

    /* If the current policy is to accept all or none, we're done. */
    if(policy == Defaults::AcceptAll)  return true;
    if(policy == Defaults::AcceptNone) return false;

    /* Perhaps the policy is to prompt the user? If so, do it. */
    if(policy == Defaults::AcceptPrompt)

        return CAcceptPromptDialog(machine, m_headers).exec();

    /* Load the pattern and run it through the list of files. */
    QRegExp pattern(Settings::Get("Security/IncomingPattern").toString());
    m_headers.GetFilesMatchingRegEx(pattern, accepted_ids);
 
    status = "accept";

    /* We have succeeded if any files are accepted. */
    return accepted_ids.size();
}
