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
#include <qjson/parser.h>
#include <qjson/serializer.h>

#include "file/CBasicSocket.h"

const char CBasicSocket::TerminatingCharacter = '\x04';

CBasicSocket::CBasicSocket(QObject * parent)
    : QTcpSocket(parent), m_expected_size(0)
{
    connect(this, SIGNAL(disconnected()), SLOT(deleteLater()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(OnError(QAbstractSocket::SocketError)));
    connect(this, SIGNAL(readyRead()),    SLOT(OnReadyRead()));
}

CBasicSocket::~CBasicSocket()
{
    qDebug() << "Socket closed.";
}

QByteArray CBasicSocket::EncodeJSON(QVariantMap input)
{
    return QJson::Serializer().serialize(QVariant(input));
}

QVariantMap CBasicSocket::DecodeJSON(QByteArray input)
{
    return QJson::Parser().parse(input).toMap();
}

void CBasicSocket::SendData(QByteArray data)
{
    /* In order to send data across the socket, we
       write the size of the data as text and then
       send our terminating character. */
    QByteArray data_size = QByteArray::number(data.size());
    data_size.append(TerminatingCharacter);
    write(data_size);
    write(data);
}

void CBasicSocket::SendStatus(QString status)
{
    /* Sends a JSON response over the wire. */
    QVariantMap map;
    map["status"] = status;

    SendData(EncodeJSON(map));
}

void CBasicSocket::OnError(QAbstractSocket::SocketError)
{
    /* Notify the user that something went wrong. */
    emit Error(errorString());

    /* If the socket is open, disconnect - otherwise delete it. */
    if(state() == QAbstractSocket::ConnectedState)
        disconnectFromHost();
    else
        deleteLater();
}

void CBasicSocket::OnReadyRead()
{
    /* Read all of the new data and append it to the buffer. */
    m_read_buffer += readAll();

    /* Note: this is probably the first time in 5 years I've used
       a do...while loop. */
    do
    {
        /* If we haven't read the size of the data we're receiving yet
           then check what we have to see if it contains the terminator. */
        if(!m_expected_size && m_read_buffer.contains(TerminatingCharacter))
        {
            int offset = m_read_buffer.indexOf(TerminatingCharacter);
            m_expected_size = m_read_buffer.left(offset).toLongLong();

            /* Remove the size of the data from the buffer. */
            m_read_buffer.remove(0, offset + 1);
        }

        /* If we have read the size, check to see if we have that many
           bytes of data in the buffer. If so, process it. */
        if(m_expected_size && m_read_buffer.size() >= m_expected_size)
        {
            emit Data(m_read_buffer);

            /* Reset everything for the next transfer, removing this
               current message from the buffer. */
            m_read_buffer.remove(0, m_expected_size);
            m_expected_size = 0;
        }
    } while(!m_expected_size && m_read_buffer.contains(TerminatingCharacter));
}
