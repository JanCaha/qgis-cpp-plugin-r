/***************************************************************************
                             qgsrstatsconsole.h
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

#ifndef QGSRSTATSCONSOLE_H
#define QGSRSTATSCONSOLE_H

#include <QWidget>

#include "qgisinterface.h"
#include "qgscodeeditorr.h"
#include "qgsdockwidget.h"

class RStatsRunner;
class QLineEdit;
class QTextBrowser;

class QgsInteractiveRWidget : public QgsCodeEditorR
{
        Q_OBJECT
    public:
        QgsInteractiveRWidget( QWidget *parent = nullptr );

        void clear() override;

    signals:

        void runCommand( const QString &command );

    protected:
        void keyPressEvent( QKeyEvent *event ) override;

        void initializeLexer() override;
        void displayPrompt( bool more = false );
};

class QgsRStatsConsole : public QgsDockWidget
{
    public:
        QgsRStatsConsole( QWidget *parent, std::shared_ptr<RStatsRunner> runner, std::shared_ptr<QgisInterface> iface );
        ~QgsRStatsConsole() override;

    private:
        std::shared_ptr<RStatsRunner> mRunner = nullptr;
        QgsInteractiveRWidget *mInputEdit = nullptr;
        QgsCodeEditorR *mOutput = nullptr;
        // QgsDockWidget *mDockWidget = nullptr;
        QAction *mReadRScript = nullptr;
};

#endif // QGSRSTATSCONSOLE_H
