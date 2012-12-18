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

#include <QFileDialog>
#include <QIntValidator>
#include <QMessageBox>
#include <QNetworkInterface>

#include "dialogs/CSettingsDialog.h"
#include "util/defaults.h"
#include "util/network.h"
#include "util/settings.h"
#include "ui_CSettingsDialog.h"

CSettingsDialog::CSettingsDialog()
    : ui(new Ui::CSettingsDialog)
{
    ui->setupUi(this);

    /* Make tool window for Mac so windows do not appear below active window */
    setWindowFlags(Qt::Tool);

    /* Set the validator for the intervals. */
    ui->BroadcastInterval->setValidator(new QIntValidator(this));
    ui->TimeoutInterval->setValidator(new QIntValidator(this));

    /* Refresh the network interface list. */
    RefreshNetworkInterfaces();

    /* Load the settings. */
    LoadSettings();
}

CSettingsDialog::~CSettingsDialog()
{
  delete ui;
}

void CSettingsDialog::OnBrowse()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select a Directory"),
                                                    ui->ReceivedFilesDirectory->text());
    if(!dir.isEmpty())
        ui->ReceivedFilesDirectory->setText(dir);
}

void CSettingsDialog::LoadSettings()
{
    ui->MachineName->setText(Settings::Get("General/MachineName").toString());
    ui->ReceivedFilesDirectory->setText(Settings::Get("General/ReceivedFilesDirectory").toString());
    ui->LoadAtStartup->setChecked(Settings::GetLoadAtStartup());
    ui->SystemTray->setChecked(Settings::GetSystemTray());
    ui->CompressFiles->setChecked(Settings::Get("General/CompressFiles").toBool());
    ui->CalculateChecksum->setChecked(Settings::Get("General/CalculateChecksum").toBool());

    int icon = Settings::Get("Appearance/Icon").toInt();
    if(icon == Defaults::IconDark)       ui->Dark->setChecked(true);
    else                                 ui->Light->setChecked(true);

    ui->NotifyNewPC->setChecked(Settings::Notify("NewPC"));
    ui->NotifyPCQuit->setChecked(Settings::Notify("PCQuit"));
    ui->NotifyFilesReceived->setChecked(Settings::Notify("FilesReceived"));
    ui->NotifyError->setChecked(Settings::Notify("Error"));

    int policy = Settings::Get("Security/IncomingPolicy").toInt();
    if(policy == Defaults::AcceptAll)          ui->AcceptAll->setChecked(true);
    else if(policy == Defaults::AcceptPattern) ui->AcceptPattern->setChecked(true);
    else if(policy == Defaults::AcceptPrompt)  ui->AcceptPrompt->setChecked(true);
    else                                       ui->AcceptNone->setChecked(true);
    ui->AcceptRegEx->setText(Settings::Get("Security/IncomingPattern").toString());

    /* Find the current network interface in the list. */
    QString interface = Settings::Get("Network/Interface").toString();
    int index = ui->InterfaceList->findData(interface);
    if(index != -1)
        ui->InterfaceList->setCurrentIndex(index);

    ui->BroadcastPort->setText(Settings::Get("Network/BroadcastPort").toString());
    ui->BroadcastInterval->setText(Settings::Get("Network/BroadcastInterval").toString());
    ui->TimeoutInterval->setText(Settings::Get("Network/TimeoutInterval").toString());
    ui->TransmissionPort->setText(Settings::Get("Network/TransmissionPort").toString());
}

void CSettingsDialog::RefreshNetworkInterfaces()
{
    ui->InterfaceList->clear();

    foreach(QNetworkInterface interface, Network::FindUsableInterfaces())
    {
        QHostAddress address;
        Network::FindIPv4Address(interface, &address);

        ui->InterfaceList->addItem(QString("%1 - %2").arg(interface.humanReadableName())
                                                     .arg(address.toString()),
                                   interface.name());
    }

    /* If there weren't any interfaces, disable the combo box. */
    if(!ui->InterfaceList->count())
        ui->InterfaceList->setEnabled(false);
}

void CSettingsDialog::accept()
{
    /* Make sure the user doesn't try to set a really low value for the broadcast interval. */
    if(ui->BroadcastInterval->text().toInt() < 2000 &&
       QMessageBox::warning(this, tr("Warning:"),
                            tr("Broadcast intervals less than 2 seconds are strongly discouraged "
                               "and will have a very negative impact on performance.\n\nAre you "
                               "sure you want to continue?"),
                            QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
        return;

    /* The dialog was accepted, so store the settings. */
    Settings::Set("General/MachineName",            ui->MachineName->text());
    Settings::Set("General/ReceivedFilesDirectory", ui->ReceivedFilesDirectory->text());
    Settings::SetLoadAtStartup(ui->LoadAtStartup->isChecked());
    Settings::SetSystemTray(ui->SystemTray->isChecked());
    Settings::Set("General/CompressFiles",          ui->CompressFiles->isChecked());
    Settings::Set("General/CalculateChecksum",      ui->CalculateChecksum->isChecked());

    int icon;
    if(ui->Dark->isChecked()) icon = Defaults::IconDark;
    else                      icon = Defaults::IconLight;
    Settings::Set("Appearance/Icon", icon);

    Settings::Set("Notifications/NewPC",         ui->NotifyNewPC->isChecked());
    Settings::Set("Notifications/PCQuit",        ui->NotifyPCQuit->isChecked());
    Settings::Set("Notifications/FilesReceived", ui->NotifyFilesReceived->isChecked());
    Settings::Set("Notifications/Error",         ui->NotifyError->isChecked());

    int policy;
    if(ui->AcceptAll->isChecked())          policy = Defaults::AcceptAll;
    else if(ui->AcceptPattern->isChecked()) policy = Defaults::AcceptPattern;
    else if(ui->AcceptPrompt->isChecked())  policy = Defaults::AcceptPrompt;
    else                                    policy = Defaults::AcceptNone;

    Settings::Set("Security/IncomingPolicy",  policy);
    Settings::Set("Security/IncomingPattern", ui->AcceptRegEx->text());

    int index = ui->InterfaceList->currentIndex();
    if(index != -1)
    {
        QString interface_name = ui->InterfaceList->itemData(index).toString();
        Settings::Set("Network/Interface", interface_name);
    }

    Settings::Set("Network/BroadcastPort",     ui->BroadcastPort->text());
    Settings::Set("Network/BroadcastInterval", ui->BroadcastInterval->text());
    Settings::Set("Network/TimeoutInterval",   ui->TimeoutInterval->text());
    Settings::Set("Network/TransmissionPort",  ui->TransmissionPort->text());

    done(Accepted);
}
