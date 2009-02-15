/* ============================================================
 *
 * This file is a part of the rekonq project
 *
 * Copyright (C) 2008 by Andrea Diamantini <adjam7 at gmail dot com>
 *
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */


// Self Includes
#include "application.h"
#include "application.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "mainwindow.h"
#include "cookiejar.h"
#include "history.h"
#include "networkaccessmanager.h"
#include "mainview.h"
#include "webview.h"
#include "download.h"

// KDE Includes
#include <KCmdLineArgs>
#include <KAboutData>
#include <KConfig>
#include <kio/job.h>
#include <kio/jobclasses.h>

// Qt Includes
#include <QBuffer>
#include <QDir>
#include <QTextStream>
#include <QDesktopServices>
#include <QFileOpenEvent>
#include <QLocalServer>
#include <QLocalSocket>
#include <QNetworkProxy>
#include <QWebSettings>
#include <QDebug>



HistoryManager *Application::s_historyManager = 0;
NetworkAccessManager *Application::s_networkAccessManager = 0;


Application::Application()
    : KUniqueApplication()
//     , m_localServer(0)
{
    m_mainWindow = new MainWindow();
    m_mainWindow->show();

//     QLocalSocket socket;
//     socket.connectToServer(serverName);
//     if (socket.waitForConnected(500)) 
//     {
//         QTextStream stream(&socket);
//         int n = args->count();
//         if (n > 1)
//             stream << args->arg(n-1);
//         else
//             stream << QString();
//         stream.flush();
//         socket.waitForBytesWritten();
//         return;
//     }
// 
//     KApplication::setQuitOnLastWindowClosed(true);
// 
//     m_localServer = new QLocalServer(this);
//     connect(m_localServer, SIGNAL(newConnection()), this, SLOT(newLocalSocketConnection()));
//     if (!m_localServer->listen(serverName)) 
//     {
//         if (m_localServer->serverError() == QAbstractSocket::AddressInUseError
//             && QFile::exists(m_localServer->serverName()))
//         {
//             QFile::remove(m_localServer->serverName());
//             m_localServer->listen(serverName);
//         }
//     }
// 
//     QDesktopServices::setUrlHandler(QLatin1String("http"), this, "openUrl");
//     QString localSysName = QLocale::system().name();
// 
//     KConfig config("rekonqrc");
//     KConfigGroup group = config.group("sessions");
//     m_lastSession = group.readEntry( QString("lastSession"), QByteArray() );

    setWindowIcon( KIcon("rekonq") );

    QTimer::singleShot(0, this, SLOT( postLaunch() ) );
}


Application::~Application()
{
    delete s_networkAccessManager;
}


int Application::newInstance()
{
    KCmdLineArgs::setCwd(QDir::currentPath().toUtf8());
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    static bool first = true;
    if (args->count() > 0) 
    {
        for (int i = 0; i < args->count(); ++i) 
        {
            KUrl url = MainWindow::guessUrlFromString( args->arg(i) );
            newTab();
            mainWindow()->loadUrl( url );
        }
    } 
    else 
    {
        if( !first || !isSessionRestored()) 
        {
            newTab();
            mainWindow()->slotHome();
        }
    }
    first = false;
    args->clear();
    return 0;
}


Application *Application::instance()
{
    return (static_cast<Application *>(QCoreApplication::instance()));
}


void Application::postLaunch()
{
    // set Icon Database Path to store "favicons" associated with web sites 
    QString directory = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    if ( directory.isEmpty() )
    {
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
    }
    QWebSettings::setIconDatabasePath(directory);

    Application::historyManager();
}


void Application::downloadUrl(const KUrl &srcUrl, const KUrl &destUrl)
{
    new Download( srcUrl, destUrl );
}


// void Application::saveSession()
// {
//     QWebSettings *globalSettings = QWebSettings::globalSettings();
//     if ( globalSettings->testAttribute( QWebSettings::PrivateBrowsingEnabled ) )
//         return;
// 
//     KConfig config("rekonqrc");
//     KConfigGroup group = config.group("sessions");
//     QByteArray data;
//     QBuffer buffer(&data);
//     QDataStream stream(&buffer);
//     buffer.open(QIODevice::ReadWrite);
// 
//     stream << m_mainWindows.count();
//     for (int i = 0; i < m_mainWindows.count(); ++i)
//     {
//         stream << m_mainWindows.at(i)->saveState();
//     }
//     
//     group.writeEntry( QString("lastSession"), data );
// }


void Application::openUrl(const KUrl &url)
{
    mainWindow()->loadUrl(url);
}


MainWindow *Application::mainWindow()
{
    return m_mainWindow;
}


WebView *Application::newTab()
{
    return m_mainWindow->tabWidget()->newTab();
}


// void Application::newLocalSocketConnection()
// {
//     QLocalSocket *socket = m_localServer->nextPendingConnection();
//     if (!socket)
//         return;
//     socket->waitForReadyRead(1000);
//     QTextStream stream(socket);
//     QString url;
//     stream >> url;
//     if (!url.isEmpty()) 
//     {
//         mainWindow()->tabWidget()->newTab();
//         openUrl(url);
//     }
//     delete socket;
//     mainWindow()->raise();
//     mainWindow()->activateWindow();
// }



CookieJar *Application::cookieJar()
{
    return (CookieJar*)networkAccessManager()->cookieJar();
}


NetworkAccessManager *Application::networkAccessManager()
{
    if (!s_networkAccessManager) 
    {
        s_networkAccessManager = new NetworkAccessManager();
        s_networkAccessManager->setCookieJar(new CookieJar);
    }
    return s_networkAccessManager;
}



HistoryManager *Application::historyManager()
{
    if (!s_historyManager) 
    {
        s_historyManager = new HistoryManager();
        QWebHistoryInterface::setDefaultInterface(s_historyManager);
    }
    return s_historyManager;
}


KIcon Application::icon(const KUrl &url) const
{
    KIcon icon = KIcon( QWebSettings::iconForUrl(url) );
    if (!icon.isNull())
        return icon;
    if (m_defaultIcon.isNull())
        m_defaultIcon = KIcon("kde");
    return m_defaultIcon;
}

