/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2008-2009 by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2009 rekonq team. Please, see AUTHORS file for details
*
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* ============================================================ */


#include "application.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>


static const char description[] =
    I18N_NOOP("KDE Browser Webkit Based");


static const char version[] = "0.1alpha";


int main(int argc, char **argv)
{
    KAboutData about("rekonq",
                     0,
                     ki18n("rekonq"),
                     version,
                     ki18n(description),
                     KAboutData::License_GPL_V3,
                     ki18n("(C) 2009 rekonq team"),
                     KLocalizedString(),
                     "http://rekonq.sourceforge.net",
                     "rekonq@kde.org"
                    );

    // about authors
    about.addAuthor(ki18n("Andrea Diamantini"), 
                    ki18n("Project Lead, Developer, Italian translations"), 
                    "adjam7@gmail.com", 
                    "http://www.adjam.org");

    // Initialize command line args
    KCmdLineArgs::init(argc, argv, &about);

    // Define the command line options using KCmdLineOptions
    KCmdLineOptions options;

    // adding URL option
    options.add("+[URL]" , ki18n("Location to open"));

    // Register the supported options
    KCmdLineArgs::addCmdLineOptions(options);

    // Add options from Application class
    Application::addCmdLineOptions();

    if (!Application::start())
    {
        kWarning() << "rekonq is already running!";
        return 0;
    }

    Application app;
    return app.exec();
}
