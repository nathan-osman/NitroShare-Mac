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
#include <QBrush>
#include <QColor>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QPixmap>
#include <QTimer>
#include <QUrl>

#include "sharebox/CShareBox.h"
#include "util/settings.h"

/* The default dimensions and opacity for the widget. */
const int CShareBox::DefaultHeight = 140;
const int CShareBox::DefaultWidth  = 140;

const double CShareBox::DefaultOpacity = 0.7;

CShareBox::CShareBox()
    : QWidget(QApplication::desktop()), m_draw_scale(140.0f / 256.0f),
      m_highlight(false), m_dragging(false), m_progress(CFileSender::ProgressNone)
{
    /* The share box should (ideally) be frameless, transparent, and stick
       to the desktop as a widget. This isn't as easy as it sounds, however and
       the values below were arrived upon by much trial and error. They seem
       to work on all platforms / window managers tested so far. */
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_MacNoShadow);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);

    setAcceptDrops(true);
    setWindowTitle("NitroShare Share Box"); /* Ideally this is never seen anywhere. */

    Init();

    QFile::copy(":/images/multiple.png", "/Applications/nitroshare.app/Contents/Resources/multiple.png");
    QFile::copy(":/images/single.png", "/Applications/nitroshare.app/Contents/Resources/single.png");
    QFile::copy(":/images/sharebox.png", "/Applications/nitroshare.app/Contents/Resources/sharebox.png");
}

void CShareBox::OnProgress(int progress)
{
    m_progress = qMin(progress, 100);

    /* If the transfer completed or an error occured, then set a timer
       after which progress will be reset to ProgressNone. */
    if(m_progress < 0)
        QTimer::singleShot(3000, this, SLOT(ResetProgress()));

    update();
}

void CShareBox::Init()
{
    /* Set the window's size and opacity. */
    resize(DefaultWidth, DefaultHeight);
    setWindowOpacity(DefaultOpacity);

    /* Set the colors and fonts. */
    m_text_font.setPixelSize(28);
    m_text_normal.setRgb(215, 215, 215);
    m_text_highlight.setRgb(127, 127, 255);
    m_border_highlight.setRgb(127, 127, 215);

    /* Create the context menu and show the box. */
    CreateContextMenu();
    showNormal();
}

void CShareBox::CreateContextMenu()
{
    m_context.addAction(tr("Add ShareBox"), this, SIGNAL(AddShareBox()));
    m_context.addAction(tr("Remove"),       this, SIGNAL(Remove()));
    m_context.addSeparator();
    m_context.addAction(tr("Quit NitroShare"), QApplication::instance(), SLOT(quit()));
}

void CShareBox::SetHighlight(bool highlight)
{
    m_highlight = highlight;
    setWindowOpacity(highlight?1.0f:DefaultOpacity);

    update();
}

void CShareBox::contextMenuEvent(QContextMenuEvent * event)
{
    /* The context menu should be shown where the user right-clicked. */
    m_context.popup(event->globalPos());
    event->accept();
}

void CShareBox::dragEnterEvent(QDragEnterEvent * event)
{
    /* We only accept URLs being dragged on to the box and only if
       we are not already involved in transferring a file. */
    if(event->mimeData()->hasUrls() && m_progress == CFileSender::ProgressNone)
    {
        SetHighlight(true);
        event->accept();
    }
}

void CShareBox::dragLeaveEvent(QDragLeaveEvent * event)
{
    SetHighlight(false);
    event->accept();
}

void CShareBox::dropEvent(QDropEvent * event)
{
    qDebug("Files were dropped on the share box.");

    /* Technically we should never receive anything that we don't support because
       we rejected everything else above. But once can never be too careful... */
    if(event->mimeData()->hasUrls() && m_progress == CFileSender::ProgressNone)
    {
        /* Convert the list of URLs to filenames. */
        QStringList filenames;
        foreach(QUrl url, event->mimeData()->urls())
            filenames.append(url.toLocalFile());

        /* Emit the signal that files were dropped and accept the drop. */
        emit FilesDropped(filenames, m_id);
        event->accept();
    }
    else
        qDebug("MIME type of dropped content cannot be converted to a URL.");

    SetHighlight(false);
}

void CShareBox::mousePressEvent(QMouseEvent * event)
{
    /* Only begin dragging the window if the left mouse button is pressed. */
    if(event->button() == Qt::LeftButton)
    {
        m_dragging     = true;
        m_original_pos = event->pos();
    }
}

void CShareBox::mouseMoveEvent(QMouseEvent * event)
{
    QPoint dest = event->globalPos() - m_original_pos;

    /* Snap the box to a 10 pixel grid. (Making it easier to align boxes
       next to each other on the desktop. */
    dest /= 10;
    dest *= 10;

    move(dest);
}

void CShareBox::mouseReleaseEvent(QMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_dragging = false;
        emit Moved();
    }
}

void CShareBox::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    /* Turn on anti-aliasing and set the scale for drawing operations
       so that everything looks good when the widget's size is changed. */
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.scale(m_draw_scale, m_draw_scale);

    /* First draw the background. */
    painter.drawPixmap(0, 0, 256, 256, QPixmap("/Applications/nitroshare.app/Contents/Resources/sharebox.png"));

    /* Then draw the blue border if applicable. */
    if(m_highlight)
    {
        painter.setPen(m_border_highlight);
        painter.drawRoundedRect(10, 10, 236, 236, 4.0f, 4.0f);
    }

    painter.setFont(m_text_font);

    /* Draw the shadow for the text followed by the actual text. */
    QString text = m_id.isEmpty()?tr("[Select Machine]"):m_name;

    /* Make sure the width of the text doesn't exceed what the box can display. */
    if(painter.boundingRect(0, 156, 256, 40, Qt::AlignHCenter | Qt::AlignVCenter, text).width() > 240)
    {
        QRect bounding_rect;

        do
        {
            text.truncate(text.length() - 1);
            bounding_rect = painter.boundingRect(0, 156, 256, 40, Qt::AlignHCenter | Qt::AlignVCenter,
                                                 text + "...");

        } while(bounding_rect.width() > 240);

        text.append("...");
    }

    painter.setPen(Qt::black);
    painter.drawText(2, 158, 256, 40, Qt::AlignHCenter | Qt::AlignVCenter, text);

    /* Draw the actual text. */
    painter.setPen(m_highlight?m_text_highlight:m_text_normal);
    painter.drawText(0, 156, 256, 40, Qt::AlignHCenter | Qt::AlignVCenter, text);

    /* Draw the progress bar if there is a transfer in progress, otherwise draw either
       a checkmark, 'x', or three circles (if nothing is happening). */
    if(m_progress >= 0)
    {
        painter.setBrush(QBrush(QColor(63, 63, 63)));
        painter.drawRoundedRect(26, 200, 204, 14, 8.0f, 8.0f);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(m_text_normal));
        painter.drawRoundedRect(28, 202, 200 * (m_progress / 100.0f), 10, 6.0f, 6.0f);
    }
    else if(m_progress == CFileSender::ProgressComplete || m_progress == CFileSender::ProgressError)
        painter.drawPixmap(112, 200, 32, 32,
                           QPixmap((m_progress == CFileSender::ProgressComplete)?
                                       ":/images/success.png":":/images/error.png"));
    else
    {
        painter.drawEllipse(90,  210, 16, 16);
        painter.drawEllipse(120, 210, 16, 16);
        painter.drawEllipse(150, 210, 16, 16);
    }

    /* Lastly draw the icon. */
    painter.drawPixmap(80, 16, 96, 96, QPixmap(m_id.isEmpty()?"/Applications/nitroshare.app/Contents/Resources/multiple.png":"/Applications/nitroshare.app/Contents/Resources/single.png"));
}
