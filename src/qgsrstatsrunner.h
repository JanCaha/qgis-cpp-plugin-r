/***************************************************************************
                             qgsrstatsrunner.h
                             --------------
    begin                : September 2022
    copyright            : (C) 2022 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRSTATSRUNNER_H
#define QGSRSTATSRUNNER_H

#include <memory>

#include <QObject>
#include <QThread>

#include "Callbacks.h"

#include "qgisinterface.h"
#include "qgscodeeditor.h"

#include "qgsrstatssession.h"

class RInside;
class QVariant;
class QString;

class QgsRStatsRunner : public QObject, public QgsCodeInterpreter
{
        Q_OBJECT
    public:
        QgsRStatsRunner( std::shared_ptr<QgisInterface> iface );
        ~QgsRStatsRunner();

        void execCommand( const QString &command );
        bool busy() const;
        void showStartupMessage();
        void setLibraryPath();
        void emptyRMemory();

    signals:

        void consoleMessage( const QString &message, int type );
        void showMessage( const QString &message );
        void errorOccurred( const QString &error );
        void busyChanged( bool busy );
        void commandFinished( const QVariant &result );
        void usedMemoryChanged( const QString memoryUsed );

    private:
        QThread mSessionThread;
        std::unique_ptr<QgsRStatsSession> mSession;
        std::shared_ptr<QgisInterface> mIface;

        QString promptForState( int state ) const;
        int execCommandImpl( const QString &command );
};

#endif // QGSRSTATSRUNNER_H
