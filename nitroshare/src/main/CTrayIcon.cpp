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

#include <QCoreApplication>
#include <QFileDialog>
#include <QIcon>
#include <QMessageBox>

#include "dialogs/CAboutDialog.h"
#include "dialogs/CMachineSelectionDialog.h"
#include "dialogs/CSettingsDialog.h"
#include "file/CFileSender.h"
#include "main/CTrayIcon.h"
#include "util/defaults.h"
#include "util/settings.h"
extern void qt_mac_set_dock_menu(QMenu *);


CTrayIcon::CTrayIcon()
    : m_send_files_menu(tr("Send Files To")), m_send_directory_menu(tr("Send Directory To"))
{
    /* We want to receive notifications when errors or status information
       is emitted from the broadcast or file servers. */
    connect(&m_broadcast_server, SIGNAL(Error(QString)),       SLOT(OnError(QString)));
    connect(&m_broadcast_server, SIGNAL(Information(QString)), SLOT(OnInformation(QString)));

    connect(&m_file_server, SIGNAL(Error(QString)),       SLOT(OnError(QString)));
    connect(&m_file_server, SIGNAL(Information(QString)), SLOT(OnInformation(QString)));

    /* We need to make the icon visible before attempting anything else because
       otherwise any error messages won't be displayed if the icon is hidden.
       First we create the context menu and set the icon. */
    CreateContextMenu();

    connect(&m_send_files_menu,     SIGNAL(aboutToShow()), SLOT(UpdateSendFilesMenu()));
    connect(&m_send_directory_menu, SIGNAL(aboutToShow()), SLOT(UpdateSendDirectoryMenu()));

    /* Initialize the tray icon and make it visible. */
    Init();
    setVisible(true);

    /* Initialize the broadcast and file servers. */
    m_broadcast_server.Init();
    m_file_server.Init();

    /* Create the share boxes. */
    CreateShareBoxes();
}

CTrayIcon::~CTrayIcon()
{
    qDeleteAll(m_shareboxes);
}

void CTrayIcon::Init()
{
    int icon = Settings::Get("Appearance/Icon").toInt();
    if(icon == Defaults::IconDark) setIcon(QIcon(":/icons/dark.png"));
    else                           setIcon(QIcon(":/icons/light.png"));
}

void CTrayIcon::OnAbout()
{
    CAboutDialog().exec();
}

void CTrayIcon::OnAddShareBox()
{
    CMachineSelectionDialog dialog(m_broadcast_server.GetMachineMap(),
                                   tr("Please select a machine to create a ShareBox for:"), true);

    if(dialog.exec())
    {
        QByteArray name;

        MachineMap machines = m_broadcast_server.GetMachineMap();
        if(machines.contains(dialog.GetMachineID()))
            name = machines[dialog.GetMachineID()].name;

        /* Determine where to put this new box. */
        QPoint pos;
        if(m_shareboxes.size())
        {
            pos = m_shareboxes.last()->pos();
            pos.setX(pos.x() - 146);
        }
        else
            pos = Defaults::GetDefaultShareBoxPos();

        CShareBox * box = CreateShareBox(dialog.GetMachineID(), name);
        box->move(pos);

        UpdateShareBoxes();
    }
}

void CTrayIcon::OnError(QString message)
{
    if(Settings::Notify("Error"))
        showMessage(tr("NitroShare Error"), message, QSystemTrayIcon::Critical);
}

void CTrayIcon::OnInformation(QString message)
{
    showMessage(tr("NitroShare Notification"), message);
}

void CTrayIcon::OnRemoveShareBox()
{
    CShareBox * box = qobject_cast<CShareBox *>(sender());
    m_shareboxes.removeOne(box);
    box->deleteLater();
    UpdateShareBoxes();
}

void CTrayIcon::OnSendFiles()
{
    /* Ask the user for the files they wish to send. */
    QStringList filenames = QFileDialog::getOpenFileNames(NULL, tr("Select Files to Send"));
    if(filenames.size())
    {
        QAction * action = qobject_cast<QAction *>(sender());
        OnSendFiles(filenames, action->data().toString());
    }
}

void CTrayIcon::OnSendFiles(QStringList filenames, QString id)
{
    if(id.isEmpty())
    {
        /* There's no point showing the dialog to the user if there are no
           machines currently available to send to. */
        if(m_broadcast_server.AnyMachinesOnline())
        {
            CMachineSelectionDialog dialog(m_broadcast_server.GetMachineMap(),
                                           tr("Please select a machine to send these files to:"));

            if(dialog.exec()) id = dialog.GetMachineID();
            else              return;
        }
        else
        {
            QMessageBox::critical(NULL, tr("Error:"), tr("There are currently no machines on the local network that can receive files."));
            return;
        }
    }
    /* If this method was invoked with a specific ID, then ensure
       that the machine does indeed exist on the network currently. */
    else if(!m_broadcast_server.MachineExists(id))
    {
        QMessageBox::critical(NULL, tr("Error:"), tr("This machine is currently offline."));
        return;
    }

    /* Create the client that will manage the transmission of files. */
    CFileSender * client = new CFileSender(this, filenames);
    connect(client, SIGNAL(Error(QString)),       SLOT(OnError(QString)));
    connect(client, SIGNAL(Information(QString)), SLOT(OnInformation(QString)));

    /* If the object emitting the signal is a CShareBox, then we can use
    it to display progress of the transfer. */
    CShareBox * box = qobject_cast<CShareBox *>(sender());
    if(box)
    {
        connect(client, SIGNAL(Progress(int)),  box, SLOT(OnProgress(int)));
        connect(client, SIGNAL(Error(QString)), box, SLOT(OnError()));
    }

    client->Start(m_broadcast_server.GetMachine(id));
}

void CTrayIcon::OnSendDirectory()
{
    /* Ask the user for the directory they wish to send. */
    QString directory = QFileDialog::getExistingDirectory(NULL, tr("Select a Directory"));
    if(!directory.isEmpty())
    {
        QAction * action = qobject_cast<QAction *>(sender());
        OnSendFiles(QStringList(directory), action->data().toString());
    }
}

void CTrayIcon::OnSettings()
{
    /* Display the settings dialog. */
    if(CSettingsDialog().exec())
    {
        /* If the user accepted the dialog, then re-initialize
           the broadcast and file servers. */
        Init();
        m_broadcast_server.Init();
        m_file_server.Init();
    }
}

void CTrayIcon::UpdateSendFilesMenu()
{
    CreateMachineMenu(m_send_files_menu, SLOT(OnSendFiles()));
}

void CTrayIcon::UpdateSendDirectoryMenu()
{
    CreateMachineMenu(m_send_directory_menu, SLOT(OnSendDirectory()));
}

void CTrayIcon::UpdateShareBoxes()
{
    /* Loop through each of the share boxes, storing their information. */
    QVariantList boxes;

    foreach(CShareBox * box, m_shareboxes)
    {
        QVariantMap map;
        map["id"]   = box->GetMachineID();
        map["name"] = box->GetMachineName();
        map["pos"]  = box->pos();

        boxes << QVariant(map);
    }

    /* Store the list. */
    Settings::Set("ShareBox/Instances", QVariant(boxes));
}

void CTrayIcon::CreateContextMenu()
{
    QMenu * menu = new QMenu;

    menu->addMenu(&m_send_files_menu);
    menu->addMenu(&m_send_directory_menu);
    menu->addSeparator();
    menu->addAction(tr("Add ShareBox"), this, SLOT(OnAddShareBox()));
    menu->addSeparator();
    menu->addAction(tr("Settings"), this, SLOT(OnSettings()));
    menu->addAction(tr("About"),    this, SLOT(OnAbout()));
    menu->addSeparator();
    menu->addAction(tr("Exit"), QCoreApplication::instance(), SLOT(quit()));

    setContextMenu(menu);
}

void CTrayIcon::CreateMachineMenu(QMenu & menu, const char * slot)
{
    /* Clear the current contents of the menu. */
    menu.clear();

    /* If there are no machines currently, add a disabled
       "empty" item to the menu. Otherwise add one for each machine. */
    if(m_broadcast_server.AnyMachinesOnline())
    {
        /* Loop through each machine, adding it to the menu. */
        for(MachineMap::const_iterator i = m_broadcast_server.GetMachineMap().constBegin();
            i != m_broadcast_server.GetMachineMap().constEnd(); ++i)
        {
            QAction * action = menu.addAction(i.value().name, this, slot);
            action->setData(i.key());
        }
    }
    else
    {
        QAction * action = menu.addAction(tr("[empty]"));
        action->setEnabled(false);
    }
}

CShareBox * CTrayIcon::CreateShareBox(QString id, QByteArray name)
{
    CShareBox * box = new CShareBox();
    box->SetMachineInfo(id, name);

    connect(box, SIGNAL(AddShareBox()), SLOT(OnAddShareBox()));
    connect(box, SIGNAL(FilesDropped(QStringList,QString)),
                 SLOT(OnSendFiles(QStringList,QString)));
    connect(box, SIGNAL(Moved()), SLOT(UpdateShareBoxes()));
    connect(box, SIGNAL(Remove()), SLOT(OnRemoveShareBox()));

    m_shareboxes.append(box);
    return box;
}

void CTrayIcon::CreateShareBoxes()
{
    /* Load the list of ShareBoxes from the settings file. Each
       one contains a serialized set of properties indicating what
       it is for and where it goes onscreen, etc. */
    QVariantList boxes = Settings::Get("ShareBox/Instances").toList();

    foreach(QVariant variant, boxes)
    {
        QVariantMap map = variant.toMap();

        /* Each entry contains the position of the box and the unique ID
           of the machine it is for. */
        CreateShareBox(map["id"].toString(), map["name"].toByteArray())->move(map["pos"].toPoint());
    }

    /* Make the menu items for the Mac Dock. Items are duplicates of the
       context menu, Except for Send Files and Send Directory, also Exit is removed
       to avoid duplicate functionality. */

    QMenu * menu = new QMenu;

    menu->addAction(tr("Send Files"), this, SLOT(OnSendFiles()));
    menu->addAction(tr("Send Directory"), this, SLOT(OnSendDirectory()));
    menu->addSeparator();
    menu->addAction(tr("Add ShareBox"), this, SLOT(OnAddShareBox()));
    menu->addSeparator();
    menu->addAction(tr("Settings"), this, SLOT(OnSettings()));
    menu->addAction(tr("About"),    this, SLOT(OnAbout()));

    qt_mac_set_dock_menu(menu);
}
