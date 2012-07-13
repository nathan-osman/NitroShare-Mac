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

#ifndef CTRAYICON_H
#define CTRAYICON_H

#include <QList>
#include <QMenu>
#include <QStringList>
#include <QSystemTrayIcon>

#include "discovery/CBroadcastServer.h"
#include "file/CFileServer.h"
#include "sharebox/CShareBox.h"

class CTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT

    public:

        CTrayIcon();
        virtual ~CTrayIcon();

        void Init();

    public slots:

        void OnSendFiles();
        void OnSendFiles(QStringList, QString);
        void OnSendDirectory();

    private slots:

        void OnAbout();
        void OnAddShareBox();
        void OnError(QString);
        void OnInformation(QString);
        void OnRemoveShareBox();
        void OnSettings();

        void UpdateSendFilesMenu();
        void UpdateSendDirectoryMenu();
        void UpdateShareBoxes();

    private:

        void CreateContextMenu();
        void CreateMachineMenu(QMenu &, const char *);
        CShareBox * CreateShareBox(QString, QByteArray);
        void CreateShareBoxes();

        CBroadcastServer m_broadcast_server;
        CFileServer      m_file_server;

        QMenu m_send_files_menu,
              m_send_directory_menu;

        QList<CShareBox *> m_shareboxes;

};

#endif // CTRAYICON_H
