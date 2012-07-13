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

#ifndef CSHAREBOX_H
#define CSHAREBOX_H

#include <QMenu>
#include <QStringList>
#include <QVariantMap>
#include <QWidget>

#include "file/CFileSender.h"

class CShareBox : public QWidget
{
    Q_OBJECT

    public:

        static const int DefaultHeight;
        static const int DefaultWidth;

        static const double DefaultOpacity;

        CShareBox();

        QString GetMachineID() { return m_id; }
        QByteArray GetMachineName() { return m_name; }

        void SetMachineInfo(QString id, QByteArray name) { m_id = id; m_name = name; }

    public slots:

        void OnError() { OnProgress(CFileSender::ProgressError); }
        void OnProgress(int);

    signals:

        void AddShareBox();
        void FilesDropped(QStringList, QString);
        void Moved();
        void Remove();

    private slots:

        void ResetProgress() { m_progress = CFileSender::ProgressNone; update(); }

    private:

        void Init();
        void CreateContextMenu();
        void SetHighlight(bool);

        void contextMenuEvent(QContextMenuEvent *);
        void dragEnterEvent(QDragEnterEvent *);
        void dragLeaveEvent(QDragLeaveEvent *);
        void dropEvent(QDropEvent *);
        void mousePressEvent(QMouseEvent *);
        void mouseMoveEvent(QMouseEvent *);
        void mouseReleaseEvent(QMouseEvent *);
        void paintEvent(QPaintEvent *);

        QString    m_id;
        QByteArray m_name;

        QMenu m_context;

        QFont   m_text_font;
        QColor  m_text_normal;
        QColor  m_text_highlight;
        QColor  m_border_highlight;

        float m_draw_scale;

        bool m_highlight;
        bool m_dragging;
        int  m_progress;

        QPoint m_original_pos;
};

#endif // CSHAREBOX_H
