/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2009 by Andrea Diamantini <adjam7 at gmail dot com>
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


// Self Includes
#include "newtabpage.h"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "historymodels.h"
#include "bookmarksmanager.h"
#include "application.h"
#include "mainwindow.h"
#include "mainview.h"

// KDE Includes
#include <KStandardDirs>
#include <KIconLoader>
#include <KDebug>
#include <KConfig>
#include <KConfigGroup>

// Qt Includes
#include <QFile>


NewTabPage::NewTabPage()
{
    m_htmlFilePath = KStandardDirs::locate("data", "rekonq/htmls/home.html");
}


NewTabPage::~NewTabPage()
{
}


QString NewTabPage::newTabPageCode(const KUrl &url)
{
    QFile file(m_htmlFilePath);
    bool isOpened = file.open(QIODevice::ReadOnly);
    if (!isOpened)
    {
        kWarning() << "Couldn't open the home.html file";
        return QString("");
    }
    QString imagesPath = QString("file://") + KGlobal::dirs()->findResourceDir("data", "rekonq/pics/bg.png") + QString("rekonq/pics");
    QString menu = browsingMenu(url);
    
    QString speed;
    QString title;
    if(url == KUrl("about:closedTabs"))
    {
        speed = closedTabsPage();
        title = i18n("Closed Tabs");
    }
    if(url == KUrl("about:history"))
    {
        speed = historyPage();
        title = i18n("History");
    }
    if(url == KUrl("about:bookmarks"))
    {
        speed = bookmarksPage();
        title = i18n("Bookmarks");
    }
    if(url == KUrl("about:home") || url == KUrl("about:favorites"))
    {
        speed = favoritesPage();
        title = i18n("Favorites");
    }
    
    QString html = QString(QLatin1String(file.readAll()))
                        .arg(title)
                        .arg(imagesPath)
                        .arg(menu)
                        .arg(speed)
                        ;
                        
    return html;
}


QString NewTabPage::favoritesPage()
{
    QStringList names = ReKonfig::previewNames();
    QStringList urls = ReKonfig::previewUrls();

    QString speed = "<div class=\"favorites\">";
    for(int i=0; i<8; ++i)
    {
        speed += "<div class=\"thumbnail\">";
        speed += "<object type=\"application/image-preview\" data=\"" + urls.at(i) + "\" >";
        speed += "<param name=\"title\" value=\"" + names.at(i) + "\" />";
        speed += "<param name=\"index\" value=\"" + QString::number(i) + "\" />";
        speed += "<param name=\"isFavorite\" value=\"true\" />";
        speed += "</object>";
        speed += "</div>";
    }

    speed += "</div>";
    return speed;
}


// FIXME : port to new PreviewImage API to use...
QString NewTabPage::lastVisitedPage()
{
    QString last;
    QList<HistoryItem> history =  Application::historyManager()->history();
    for (int i = 0; i < 8 && i < history.size(); ++i) 
    {
        HistoryItem it = history.at(i);
        last += "<div class=\"thumbnail\">";
        last += "<object type=\"application/image-preview\" data=\"" + it.url +  "\" >";
        last += "</object>";
        last += "<br />";
        last += "<a href=\"" + it.url + "\">" + it.title + "</a></div>";
    }

    return last;

}


QString NewTabPage::browsingMenu(const KUrl &currentUrl)
{
    QString menu;
    
    KIconLoader *loader = KIconLoader::global();
    
    menu += "<div class=\"link";
    if(currentUrl == "about:favorites" || currentUrl == "about:home")
        menu += " current";
    menu += "\"><a href=\"about:favorites\">";
    menu += "<img src=\"file:///" + loader->iconPath("emblem-favorite", KIconLoader::Desktop || KIconLoader::SizeSmall) + "\" />";
    menu += i18n("Favorites");
    menu += "</a></div>";
    
    menu += "<div class=\"link";
    if(currentUrl == "about:closedTabs")
        menu += " current";
    menu += "\"><a href=\"about:closedTabs\">";
    menu += "<img src=\"file:///" + loader->iconPath("tab-close", KIconLoader::Desktop || KIconLoader::SizeSmall) + "\" />";
    menu += i18n("Closed Tabs");
    menu += "</a></div>";
    
    menu += "<div class=\"link";
    if(currentUrl == "about:bookmarks")
        menu += " current";
    menu += "\"><a href=\"about:bookmarks\">";
    menu += "<img src=\"file:///" + loader->iconPath("bookmarks", KIconLoader::Desktop || KIconLoader::SizeSmall) + "\" />";
    menu += i18n("Bookmarks");
    menu += "</a></div>";
    
    menu += "<div class=\"link";
    if(currentUrl == "about:history")
        menu += " current";
    menu += "\"><a href=\"about:history\">";
    menu += "<img src=\"file:///" + loader->iconPath("view-history", KIconLoader::Desktop || KIconLoader::SizeSmall) + "\" />";
    menu += i18n("History");
    menu += "</a></div>";
    
    return menu;
}


QString NewTabPage::historyPage()
{
    HistoryTreeModel *model = Application::historyManager()->historyTreeModel();
    
    QString history;
    int i = 0;
    do
    {
        QModelIndex index = model->index(i, 0, QModelIndex() );
        if(model->hasChildren(index))
        {
            history += "<h3>" + index.data().toString() + "</h3>";
            for(int j=0; j< model->rowCount(index); ++j)
            {
                QModelIndex son = model->index(j, 0, index );
                history += son.data(HistoryModel::DateTimeRole).toDateTime().toString("hh:mm");
                history += ' ';
                history += QString("<a href=\"") + son.data(HistoryModel::UrlStringRole).toString() + QString("\">") + 
                        son.data().toString() + QString("</a>");
                history += "<br />";
            }
        }
        i++;
    }
    while( model->hasIndex( i , 0 , QModelIndex() ) );

    history += "</table>";
    return history;
}


QString NewTabPage::bookmarksPage()
{
    KBookmarkGroup bookGroup = Application::bookmarkProvider()->rootGroup();
    if (bookGroup.isNull())
    {
        return QString("Error retrieving bookmarks!");
    }

    QString str;
    KBookmark bookmark = bookGroup.first();
    while (!bookmark.isNull())
    {
        str += createBookItem(bookmark);
        bookmark = bookGroup.next(bookmark);
    }
    return str;
}


QString NewTabPage::createBookItem(const KBookmark &bookmark)
{
    if (bookmark.isGroup())
    {
        QString result;
        KBookmarkGroup group = bookmark.toGroup();
        KBookmark bm = group.first();
        result += "<h3>" + bookmark.text() + "</h3>";
        result += "<p class=\"bookfolder\">";
        while (!bm.isNull())
        {
            result += createBookItem(bm);
            bm = group.next(bm);
        }
        result += "</p>";
        return result;
    }
 
    if(bookmark.isSeparator())
    {
        return QString("<hr />");
    }
    
    QString books = "<a href=\"" + bookmark.url().prettyUrl() + "\">" + bookmark.text() + "</a><br />";
    return books;
}


QString NewTabPage::closedTabsPage()
{
    QList<HistoryItem> links = Application::instance()->mainWindow()->mainView()->recentlyClosedTabs();
    QString closed;

    Q_FOREACH( const HistoryItem &item, links)
    {
        closed += "<div class=\"thumbnail\">";
        closed += "<object type=\"application/image-preview\" data=\"" + item.url + "\" >";
        closed += "<param name=\"title\" value=\"" + item.title + "\" />";
        closed += "</object>";
        closed += "</div>";
  }

  return closed;
}