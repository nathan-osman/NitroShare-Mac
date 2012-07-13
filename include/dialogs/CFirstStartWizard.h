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

#ifndef CFIRSTSTARTWIZARD_H
#define CFIRSTSTARTWIZARD_H

#include <QWizard>
#include <QWizardPage>

class CWizardPage : public QWizardPage
{
    Q_OBJECT

    public:

        void AddRequiredField(QWidget *, const char * = NULL, const char * = NULL);
};

namespace Ui {
    class CFirstStartWizard;
}

class CFirstStartWizard : public QWizard
{
    Q_OBJECT
        
    public:

        enum {
            WelcomePage,
            MachineNamePage,
            BroadcastDiscoveryPage,
            NotificationsPage,
            FinishPage
        };

        enum {
            NoNotifications = 0,
            ErrorNotifications,
            StandardNotifications,
            AllNotifications
        };

        CFirstStartWizard();
        ~CFirstStartWizard();

    private slots:

        void OnFinished();
        void OnNotificationsChanged(int);
        void OnPageChanged(int);
        
    private:

        int nextId() const;

        Ui::CFirstStartWizard *ui;
};

#endif // CFIRSTSTARTWIZARD_H
