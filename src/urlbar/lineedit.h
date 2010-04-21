/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 by Paweł Prażak <pawelprazak at gmail dot com>
* Copyright (C) 2009 by Lionel Chauvin <megabigbug@yahoo.fr>
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


#ifndef LINEEDIT_H
#define LINEEDIT_H


// KDE Includes
#include <KLineEdit>
#include <KIcon>

// Qt Includes
#include <QToolButton>

// Forward Declarations
class QContextMenuEvent;
class QFocusEvent;
class QKeyEvent;
class QStyleOptionFrameV2;


class IconButton : public QToolButton
{
    Q_OBJECT

public:
    IconButton(QWidget *parent = 0);
};


// ------------------------------------------------------------------------------------


// Definitions
typedef QList<IconButton *> IconButtonPointerList;


class LineEdit : public KLineEdit
{
    Q_OBJECT

public:
    
    enum icon
    { 
        KGet    = 0x00000001,
        RSS     = 0x00000010,
        SSL     = 0x00000100,
    };   

    explicit LineEdit(QWidget *parent = 0);
    virtual ~LineEdit();
    
    IconButton *iconButton() const;

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    
    IconButton *addRightIcon(LineEdit::icon );

private slots:
    void clearRightIcons();

private:    
    IconButton *_icon;
    IconButtonPointerList _rightIconsList;
};

#endif // LINEEDIT_H
