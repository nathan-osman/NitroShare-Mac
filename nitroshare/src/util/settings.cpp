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
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QUuid>

#include "util/defaults.h"
#include "util/settings.h"

QVariant Settings::Get(QString key)
{
    // Returns either:
    // * the user's preference if set
    // * the default if set
    // * an empty constructed value
    return QSettings().value(key, Defaults::Map[key]);
}

void Settings::Set(QString key, QVariant value)
{
    // This is basically just a wrapper for QSettings::setValue()
    QSettings().setValue(key, value);
}

bool Settings::Notify(QString type)
{
    return Get("Notifications/" + type).toBool();
}

QString Settings::GetID()
{
    QSettings settings;

    /* If there is an exisiting ID, return it. */
    if(settings.contains("Internal/ID"))
        return settings.value("Internal/ID").toString();

    /* Otherwise we need to generate one and store it. */
    QString uuid = QUuid::createUuid().toString();
    settings.setValue("Internal/ID", uuid);
    return uuid;
}

#ifdef Q_OS_LINUX
QDir GetStartupDir()
{
    /* Find and make sure the autostart directory exists. */
    QDir dir = QDir::home();
    if(!dir.exists(".config/autostart"))
        dir.mkpath(".config/autostart");

    dir.cd(".config/autostart");
    return dir.absolutePath();
}
#endif

bool Settings::GetLoadAtStartup()
{
    /* This code is platform dependent. */
#if defined(Q_OS_WIN)
    QSettings registry("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                       QSettings::NativeFormat);

    return registry.contains("NitroShare");
#elif defined(Q_OS_LINUX)
    return GetStartupDir().exists("extras-nitroshare.desktop");
#endif

    /* Just return false on other platforms. */
    return false;
}

void Settings::SetLoadAtStartup(bool load)
{
    /* This code is platform dependent. */
#if defined(Q_OS_WIN)
    QSettings registry("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                       QSettings::NativeFormat);

    if(load) registry.setValue("NitroShare", QCoreApplication::applicationFilePath());
    else     registry.remove("NitroShare");
#elif defined(Q_OS_LINUX)
    QString desktop_file = GetStartupDir().absoluteFilePath("nitroshare.desktop");

    if(load) QFile::copy(":/other/extras-nitroshare.desktop", desktop_file);
    else     QFile::remove(desktop_file);
#else
    /* Mark the variable as unused. */
    Q_UNUSED(load);
#endif
}
