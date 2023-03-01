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

#include "../rstatsrunner.h"
#include "interactiverwidget.h"

class RStatsConsole : public QgsDockWidget
{
    public:
        RStatsConsole( QWidget *parent, std::shared_ptr<RStatsRunner> runner, std::shared_ptr<QgisInterface> iface );
        ~RStatsConsole() override;

    private:
        std::shared_ptr<RStatsRunner> mRunner = nullptr;
        InteractiveRWidget *mInputEdit = nullptr;
        QgsCodeEditorR *mOutput = nullptr;
        QAction *mReadRScript = nullptr;
        QAction *mSettings = nullptr;
        QAction *mEmptyRMemory = nullptr;
};

#endif // QGSRSTATSCONSOLE_H
