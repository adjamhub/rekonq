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
#include "settingsdialog.h"
#include "settingsdialog.moc"

// Auto Includes
#include "rekonq.h"

// Local Includes
#include "application.h"
#include "mainwindow.h"
#include "webtab.h"
#include "searchengine.h"

// Widget Includes
#include "adblockwidget.h"
#include "networkwidget.h"
#include "generalwidget.h"
#include "appearancewidget.h"
#include "webkitwidget.h"
#include "tabswidget.h"

// KDE Includes
#include <KConfig>
#include <KStandardDirs>
#include <KPageWidgetItem>
#include <KShortcutsEditor>
#include <KCModuleInfo>
#include <KCModuleProxy>

// Qt Includes
#include <QtGui/QWidget>


class Private
{
private:
    Private(SettingsDialog *parent);

private:
    GeneralWidget *generalWidg;
    TabsWidget *tabsWidg;
    AppearanceWidget *appearanceWidg;
    WebKitWidget *webkitWidg;
    NetworkWidget *networkWidg;
    AdBlockWidget *adBlockWidg;
    
    KCModuleProxy *ebrowsingModule;

    KShortcutsEditor *shortcutsEditor;

    friend class SettingsDialog;
};


Private::Private(SettingsDialog *parent)
{
    KPageWidgetItem *pageItem;

    // -- 1
    generalWidg = new GeneralWidget(parent);
    generalWidg->layout()->setMargin(0);
    pageItem = parent->addPage(generalWidg, i18n("General"));
    pageItem->setIcon(KIcon("rekonq"));

    // -- 2
    tabsWidg = new TabsWidget(parent);
    tabsWidg->layout()->setMargin(0);
    pageItem = parent->addPage(tabsWidg, i18n("Tabs"));
    pageItem->setIcon(KIcon("tab-duplicate"));
    
    // -- 3
    appearanceWidg = new AppearanceWidget(parent);
    appearanceWidg->layout()->setMargin(0);
    pageItem = parent->addPage(appearanceWidg, i18n("Appearance"));
    pageItem->setIcon(KIcon("preferences-desktop-font"));
    
    // -- 4
    webkitWidg = new WebKitWidget(parent);
    webkitWidg->layout()->setMargin(0);
    pageItem = parent->addPage(webkitWidg, i18n("WebKit"));
    QString webkitIconPath = KStandardDirs::locate("appdata", "pics/webkit-icon.png");
    KIcon webkitIcon = KIcon(QIcon(webkitIconPath));
    pageItem->setIcon(webkitIcon);
    
    // -- 5
    networkWidg = new NetworkWidget(parent);
    networkWidg->layout()->setMargin(0);
    pageItem = parent->addPage(networkWidg , i18n("Network"));
    pageItem->setIcon(KIcon("preferences-system-network"));

    // -- 6
    adBlockWidg = new AdBlockWidget(parent);
    adBlockWidg->layout()->setMargin(0);
    pageItem = parent->addPage(adBlockWidg , i18n("Ad Block"));
    pageItem->setIcon(KIcon("preferences-web-browser-adblock"));

    // -- 7
    shortcutsEditor = new KShortcutsEditor(Application::instance()->mainWindow()->actionCollection(), parent);
    pageItem = parent->addPage(shortcutsEditor , i18n("Shortcuts"));
    pageItem->setIcon(KIcon("configure-shortcuts"));

    // -- 8
    KCModuleInfo ebrowsingInfo("ebrowsing.desktop");
    ebrowsingModule = new KCModuleProxy(ebrowsingInfo, parent);
    pageItem = parent->addPage(ebrowsingModule, i18n(ebrowsingInfo.moduleName().toUtf8()));
    pageItem->setIcon(KIcon(ebrowsingInfo.icon()));

    // WARNING 
    // remember wheh changing here that the smallest netbooks
    // have a 1024x576 resolution. So DON'T bother that limits!!
    parent->setMinimumSize(700, 525);
}


// -----------------------------------------------------------------------------------------------------


SettingsDialog::SettingsDialog(QWidget *parent)
        : KConfigDialog(parent, "rekonfig", ReKonfig::self())
        , d(new Private(this))
{
    showButtonSeparator(false);
    setWindowTitle(i18nc("Window title of the settings dialog", "Configure – rekonq"));
    setModal(true);

    readConfig();
    
    // update buttons
    connect(d->generalWidg,     SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->tabsWidg,        SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->appearanceWidg,  SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->webkitWidg,      SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->networkWidg,     SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->adBlockWidg,     SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->ebrowsingModule, SIGNAL(changed(bool)), this, SLOT(updateButtons()));
    connect(d->shortcutsEditor, SIGNAL(keyChange()),   this, SLOT(updateButtons()));

    // save settings
    connect(this, SIGNAL(applyClicked()), this, SLOT(saveSettings()));
    connect(this, SIGNAL(okClicked()),    this, SLOT(saveSettings()));
}


SettingsDialog::~SettingsDialog()
{
    delete d;
}


// we need this function to UPDATE the config widget data..
void SettingsDialog::readConfig()
{
}


// we need this function to SAVE settings in rc file..
void SettingsDialog::saveSettings()
{
    if (!hasChanged())
        return;

    ReKonfig::self()->writeConfig();
    
    d->generalWidg->save();
    d->tabsWidg->save();
    d->appearanceWidg->save();
    d->webkitWidg->save();
    d->networkWidg->save();
    d->adBlockWidg->save();
    d->shortcutsEditor->save();
    d->ebrowsingModule->save();
    
    SearchEngine::reload();

    updateButtons();
    emit settingsChanged("ReKonfig");
}


bool SettingsDialog::hasChanged()
{
    return KConfigDialog::hasChanged()
           || d->generalWidg->changed()
           || d->tabsWidg->changed()
           || d->appearanceWidg->changed()
           || d->webkitWidg->changed()
           || d->networkWidg->changed()
           || d->adBlockWidg->changed()
           || d->ebrowsingModule->changed()
           || d->shortcutsEditor->isModified();
    ;
}


bool SettingsDialog::isDefault()
{
    bool isDef = KConfigDialog::isDefault();
    
    if(isDef)
    {
        // check our private widget values
        isDef = d->appearanceWidg->isDefault();
    }
    return isDef;
}
