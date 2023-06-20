#include "qgsrstatssession.h"

#include <RInside.h>

#include "qgsapplication.h"
#include "qgsrstatsapplicationwrapper.h"
#include "qgsrstatsfunctions.h"
#include "qgssettings.h"
#include "qgssettings.h"

void QgsRStatsSession::prepareQgisApplicationWrapper()
{
  mRSession->assign( QgsRstatsApplicationWrapper::instance(mIface), "QGIS" );
}


void QgsRStatsSession::prepareConvertFunctions()
{
  mRSession->assign( Rcpp::InternalFunction( &QgRstatsFunctions::Dollar ), "$.QGIS" );
  mRSession->assign( Rcpp::InternalFunction( &QgsRstatsApplicationWrapper::functions ), "names.QGIS" );
  mRSession->assign( Rcpp::InternalFunction( &QgRstatsFunctions::printApplicationWrapper ),  "print.QGIS" );

  mRSession->assign( Rcpp::InternalFunction( &QgRstatsFunctions::DollarMapLayer ), QgsRstatsMapLayerWrapper::s3FunctionForClass( "$" ) );
  mRSession->assign( Rcpp::InternalFunction( &QgsRstatsMapLayerWrapper::functions ), QgsRstatsMapLayerWrapper::s3FunctionForClass( "names" ) );
  mRSession->assign( Rcpp::InternalFunction( &QgRstatsFunctions::asDataFrame ), QgsRstatsMapLayerWrapper::s3FunctionForClass( "as.data.frame" ) );
  mRSession->assign( Rcpp::InternalFunction( &QgRstatsFunctions::printMapLayerWrapper ), QgsRstatsMapLayerWrapper::s3FunctionForClass( "print" ) );

}

QgsRStatsSession::QgsRStatsSession(std::shared_ptr<QgisInterface> iface): mIface(iface)
{
  QgsSettings settings;
  mVerboseR = settings.value(QStringLiteral( "RStats/VerboseR" ), false).toBool();

  mRSession = std::make_unique<RInside>( 0, nullptr, true, mVerboseR, true );
  mRSession->set_callbacks( this );

  prepareQgisApplicationWrapper();
  prepareConvertFunctions();
}

void QgsRStatsSession::setLibraryPath()
{
  QgsSettings settings;
  QString rLibPath = settings.value(QStringLiteral( "RStats/LibraryPath" ), "").toString();

  if (! rLibPath.isEmpty())
  {
    QString command = QStringLiteral( ".libPaths(\"%1\")" ).arg( rLibPath );
    emit showMessage( command );
    execCommand( command );
  }
}

void QgsRStatsSession::showStartupMessage()
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
  emit showMessage( QStringLiteral( "Copyright (C) %1 The R Foundation for Statistical Computing" ).arg( yearString.toString() ) );
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

QgsRStatsSession::~QgsRStatsSession() = default;

QString QgsRStatsSession::sexpToString( const SEXP exp )
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
      // QgsDebugMsg( QStringLiteral( "Possibly unsafe type: %1" ).arg( TYPEOF( exp ) ) );
      break;
  }

  Rcpp::StringVector lines = Rcpp::StringVector( Rf_eval( Rf_lang2( Rf_install( "capture.output" ), exp ), R_GlobalEnv ) );
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

QVariant QgsRStatsSession::sexpToVariant( const SEXP exp )
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
    case VECSXP:
      break;

    default:
      // QgsDebugMsg( QStringLiteral( "Trying to convert potentially unsafe SEXP type %1 to variant... watch out!" ).arg( TYPEOF( exp ) ) );
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
    case VECSXP: // data.frame - could be converted to QgsVectorLayer, but would it be helpfull ???
      // unreachable, handled earlier
      return QVariant();

    default:
      // QgsDebugMsg( QStringLiteral( "Unhandled type: %1" ).arg( TYPEOF( exp ) ) );
      return QVariant();
  }

  return QVariant();
}

SEXP QgsRStatsSession::variantToSexp( const QVariant &variant )
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
      // QgsDebugMsg( QStringLiteral( "unsupported user variant type %1" ).arg( QMetaType::typeName( variant.userType() ) ) );
      return nullptr;

    default:
      // QgsDebugMsg( QStringLiteral( "unsupported variant type %1" ).arg( QVariant::typeToName( variant.type() ) ) );
      return nullptr;
  }
}

void QgsRStatsSession::execCommandPrivate( const QString &command, QString &error, QVariant *res, QString *output )
{
  try
  {
    const SEXP sexpRes = mRSession->parseEval( command.toStdString() );

    // if (mRSession->parseComplete())
    // {
      if ( res )
        *res = sexpToVariant( sexpRes );
      if ( output )
        *output = sexpToString( sexpRes );
    // }
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

void QgsRStatsSession::execCommandNR( const QString &command )
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

void QgsRStatsSession::execCommand( const QString &command )
{
  if ( mBusy )
    return;

  if ( command.isEmpty() )
    return;

  if ( mEmptyCommandCheck.exactMatch(command) )
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
    emit commandFinished( res );
  }

  mBusy = false;
  emit busyChanged( false );
}

void QgsRStatsSession::WriteConsole( const std::string &line, int type )
{
  if ( type > 0 )
    mEncounteredErrorMessageType = true;

  const QString message = QString::fromStdString( line );
  emit consoleMessage( message, type );
}

bool QgsRStatsSession::has_WriteConsole()
{
  return true;
}

void QgsRStatsSession::ShowMessage( const char *message )
{
  const QString messageString( message );
  emit showMessage( messageString );
}

bool QgsRStatsSession::has_ShowMessage()
{
  return true;
}

void QgsRStatsSession::emptyRMemory()
{
  QString error;
  execCommandPrivate( QStringLiteral( "rm(list = ls())" ), error );

  if ( !error.isEmpty() )
  {
    // QgsDebugMsg( error );
  }

  prepareQgisApplicationWrapper();
  prepareConvertFunctions();
}
