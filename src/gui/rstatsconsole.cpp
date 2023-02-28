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

#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFormLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QSplitter>
#include <QTextBrowser>
#include <QTextStream>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include "qgscodeeditorr.h"
#include "qgsdockwidgetplugin.h"

#include "../rstatsrunner.h"
#include "rsettingsdialog.h"
#include "rstatsconsole.h"

RStatsConsole::RStatsConsole( QWidget *parent, std::shared_ptr<RStatsRunner> runner,
                              std::shared_ptr<QgisInterface> iface )
    : QgsDockWidget( parent ), mRunner( runner )
{
    setWindowTitle( QString( "R Console" ) );
    setObjectName( QString( "R Console" ) );
    setWindowFlags( Qt::WindowType::Window );

    iface->addDockWidget( Qt::DockWidgetArea::BottomDockWidgetArea, this );

    QToolBar *toolBar = new QToolBar( this );
    toolBar->setIconSize( iface->iconSize( true ) );

    // QToolButton *toggleButton = mDockableWidgetHelper->createDockUndockToolButton();
    // toggleButton->setToolTip( tr( "Dock R Stats Console" ) );
    // toolBar->addWidget( toggleButton );

    mReadRScript = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileOpen.svg" ) ),
                                tr( "Run Script" ), this );
    toolBar->addAction( mReadRScript );

    connect( mReadRScript, &QAction::triggered, this,
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

    mSettings = new QAction( "Settings", this );
    toolBar->addAction( mSettings );

    connect( mSettings, &QAction::triggered, this,
             [=]()
             {
                 RSettingsDialog *dialog = new RSettingsDialog( this );
                 int result = dialog->exec();

                 if ( result == RSettingsDialog::Accepted )
                 {
                     //  mRunner->execCommand( QStringLiteral( ".libPaths(\"%1\")" ).arg( dialog->rLibraryPath() ) );
                 }
             } );

    QVBoxLayout *vl = new QVBoxLayout();
    vl->setContentsMargins( 0, 0, 0, 0 );
    vl->addWidget( toolBar );

    QSplitter *splitter = new QSplitter();
    splitter->setOrientation( Qt::Vertical );
    splitter->setHandleWidth( 3 );
    splitter->setChildrenCollapsible( false );

    mOutput = new QgsCodeEditorR( nullptr, QgsCodeEditor::Mode::OutputDisplay );
    splitter->addWidget( mOutput );
    mInputEdit = new InteractiveRWidget();
    mInputEdit->setFont( QgsCodeEditor::getMonospaceFont() );
    splitter->addWidget( mInputEdit );

    vl->addWidget( splitter );

    QWidget *w = new QWidget( this );
    w->setLayout( vl );

    setWidget( w );

    connect( mInputEdit, &InteractiveRWidget::runCommand, this,
             [=]( const QString &command )
             {
                 if ( mRunner->busy() )
                     return;

                 mInputEdit->clear();
                 mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + QStringLiteral( "> " ) +
                                  command );
                 mOutput->moveCursorToEnd();
                 mRunner->execCommand( command );
             } );

    connect( mRunner.get(), &RStatsRunner::errorOccurred, this,
             [=]( const QString &error )
             {
                 mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + error );
                 mOutput->moveCursorToEnd();
             } );

    connect( mRunner.get(), &RStatsRunner::consoleMessage, this,
             [=]( const QString &message, int type )
             {
                 if ( type == 0 )
                     mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + message );
                 else // TODO should we format errors differently?
                     mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + message );
                 mOutput->moveCursorToEnd();
             } );

    connect( mRunner.get(), &RStatsRunner::showMessage, this,
             [=]( const QString &message )
             {
                 mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + message );
                 mOutput->moveCursorToEnd();
             } );

    connect( mRunner.get(), &RStatsRunner::busyChanged, this,
             [=]( bool busy )
             {
                 // mInputEdit->setEnabled( !busy );
             } );

    // setLayout( vl );

    mRunner->showStartupMessage();
}

RStatsConsole::~RStatsConsole() {}
