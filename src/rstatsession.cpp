#include "QDir"
#include "QString"

#include "qgsvariantutils.h"

#include "qgisapplicationrwrapper.h"
#include "rstatsession.h"

RStatsSession::RStatsSession( std::shared_ptr<QgisInterface> iface ) : mIface( iface )
{

    mRSession = std::make_unique<RInside>( 0, nullptr, true, false, true );
    mRSession->set_callbacks( this );

    const QString userPath = QgsApplication::qgisSettingsDirPath() + QStringLiteral( "r_libs" );
    if ( !QFile::exists( userPath ) )
    {
        QDir().mkpath( userPath );
    }
    execCommandNR( QStringLiteral( ".libPaths(\"%1\")" ).arg( userPath ) );

    Rcpp::XPtr<QgisApplicationRWrapper> wr( new QgisApplicationRWrapper( mIface ) );
    wr.attr( "class" ) = ".QGISPrivate";
    mRSession->assign( wr, ".QGISPrivate" );
    // mRSession->assign( Rcpp::InternalFunction( &Dollar ), "$..QGISPrivate" );

    QString error;
    execCommandPrivate( QStringLiteral( R"""(
  QGIS <- list(
    versionInt=function() { .QGISPrivate$versionInt },
    mapLayerByName=function(name) { .QGISPrivate$mapLayerByName(name) },
    activeLayer=function() { .QGISPrivate$activeLayer },
    toDataFrame=function(layer, selectedOnly=FALSE) { .QGISPrivate$toDataFrame(layer, selectedOnly) },
    toNumericVector=function(layer, field, selectedOnly=FALSE) { .QGISPrivate$toNumericVector(layer, field, selectedOnly) },
    toSf=function(layer) { .QGISPrivate$toSf(layer) },
    toRaster=function(layer) {.QGISPrivate$toRaster(layer)},
    toTerra=function(layer) {.QGISPrivate$toTerra(layer)},
    toStars=function(layer) {.QGISPrivate$toStars(layer)},
    isVectorLayer=function(layer) { .QGISPrivate$isVectorLayer(layer) },
    isRasterLayer=function(layer) { .QGISPrivate$isRasterLayer(layer) },
    dfToQGIS=function(df) { .QGISPrivate$dfToQGIS(df) }
  )
  class(QGIS) <- "QGIS"
  )""" ),
                        error );

    if ( !error.isEmpty() )
    {
        QgsDebugMsg( error );
    }
}

RStatsSession::~RStatsSession() = default;

void RStatsSession::showStartupMessage()
{
    QVariant versionString;
    QString error;
    execCommandPrivate( QStringLiteral( "R.version$version.string" ), error, &versionString );
    QVariant nicknameString;
    execCommandPrivate( QStringLiteral( "R.version$nickname" ), error, &nicknameString );
    QVariant platformString;
    execCommandPrivate( QStringLiteral( "R.version$platform" ), error, &platformString );
    QVariant yearString;
    execCommandPrivate( QStringLiteral( "R.version$year" ), error, &yearString );
    QVariant sizeInt;
    execCommandPrivate( QStringLiteral( ".Machine$sizeof.pointer" ), error, &sizeInt );

    emit showMessage( QStringLiteral( "%1 -- %2" ).arg( versionString.toString(), nicknameString.toString() ) );
    emit showMessage(
        QStringLiteral( "Copyright (C) %1 The R Foundation for Statistical Computing" ).arg( yearString.toString() ) );
    const int bits = sizeInt.toInt() == 8 ? 64 : 32;
    emit showMessage( QStringLiteral( "Platform: %1 (%2-bit)" ).arg( platformString.toString() ).arg( bits ) );
    emit showMessage( QString() );

    emit showMessage( QStringLiteral( "R is free software and comes with ABSOLUTELY NO WARRANTY." ) );
    emit showMessage( QStringLiteral( "You are welcome to redistribute it under certain conditions." ) );
    emit showMessage( QStringLiteral( "Type 'license()' or 'licence()' for distribution details." ) );
    emit showMessage( QString() );

    emit showMessage( QStringLiteral( "R is a collaborative project with many contributors." ) );
    emit showMessage( QStringLiteral( "Type 'contributors()' for more information and" ) );
    emit showMessage( QStringLiteral( "'citation()' on how to cite R or R packages in publications." ) );
    emit showMessage( QString() );

    // TODO -- these don't actually work!
    // emit showMessage( QStringLiteral( "Type 'demo()' for some demos, 'help()' for on-line help, or" ) );
    // emit showMessage( QStringLiteral( "'help.start()' for an HTML browser interface to help." ) );
    emit showMessage( QString() );
}

QString RStatsSession::sexpToString( const SEXP exp )
{
    switch ( TYPEOF( exp ) )
    {
        case EXPRSXP:
        case CLOSXP:
        case ENVSXP:
        case LANGSXP:
        case S4SXP:
        case PROMSXP:
        case DOTSXP:
            // these types can't be converted to StringVector, will raise exceptions
            return QString();

        case CHARSXP:
        {
            // special case
            return QStringLiteral( "[1] \"%1\"" ).arg( QString::fromStdString( Rcpp::as<std::string>( exp ) ) );
        }

        case LGLSXP:
        case INTSXP:
        case REALSXP:
        case STRSXP:
        case EXTPTRSXP:
        case VECSXP:
            break; // we know these types are fine to convert to StringVector

        case NILSXP:
            return QStringLiteral( "NULL" );

        default:
            QgsDebugMsg( QStringLiteral( "Possibly unsafe type: %1" ).arg( TYPEOF( exp ) ) );
            break;
    }

    Rcpp::StringVector lines =
        Rcpp::StringVector( Rf_eval( Rf_lang2( Rf_install( "capture.output" ), exp ), R_GlobalEnv ) );
    std::string outcome = "";
    for ( auto it = lines.begin(); it != lines.end(); it++ )
    {
        Rcpp::String line( it->get() );
        outcome.append( line );
        if ( it < lines.end() - 1 )
            outcome.append( "\n" );
    }
    return QString::fromStdString( outcome );
}

QVariant RStatsSession::sexpToVariant( const SEXP exp )
{
    switch ( TYPEOF( exp ) )
    {
        // these types are not safe to call LENGTH on, and don't make sense to convert to a variant anyway
        case S4SXP:
        case LANGSXP:
        case SYMSXP:
        case EXTPTRSXP:
        case CLOSXP:
        case ENVSXP:
        case PROMSXP:
        case DOTSXP:
        case BCODESXP:
        case WEAKREFSXP:
        case 26: // ???
            return QVariant();

        // confirmed safe types, handled in depth below
        case NILSXP:
        case LGLSXP:
        case INTSXP:
        case REALSXP:
        case STRSXP:
        case CHARSXP:
        case EXPRSXP:
            break;

        default:
            QgsDebugMsg( QStringLiteral( "Trying to convert potentially unsafe SEXP type %1 to variant... watch out!" )
                             .arg( TYPEOF( exp ) ) );
            break;
    }

    const int length = LENGTH( exp );
    if ( length == 0 )
    {
        if ( TYPEOF( exp ) == NILSXP )
            return QVariant();
        else if ( TYPEOF( exp ) == CHARSXP )
            return QString( "" );
        else
            return QVariantList();
    }

    switch ( TYPEOF( exp ) )
    {
        case NILSXP:
            return QVariant();

        case LGLSXP:
        {
            if ( length > 1 )
            {
                const Rcpp::LogicalVector logicalVector( exp );

                QVariantList res;
                res.reserve( length );
                for ( int i = 0; i < length; i++ )
                {
                    const int expInt = logicalVector[i];
                    if ( expInt < 0 )
                        res << QVariant();
                    else
                        res << static_cast<bool>( expInt );
                }
                return res;
            }
            else
            {
                const int expInt = Rcpp::as<int>( exp );
                if ( expInt < 0 )
                    return QVariant();
                else
                    return static_cast<bool>( expInt );
            }
        }

        case INTSXP:
        {
            if ( length > 1 )
            {
                const Rcpp::IntegerVector intVector( exp );

                QVariantList res;
                res.reserve( length );
                for ( int i = 0; i < length; i++ )
                {
                    const int elementInt = intVector[i];
                    res << ( elementInt == NA_INTEGER ? QVariant() : QVariant( elementInt ) );
                }
                return res;
            }
            else
            {
                const int res = Rcpp::as<int>( exp );
                return res == NA_INTEGER ? QVariant() : QVariant( res );
            }
        }

        case REALSXP:
        {
            if ( length > 1 )
            {
                const Rcpp::DoubleVector realVector( exp );

                QVariantList res;
                res.reserve( length );
                for ( int i = 0; i < length; i++ )
                {
                    const double elementReal = realVector[i];
                    res << ( std::isnan( elementReal ) ? QVariant() : QVariant( elementReal ) );
                }
                return res;
            }
            else
            {
                const double res = Rcpp::as<double>( exp );
                return std::isnan( res ) ? QVariant() : res;
            }
        }

        case STRSXP:
            if ( length > 1 )
            {
                const Rcpp::StringVector stringVector( exp );

                QVariantList res;
                res.reserve( length );
                for ( int i = 0; i < length; i++ )
                {
                    const char *elementString = stringVector[i];
                    res << QVariant( QString( elementString ) );
                }
                return res;
            }
            else
            {
                return QString::fromStdString( Rcpp::as<std::string>( exp ) );
            }

        case CHARSXP:
            return QString::fromStdString( Rcpp::as<std::string>( exp ) );

            // case RAWSXP:
            //   return R::rawPointer( exp );

        case EXPRSXP:
            // we don't have any variant type which matches this one
            return QVariant();

        case S4SXP:
        case LANGSXP:
        case SYMSXP:
        case EXTPTRSXP:
        case CLOSXP:
        case ENVSXP:
        case PROMSXP:
            // unreachable, handled earlier
            return QVariant();

        default:
            QgsDebugMsg( QStringLiteral( "Unhandled type: %1" ).arg( TYPEOF( exp ) ) );
            return QVariant();
    }

    return QVariant();
}

SEXP RStatsSession::variantToSexp( const QVariant &variant )
{
    switch ( variant.type() )
    {
        case QVariant::Invalid:
            return R_NilValue;

        case QVariant::Bool:
            if ( QgsVariantUtils::isNull( variant ) )
                return Rcpp::wrap( NA_LOGICAL );

            return Rcpp::wrap( variant.toBool() ? 1 : 0 );

        case QVariant::Int:
            if ( QgsVariantUtils::isNull( variant ) )
                return Rcpp::wrap( NA_INTEGER );

            return Rcpp::wrap( variant.toInt() );

        case QVariant::Double:
            if ( QgsVariantUtils::isNull( variant ) )
                return Rcpp::wrap( std::numeric_limits<double>::quiet_NaN() );

            return Rcpp::wrap( variant.toDouble() );

        case QVariant::String:
            return Rcpp::wrap( variant.toString().toStdString() );

        case QVariant::UserType:
            QgsDebugMsg(
                QStringLiteral( "unsupported user variant type %1" ).arg( QMetaType::typeName( variant.userType() ) ) );
            return nullptr;

        default:
            QgsDebugMsg(
                QStringLiteral( "unsupported variant type %1" ).arg( QVariant::typeToName( variant.type() ) ) );
            return nullptr;
    }
}

void RStatsSession::execCommandPrivate( const QString &command, QString &error, QVariant *res, QString *output )
{
    try
    {
        const SEXP sexpRes = mRSession->parseEval( command.toStdString() );
        if ( res )
            *res = sexpToVariant( sexpRes );
        if ( output )
            *output = sexpToString( sexpRes );
    }
    catch ( std::exception &ex )
    {
        error = QString::fromStdString( ex.what() );
    }
    catch ( ... )
    {
        std::cerr << "Unknown exception caught" << std::endl;
    }
}

void RStatsSession::execCommandNR( const QString &command )
{
    if ( mBusy )
        return;

    mBusy = true;
    emit busyChanged( true );

    mEncounteredErrorMessageType = false;
    QString error;
    execCommandPrivate( command, error );

    if ( !error.isEmpty() && !mEncounteredErrorMessageType )
        emit errorOccurred( error );

    mBusy = false;
    emit busyChanged( false );
}

void RStatsSession::execCommand( const QString &command )
{
    if ( mBusy )
        return;

    mBusy = true;
    emit busyChanged( true );
    QString error;
    QVariant res;
    QString output;
    mEncounteredErrorMessageType = false;
    execCommandPrivate( command, error, &res, &output );

    if ( !error.isEmpty() )
    {
        if ( !mEncounteredErrorMessageType )
            emit errorOccurred( error );
    }
    else
    {
        if ( !output.isEmpty() )
            emit consoleMessage( output, 0 );
        emit commandFinished( res );
    }

    mBusy = false;
    emit busyChanged( false );
}

void RStatsSession::WriteConsole( const std::string &line, int type )
{
    if ( type > 0 )
        mEncounteredErrorMessageType = true;

    const QString message = QString::fromStdString( line );
    emit consoleMessage( message, type );
}

bool RStatsSession::has_WriteConsole() { return true; }

void RStatsSession::ShowMessage( const char *message )
{
    const QString messageString( message );
    emit showMessage( messageString );
}

bool RStatsSession::has_ShowMessage() { return true; }