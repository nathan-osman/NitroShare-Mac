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

#include <QObject>

class CFileHeader : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString filename          READ GetFilename         WRITE SetFilename)
    Q_PROPERTY(qint64  uncompressed_size READ GetUncompressedSize WRITE SetUncompressedSize)
    Q_PROPERTY(bool    compressed        READ GetCompressed       WRITE SetCompressed)
    Q_PROPERTY(bool    checksum          READ GetChecksum         WRITE SetChecksum)

    public:

        CFileHeader() : m_uncompressed_size(0), m_compressed(false), m_checksum(false) {}
        CFileHeader(QString full_filename) : m_full_filename(full_filename), m_uncompressed_size(0),
                                             m_compressed(false), m_checksum(false) {}

        QString GetFullFilename() { return m_full_filename; }

        QString GetFilename() { return m_filename; }
        void SetFilename(QString filename) { m_filename = filename; }

        qint64 GetUncompressedSize() { return m_uncompressed_size; }
        void SetUncompressedSize(qint64 size) { m_uncompressed_size = size; }

        bool GetCompressed() { return m_compressed; }
        void SetCompressed(qint64 compressed) { m_compressed = compressed; }

        bool GetChecksum() { return m_checksum; }
        void SetChecksum(bool checksum) { m_checksum = checksum; }

        bool GetContents(QByteArray &, qint64 &);

    private:

        QString m_full_filename;

        QString m_filename;
        qint64  m_uncompressed_size;
        bool    m_compressed;
        bool    m_checksum;
};

#endif // CFILEHEADER_H
