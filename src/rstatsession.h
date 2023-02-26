/***************************************************************************
                             qgsrstatsrunner.h
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

#ifndef RSTATSSESSION_H
#define RSTATSSESSION_H

#include <memory>

#include "Callbacks.h"
#include "RInside.h"

#include <QObject>
#include <QThread>

#include "qgisinterface.h"
#include "qgsapplication.h"

class RInside;
class QVariant;
class QString;

class RStatsSession : public QObject, public Callbacks
{
        Q_OBJECT
    public:
        RStatsSession( std::shared_ptr<QgisInterface> iface );
        ~RStatsSession() override;

        bool busy() const { return mBusy; }

        void execCommandNR( const QString &command );

        void WriteConsole( const std::string &line, int type ) override;

        bool has_WriteConsole() override;

        void ShowMessage( const char *message ) override;

        bool has_ShowMessage() override;

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

    public slots:

        void execCommand( const QString &command );

        void showStartupMessage();

    signals:

        void busyChanged( bool busy );

        void consoleMessage( const QString &message, int type );
        void showMessage( const QString &message );
        void errorOccurred( const QString &error );
        void commandFinished( const QVariant &result );

    private:
        std::unique_ptr<RInside> mRSession;
        bool mBusy = false;
        bool mEncounteredErrorMessageType = false;
        std::shared_ptr<QgisInterface> mIface;

        void execCommandPrivate( const QString &command, QString &error, QVariant *res = nullptr,
                                 QString *output = nullptr );
};

#endif
