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


#include "file/CFileHeader.h"

CFileHeader::CFileHeader()
    : m_id(0), m_size(0), m_executable(false), m_read_only(false)
{
    //...
}

bool CFileHeader::OpenForReading()
{
    m_file.setFileName(m_absolute_filename);
    return m_file.open(QIODevice::ReadOnly);
}

bool CFileHeader::OpenForWriting(QDir dir)
{
    /* The filename we have stored is relative and therefore
       we need to resolve it against the supplied directory. */

    /* TODO: sanitize filename. */
    /* TODO: Create path if needed. */
    /* TODO: implement attributes. */

    m_file.setFileName(dir.absoluteFilePath(m_filename));
    return m_file.open(QIODevice::WriteOnly);
}

bool CFileHeader::IsComplete()
{
    return m_file.atEnd();
}

QByteArray CFileHeader::ReadChunk(qint64 max_size)
{
    return m_file.read(max_size);
}

void CFileHeader::WriteChunk(QByteArray chunk)
{
    m_file.write(chunk);
}
