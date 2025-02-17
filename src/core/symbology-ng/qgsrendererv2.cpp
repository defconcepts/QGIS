/***************************************************************************
    qgsrendererv2.cpp
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrendererv2.h"
#include "qgssymbolv2.h"
#include "qgssymbollayerv2utils.h"
#include "qgsrulebasedrendererv2.h"
#include "qgsdatadefined.h"

#include "qgssinglesymbolrendererv2.h" // for default renderer

#include "qgsrendererv2registry.h"

#include "qgsrendercontext.h"
#include "qgsclipper.h"
#include "qgsgeometry.h"
#include "qgsgeometrycollectionv2.h"
#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgspainteffect.h"
#include "qgseffectstack.h"
#include "qgspainteffectregistry.h"
#include "qgswkbptr.h"
#include "qgspointv2.h"

#include <QDomElement>
#include <QDomDocument>
#include <QPolygonF>



const unsigned char* QgsFeatureRendererV2::_getPoint( QPointF& pt, QgsRenderContext& context, const unsigned char* wkb )
{
  QgsConstWkbPtr wkbPtr( wkb + 1 );
  unsigned int wkbType;
  wkbPtr >> wkbType >> pt.rx() >> pt.ry();

  if (( QgsWKBTypes::Type )wkbType == QgsWKBTypes::Point25D || ( QgsWKBTypes::Type )wkbType == QgsWKBTypes::PointZ )
    wkbPtr += sizeof( double );

  if ( context.coordinateTransform() )
  {
    double z = 0; // dummy variable for coordiante transform
    context.coordinateTransform()->transformInPlace( pt.rx(), pt.ry(), z );
  }

  context.mapToPixel().transformInPlace( pt.rx(), pt.ry() );

  return wkbPtr;
}

const unsigned char* QgsFeatureRendererV2::_getLineString( QPolygonF& pts, QgsRenderContext& context, const unsigned char* wkb, bool clipToExtent )
{
  QgsConstWkbPtr wkbPtr( wkb + 1 );
  unsigned int wkbType, nPoints;
  wkbPtr >> wkbType >> nPoints;

  bool hasZValue = QgsWKBTypes::hasZ(( QgsWKBTypes::Type )wkbType );
  bool hasMValue = QgsWKBTypes::hasM(( QgsWKBTypes::Type )wkbType );

  double x = 0.0;
  double y = 0.0;
  const QgsCoordinateTransform* ct = context.coordinateTransform();
  const QgsMapToPixel& mtp = context.mapToPixel();

  //apply clipping for large lines to achieve a better rendering performance
  if ( clipToExtent && nPoints > 1 )
  {
    const QgsRectangle& e = context.extent();
    double cw = e.width() / 10; double ch = e.height() / 10;
    QgsRectangle clipRect( e.xMinimum() - cw, e.yMinimum() - ch, e.xMaximum() + cw, e.yMaximum() + ch );
    wkbPtr = QgsConstWkbPtr( QgsClipper::clippedLineWKB( wkb, clipRect, pts ) );
  }
  else
  {
    pts.resize( nPoints );

    QPointF* ptr = pts.data();
    for ( unsigned int i = 0; i < nPoints; ++i, ++ptr )
    {
      wkbPtr >> x >> y;
      if ( hasZValue )
        wkbPtr += sizeof( double );
      if ( hasMValue )
        wkbPtr += sizeof( double );

      *ptr = QPointF( x, y );
    }
  }

  //transform the QPolygonF to screen coordinates
  if ( ct )
  {
    ct->transformPolygon( pts );
  }

  QPointF* ptr = pts.data();
  for ( int i = 0; i < pts.size(); ++i, ++ptr )
  {
    mtp.transformInPlace( ptr->rx(), ptr->ry() );
  }

  return wkbPtr;
}

const unsigned char* QgsFeatureRendererV2::_getPolygon( QPolygonF& pts, QList<QPolygonF>& holes, QgsRenderContext& context, const unsigned char* wkb, bool clipToExtent )
{
  QgsConstWkbPtr wkbPtr( wkb + 1 );

  unsigned int wkbType, numRings;
  wkbPtr >> wkbType >> numRings;

  if ( numRings == 0 )  // sanity check for zero rings in polygon
    return wkbPtr;

  bool hasZValue = QgsWKBTypes::hasZ(( QgsWKBTypes::Type )wkbType );
  bool hasMValue = QgsWKBTypes::hasM(( QgsWKBTypes::Type )wkbType );

  double x, y;
  holes.clear();

  const QgsCoordinateTransform* ct = context.coordinateTransform();
  const QgsMapToPixel& mtp = context.mapToPixel();
  const QgsRectangle& e = context.extent();
  double cw = e.width() / 10; double ch = e.height() / 10;
  QgsRectangle clipRect( e.xMinimum() - cw, e.yMinimum() - ch, e.xMaximum() + cw, e.yMaximum() + ch );

  for ( unsigned int idx = 0; idx < numRings; idx++ )
  {
    unsigned int nPoints;
    wkbPtr >> nPoints;

    QPolygonF poly( nPoints );

    // Extract the points from the WKB and store in a pair of vectors.
    QPointF* ptr = poly.data();
    for ( unsigned int jdx = 0; jdx < nPoints; ++jdx, ++ptr )
    {
      wkbPtr >> x >> y;
      if ( hasZValue )
        wkbPtr += sizeof( double );
      if ( hasMValue )
        wkbPtr += sizeof( double );

      *ptr = QPointF( x, y );
    }

    if ( nPoints < 1 )
      continue;

    //clip close to view extent, if needed
    QRectF ptsRect = poly.boundingRect();
    if ( clipToExtent && !context.extent().contains( ptsRect ) ) QgsClipper::trimPolygon( poly, clipRect );

    //transform the QPolygonF to screen coordinates
    if ( ct )
    {
      ct->transformPolygon( poly );
    }


    ptr = poly.data();
    for ( int i = 0; i < poly.size(); ++i, ++ptr )
    {
      mtp.transformInPlace( ptr->rx(), ptr->ry() );
    }

    if ( idx == 0 )
      pts = poly;
    else
      holes.append( poly );
  }

  return wkbPtr;
}

void QgsFeatureRendererV2::setScaleMethodToSymbol( QgsSymbolV2* symbol, int scaleMethod )
{
  if ( symbol )
  {
    if ( symbol->type() == QgsSymbolV2::Marker )
    {
      QgsMarkerSymbolV2* ms = static_cast<QgsMarkerSymbolV2*>( symbol );
      if ( ms )
      {
        ms->setScaleMethod(( QgsSymbolV2::ScaleMethod )scaleMethod );
      }
    }
  }
}

void QgsFeatureRendererV2::copyPaintEffect( QgsFeatureRendererV2 *destRenderer ) const
{
  if ( !destRenderer || !mPaintEffect )
    return;

  destRenderer->setPaintEffect( mPaintEffect->clone() );
}


QgsFeatureRendererV2::QgsFeatureRendererV2( const QString& type )
    : mType( type )
    , mUsingSymbolLevels( false )
    , mCurrentVertexMarkerType( QgsVectorLayer::Cross )
    , mCurrentVertexMarkerSize( 3 )
    , mPaintEffect( 0 )
    , mForceRaster( false )
{
  mPaintEffect = QgsPaintEffectRegistry::defaultStack();
  mPaintEffect->setEnabled( false );
}

QgsFeatureRendererV2::~QgsFeatureRendererV2()
{
  delete mPaintEffect;
}

QgsFeatureRendererV2* QgsFeatureRendererV2::defaultRenderer( QGis::GeometryType geomType )
{
  return new QgsSingleSymbolRendererV2( QgsSymbolV2::defaultSymbol( geomType ) );
}

QgsSymbolV2* QgsFeatureRendererV2::symbolForFeature( QgsFeature& feature )
{
  QgsRenderContext context;
  context.setExpressionContext( QgsExpressionContextUtils::createFeatureBasedContext( feature, QgsFields() ) );
  return symbolForFeature( feature, context );
}

QgsSymbolV2* QgsFeatureRendererV2::symbolForFeature( QgsFeature &feature, QgsRenderContext &context )
{
  Q_UNUSED( context );
  // base method calls deprecated symbolForFeature to maintain API
  Q_NOWARN_DEPRECATED_PUSH
  return symbolForFeature( feature );
  Q_NOWARN_DEPRECATED_POP
}

QgsSymbolV2 *QgsFeatureRendererV2::originalSymbolForFeature( QgsFeature &feature )
{
  Q_NOWARN_DEPRECATED_PUSH
  return symbolForFeature( feature );
  Q_NOWARN_DEPRECATED_POP
}

QgsSymbolV2 *QgsFeatureRendererV2::originalSymbolForFeature( QgsFeature &feature, QgsRenderContext &context )
{
  return symbolForFeature( feature, context );
}

void QgsFeatureRendererV2::startRender( QgsRenderContext& context, const QgsVectorLayer* vlayer )
{
  startRender( context, vlayer->fields() );
}

bool QgsFeatureRendererV2::renderFeature( QgsFeature& feature, QgsRenderContext& context, int layer, bool selected, bool drawVertexMarker )
{
  QgsSymbolV2* symbol = symbolForFeature( feature, context );
  if ( symbol == NULL )
    return false;

  renderFeatureWithSymbol( feature, symbol, context, layer, selected, drawVertexMarker );
  return true;
}

void QgsFeatureRendererV2::renderFeatureWithSymbol( QgsFeature& feature, QgsSymbolV2* symbol, QgsRenderContext& context, int layer, bool selected, bool drawVertexMarker )
{
  QgsSymbolV2::SymbolType symbolType = symbol->type();


  const QgsGeometry* geom = feature.constGeometry();
  if ( !geom || !geom->geometry() )
  {
    return;
  }

  const QgsGeometry* segmentizedGeometry = geom;
  bool deleteSegmentizedGeometry = false;
  context.setGeometry( geom->geometry() );

  //convert curve types to normal point/line/polygon ones
  switch ( QgsWKBTypes::flatType( geom->geometry()->wkbType() ) )
  {
    case QgsWKBTypes::CurvePolygon:
    case QgsWKBTypes::CircularString:
    case QgsWKBTypes::CompoundCurve:
    case QgsWKBTypes::MultiSurface:
    case QgsWKBTypes::MultiCurve:
    {
      QgsAbstractGeometryV2* g = geom->geometry()->segmentize();
      if ( !g )
      {
        return;
      }
      segmentizedGeometry = new QgsGeometry( g );
      deleteSegmentizedGeometry = true;
      break;
    }

    default:
      break;
  }

  switch ( QgsWKBTypes::flatType( segmentizedGeometry->geometry()->wkbType() ) )
  {
    case QgsWKBTypes::Point:
    {
      if ( symbolType != QgsSymbolV2::Marker )
      {
        QgsDebugMsg( "point can be drawn only with marker symbol!" );
        break;
      }
      QPointF pt;
      _getPoint( pt, context, segmentizedGeometry->asWkb() );
      (( QgsMarkerSymbolV2* )symbol )->renderPoint( pt, &feature, context, layer, selected );
      if ( context.testFlag( QgsRenderContext::DrawSymbolBounds ) )
      {
        //draw debugging rect
        context.painter()->setPen( Qt::red );
        context.painter()->setBrush( QColor( 255, 0, 0, 100 ) );
        context.painter()->drawRect((( QgsMarkerSymbolV2* )symbol )->bounds( pt, context ) );
      }
    }
    break;
    case QgsWKBTypes::LineString:
    {
      if ( symbolType != QgsSymbolV2::Line )
      {
        QgsDebugMsg( "linestring can be drawn only with line symbol!" );
        break;
      }
      QPolygonF pts;
      _getLineString( pts, context, segmentizedGeometry->asWkb(), symbol->clipFeaturesToExtent() );
      (( QgsLineSymbolV2* )symbol )->renderPolyline( pts, &feature, context, layer, selected );
    }
    break;
    case QgsWKBTypes::Polygon:
    {
      if ( symbolType != QgsSymbolV2::Fill )
      {
        QgsDebugMsg( "polygon can be drawn only with fill symbol!" );
        break;
      }
      QPolygonF pts;
      QList<QPolygonF> holes;
      _getPolygon( pts, holes, context, segmentizedGeometry->asWkb(), symbol->clipFeaturesToExtent() );
      (( QgsFillSymbolV2* )symbol )->renderPolygon( pts, ( holes.count() ? &holes : NULL ), &feature, context, layer, selected );
    }
    break;

    case QgsWKBTypes::MultiPoint:
    {
      if ( symbolType != QgsSymbolV2::Marker )
      {
        QgsDebugMsg( "multi-point can be drawn only with marker symbol!" );
        break;
      }

      QgsConstWkbPtr wkbPtr( segmentizedGeometry->asWkb() + 1 + sizeof( int ) );
      unsigned int num;
      wkbPtr >> num;
      const unsigned char* ptr = wkbPtr;
      QPointF pt;

      for ( unsigned int i = 0; i < num; ++i )
      {
        ptr = QgsConstWkbPtr( _getPoint( pt, context, ptr ) );
        (( QgsMarkerSymbolV2* )symbol )->renderPoint( pt, &feature, context, layer, selected );
      }
    }
    break;

    case QgsWKBTypes::MultiCurve:
    case QgsWKBTypes::MultiLineString:
    {
      if ( symbolType != QgsSymbolV2::Line )
      {
        QgsDebugMsg( "multi-linestring can be drawn only with line symbol!" );
        break;
      }

      QgsConstWkbPtr wkbPtr( segmentizedGeometry->asWkb() + 1 + sizeof( int ) );
      unsigned int num;
      wkbPtr >> num;
      const unsigned char* ptr = wkbPtr;
      QPolygonF pts;

      const QgsGeometryCollectionV2* geomCollection = dynamic_cast<const QgsGeometryCollectionV2*>( geom->geometry() );

      for ( unsigned int i = 0; i < num; ++i )
      {
        if ( geomCollection )
        {
          context.setGeometry( geomCollection->geometryN( i ) );
        }
        ptr = QgsConstWkbPtr( _getLineString( pts, context, ptr, symbol->clipFeaturesToExtent() ) );
        (( QgsLineSymbolV2* )symbol )->renderPolyline( pts, &feature, context, layer, selected );
      }
    }
    break;

    case QgsWKBTypes::MultiSurface:
    case QgsWKBTypes::MultiPolygon:
    {
      if ( symbolType != QgsSymbolV2::Fill )
      {
        QgsDebugMsg( "multi-polygon can be drawn only with fill symbol!" );
        break;
      }

      QgsConstWkbPtr wkbPtr( segmentizedGeometry->asWkb() + 1 + sizeof( int ) );
      unsigned int num;
      wkbPtr >> num;
      const unsigned char* ptr = wkbPtr;
      QPolygonF pts;
      QList<QPolygonF> holes;

      const QgsGeometryCollectionV2* geomCollection = dynamic_cast<const QgsGeometryCollectionV2*>( geom->geometry() );

      for ( unsigned int i = 0; i < num; ++i )
      {
        if ( geomCollection )
        {
          context.setGeometry( geomCollection->geometryN( i ) );
        }
        ptr = _getPolygon( pts, holes, context, ptr, symbol->clipFeaturesToExtent() );
        (( QgsFillSymbolV2* )symbol )->renderPolygon( pts, ( holes.count() ? &holes : NULL ), &feature, context, layer, selected );
      }
      break;
    }
    default:
      QgsDebugMsg( QString( "feature %1: unsupported wkb type 0x%2 for rendering" ).arg( feature.id() ).arg( geom->wkbType(), 0, 16 ) );
  }

  if ( drawVertexMarker )
  {
    const QgsCoordinateTransform* ct = context.coordinateTransform();
    const QgsMapToPixel& mtp = context.mapToPixel();

    QgsPointV2 vertexPoint;
    QgsVertexId vertexId;
    double x, y, z;
    QPointF mapPoint;
    while ( geom->geometry()->nextVertex( vertexId, vertexPoint ) )
    {
      //transform
      x = vertexPoint.x(); y = vertexPoint.y(); z = vertexPoint.z();
      if ( ct )
      {
        ct->transformInPlace( x, y, z );
      }
      mapPoint.setX( x ); mapPoint.setY( y );
      mtp.transformInPlace( mapPoint.rx(), mapPoint.ry() );
      renderVertexMarker( mapPoint, context );
    }
  }

  if ( deleteSegmentizedGeometry )
  {
    delete segmentizedGeometry;
  }
}

QString QgsFeatureRendererV2::dump() const
{
  return "UNKNOWN RENDERER\n";
}

QgsSymbolV2List QgsFeatureRendererV2::symbols()
{
  QgsRenderContext context;
  return symbols( context );
}

QgsSymbolV2List QgsFeatureRendererV2::symbols( QgsRenderContext &context )
{
  Q_UNUSED( context );

  //base implementation calls deprecated method to maintain API
  Q_NOWARN_DEPRECATED_PUSH
  return symbols();
  Q_NOWARN_DEPRECATED_POP
}


QgsFeatureRendererV2* QgsFeatureRendererV2::load( QDomElement& element )
{
  // <renderer-v2 type=""> ... </renderer-v2>

  if ( element.isNull() )
    return NULL;

  // load renderer
  QString rendererType = element.attribute( "type" );

  QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( rendererType );
  if ( m == NULL )
    return NULL;

  QgsFeatureRendererV2* r = m->createRenderer( element );
  if ( r )
  {
    r->setUsingSymbolLevels( element.attribute( "symbollevels", "0" ).toInt() );
    r->setForceRasterRender( element.attribute( "forceraster", "0" ).toInt() );

    //restore layer effect
    QDomElement effectElem = element.firstChildElement( "effect" );
    if ( !effectElem.isNull() )
    {
      r->setPaintEffect( QgsPaintEffectRegistry::instance()->createEffect( effectElem ) );
    }
  }
  return r;
}

QDomElement QgsFeatureRendererV2::save( QDomDocument& doc )
{
  // create empty renderer element
  QDomElement rendererElem = doc.createElement( RENDERER_TAG_NAME );
  rendererElem.setAttribute( "forceraster", ( mForceRaster ? "1" : "0" ) );

  if ( mPaintEffect && !QgsPaintEffectRegistry::isDefaultStack( mPaintEffect ) )
    mPaintEffect->saveProperties( doc, rendererElem );

  return rendererElem;
}

QgsFeatureRendererV2* QgsFeatureRendererV2::loadSld( const QDomNode &node, QGis::GeometryType geomType, QString &errorMessage )
{
  QDomElement element = node.toElement();
  if ( element.isNull() )
    return NULL;

  // get the UserStyle element
  QDomElement userStyleElem = element.firstChildElement( "UserStyle" );
  if ( userStyleElem.isNull() )
  {
    // UserStyle element not found, nothing will be rendered
    errorMessage = "Info: UserStyle element not found.";
    return NULL;
  }

  // get the FeatureTypeStyle element
  QDomElement featTypeStyleElem = userStyleElem.firstChildElement( "FeatureTypeStyle" );
  if ( featTypeStyleElem.isNull() )
  {
    errorMessage = "Info: FeatureTypeStyle element not found.";
    return NULL;
  }

  // use the RuleRenderer when more rules are present or the rule
  // has filters or min/max scale denominators set,
  // otherwise use the SingleSymbol renderer
  bool needRuleRenderer = false;
  int ruleCount = 0;

  QDomElement ruleElem = featTypeStyleElem.firstChildElement( "Rule" );
  while ( !ruleElem.isNull() )
  {
    ruleCount++;

    // more rules present, use the RuleRenderer
    if ( ruleCount > 1 )
    {
      QgsDebugMsg( "more Rule elements found: need a RuleRenderer" );
      needRuleRenderer = true;
      break;
    }

    QDomElement ruleChildElem = ruleElem.firstChildElement();
    while ( !ruleChildElem.isNull() )
    {
      // rule has filter or min/max scale denominator, use the RuleRenderer
      if ( ruleChildElem.localName() == "Filter" ||
           ruleChildElem.localName() == "MinScaleDenominator" ||
           ruleChildElem.localName() == "MaxScaleDenominator" )
      {
        QgsDebugMsg( "Filter or Min/MaxScaleDenominator element found: need a RuleRenderer" );
        needRuleRenderer = true;
        break;
      }

      ruleChildElem = ruleChildElem.nextSiblingElement();
    }

    if ( needRuleRenderer )
    {
      break;
    }

    ruleElem = ruleElem.nextSiblingElement( "Rule" );
  }

  QString rendererType;
  if ( needRuleRenderer )
  {
    rendererType = "RuleRenderer";
  }
  else
  {
    rendererType = "singleSymbol";
  }
  QgsDebugMsg( QString( "Instantiating a '%1' renderer..." ).arg( rendererType ) );

  // create the renderer and return it
  QgsRendererV2AbstractMetadata* m = QgsRendererV2Registry::instance()->rendererMetadata( rendererType );
  if ( m == NULL )
  {
    errorMessage = QString( "Error: Unable to get metadata for '%1' renderer." ).arg( rendererType );
    return NULL;
  }

  QgsFeatureRendererV2* r = m->createRendererFromSld( featTypeStyleElem, geomType );
  return r;
}

QDomElement QgsFeatureRendererV2::writeSld( QDomDocument& doc, const QgsVectorLayer &layer ) const
{
  return writeSld( doc, layer.name() );
}

QDomElement QgsFeatureRendererV2::writeSld( QDomDocument& doc, const QString& styleName ) const
{
  QDomElement userStyleElem = doc.createElement( "UserStyle" );

  QDomElement nameElem = doc.createElement( "se:Name" );
  nameElem.appendChild( doc.createTextNode( styleName ) );
  userStyleElem.appendChild( nameElem );

  QDomElement featureTypeStyleElem = doc.createElement( "se:FeatureTypeStyle" );
  toSld( doc, featureTypeStyleElem );
  userStyleElem.appendChild( featureTypeStyleElem );

  return userStyleElem;
}

QgsLegendSymbologyList QgsFeatureRendererV2::legendSymbologyItems( QSize iconSize )
{
  Q_UNUSED( iconSize );
  // empty list by default
  return QgsLegendSymbologyList();
}

bool QgsFeatureRendererV2::legendSymbolItemsCheckable() const
{
  return false;
}

bool QgsFeatureRendererV2::legendSymbolItemChecked( const QString& key )
{
  Q_UNUSED( key );
  return false;
}

void QgsFeatureRendererV2::checkLegendSymbolItem( const QString& key, bool state )
{
  Q_UNUSED( key );
  Q_UNUSED( state );
}

QgsLegendSymbolList QgsFeatureRendererV2::legendSymbolItems( double scaleDenominator, const QString& rule )
{
  Q_UNUSED( scaleDenominator );
  Q_UNUSED( rule );
  return QgsLegendSymbolList();
}

QgsLegendSymbolListV2 QgsFeatureRendererV2::legendSymbolItemsV2() const
{
  QgsLegendSymbolList lst = const_cast<QgsFeatureRendererV2*>( this )->legendSymbolItems();
  QgsLegendSymbolListV2 lst2;
  int i = 0;
  for ( QgsLegendSymbolList::const_iterator it = lst.begin(); it != lst.end(); ++it, ++i )
  {
    lst2 << QgsLegendSymbolItemV2( it->second, it->first, QString::number( i ), legendSymbolItemsCheckable() );
  }
  return lst2;
}

void QgsFeatureRendererV2::setVertexMarkerAppearance( int type, int size )
{
  mCurrentVertexMarkerType = type;
  mCurrentVertexMarkerSize = size;
}

bool QgsFeatureRendererV2::willRenderFeature( QgsFeature &feat )
{
  Q_NOWARN_DEPRECATED_PUSH
  return symbolForFeature( feat ) != NULL;
  Q_NOWARN_DEPRECATED_POP
}

bool QgsFeatureRendererV2::willRenderFeature( QgsFeature &feat, QgsRenderContext &context )
{
  return symbolForFeature( feat, context ) != NULL;
}

void QgsFeatureRendererV2::renderVertexMarker( const QPointF &pt, QgsRenderContext& context )
{
  QgsVectorLayer::drawVertexMarker( pt.x(), pt.y(), *context.painter(),
                                    ( QgsVectorLayer::VertexMarkerType ) mCurrentVertexMarkerType,
                                    mCurrentVertexMarkerSize );
}

void QgsFeatureRendererV2::renderVertexMarkerPolyline( QPolygonF& pts, QgsRenderContext& context )
{
  Q_FOREACH ( const QPointF& pt, pts )
    renderVertexMarker( pt, context );
}

void QgsFeatureRendererV2::renderVertexMarkerPolygon( QPolygonF& pts, QList<QPolygonF>* rings, QgsRenderContext& context )
{
  Q_FOREACH ( const QPointF& pt, pts )
    renderVertexMarker( pt, context );

  if ( rings )
  {
    Q_FOREACH ( const QPolygonF& ring, *rings )
    {
      Q_FOREACH ( const QPointF& pt, ring )
        renderVertexMarker( pt, context );
    }
  }
}

QgsSymbolV2List QgsFeatureRendererV2::symbolsForFeature( QgsFeature& feat )
{
  QgsSymbolV2List lst;
  Q_NOWARN_DEPRECATED_PUSH
  QgsSymbolV2* s = symbolForFeature( feat );
  Q_NOWARN_DEPRECATED_POP
  if ( s ) lst.append( s );
  return lst;
}

QgsSymbolV2List QgsFeatureRendererV2::symbolsForFeature( QgsFeature &feat, QgsRenderContext &context )
{
  QgsSymbolV2List lst;
  QgsSymbolV2* s = symbolForFeature( feat, context );
  if ( s ) lst.append( s );
  return lst;
}

QgsSymbolV2List QgsFeatureRendererV2::originalSymbolsForFeature( QgsFeature& feat )
{
  QgsSymbolV2List lst;
  Q_NOWARN_DEPRECATED_PUSH
  QgsSymbolV2* s = originalSymbolForFeature( feat );
  Q_NOWARN_DEPRECATED_POP
  if ( s ) lst.append( s );
  return lst;
}

QgsSymbolV2List QgsFeatureRendererV2::originalSymbolsForFeature( QgsFeature &feat, QgsRenderContext &context )
{
  QgsSymbolV2List lst;
  QgsSymbolV2* s = originalSymbolForFeature( feat, context );
  if ( s ) lst.append( s );
  return lst;
}

QgsPaintEffect *QgsFeatureRendererV2::paintEffect() const
{
  return mPaintEffect;
}

void QgsFeatureRendererV2::setPaintEffect( QgsPaintEffect *effect )
{
  delete mPaintEffect;
  mPaintEffect = effect;
}

void QgsFeatureRendererV2::convertSymbolSizeScale( QgsSymbolV2 * symbol, QgsSymbolV2::ScaleMethod method, const QString & field )
{
  if ( symbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2 * s = static_cast<QgsMarkerSymbolV2 *>( symbol );
    if ( QgsSymbolV2::ScaleArea == QgsSymbolV2::ScaleMethod( method ) )
    {
      const QgsDataDefined dd( "coalesce(sqrt(" + QString::number( s->size() ) + " * (" + field + ")),0)" );
      s->setDataDefinedSize( dd );
    }
    else
    {
      const QgsDataDefined dd( "coalesce(" + QString::number( s->size() ) + " * (" + field + "),0)" );
      s->setDataDefinedSize( dd );
    }
    s->setScaleMethod( QgsSymbolV2::ScaleDiameter );
  }
  else if ( symbol->type() == QgsSymbolV2::Line )
  {
    QgsLineSymbolV2 * s = static_cast<QgsLineSymbolV2 *>( symbol );
    const QgsDataDefined dd( "coalesce(" + QString::number( s->width() ) + " * (" + field + "),0)" );
    s->setDataDefinedWidth( dd );
  }
}

void QgsFeatureRendererV2::convertSymbolRotation( QgsSymbolV2 * symbol, const QString & field )
{
  if ( symbol->type() == QgsSymbolV2::Marker )
  {
    QgsMarkerSymbolV2 * s = static_cast<QgsMarkerSymbolV2 *>( symbol );
    const QgsDataDefined dd(( s->angle()
                              ? QString::number( s->angle() ) + " + "
                              : QString() ) + field );
    s->setDataDefinedAngle( dd );
  }
}
