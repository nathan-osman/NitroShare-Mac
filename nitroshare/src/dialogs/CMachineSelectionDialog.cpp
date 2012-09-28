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

#include "dialogs/CMachineSelectionDialog.h"
#include "ui_CMachineSelectionDialog.h"

CMachineSelectionDialog::CMachineSelectionDialog(MachineMap machine_list, QString label, bool include_select)
    : ui(new Ui::CMachineSelectionDialog)
{
    ui->setupUi(this);
    ui->MachinesLabel->setText(label);

    /* Make tool window for Mac so windows do not appear below active window */
    setWindowFlags(Qt::Tool);

    /* Populate the machine list. */
    for(MachineMap::const_iterator i = machine_list.constBegin();i != machine_list.constEnd();++i)
    {
        QListWidgetItem * item = new QListWidgetItem(i.value().name, ui->Machines);
        item->setData(Qt::UserRole, i.key());
    }

    /* Add the "select machine" option if the caller requested it. */
    if(include_select)
        ui->Machines->addItem(tr("[Select Machine]"));

    ui->Machines->setCurrentRow(0);
}

CMachineSelectionDialog::~CMachineSelectionDialog()
{
    delete ui;
}

QString CMachineSelectionDialog::GetMachineID()
{
    return ui->Machines->currentItem()->data(Qt::UserRole).toString();
}
