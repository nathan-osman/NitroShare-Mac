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

#include "dialogs/CAboutDialog.h"
#include "util/definitions.h"
#include "ui_CAboutDialog.h"

CAboutDialog::CAboutDialog()
    : ui(new Ui::CAboutDialog)
{
    ui->setupUi(this);

    /* Make tool window for Mac so windows do not appear below active window */
    setWindowFlags(Qt::Tool);

    /* Retrieve the current version and display it in the dialog. */
    ui->Version->setText(tr("Version %1 - <i>built %2 %3</i>").arg(Definitions::Version)
                                                              .arg(__DATE__)
                                                              .arg(__TIME__));

    /* Gather the technical information. */
    GatherTechInfo();
}

CAboutDialog::~CAboutDialog()
{
    delete ui;
}

void CAboutDialog::GatherTechInfo()
{
    ui->TechData->insertRow(0);
    ui->TechData->setItem(0, 0, new QTableWidgetItem(tr("Debug build")));
#ifdef QT_DEBUG
    ui->TechData->setItem(0, 1, new QTableWidgetItem(tr("Yes")));
#else
    ui->TechData->setItem(0, 1, new QTableWidgetItem(tr("No")));
#endif

    ui->TechData->insertRow(0);
    ui->TechData->setItem(0, 0, new QTableWidgetItem(tr("Qt version")));
    ui->TechData->setItem(0, 1, new QTableWidgetItem(qVersion()));
}
