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

#include "dialogs/CAcceptPromptDialog.h"
#include "ui_CAcceptPromptDialog.h"

CAcceptPromptDialog::CAcceptPromptDialog(QString name, const CFileHeaderManager & headers)

    : ui(new Ui::CAcceptPromptDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowStaysOnTopHint);

    /* Make the first column a little bit wider. */
    ui->FileList->setColumnWidth(ui->FileList->columnAt(1), 250);

    /* Enter the files into the table, totalling up the size. */
    int num_files = 0;
    qint64 total_size = 0;

    HeaderIterator i = headers.GetIterator();
    while(i.hasNext())
    {
        i.next();

        ui->FileList->insertRow(0);
        ui->FileList->setItem(0, 0, new QTableWidgetItem(i.value()->GetFilename()));

        QTableWidgetItem * cell = new QTableWidgetItem(FormatSize(i.value()->GetSize()));
        cell->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->FileList->setItem(0, 1, cell);

        ++num_files;
        total_size += i.value()->GetSize();
    }

    /* Set the labels according to the information supplied. */
    ui->DescriptionLabel->setText(tr("%1 would like to send you the following file(s) (%2):", "",
                                     num_files).arg(name).arg(FormatSize(total_size)));
    ui->AcceptLabel->setText(tr("Do you wish to accept the file(s)?", "", num_files));

}

CAcceptPromptDialog::~CAcceptPromptDialog()
{
    delete ui;
}

QString CAcceptPromptDialog::FormatSize(qint64 size)
{
    if(size < 1024)
        return tr("%1 bytes").arg(size);
    else if(size < 1048576)
        return tr("%1 KB").arg((size / 102) / 10.0);
    else if(size < 1073741824)
        return tr("%1 MB").arg((size / 104858) / 10.0);
    else
        return tr("%1 GB").arg((size / 107374182) / 10.0);
}
