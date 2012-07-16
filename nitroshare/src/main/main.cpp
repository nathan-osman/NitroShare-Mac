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

#include <QApplication>
#include <QDebug>
#include <QLocale>
#include <QMessageBox>
#include <QTranslator>

#include "dialogs/CFirstStartWizard.h"
#include "main/CTrayIcon.h"
#include "rpc/CRPCServer.h"
#include "util/defaults.h"
#include "util/settings.h"

void InitLocale()
{
    /* Determine what the user's default locale is set to
       and attempt to load the appropriate translation. */
    QString locale = QLocale::system().name();
    qDebug() << "Current locale is" << locale;

    QTranslator * translator = new QTranslator(QApplication::instance());
    if(!translator->load(QString("translations/%1.qm").arg(locale)))
        qDebug() << "Unable to load translation" << QString("translations/%1.qm").arg(locale);
    else
        QApplication::installTranslator(translator);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    /* Set information about the application. */
    a.setApplicationName("NitroShare");
    a.setOrganizationName("Nathan Osman");
    a.setOrganizationDomain("quickmediasolutions.com");

    /* Set icon for Mac */
    QApplication::setWindowIcon(QIcon("icon.icns"));
    // TODO: change icon by copying into app bundle, current
    // implementation only changes icon after the program
    // launches, and shows default icon while launching.

    /* Initialize the locale and load the application defaults. */
    InitLocale();
    Defaults::Init();

    // TODO: we need to send a message to anything occupying the RPC
    // port to see if it is NitroShare - if it is, then tell the user
    // that we are already running. If not, then try another port and
    // write it to the configuration file immediately.

    /* Determine if NitroShare is already running. */
    CRPCServer server;
    if(!server.Init())
    {
        qDebug() << "Unable to initialize RPC server. Assuming the application is already running.";

        QMessageBox::critical(NULL, QObject::tr("Error:"), QObject::tr("NitroShare is already running. Please close the current instance before launching the application again."));
        return 1;
    }

    /* If this is the first time the application is started, display
       the initial setup and configuration wizard. */
    if(!Settings::Get("Internal/FirstStart").toBool())
    {
        qDebug() << "First run, displaying first start wizard.";

        if(!CFirstStartWizard().exec())
            return 1;

        /* If we reach this point, the user completed the wizard and
           we should avoid showing it in the future. */
        Settings::Set("Internal/FirstStart", true);
    }

    CTrayIcon icon;
    icon.connect(&server, SIGNAL(SendFiles(QStringList,QString)),
                          SLOT(OnSendFiles(QStringList,QString)),
                 Qt::QueuedConnection);
    
    return a.exec();
}
