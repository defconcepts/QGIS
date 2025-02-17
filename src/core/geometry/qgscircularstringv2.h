/***************************************************************************
                         qgscircularstringv2.h
                         ---------------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCIRCULARSTRING_H
#define QGSCIRCULARSTRING_H

#include "qgscurvev2.h"
#include <QVector>

/** \ingroup core
 * \class QgsCircularStringV2
 * \brief Circular string geometry type
 * \note added in QGIS 2.10
 * \note this API is not considered stable and may change for 2.12
 */
class CORE_EXPORT QgsCircularStringV2: public QgsCurveV2
{
  public:
    QgsCircularStringV2();
    ~QgsCircularStringV2();

    virtual QString geometryType() const override { return "CircularString"; }
    virtual int dimension() const override { return 1; }
    virtual QgsCircularStringV2* clone() const override;
    virtual void clear() override;

    virtual QgsRectangle calculateBoundingBox() const override;

    virtual bool fromWkb( const unsigned char * wkb ) override;
    virtual bool fromWkt( const QString& wkt ) override;

    int wkbSize() const override;
    unsigned char* asWkb( int& binarySize ) const override;
    QString asWkt( int precision = 17 ) const override;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const override;
    QString asJSON( int precision = 17 ) const override;

    int numPoints() const override;

    /** Returns the point at index i within the circular string.
     */
    QgsPointV2 pointN( int i ) const;

    /**
     * @copydoc QgsCurveV2::points()
     */
    void points( QList<QgsPointV2>& pts ) const override;

    /** Sets the circular string's points
     */
    void setPoints( const QList<QgsPointV2>& points );

    /**
     * @copydoc QgsAbstractGeometryV2::length()
     */
    virtual double length() const override;

    /**
     * @copydoc QgsCurveV2::startPoint()
     */
    virtual QgsPointV2 startPoint() const override;
    /**
     * @copydoc QgsCurveV2::endPoint()
     */
    virtual QgsPointV2 endPoint() const override;
    /**
     * @copydoc QgsCurveV2::curveToLine()
     */
    virtual QgsLineStringV2* curveToLine() const override;

    void draw( QPainter& p ) const override;
    /** Transforms the geometry using a coordinate transform
     * @param ct coordinate transform
     * @param d transformation direction
     */
    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform ) override;
    void transform( const QTransform& t ) override;
#if 0
    void clip( const QgsRectangle& rect ) override;
#endif
    void addToPainterPath( QPainterPath& path ) const override;

    /**
     * @copydoc QgsCurveV2::drawAsPolygon()
     */
    void drawAsPolygon( QPainter& p ) const override;

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex ) override;
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos ) override;
    virtual bool deleteVertex( const QgsVertexId& position ) override;

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const override;
    /**
     * @copydoc QgsCurveV2::pointAt()
     */
    bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const override;

    /**
     * @copydoc QgsCurveV2::sumUpArea()
     */
    void sumUpArea( double& sum ) const override;

    /**
     * @copydoc QgsAbstractGeometryV2::hasCurvedSegments()
     */
    bool hasCurvedSegments() const override { return true; }

    /** Returns approximate rotation angle for a vertex. Usually average angle between adjacent segments.
        @param vertex the vertex id
        @return rotation in radians, clockwise from north*/
    double vertexAngle( const QgsVertexId& vertex ) const override;

    virtual QgsCircularStringV2* reversed() const override;

    virtual bool addZValue( double zValue = 0 ) override;
    virtual bool addMValue( double mValue = 0 ) override;

  private:
    QVector<double> mX;
    QVector<double> mY;
    QVector<double> mZ;
    QVector<double> mM;

    //helper methods for curveToLine
    void segmentize( const QgsPointV2& p1, const QgsPointV2& p2, const QgsPointV2& p3, QList<QgsPointV2>& points ) const;
    int segmentSide( const QgsPointV2& pt1, const QgsPointV2& pt3, const QgsPointV2& pt2 ) const;
    double interpolateArc( double angle, double a1, double a2, double a3, double zm1, double zm2, double zm3 ) const;
    static void arcTo( QPainterPath& path, const QPointF& pt1, const QPointF& pt2, const QPointF& pt3 );
    //bounding box of a single segment
    static QgsRectangle segmentBoundingBox( const QgsPointV2& pt1, const QgsPointV2& pt2, const QgsPointV2& pt3 );
    static QList<QgsPointV2> compassPointsOnSegment( double p1Angle, double p2Angle, double p3Angle, double centerX, double centerY, double radius );
    static double closestPointOnArc( double x1, double y1, double x2, double y2, double x3, double y3,
                                     const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon );
    void insertVertexBetween( int after, int before, int pointOnCircle );
    void deleteVertex( int i );
};

#endif // QGSCIRCULARSTRING_H
