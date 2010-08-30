/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Nils Weigel <nehlsen at gmail dot com>
* Copyright (C) 2010 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2010 by Yoann Laissus <yoann dot laissus at gmail dot com>
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


// Self Includes
#include "bookmarkspanel.h"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "panels/urlfilterproxymodel.h"
#include "application.h"
#include "bookmarkprovider.h"
#include "bookmarkstreemodel.h"
#include "bookmarkscontextmenu.h"
#include "bookmarkowner.h"
#include "paneltreeview.h"


BookmarksPanel::BookmarksPanel(const QString &title, QWidget *parent, Qt::WindowFlags flags)
        : UrlPanel(title, parent, flags)
        , m_loadingState(false)
{
    setObjectName("bookmarksPanel");
    setVisible(ReKonfig::showBookmarksPanel());
}


BookmarksPanel::~BookmarksPanel()
{
    ReKonfig::setShowBookmarksPanel(!isHidden());
}


void BookmarksPanel::startLoadFoldedState()
{
    m_loadingState = true;
    loadFoldedState(QModelIndex());
    m_loadingState = false;
}


void BookmarksPanel::contextMenu(const QPoint &pos)
{
    if (m_loadingState)
        return;

    BookmarksContextMenu menu(bookmarkForIndex( m_treeView->indexAt(pos) ),
                              Application::bookmarkProvider()->bookmarkManager(),
                              Application::bookmarkProvider()->bookmarkOwner(),
                              this
                             );

    menu.exec(m_treeView->mapToGlobal(pos));
}


void BookmarksPanel::contextMenuItem(const QPoint &pos)
{
    contextMenu(pos);
}


void BookmarksPanel::contextMenuGroup(const QPoint &pos)
{
    contextMenu(pos);
}


void BookmarksPanel::contextMenuEmpty(const QPoint &pos)
{
    contextMenu(pos);
}


void BookmarksPanel::deleteBookmark()
{
    QModelIndex index = m_treeView->currentIndex();
    if (m_loadingState || !index.isValid())
        return;

    Application::bookmarkProvider()->bookmarkOwner()->deleteBookmark(bookmarkForIndex(index));
}


void BookmarksPanel::onCollapse(const QModelIndex &index)
{
    if (m_loadingState)
        return;

    bookmarkForIndex(index).internalElement().setAttribute("folded", "yes");
    emit expansionChanged();
}


void BookmarksPanel::onExpand(const QModelIndex &index)
{
    if (m_loadingState)
        return;

    bookmarkForIndex(index).internalElement().setAttribute("folded", "no");
    emit expansionChanged();
}


void BookmarksPanel::loadFoldedState(const QModelIndex &root)
{
    QAbstractItemModel *model = m_treeView->model();
    int count = model->rowCount(root);
    QModelIndex index;

    for (int i = 0; i < count; ++i)
    {
        index = model->index(i, 0, root);
        if (index.isValid())
        {
            KBookmark bm = bookmarkForIndex(index);
            if (bm.isGroup())
            {
                m_treeView->setExpanded(index, bm.toGroup().isOpen());
                loadFoldedState(index);
            }
        }
    }
}


void BookmarksPanel::setup()
{
    UrlPanel::setup();
    kDebug() << "Bookmarks panel...";

    connect(m_treeView, SIGNAL(delKeyPressed()), this, SLOT(deleteBookmark()));
    connect(m_treeView, SIGNAL(collapsed(const QModelIndex &)), this, SLOT(onCollapse(const QModelIndex &)));
    connect(m_treeView, SIGNAL(expanded(const QModelIndex &)), this, SLOT(onExpand(const QModelIndex &)));

    startLoadFoldedState();
}


KBookmark BookmarksPanel::bookmarkForIndex(const QModelIndex &index)
{
    if (!index.isValid())
        return KBookmark();

    const UrlFilterProxyModel *proxyModel = static_cast<const UrlFilterProxyModel*>(index.model());
    QModelIndex originalIndex = proxyModel->mapToSource(index);

    BtmItem *node = static_cast<BtmItem*>(originalIndex.internalPointer());
    return node->getBkm();
}


QAbstractItemModel* BookmarksPanel::getModel()
{
    BookmarksTreeModel *model = new BookmarksTreeModel(this);
    connect(model, SIGNAL(bookmarksUpdated()), this, SLOT(startLoadFoldedState()));
    return model;
}
