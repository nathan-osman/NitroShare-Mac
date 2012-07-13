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

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <QPoint>
#include <QVariantMap>

namespace Defaults {

    enum {
        AcceptAll,
        AcceptPrompt,
        AcceptPattern,
        AcceptNone
    };

    enum {
        IconDark,
        IconLight
    };

    /* This map contains all of the default settings for the application. */
    extern QVariantMap Map;

    QPoint GetDefaultShareBoxPos();
    void Init();
}

#endif // DEFAULTS_H
