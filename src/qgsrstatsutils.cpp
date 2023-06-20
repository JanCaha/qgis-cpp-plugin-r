#include "qgsrstatsutils.h"

#include "qgsfields.h"
#include "qgsfeature.h"
#include "qgscoordinatereferencesystem.h"

void QgsRstatsUtils::addFeatureToDf( QgsFeature feature, std::size_t featureNumber, Rcpp::DataFrame &df )
{
  int settingColumn = 0;

  QgsFields fields = feature.fields();

  const QgsAttributes attributes = feature.attributes();
  const QVariant *attributeData = attributes.constData();

  for ( int i = 0; i < fields.count(); i++, attributeData++ )
  {
    QgsField field = fields.at( i );

    switch ( field.type() )
    {
      case QVariant::Bool:
      {
        Rcpp::LogicalVector column = df[settingColumn];
        column[featureNumber] = attributeData->toBool();
        break;
      }
      case QVariant::Int:
      {
        Rcpp::IntegerVector column = df[settingColumn];
        column[featureNumber] = attributeData->toInt();
        break;
      }
      case QVariant::LongLong:
      {
        Rcpp::DoubleVector column = df[settingColumn];
        bool ok;
        double val = attributeData->toDouble( &ok );
        if ( ok )
          column[featureNumber] = val;
        else
          column[featureNumber] = R_NaReal;
        break;
      }
      case QVariant::Double:
      {
        Rcpp::DoubleVector column = df[settingColumn];
        column[featureNumber] = attributeData->toDouble();
        break;
      }
      case QVariant::String:
      {
        Rcpp::StringVector column = df[settingColumn];
        column[featureNumber] = attributeData->toString().toStdString();
        break;
      }

      default:
        continue;
    }
    settingColumn++;
  }
}

bool QgsRstatsUtils::canConvertToRcpp( QgsField field )
{
  std::vector<QVariant::Type> types{QVariant::Bool, QVariant::Int, QVariant::Double, QVariant::LongLong, QVariant::String};

  QVariant::Type fieldType = field.type();

  std::vector<QVariant::Type>::iterator it = std::find( std::begin( types ), std::end( types ), fieldType );

  if ( it == types.end() )
  {
    return false;
  }

  return true;
}

SEXP QgsRstatsUtils::fieldToRcppVector( QgsField field, std::size_t featureCount )
{
  Rcpp::RObject column;

  switch ( field.type() )
  {
    case QVariant::Bool:
    {
      column = Rcpp::LogicalVector( featureCount );
      break;
    }
    case QVariant::Int:
    {
      column = Rcpp::IntegerVector( featureCount );
      break;
    }
    case QVariant::Double:
    {
      column = Rcpp::DoubleVector( featureCount );
      break;
    }
    case QVariant::LongLong:
    {
      column = Rcpp::DoubleVector( featureCount );
      break;
    }
    case QVariant::String:
    {
      column = Rcpp::StringVector( featureCount );
      break;
    }

    default:
      break;
  }

  return column;
}

void QgsRstatsUtils::preparedFieldsFromDf(Rcpp::DataFrame &df, QgsFields &fields)
{
  Rcpp::StringVector dfColumnNames = df.names();

  for ( int i = 0; i < df.ncol(); i++ )
  {
    QgsField field;
    bool addCurrentField = false;
    QString fieldName = QString::fromStdString( Rcpp::as<std::string>( dfColumnNames( i ) ) );

    switch ( TYPEOF( df[i] ) )
    {
      case ( LGLSXP ):
      {
        field = QgsField( fieldName, QVariant::Bool );
        addCurrentField = true;
        break;
      }
      case ( INTSXP ):
      {
        field = QgsField( fieldName, QVariant::Int );
        addCurrentField = true;
        break;
      }
      case ( REALSXP ):
      {
        field = QgsField( fieldName, QVariant::Double );
        addCurrentField = true;
        break;
      }
      case ( STRSXP ):
      {
        field = QgsField( fieldName, QVariant::String );
        addCurrentField = true;
        break;
      }
    }
    if ( addCurrentField )
      fields.append( field );
  }
}

Qgis::WkbType QgsRstatsUtils::wkbType(Rcpp::DataFrame &df)
{
  Qgis::WkbType wkbType;

  if ( hasSfColumn(df) )
  {
    Rcpp::Function st_geometry_type = Rcpp::Function( "st_geometry_type", Rcpp::Environment::namespace_env( "sf" ) );
    Rcpp::StringVector geometryTypeList = st_geometry_type( df, Rcpp::Named( "by_geometry" ) = false );
    QString geometryTypeNameString = QString::fromStdString( Rcpp::as<std::string>( geometryTypeList[0] ) );
    wkbType = QgsGeometry::fromWkt( QString( "%1 ()" ).arg( geometryTypeNameString ) ).wkbType();
  }
  else
  {
    wkbType = Qgis::WkbType::NoGeometry;
  }

  return wkbType;
}

bool QgsRstatsUtils::isSf(Rcpp::DataFrame &df)
{
  return df.inherits( "sf" );
}

bool QgsRstatsUtils::hasSfColumn(Rcpp::DataFrame &df)
{
  return df.hasAttribute( "sf_column" );
}

QgsCoordinateReferenceSystem QgsRstatsUtils::crs(Rcpp::DataFrame &df)
{
  QgsCoordinateReferenceSystem crs;

  if ( hasSfColumn(df) )
  {
    Rcpp::Function st_crs = Rcpp::Function( "st_crs", Rcpp::Environment::namespace_env( "sf" ) );
    Rcpp::List crsList = st_crs( df );
    Rcpp::StringVector crsWkt = crsList["wkt"];
    QString wkt = QString::fromStdString( Rcpp::as<std::string>( crsWkt[0] ) );
    crs = QgsCoordinateReferenceSystem::fromWkt( wkt );
  }

  return crs;
}

std::string QgsRstatsUtils::geometryColumn(Rcpp::DataFrame &df)
{
  return Rcpp::as<std::string>( df.attr( "sf_column" ) );
}

Rcpp::StringVector QgsRstatsUtils::geometries(Rcpp::DataFrame &df)
{
  Rcpp::StringVector geometries;
  Rcpp::List geometriesWKB;

  if ( isSf(df) && hasSfColumn(df) )
  {
    std::string geometryColumnName = geometryColumn(df);

    Rcpp::Function st_as_binary = Rcpp::Function( "st_as_binary", Rcpp::Environment::namespace_env( "sf" ) );
    Rcpp::Function wkb_translate_wkt = Rcpp::Function( "wkb_translate_wkt", Rcpp::Environment::namespace_env( "wk" ) );
    SEXP geometryColumnCall = Rf_lang3( R_DollarSymbol, df, Rf_mkString( geometryColumnName.c_str() ) );
    geometries = wkb_translate_wkt( st_as_binary( Rf_eval( geometryColumnCall, R_GlobalEnv ) ) );
  }

  return geometries;
}

void QgsRstatsUtils::prepareFeature(QgsFeature &feature, Rcpp::DataFrame &df, int row, Rcpp::StringVector &geometries)
{
  Rcpp::StringVector dfColumnNames = df.names();

  QgsAttributes featureAttributes;
  featureAttributes.reserve( feature.fields().count());

  int currentAttributeField = 0;

  for ( int j = 0; j < df.ncol(); j++ )
  {
    switch ( TYPEOF( df[j] ) )
    {
      case ( LGLSXP ):
      {
        Rcpp::LogicalVector column = Rcpp::as<Rcpp::LogicalVector>( df( j ) );
        featureAttributes.insert( currentAttributeField, column( row ) );
        currentAttributeField++;
        break;
      }
      case ( INTSXP ):
      {
        if ( Rcpp::as<std::string>( dfColumnNames( j ) ) == "fid" )
          break;
        Rcpp::IntegerVector column = Rcpp::as<Rcpp::IntegerVector>( df( j ) );
        featureAttributes.insert( currentAttributeField, column( row ) );
        currentAttributeField++;
        break;
      }
      case ( REALSXP ):
      {
        Rcpp::DoubleVector column = Rcpp::as<Rcpp::DoubleVector>( df( j ) );
        featureAttributes.insert( currentAttributeField, column( row ) );
        currentAttributeField++;
        break;
      }
      case ( STRSXP ):
      {
        Rcpp::StringVector column = Rcpp::as<Rcpp::StringVector>( df( j ) );
        featureAttributes.insert( currentAttributeField, QString::fromStdString( Rcpp::as<std::string>( column( row ) ) ) );
        currentAttributeField++;
        break;
      }
    }
  }

  feature.setAttributes( featureAttributes );

  if ( hasSfColumn(df) )
  {
    std::string wkt = Rcpp::as<std::string>( geometries[row] );
    QgsGeometry geom = QgsGeometry::fromWkt( QString::fromStdString( wkt ) );
    feature.setGeometry( geom );
  }
}
