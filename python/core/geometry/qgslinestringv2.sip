/** \ingroup core
 * \class QgsLineStringV2
 * \brief Line string geometry type, with support for z-dimension and m-values.
 * \note added in QGIS 2.10
 */

class QgsLineStringV2: public QgsCurveV2
{
%TypeHeaderCode
#include <qgslinestringv2.h>
%End

  public:
    QgsLineStringV2();
    ~QgsLineStringV2();

    /** Resets the line string to match the line string in a WKB geometry.
     * @param type WKB type
     * @param wkb WKB representation of line geometry
     * @note not available in Python bindings
     */
    //void fromWkbPoints( QgsWKBTypes::Type type, const QgsConstWkbPtr& wkb );

    /** Returns the specified point from inside the line string.
     * @param i index of point, starting at 0 for the first point
     */
    QgsPointV2 pointN( int i ) const;

    /** Returns the x-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0
     * @returns x-coordinate of node, or 0.0 if index is out of bounds
     * @see setXAt()
     */
    double xAt( int index ) const;

    /** Returns the y-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0
     * @returns y-coordinate of node, or 0.0 if index is out of bounds
     * @see setYAt()
     */
    double yAt( int index ) const;

    /** Returns the z-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0
     * @returns z-coordinate of node, or 0.0 if index is out of bounds or the line
     * does not have a z dimension
     * @see setZAt()
     */
    double zAt( int index ) const;

    /** Returns the m value of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0
     * @returns m value of node, or 0.0 if index is out of bounds or the line
     * does not have m values
     * @see setMAt()
     */
    double mAt( int index ) const;

    /** Sets the x-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string.
     * @param x x-coordinate of node
     * @see xAt()
     */
    void setXAt( int index, double x );

    /** Sets the y-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string.
     * @param y y-coordinate of node
     * @see yAt()
     */
    void setYAt( int index, double y );

    /** Sets the z-coordinate of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string, and the line string must have z-dimension.
     * @param z z-coordinate of node
     * @see zAt()
     */
    void setZAt( int index, double z );

    /** Sets the m value of the specified node in the line string.
     * @param index index of node, where the first node in the line is 0. Corresponding
     * node must already exist in line string, and the line string must have m values.
     * @param m m value of node
     * @see mAt()
     */
    void setMAt( int index, double m );

    /** Resets the line string to match the specified list of points.
     * @param points new points for line string. If empty, line string will be cleared.
     */
    void setPoints( const QList<QgsPointV2>& points );

    /** Appends the contents of another line string to the end of this line string.
     * @param line line to append. Ownership is not transferred.
     */
    void append( const QgsLineStringV2* line );

    /** Adds a new vertex to the end of the line string.
     * @param pt vertex to add
     */
    void addVertex( const QgsPointV2& pt );

    /** Closes the line string by appending the first point to the end of the line, if it is not already closed.*/
    void close();

    virtual QgsLineStringV2* reversed() const /Factory/;

    /** Returns a QPolygonF representing the line string.
     */
    QPolygonF asQPolygonF() const;

    //reimplemented methods

    virtual QString geometryType() const;
    virtual int dimension() const;
    virtual QgsLineStringV2* clone() const /Factory/;

    virtual bool fromWkb( const unsigned char* wkb );
    virtual bool fromWkt( const QString& wkt );

    int wkbSize() const;
    unsigned char* asWkb( int& binarySize ) const;
    QString asWkt( int precision = 17 ) const;
    QDomElement asGML2( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QDomElement asGML3( QDomDocument& doc, int precision = 17, const QString& ns = "gml" ) const;
    QString asJSON( int precision = 17 ) const;

    //curve interface
    virtual double length() const;
    virtual QgsPointV2 startPoint() const;
    virtual QgsPointV2 endPoint() const;
    virtual QgsLineStringV2* curveToLine() const /Factory/;

    int numPoints() const;
    void points( QList<QgsPointV2>& pt ) const;

    void draw( QPainter& p ) const;

    void transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d = QgsCoordinateTransform::ForwardTransform );
    void transform( const QTransform& t );

    void addToPainterPath( QPainterPath& path ) const;
    void drawAsPolygon( QPainter& p ) const;

    virtual bool insertVertex( const QgsVertexId& position, const QgsPointV2& vertex );
    virtual bool moveVertex( const QgsVertexId& position, const QgsPointV2& newPos );
    virtual bool deleteVertex( const QgsVertexId& position );

    double closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const;
    bool pointAt( int i, QgsPointV2& vertex, QgsVertexId::VertexType& type ) const;

    void sumUpArea( double& sum ) const;
    double vertexAngle( const QgsVertexId& vertex ) const;

    virtual bool addZValue( double zValue = 0 );
    virtual bool addMValue( double mValue = 0 );

};
