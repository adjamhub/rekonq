/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009-2010 by Lionel Chauvin <megabigbug@yahoo.fr>
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* ============================================================ */

#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H


// Rekonq Includes
#include "rekonq_defines.h"

// KDE Includes
#include <KService>

//Qt Includes
#include <QString>


class SearchEngine
{
public:
    static void reload();
    static QString delimiter();
    static KService::Ptr defaultEngine();
    static KService::List favorites();
    static KService::Ptr fromString(const QString &text);
    static QString buildQuery(KService::Ptr engine, const QString &text);
    static QString extractQuery(const QString &text);
    
private:
    static bool m_loaded;
    static QString m_delimiter;
    static KService::List m_favorites;
    static KService::Ptr m_defaultEngine;
};

#endif
