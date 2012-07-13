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

#include <QPixmap>

#include "dialogs/CFirstStartWizard.h"
#include "util/settings.h"
#include "ui_CFirstStartWizard.h"

void CWizardPage::AddRequiredField(QWidget * widget, const char * property, const char * signal)
{
    /* Generate a unique name for each field. */
    static int unique_id = 0;
    registerField(QString("%1*").arg(unique_id++), widget, property, signal);
}

CFirstStartWizard::CFirstStartWizard()
    : ui(new Ui::CFirstStartWizard)
{
    ui->setupUi(this);
    setPixmap(LogoPixmap, QPixmap(":/images/config.png"));

    /* Register each of the required fields. */
    ui->MachineNamePage->AddRequiredField(ui->MachineName);
    ui->BroadcastDiscoveryPage->AddRequiredField(ui->BroadcastDiscovery, "interface", SIGNAL(FoundInterface()));

    OnNotificationsChanged(StandardNotifications);

    /* Load the defaults. */
    ui->MachineName->setText(Settings::Get("General/MachineName").toString());
}

CFirstStartWizard::~CFirstStartWizard()
{
    delete ui;
}

void CFirstStartWizard::OnFinished()
{
    /* Store the settings we have collected. */
    Settings::Set("General/MachineName", ui->MachineName->text());
    Settings::Set("Network/Interface",   ui->BroadcastDiscovery->GetInterface());

    /* Grab the slider value and determine which notifications should be turned on. */
    int slider_value = ui->NotificationsSlider->value();

    Settings::Set("Notifications/NewPC", slider_value == AllNotifications);
    Settings::Set("Notifications/PCQuit", slider_value == AllNotifications);
    Settings::Set("Notifications/FilesReceived", slider_value == AllNotifications ||
                                                 slider_value == StandardNotifications);
    Settings::Set("Notifications/Error", slider_value != NoNotifications);
}

void CFirstStartWizard::OnNotificationsChanged(int value)
{
    QString description;

    switch(value)
    {
        case NoNotifications:       description = tr("<b>No notifications:</b> you will not receive any notifications."); break;
        case ErrorNotifications:    description = tr("<b>Error notifications:</b> you will only receive a notification when a critical error occurs."); break;
        case StandardNotifications: description = tr("<b>Standard notifications:</b> you will receive a notification when errors occur and when files are received."); break;
        case AllNotifications:      description = tr("<b>All notifications:</b> you will receive notifications for all events, including new machines joining and leaving the local network."); break;
    }

    ui->NotificationsDescription->setText(description);
}

void CFirstStartWizard::OnPageChanged(int id)
{
    if(id == BroadcastDiscoveryPage)
        ui->BroadcastDiscovery->StartBroadcasting();
}

int CFirstStartWizard::nextId() const
{
    /* Returns the next page to display - we use the default unless
       the user is on the first page and wants to skip this wizard. */
    if(currentId() == WelcomePage && ui->ManualConfigure->isChecked())
        return FinishPage;

    return QWizard::nextId();
}
