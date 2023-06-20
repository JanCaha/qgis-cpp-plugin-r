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

#include "qgsrstatsconsole.h"
#include "qgsapplication.h"
#include "qgscodeeditor.h"
#include "qgscodeeditorr.h"
#include "qgsdockablewidgethelper.h"
#include "qgsrstatsrunner.h"

#include <QFileDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QSplitter>
#include <QTextBrowser>
#include <QTextStream>
#include <QToolBar>
#include <QVBoxLayout>

QgsRStatsConsole::QgsRStatsConsole( QWidget *parent, QgsRStatsRunner *runner, std::shared_ptr<QgisInterface> iface )
    : QWidget( parent ), mRunner( runner ), mIface( iface )
{
    QToolBar *toolBar = new QToolBar( this );
    toolBar->setIconSize( mIface->iconSize( true ) );

    mDockableWidgetHelper = new QgsDockableWidgetHelper( true, tr( "R Stats Console" ), this, nullptr,
                                                         Qt::BottomDockWidgetArea, QStringList(), true );
    mDockableWidgetHelper->setDockObjectName( QStringLiteral( "RStatsConsole" ) );
    QToolButton *toggleButton = mDockableWidgetHelper->createDockUndockToolButton();
    toggleButton->setToolTip( tr( "Dock R Stats Console" ) );
    toolBar->addWidget( toggleButton );

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

    mEmptyRMemory = new QAction( tr( "Empty R Memory" ), this );
    mEmptyRMemory->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "console/iconClearConsole.svg" ) ) );
    toolBar->addAction( mEmptyRMemory );
    connect( mEmptyRMemory, &QAction::triggered, this, [=]() { mRunner->emptyRMemory(); } );

    QVBoxLayout *vl = new QVBoxLayout();
    vl->setContentsMargins( 0, 0, 0, 0 );
    vl->addWidget( toolBar );

    QSplitter *splitter = new QSplitter();
    splitter->setOrientation( Qt::Vertical );
    splitter->setHandleWidth( 3 );
    splitter->setChildrenCollapsible( false );

    mOutput = new QgsCodeEditorR( nullptr, QgsCodeEditor::Mode::OutputDisplay );
    splitter->addWidget( mOutput );
    mInputEdit = new QgsInteractiveRWidget();
    mInputEdit->setFont( QgsCodeEditor::getMonospaceFont() );
    splitter->addWidget( mInputEdit );

    vl->addWidget( splitter );

    connect( mInputEdit, &QgsInteractiveRWidget::runCommand, this,
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

    connect( mRunner, &QgsRStatsRunner::errorOccurred, this,
             [=]( const QString &error )
             {
                 mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + error );
                 mOutput->moveCursorToEnd();
             } );

    connect( mRunner, &QgsRStatsRunner::consoleMessage, this,
             [=]( const QString &message, int type )
             {
                 if ( type == 0 )
                     mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + message );
                 else // TODO should we format errors differently?
                     mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + message );
                 mOutput->moveCursorToEnd();
             } );

    connect( mRunner, &QgsRStatsRunner::showMessage, this,
             [=]( const QString &message )
             {
                 mOutput->append( ( mOutput->text().isEmpty() ? QString() : QString( '\n' ) ) + message );
                 mOutput->moveCursorToEnd();
             } );

    connect( mRunner, &QgsRStatsRunner::busyChanged, this,
             [=]( bool busy )
             {
                 Q_UNUSED( busy );
                 // mInputEdit->setEnabled( !busy );
             } );

    setLayout( vl );

    mRunner->showStartupMessage();
    mRunner->setLibraryPath();
}

QgsRStatsConsole::~QgsRStatsConsole() { delete mDockableWidgetHelper; }

QgsInteractiveRWidget::QgsInteractiveRWidget( QWidget *parent )
    : QgsCodeEditorR( parent, QgsCodeEditor::Mode::CommandInput )
{
    displayPrompt( false );

    QgsInteractiveRWidget::initializeLexer();
}

void QgsInteractiveRWidget::clear()
{
    QgsCodeEditorR::clear();
    displayPrompt( false );
}

void QgsInteractiveRWidget::keyPressEvent( QKeyEvent *event )
{
    switch ( event->key() )
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit runCommand( text() );

            break;

        default:
            QgsCodeEditorR::keyPressEvent( event );
    }
}

void QgsInteractiveRWidget::initializeLexer()
{
    QgsCodeEditorR::initializeLexer();

    setCaretLineVisible( false );
    setLineNumbersVisible( false ); // NO linenumbers for the input line
    // Margin 1 is used for the '>' prompt (console input)
    setMarginLineNumbers( 1, true );
    setMarginWidth( 1, "00" );
    setMarginType( 1, QsciScintilla::MarginType::TextMarginRightJustified );
    setMarginsBackgroundColor( color( QgsCodeEditorColorScheme::ColorRole::Background ) );
    setEdgeMode( QsciScintilla::EdgeNone );
}

void QgsInteractiveRWidget::displayPrompt( bool more )
{
    const QString prompt = !more ? ">" : "+";
    SendScintilla( QsciScintilla::SCI_MARGINSETTEXT, static_cast<uintptr_t>( 0 ), prompt.toUtf8().constData() );
}
