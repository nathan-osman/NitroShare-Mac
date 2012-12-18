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

#ifndef CFILEHEADERMANAGER_H
#define CFILEHEADERMANAGER_H

#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QHashIterator>
#include <QList>
#include <QRegExp>
#include <QVariantList>
#include <QVariantMap>

#include "file/CFileHeader.h"

typedef QHashIterator<int, CFileHeader *> HeaderIterator;

class CFileHeaderManager
{
    public:

        CFileHeaderManager();
        ~CFileHeaderManager();

        /* Methods for adding files / directories for transferring. */
        void AddFile(QFileInfo, QString="");
        bool AddDirectory(QString);

        /* Methods for setting / getting the headers. */
        HeaderIterator GetIterator() const;
        void GetHeaders(QVariantList &);
        void ParseHeaders(const QVariantList &);

        /* Methods for deciding which files should be transferred. */
        void AcceptAll();
        void SetAcceptedFiles(const QVariantList &);
        void GetFilesMatchingRegEx(QRegExp, QVariantList &);

        /* Methods for sending and receiving parts of files. */
        bool GetNextChunk(QByteArray &, QVariantMap &);
        void WriteNextChunk(QDir, const QByteArray &, const QVariantMap &);

    private:

        /* Sequence number for the files (unique ID). */
        int m_sequence;

        /* Contains a mapping of file IDs => headers. */
        QHash<int, CFileHeader *> m_header_map;

        /* Contains a list of files to transfer. */
        QList<CFileHeader *> m_header_list;

        /* The current file we are transferring. */
        CFileHeader * m_current_file;
};

#endif // CFILEHEADERMANAGER_H