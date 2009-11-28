/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
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


#ifndef BOOKMARKSPANEL_H
#define BOOKMARKSPANEL_H

// Qt Includes
#include <QDockWidget>

// Forward Declarations
class KUrl;
class QModelIndex;

class BookmarksPanel : public QDockWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(BookmarksPanel)

public:
    explicit BookmarksPanel(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~BookmarksPanel();

signals:
    void openUrl(const KUrl &);

private slots:
	void bookmarkActivated( const QModelIndex &index );

private:
	void setup();

    QWidget *ui;
};

#endif // BOOKMARKSPANEL_H