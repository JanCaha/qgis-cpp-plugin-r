/***************************************************************************
                             qgsrstatsconsole.cpp
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

#include "qgsapplication.h"
#include "qgscodeeditor.h"
#include "qgscodeeditorr.h"

#include <QFileDialog>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTextBrowser>
#include <QTextStream>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "qgsrstatsconsole.h"
#include "qgsrstatsrunner.h"

QgsRStatsConsole::QgsRStatsConsole( QWidget *parent, std::shared_ptr<QgsRStatsRunner> runner,
                                    std::shared_ptr<QgisInterface> iface )
    : QgsCodeEditorDockWidget( "RConsole", true ), mRunner( runner ), mIface( iface )
{
    setWindowTitle( QString( "R Console" ) );
    setObjectName( QString( "R Console" ) );
    setWindowFlags( Qt::WindowType::Window );

    // iface->addDockWidget( Qt::DockWidgetArea::BottomDockWidgetArea, this );

    QToolBar *toolBar = new QToolBar( this );
    toolBar->setIconSize( iface->iconSize( true ) );

    // QToolButton *toggleButton = mDockableWidgetHelper->createDockUndockToolButton();
    // toggleButton->setToolTip( tr( "Dock R Stats Console" ) );
    // toolBar->addWidget( toggleButton );

    mActionClearConsole =
        new QAction( QgsApplication::getThemeIcon( "console/iconClearConsole.svg" ), "Clear Console", this );
    toolBar->addAction( mActionClearConsole );
    connect( mActionClearConsole, &QAction::triggered, this,
             [=]()
             {
                 mOutput->setText( "" );
                 mRunner->showStartupMessage();
             } );

    mActionReadRScript = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileOpen.svg" ) ),
                                      tr( "Run Script" ), this );
    toolBar->addAction( mActionReadRScript );

    connect( mActionReadRScript, &QAction::triggered, this,
             [=]()
             {
                 QString fileName =
                     QFileDialog::getOpenFileName( this, tr( "Open Script" ), "/home", tr( "R Files (*.R *.r)" ) );
                 QFile inputFile( fileName );
                 if ( inputFile.open( QIODevice::ReadOnly ) )
                 {
                     QTextStream in( &inputFile );
                     while ( !in.atEnd() )
                     {
                         QString line = in.readLine();
                         mRunner->execCommand( line );
                     }
                     inputFile.close();
                 }
             } );

    mActionEmptyRMemory = new QAction( tr( "Empty R Memory" ), this );
    toolBar->addAction( mActionEmptyRMemory );

    connect( mActionEmptyRMemory, &QAction::triggered, this, [=]() { mRunner->emptyRMemory(); } );

    QVBoxLayout *vl = new QVBoxLayout();
    vl->setContentsMargins( 0, 0, 0, 0 );
    vl->addWidget( toolBar );

    QSplitter *splitter = new QSplitter();
    splitter->setOrientation( Qt::Vertical );
    splitter->setHandleWidth( 3 );
    splitter->setChildrenCollapsible( false );

    mOutput = new QgsCodeEditorR( this, QgsCodeEditor::Mode::OutputDisplay );
    splitter->addWidget( mOutput );

    mInputEdit = new QgsCodeEditorR( this, QgsCodeEditor::Mode::CommandInput );
    mInputEdit->setFont( QgsCodeEditor::getMonospaceFont() );
    mInputEdit->setInterpreter( mRunner.get() );

    splitter->addWidget( mInputEdit );

    vl->addWidget( splitter );

    setLayout( vl );

    connect( mRunner.get(), &QgsRStatsRunner::errorOccurred, this,
             [=]( const QString &error )
             {
                 mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + error );
                 mOutput->moveCursorToEnd();
             } );

    connect( mRunner.get(), &QgsRStatsRunner::consoleMessage, this,
             [=]( const QString &message, int type )
             {
                 if ( type == 0 )
                     mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + message );
                 else // TODO should we format errors differently?
                     mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + message );
                 mOutput->moveCursorToEnd();
             } );

    connect( mRunner.get(), &QgsRStatsRunner::showMessage, this,
             [=]( const QString &message )
             {
                 mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + message );
                 mOutput->moveCursorToEnd();
             } );

    connect( mRunner.get(), &QgsRStatsRunner::busyChanged, this,
             [=]( bool busy )
             {
                 // mInputEdit->setEnabled( !busy );
             } );

    // setLayout( vl );

    toolBar->addSeparator();
    toolBar->addWidget( dockToggleButton() );

    mRunner->showStartupMessage();
}

QgsRStatsConsole::~QgsRStatsConsole()
{
    delete mInputEdit;
    delete mOutput;
    delete mActionReadRScript;
    delete mActionEmptyRMemory;
}
