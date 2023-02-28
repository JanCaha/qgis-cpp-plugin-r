#ifndef RSETTINGSDIALOG_H
#define RSETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QWidget>

class RSettingsDialog : public QDialog
{
    public:
        RSettingsDialog( QWidget *parent );
        QString rLibraryPath();

    private:
        QLineEdit *mLibraryPath;
};

#endif