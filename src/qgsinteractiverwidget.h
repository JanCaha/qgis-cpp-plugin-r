#ifndef QGSRSTATSINTERACTIVERWIDGET_H
#define QGSRSTATSINTERACTIVERWIDGET_H

#include "qgscodeeditorr.h"

class QgsInteractiveRWidget : public QgsCodeEditorR
{
        Q_OBJECT
    public:
        QgsInteractiveRWidget( QWidget *parent = nullptr );

        void clear() override;

    signals:

        void runCommand( const QString &command );

    protected:
        void keyPressEvent( QKeyEvent *event ) override;

        void initializeLexer() override;
        void displayPrompt( bool more = false );
};

#endif // QGSRSTATSINTERACTIVERWIDGET_H