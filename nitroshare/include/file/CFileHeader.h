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

#ifndef CFILEHEADER_H
#define CFILEHEADER_H

#include <QDir>
#include <QFile>
#include <QMap>
#include <QObject>

class CFileHeader : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int     id         READ GetID         WRITE SetID)
    Q_PROPERTY(QString filename   READ GetFilename   WRITE SetFilename)
    Q_PROPERTY(qint64  size       READ GetSize       WRITE SetSize)
    Q_PROPERTY(bool    executable READ GetExecutable WRITE SetExecutable)
    Q_PROPERTY(bool    read_only  READ GetReadOnly   WRITE SetReadOnly)
 

    public:

        CFileHeader();

        int GetID() { return m_id; }
        void SetID(int id) { m_id = id; }

        QString GetFilename() { return m_filename; }
        void SetFilename(QString filename) { m_filename = filename; }

        qint64 GetSize() { return m_size; }
        void SetSize(qint64 size) { m_size = size; }

        bool GetExecutable() { return m_executable; }
        void SetExecutable(bool executable) { m_executable = executable; }
 
        bool GetReadOnly() { return m_read_only; }
        void SetReadOnly(bool read_only) { m_read_only = read_only; }
 
        void SetAbsoluteFilename(QString filename) { m_absolute_filename = filename; }

        bool OpenForReading();
        bool OpenForWriting(QDir);

        bool IsComplete();

        QByteArray ReadChunk(qint64);
        void WriteChunk(QByteArray);
 

    private:

        QFile m_file;
        QString m_absolute_filename;

        int m_id;
        QString m_filename;
        qint64 m_size;
        bool m_executable;
        bool m_read_only;
};

#endif // CFILEHEADER_H
