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
#include "qgscodeeditordockwidget.h"

class QgsInteractiveRWidget;
class QgsRStatsRunner;
class QLineEdit;
class QTextBrowser;
class QgsCodeEditorR;

class QgsRStatsConsole : public QgsCodeEditorDockWidget
{
    public:
        QgsRStatsConsole( QWidget *parent, std::shared_ptr<QgsRStatsRunner> runner,
                          std::shared_ptr<QgisInterface> iface );
        ~QgsRStatsConsole() override;

    private:
        std::shared_ptr<QgsRStatsRunner> mRunner = nullptr;
        QgsCodeEditorR *mInputEdit = nullptr;
        QgsCodeEditorR *mOutput = nullptr;
        QAction *mActionReadRScript = nullptr;
        QAction *mActionEmptyRMemory = nullptr;
        QAction *mActionClearConsole = nullptr;
        std::shared_ptr<QgisInterface> mIface;
        QString mDefaulEmptryRMemoryLabel = QString( "Empty R Memory (used %1)" );
};

#endif // QGSRSTATSCONSOLE_H
