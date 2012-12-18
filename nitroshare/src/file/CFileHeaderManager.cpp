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
#include <QFileInfoList>
#include <QPair>
#include <QProgressDialog>
#include <QStack>
#include <qjson/qobjecthelper.h>

#include "file/CFileHeaderManager.h"

typedef QPair<QDir, QString> PathInfo;

CFileHeaderManager::CFileHeaderManager()
    : m_sequence(0), m_current_file(NULL)
{
    //...
}

CFileHeaderManager::~CFileHeaderManager()
{
    qDeleteAll(m_header_map);
    qDeleteAll(m_header_list);

    if(m_current_file)
        delete m_current_file;
}

void CFileHeaderManager::AddFile(QFileInfo info, QString prefix)
{
    /* Obtain the next number in the sequence for this file. */
    int id = m_sequence++;

    CFileHeader * header = new CFileHeader;
    header->SetAbsoluteFilename(info.absoluteFilePath());

    /* Set the attributes for the file. */
    header->SetID(id);
    header->SetFilename(prefix + "/" + info.fileName());
    header->SetSize(info.size());
    header->SetExecutable(info.isExecutable());
    header->SetReadOnly(!info.isWritable());

    /* Append this file to the map. */
    m_header_map.insert(id, header);
}

bool CFileHeaderManager::AddDirectory(QString directory)
{
    /* Originally, the code below was implemented as a recursive function, which
       was not terribly efficient. Therefore it has been rewritten as a loop
       (using a stack). When traversing really large directories, there should be
       some indication to the user that something is happening. Therefore, we show
       an indeterminate progress bar during the entire operation. */
    QProgressDialog dialog(QObject::tr("Scanning directories..."), QObject::tr("Abort"), 0, 0);
    dialog.setWindowModality(Qt::WindowModal);

    /* We traverse the directory with this stack, to which newly discovered directories
       are added. We begin by pushing the root directory on to the stack. */
    QStack<PathInfo> stack;
    stack.push(PathInfo(directory, ""));

    while(stack.size())
    {
        /* The user may have clicked the cancel button in the dialog. */
        if(dialog.wasCanceled())
            return false;

        /* Pop an item off the top of the stack and enumerate its contents. */
        PathInfo tos = stack.pop();
        QFileInfoList contents = tos.first.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

        foreach(QFileInfo info, contents)
        {
            /* It the item is a directory, add it to the stack, otherwise add it to the map. */
            if(info.isDir())
                stack.push(PathInfo(info.absoluteFilePath(), tos.second + "/" + info.fileName()));
            else
                AddFile(info, tos.second);
        }

        /* Process events for the progress dialog. */
        dialog.setValue(0);
    }

    return true;
}

HeaderIterator CFileHeaderManager::GetIterator() const
{
    return HeaderIterator(m_header_map);
}

void CFileHeaderManager::GetHeaders(QVariantList & header_list)
{
    foreach(CFileHeader * header, m_header_map.values())
        header_list.append(QJson::QObjectHelper::qobject2qvariant(header));
}

void CFileHeaderManager::ParseHeaders(const QVariantList & headers)
{
    foreach(QVariant variant, headers)
    {
        /* Create a file header pointer and decode the JSON into it. */
        CFileHeader * header = new CFileHeader;
        QJson::QObjectHelper::qvariant2qobject(variant.toMap(), header);

        /* Insert the header into the map, using its ID as the key. */
        m_header_map.insert(header->GetID(), header);
    }
}

void CFileHeaderManager::AcceptAll()
{
    /* Copy all of the values to the list and empty the map. */
    m_header_list = m_header_map.values();
    m_header_map.clear();
}

void CFileHeaderManager::SetAcceptedFiles(const QVariantList & accept_list)
{
    /* For each ID in the list, move it to the map. */
    foreach(QVariant id, accept_list)
        if(m_header_map.contains(id.toInt()))
            m_header_list.append(m_header_map.take(id.toInt()));

    /* TODO: decide whether the map should be emptied now or
       in the destructor (the current choice). */
}

void CFileHeaderManager::GetFilesMatchingRegEx(QRegExp pattern, QVariantList & accepted_ids)
{
    foreach(CFileHeader * header, m_header_map.values())
        if(pattern.indexIn(header->GetFilename()) >= 0)
            accepted_ids.append(header->GetID());
}

bool CFileHeaderManager::GetNextChunk(QByteArray & chunk, QVariantMap & sizes)
{
    /* TODO: chunk size should be a configurable setting. */
    const int chunk_size = 100000;

    /* Read chunks from files until we read a maximum of 'chunk_size'
       bytes in total. Keep track of the ID and size for each file we
       read from in this chunk. */
    while(chunk.size() <= chunk_size)
    {
        /* If we don't have a file on the go, then attempt to start the next one. */
        while(!m_current_file)
        {
            /* If there are no more files, we're done. */
            if(!m_header_list.size())
                return false;

            m_current_file = m_header_list.takeFirst();

            /* If we can't open the file for reading, then delete the header for it and
               move on to the next file. */
            if(!m_current_file->OpenForReading())
            {
                delete m_current_file;
                m_current_file = NULL;
            }
        }

        /* Read as much as we can (to fill up this chunk) from the current
           file, adding the ID and size to our variant map. */
        QByteArray data = m_current_file->ReadChunk(chunk_size - chunk.size());
        sizes[QString::number(m_current_file->GetID())] = chunk.size();

        chunk.append(data);

        /* If we read everything (or 0 bytes) from the file, then delete it. */
        if(m_current_file->IsComplete() || !data.size())
        {
            delete m_current_file;
            m_current_file = NULL;
        }
    }

    /* The return value of this method does NOT indicate success or failure
       but rather whether or not there are chunks remaining to be read. */
    return true;
}

void CFileHeaderManager::WriteNextChunk(QDir directory, const QByteArray & chunk, const QVariantMap & sizes)
{
    /* Enumerate each of the entries in 'sizes', writing the data
       to the appropriate file. We assume that we are writing to files
       sequentially (not necessarily the order, but we write the entire
       contents of each file sequentially). */
    int offset = 0;

    for(QVariantMap::const_iterator i = sizes.constBegin(); i != sizes.constEnd(); ++i)
    {
        /* Get the data that we'll be writing to disk. */
        QByteArray data = chunk.mid(offset, i.value().toInt());
        offset += i.value().toInt();

        if(m_header_map.contains(i.key().toInt()))
        {
            if(!m_current_file || m_current_file->GetID() != i.key().toInt())
            {
                /* We need to close the current file (if open), and open
                   the next one. */
                if(m_current_file)
                    delete m_current_file;

                m_current_file = m_header_map.take(i.key().toInt());
                m_current_file->OpenForWriting(directory);
            }

            /* Write the chunk to the file. */
            m_current_file->WriteChunk(data);
        }
    }
}