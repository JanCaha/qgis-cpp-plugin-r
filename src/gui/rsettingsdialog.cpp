#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSettings>

#include "qgsapplication.h"

#include "rsettingsdialog.h"

RSettingsDialog::RSettingsDialog( QWidget *parent ) : QDialog( parent )
{
    setMinimumSize( 500, 200 );

    QFormLayout *fl = new QFormLayout();
    setLayout( fl );

    QLineEdit *mLibraryPath = new QLineEdit( this );
    mLibraryPath->setText( QSettings()
                               .value( QStringLiteral( "RStats/libraryPath" ),
                                       QVariant( QgsApplication::qgisSettingsDirPath() + QStringLiteral( "r_libs" ) ) )
                               .toString() );

    QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Close, this );

    connect( buttonBox, &QDialogButtonBox::accepted, this,
             [=]()
             {
                 QSettings settings = QSettings();
                 settings.setValue( QStringLiteral( "RStats/libraryPath" ), QVariant( mLibraryPath->text() ) );
             } );
    connect( buttonBox, &QDialogButtonBox::accepted, this, &RSettingsDialog::accept );
    connect( buttonBox, &QDialogButtonBox::rejected, this, &RSettingsDialog::reject );

    fl->addRow( QStringLiteral( "Path to R Libraries:" ), mLibraryPath );
    fl->addWidget( buttonBox );
};

QString RSettingsDialog::rLibraryPath() { return mLibraryPath->text(); }
