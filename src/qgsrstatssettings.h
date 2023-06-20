#ifndef QGSRSTATSSETTINGS_H
#define QGSRSTATSSETTINGS_H

#include "qgsoptionswidgetfactory.h"

class QgsFileWidget;
class QCheckBox;

class QgsRStatsSettingsWidget : public QgsOptionsPageWidget
{
    Q_OBJECT

  public:


    QgsRStatsSettingsWidget( QWidget *parent );
    ~QgsRStatsSettingsWidget() override;

    //QString helpKey() const override;

    void apply() override;

    void saveSettings();

  private:

    QgsFileWidget *mRLibrariesFolder;
    QCheckBox *mRVerbose;
};


class QgsRStatsSettingsOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:

    QgsRStatsSettingsOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QStringList path() const override;
    QString pagePositionHint() const override;
};

#endif // QGSRSTATSSETTINGS_H
