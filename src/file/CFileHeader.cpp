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

#include <QFile>

#include "file/CFileHeader.h"

bool CFileHeader::GetContents(QByteArray & contents, qint64 & uncompressed_size)
{
    QFile file(m_full_filename);

    /* Attempt to open the file for reading. */
    if(file.open(QIODevice::ReadOnly))
    {
        /* Grab the file size and read the file contents. */
        uncompressed_size = file.size();
        contents          = file.readAll();

        /* If the user has requested compression, then perform that. */
        if(m_compressed)
            contents = qCompress(contents, 9);

        return true;
    }

    /* The function calling this needs to take care of displaying an error message. */
    return false;
}
