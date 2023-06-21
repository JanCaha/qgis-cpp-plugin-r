#include "qgsrstatssettings.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgscollapsiblegroupbox.h"
#include "qgsfilewidget.h"
#include "qgsgui.h"
#include "qgssettings.h"

//
// QgsRStatsSettingsWidget
//

QgsRStatsSettingsWidget::QgsRStatsSettingsWidget( QWidget *parent ) : QgsOptionsPageWidget( parent )
{
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout( layout );

    QgsCollapsibleGroupBox *boxLibraryPath = new QgsCollapsibleGroupBox( this );
    boxLibraryPath->setTitle( "R Library Path" );
    layout->addWidget( boxLibraryPath );

    QFormLayout *box1Layout = new QFormLayout();
    boxLibraryPath->setLayout( box1Layout );

    mRLibrariesFolder = new QgsFileWidget( this );
    mRLibrariesFolder->setStorageMode( QgsFileWidget::StorageMode::GetDirectory );

    box1Layout->addRow( "Path for R Libraries:", mRLibrariesFolder );

    QgsCollapsibleGroupBox *boxVerboseR = new QgsCollapsibleGroupBox( this );
    boxVerboseR->setTitle( "R Verbosity" );
    layout->addWidget( boxVerboseR );

    QFormLayout *box2Layout = new QFormLayout();
    boxVerboseR->setLayout( box2Layout );

    mRVerbose = new QCheckBox( this );

    box2Layout->addRow( new QLabel( "Changing this requires QGIS restart!" ) );
    box2Layout->addRow( new QLabel( "Should R be very verbose (print result of every statement)? Otherwise, it only "
                                    "prints when functions `print()` or `cat()` are called." ) );
    box2Layout->addRow( "Verbose R:", mRVerbose );

    QgsSettings settings;
    mRLibrariesFolder->setFilePath( settings.value( QStringLiteral( "RStats/LibraryPath" ), "" ).toString() );
    mRVerbose->setChecked( settings.value( QStringLiteral( "RStats/VerboseR" ), false ).toBool() );
}

QgsRStatsSettingsWidget::~QgsRStatsSettingsWidget() { saveSettings(); }

void QgsRStatsSettingsWidget::saveSettings()
{
    QgsSettings settings;
    settings.setValue( QStringLiteral( "RStats/LibraryPath" ), mRLibrariesFolder->filePath() );
    settings.setValue( QStringLiteral( "RStats/VerboseR" ), mRVerbose->isChecked() );
}

void QgsRStatsSettingsWidget::apply() { saveSettings(); }

//
// QgsRStatsSettingsOptionsFactory
//
QgsRStatsSettingsOptionsFactory::QgsRStatsSettingsOptionsFactory()
    : QgsOptionsWidgetFactory( tr( "R Options" ), QIcon() )
{
}

QIcon QgsRStatsSettingsOptionsFactory::icon() const { return QIcon( QStringLiteral( ":/rplugin/icons/R_logo.svg" ) ); }

QgsOptionsPageWidget *QgsRStatsSettingsOptionsFactory::createWidget( QWidget *parent ) const
{
    return new QgsRStatsSettingsWidget( parent );
}

QStringList QgsRStatsSettingsOptionsFactory::path() const { return { QStringLiteral( "ide" ) }; }

QString QgsRStatsSettingsOptionsFactory::pagePositionHint() const { return QStringLiteral( "ROptions" ); }
