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

#ifndef CBASICSOCKET_H
#define CBASICSOCKET_H

#include <QTcpSocket>
#include <QVariantMap>

/* Basically the rationale for having this class is the tedious
   amount of error checking code that both the CFileSender and
   CFileReceiver would otherwise have to ensure proper cleanup
   and error handling. This class performs a number of those
   common tasks itself, making the two above classes much simpler.
   It also contains some common code for reading from a socket. */
class CBasicSocket : public QTcpSocket
{
    Q_OBJECT

    public:

        /* The terminating character is used as a delimiter of sorts
           for each message sent to the remote host. */
        static const char TerminatingCharacter;

        CBasicSocket(QObject *);
        virtual ~CBasicSocket();

    signals:

        void Data(QByteArray);
        void Error(QString);
        void Information(QString);

    protected:

        /* Utility JSON methods for parsing and serializing data. */
        QByteArray EncodeJSON(QVariantMap);
        QVariantMap DecodeJSON(QByteArray);

        /* Methods for sending data and status updates to the remote host. */
        void SendData(QByteArray);
        void SendStatus(QString);

        /* Utility method for calculating floating point progress. */
        double CalculateProgress(double current, double total) { return current / total; }

    private slots:

        void OnError(QAbstractSocket::SocketError);
        void OnReadyRead();

    private:

        QByteArray m_read_buffer;
        qint64     m_expected_size;
};

#endif // CBASICSOCKET_H
