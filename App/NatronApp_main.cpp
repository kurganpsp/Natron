//  Natron
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
 * Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012.
 * contact: immarespond at gmail dot com
 *
 */

#include <csignal>
#include <cstdio>  // perror
#include <cstdlib> // exit
#include <fstream>
#include <sstream>

#if defined(Q_OS_UNIX)
#include <sys/signal.h>
#endif

#include <QApplication>

#include "Gui/GuiApplicationManager.h"

static void setShutDownSignal(int signalId);
static void handleShutDownSignal(int signalId);

int
main(int argc,
     char *argv[])
{
    bool isBackground;
    QString projectName,mainProcessServerName;
    QStringList writers;
    std::list<std::pair<int,int> > frameRanges;
    AppManager::parseCmdLineArgs(argc,argv,&isBackground,projectName,writers,frameRanges,mainProcessServerName);

    setShutDownSignal(SIGINT);   // shut down on ctrl-c
    setShutDownSignal(SIGTERM);   // shut down on killall
#if defined(Q_OS_UNIX)
    if ( !projectName.isEmpty() ) {
        projectName = AppManager::qt_tildeExpansion(projectName);
    }
#endif
    if (isBackground) {
        if ( projectName.isEmpty() ) {
            ///Autobackground without a project file name is not correct
            AppManager::printUsage(argv[0]);

            return 1;
        }
        AppManager manager;

        if ( !manager.load(argc,argv,projectName,writers,frameRanges,mainProcessServerName) ) {
            AppManager::printUsage(argv[0]);
            return 1;
        } else {
            return 0;
        }
    } else {
        GuiApplicationManager manager;
        bool loaded = manager.load(argc,argv,projectName, QStringList(), std::list<std::pair<int, int> >(), QString());
        if (!loaded) {
            return 1;
        }

        return manager.exec();
    }
} // main

void
setShutDownSignal(int signalId)
{
#if defined(Q_OS_UNIX)
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handleShutDownSignal;
    if (sigaction(signalId, &sa, NULL) == -1) {
        std::perror("setting up termination signal");
        std::exit(1);
    }
#else
    std::signal(signalId, handleShutDownSignal);
#endif
}

void
handleShutDownSignal( int /*signalId*/ )
{
    QCoreApplication::exit(0);
}

