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


// Self Includes
#include "webtab.h"
#include "webtab.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "urlbar.h"
#include "previewselectorbar.h"
#include "rsswidget.h"
#include "walletbar.h"
#include "webpage.h"

// KDE Includes
#include <KWebWallet>

// Qt Includes
#include <QtGui/QVBoxLayout>


WebTab::WebTab(QWidget *parent)
        : QWidget(parent)
        , m_webView(new WebView(this))
        , m_urlBar(new UrlBar(this))
        , m_progress(0)
        , m_part(0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setMargin(0);
    l->setSpacing(0);

    l->addWidget(m_webView);
    m_webView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // fix focus handling
    setFocusProxy(m_webView);

    KWebWallet *wallet = m_webView->page()->wallet();

    if (wallet)
    {
        connect(wallet, SIGNAL(saveFormDataRequested(const QString &, const QUrl &)),
                this, SLOT(createWalletBar(const QString &, const QUrl &)));
    }

    connect(m_webView, SIGNAL(loadProgress(int)), this, SLOT(updateProgress(int)));
    connect(m_webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
    connect(m_webView, SIGNAL(titleChanged(const QString &)), this, SIGNAL(titleChanged(const QString &)));
}


WebTab::~WebTab()
{
    m_walletBar.clear();
    m_previewSelectorBar.clear();

    delete m_part;
}


KUrl WebTab::url()
{
    if(page() && page()->isOnRekonqPage())
    {
        kDebug() << "REKONQ PAGE. URL = " << page()->loadingUrl();
        return page()->loadingUrl();
    }
    
    return view()->url();
}


void WebTab::updateProgress(int p)
{
    m_progress = p;
    emit loadProgressing();
}


void WebTab::loadFinished(bool)
{
    m_progress = 0;
}


bool WebTab::isPageLoading()
{
    return m_progress != 0;
}


void WebTab::createWalletBar(const QString &key, const QUrl &url)
{
    // check if the url is in the wallet blacklist
    QString urlString = url.toString();
    QStringList blackList = ReKonfig::walletBlackList();
    if (blackList.contains(urlString))
        return;

    KWebWallet *wallet = page()->wallet();
    if(m_walletBar.isNull()) {
        m_walletBar = new WalletBar(this);
        m_walletBar.data()->onSaveFormData(key, url);
        qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, m_walletBar.data() );
    } else {
        disconnect(wallet);
        m_walletBar.data()->notifyUser();
    }

    connect(m_walletBar.data(), SIGNAL(saveFormDataAccepted(const QString &)),
            wallet, SLOT(acceptSaveFormDataRequest(const QString &)), Qt::UniqueConnection);
    connect(m_walletBar.data(), SIGNAL(saveFormDataRejected(const QString &)),
            wallet, SLOT(rejectSaveFormDataRequest(const QString &)), Qt::UniqueConnection);
}


void WebTab::createPreviewSelectorBar(int index)
{
    if(m_previewSelectorBar.isNull()) {
        m_previewSelectorBar = new PreviewSelectorBar(index, this);
        qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, m_previewSelectorBar.data());
    } else {
        disconnect(m_previewSelectorBar.data());
        m_previewSelectorBar.data()->setIndex(index);
        m_previewSelectorBar.data()->notifyUser();
    }

    connect(page(),             SIGNAL(loadStarted()),      m_previewSelectorBar.data(), SLOT(loadProgress()), Qt::UniqueConnection);
    connect(page(),             SIGNAL(loadProgress(int)),  m_previewSelectorBar.data(), SLOT(loadProgress()), Qt::UniqueConnection);
    connect(page(),             SIGNAL(loadFinished(bool)), m_previewSelectorBar.data(), SLOT(loadFinished()), Qt::UniqueConnection);
    connect(page()->mainFrame(), SIGNAL(urlChanged(QUrl)),  m_previewSelectorBar.data(), SLOT(verifyUrl()), Qt::UniqueConnection);
}

void WebTab::insertBar(NotificationBar *bar)
{
    qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, bar);
}


bool WebTab::hasRSSInfo()
{
    QWebElementCollection col = page()->mainFrame()->findAllElements("link[type=\"application/rss+xml\"]");
    col.append(page()->mainFrame()->findAllElements("link[type=\"application/atom+xml\"]"));
    if (col.count() != 0)
        return true;

    return false;
}


void WebTab::showRSSInfo(QPoint pos)
{
    QWebElementCollection col = page()->mainFrame()->findAllElements("link[type=\"application/rss+xml\"]");
    col.append(page()->mainFrame()->findAllElements("link[type=\"application/atom+xml\"]"));

    QMap<KUrl, QString> map;

    foreach(const QWebElement &el, col)
    {
        QString urlString;
        if (el.attribute("href").startsWith(QL1S("http")))
            urlString = el.attribute("href");
        else
        {
            KUrl u = url();
            // NOTE
            // cd() is probably better than setPath() here,
            // for all those url sites just having a path
            if (u.cd(el.attribute("href")))
                urlString = u.toMimeDataString();
        }

        QString title = el.attribute("title");
        if (title.isEmpty())
            title = el.attribute("href");

        map.insert(KUrl(urlString), title);
    }

    RSSWidget *widget = new RSSWidget(map, window());
    widget->showAt(pos);
}


void WebTab::setPart(KParts::ReadOnlyPart *p, const KUrl &u)
{
    if(p)
    {
        // Ok, part exists. Insert & show it..
        m_part = p;
        qobject_cast<QVBoxLayout *>(layout())->insertWidget(1, p->widget());
        p->openUrl(u);
        m_webView->hide();

        emit titleChanged(u.url());
        return;
    }

    if(!m_part)
        return;
    
    // Part NO more exists. Let's clean up from webtab
    m_webView->show();
    qobject_cast<QVBoxLayout *>(layout())->removeWidget(m_part->widget());
    delete m_part;
    m_part = 0;
}
