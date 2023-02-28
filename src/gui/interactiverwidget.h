#ifndef INTERACTIVERWIDGET_H
#define INTERACTIVERWIDGET_H

#include <QKeyEvent>
#include <QString>
#include <QWidget>

#include "qgscodeeditorr.h"

class InteractiveRWidget : public QgsCodeEditorR
{
        Q_OBJECT
    public:
        InteractiveRWidget( QWidget *parent = nullptr );

        void clear() override;

    signals:

        void runCommand( const QString &command );

    protected:
        void keyPressEvent( QKeyEvent *event ) override;

        void initializeLexer() override;
        void displayPrompt( bool more = false );
};

#endif