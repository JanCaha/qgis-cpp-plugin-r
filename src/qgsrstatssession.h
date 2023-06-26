#ifndef QGSRSTATSSESSION_H
#define QGSRSTATSSESSION_H

#include <QObject>
#include <QRegExp>
#include <QThread>

#include "Callbacks.h"

#include "qgisinterface.h"

class RInside;
class QVariant;
class QString;

class QgsRStatsSession : public QObject, public Callbacks
{
        Q_OBJECT
    public:
        QgsRStatsSession( std::shared_ptr<QgisInterface> iface );
        ~QgsRStatsSession() override;

        void execCommandNR( const QString &command );

        void WriteConsole( const std::string &line, int type ) override;

        bool has_WriteConsole() override;

        void ShowMessage( const char *message ) override;

        bool has_ShowMessage() override;

        bool busy() const { return mBusy; }

        /**
         * Converts a SEXP object to a string.
         */
        static QString sexpToString( const SEXP exp );

        /**
         * Converts a SEXP object to a QVariant.
         */
        static QVariant sexpToVariant( const SEXP exp );

        /**
         * Converts a variant to a SEXP.
         */
        static SEXP variantToSexp( const QVariant &variant );

        /**
         * Empty R memory, remove all objects and stored variables, but leave packages loaded.
         */
        void emptyRMemory();

    public slots:

        void execCommand( const QString &command );

        void showStartupMessage();

        void setLibraryPath();

    signals:

        void busyChanged( bool busy );

        void consoleMessage( const QString &message, int type );
        void showMessage( const QString &message );
        void errorOccurred( const QString &error );
        void commandFinished( const QVariant &result );

    private:
        void execCommandPrivate( const QString &command, QString &error, QVariant *res = nullptr,
                                 QString *output = nullptr );

        void prepareQgisApplicationWrapper();
        void prepareConvertFunctions();

        std::shared_ptr<QgisInterface> mIface = nullptr;
        std::unique_ptr<RInside> mRSession;
        bool mBusy = false;
        bool mEncounteredErrorMessageType = false;

        bool mVerboseR = false;
        QRegExp mEmptyCommandCheck = QRegExp( "\\s+" );
};

#endif // QGSRSTATSSESSION_H
