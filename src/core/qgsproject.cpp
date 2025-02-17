/***************************************************************************
                          qgsproject.cpp -  description
                             -------------------
  begin                : July 23, 2004
  copyright            : (C) 2004 by Mark Coletti
  email                : mcoletti at gmail.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproject.h"

#include <deque>
#include <memory>

#include "qgsdatasourceuri.h"
#include "qgsexception.h"
#include "qgslayertree.h"
#include "qgslayertreeutils.h"
#include "qgslayertreeregistrybridge.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsmessagelog.h"
#include "qgspluginlayer.h"
#include "qgspluginlayerregistry.h"
#include "qgsprojectfiletransform.h"
#include "qgsprojectproperty.h"
#include "qgsprojectversion.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"
#include "qgsvisibilitypresetcollection.h"

#include <QApplication>
#include <QFileInfo>
#include <QDomNode>
#include <QObject>
#include <QTextStream>
#include <QDir>
#include <QUrl>

// canonical project instance
QgsProject *QgsProject::theProject_ = 0;

/**
    Take the given scope and key and convert them to a string list of key
    tokens that will be used to navigate through a Property hierarchy

    E.g., scope "someplugin" and key "/foo/bar/baz" will become a string list
    of { "properties", "someplugin", "foo", "bar", "baz" }.  "properties" is
    always first because that's the permanent ``root'' Property node.
 */
static
QStringList makeKeyTokens_( QString const &scope, QString const &key )
{
  QStringList keyTokens = QStringList( scope );
  keyTokens += key.split( '/', QString::SkipEmptyParts );

  // be sure to include the canonical root node
  keyTokens.push_front( "properties" );

  //check validy of keys since an unvalid xml name will will be dropped upon saving the xml file. If not valid, we print a message to the console.
  for (int i = 0; i < keyTokens.size(); ++i){
    QString keyToken = keyTokens.at(i);

    //invalid chars in XML are found at http://www.w3.org/TR/REC-xml/#NT-NameChar
    //note : it seems \x10000-\xEFFFF is valid, but it when added to the regexp, a lot of unwanted characters remain
    QString nameCharRegexp = QString( "[^:A-Z_a-z\\xC0-\\xD6\\xD8-\\xF6\\xF8-\\x2FF\\x370-\\x37D\\x37F-\\x1FFF\\x200C-\\x200D\\x2070-\\x218F\\x2C00-\\x2FEF\\x3001-\\xD7FF\\xF900-\\xFDCF\\xFDF0-\\xFFFD\\-\\.0-9\\xB7\\x0300-\\x036F\\x203F-\\x2040]" );
    QString nameStartCharRegexp = QString( "^[^:A-Z_a-z\\xC0-\\xD6\\xD8-\\xF6\\xF8-\\x2FF\\x370-\\x37D\\x37F-\\x1FFF\\x200C-\\x200D\\x2070-\\x218F\\x2C00-\\x2FEF\\x3001-\\xD7FF\\xF900-\\xFDCF\\xFDF0-\\xFFFD]" );

    if( keyToken.contains( QRegExp(nameCharRegexp) ) || keyToken.contains( QRegExp(nameStartCharRegexp) ) ){

      QString errorString = QObject::tr("Entry token invalid : '%1'. The token will not be saved to file.").arg(keyToken);
      QgsMessageLog::logMessage( errorString, QString::null, QgsMessageLog::CRITICAL );

    }

  }

  return keyTokens;
} // makeKeyTokens_




/**
   return the property that matches the given key sequence, if any

   @param scope scope of key
   @param key keyname
   @param rootProperty is likely to be the top level QgsPropertyKey in QgsProject:e:Imp.

   @return null if not found, otherwise located Property
*/
static
QgsProperty *findKey_( QString const &scope,
                       QString const &key,
                       QgsPropertyKey &rootProperty )
{
  QgsPropertyKey *currentProperty = &rootProperty;
  QgsProperty *nextProperty;           // link to next property down hiearchy

  QStringList keySequence = makeKeyTokens_( scope, key );

  while ( !keySequence.isEmpty() )
  {
    // if the current head of the sequence list matches the property name,
    // then traverse down the property hierarchy
    if ( keySequence.first() == currentProperty->name() )
    {
      // remove front key since we're traversing down a level
      keySequence.pop_front();

      if ( 1 == keySequence.count() )
      {
        // if we have only one key name left, then return the key found
        return currentProperty->find( keySequence.front() );
      }
      else if ( keySequence.isEmpty() )
      {
        // if we're out of keys then the current property is the one we
        // want; i.e., we're in the rate case of being at the top-most
        // property node
        return currentProperty;
      }
      else if (( nextProperty = currentProperty->find( keySequence.first() ) ) )
      {
        if ( nextProperty->isKey() )
        {
          currentProperty = static_cast<QgsPropertyKey*>( nextProperty );
        }
        else if ( nextProperty->isValue() && 1 == keySequence.count() )
        {
          // it may be that this may be one of several property value
          // nodes keyed by QDict string; if this is the last remaining
          // key token and the next property is a value node, then
          // that's the situation, so return the currentProperty
          return currentProperty;
        }
        else
        {
          // QgsPropertyValue not Key, so return null
          return 0;
        }
      }
      else
      {
        // if the next key down isn't found
        // then the overall key sequence doesn't exist
        return 0;
      }
    }
    else
    {
      return 0;
    }
  }

  return 0;
} // findKey_



/** Add the given key and value

@param scope scope of key
@param key key name
@param rootProperty is the property from which to start adding
@param value the value associated with the key
*/
static
QgsProperty *addKey_( QString const &scope,
                      QString const &key,
                      QgsPropertyKey *rootProperty,
                      const QVariant& value )
{
  QStringList keySequence = makeKeyTokens_( scope, key );

  // cursor through property key/value hierarchy
  QgsPropertyKey *currentProperty = rootProperty;
  QgsProperty *nextProperty; // link to next property down hiearchy
  QgsPropertyKey* newPropertyKey;

  while ( ! keySequence.isEmpty() )
  {
    // if the current head of the sequence list matches the property name,
    // then traverse down the property hierarchy
    if ( keySequence.first() == currentProperty->name() )
    {
      // remove front key since we're traversing down a level
      keySequence.pop_front();

      // if key sequence has one last element, then we use that as the
      // name to store the value
      if ( 1 == keySequence.count() )
      {
        currentProperty->setValue( keySequence.front(), value );
        return currentProperty;
      }
      // we're at the top element if popping the keySequence element
      // will leave it empty; in that case, just add the key
      else if ( keySequence.isEmpty() )
      {
        currentProperty->setValue( value );

        return currentProperty;
      }
      else if (( nextProperty = currentProperty->find( keySequence.first() ) ) )
      {
        currentProperty = dynamic_cast<QgsPropertyKey*>( nextProperty );

        if ( currentProperty )
        {
          continue;
        }
        else            // QgsPropertyValue not Key, so return null
        {
          return 0;
        }
      }
      else                // the next subkey doesn't exist, so add it
      {
        if (( newPropertyKey = currentProperty->addKey( keySequence.first() ) ) )
        {
          currentProperty = newPropertyKey;
        }
        continue;
      }
    }
    else
    {
      return 0;
    }
  }

  return 0;

} // addKey_



static
void removeKey_( QString const &scope,
                 QString const &key,
                 QgsPropertyKey &rootProperty )
{
  QgsPropertyKey *currentProperty = &rootProperty;

  QgsProperty *nextProperty = 0;   // link to next property down hiearchy
  QgsPropertyKey *previousQgsPropertyKey = 0; // link to previous property up hiearchy

  QStringList keySequence = makeKeyTokens_( scope, key );

  while ( ! keySequence.isEmpty() )
  {
    // if the current head of the sequence list matches the property name,
    // then traverse down the property hierarchy
    if ( keySequence.first() == currentProperty->name() )
    {
      // remove front key since we're traversing down a level
      keySequence.pop_front();

      // if we have only one key name left, then try to remove the key
      // with that name
      if ( 1 == keySequence.count() )
      {
        currentProperty->removeKey( keySequence.front() );
      }
      // if we're out of keys then the current property is the one we
      // want to remove, but we can't delete it directly; we need to
      // delete it from the parent property key container
      else if ( keySequence.isEmpty() )
      {
        previousQgsPropertyKey->removeKey( currentProperty->name() );
      }
      else if (( nextProperty = currentProperty->find( keySequence.first() ) ) )
      {
        previousQgsPropertyKey = currentProperty;
        currentProperty = dynamic_cast<QgsPropertyKey*>( nextProperty );

        if ( currentProperty )
        {
          continue;
        }
        else            // QgsPropertyValue not Key, so return null
        {
          return;
        }
      }
      else                // if the next key down isn't found
      {                   // then the overall key sequence doesn't exist
        return;
      }
    }
    else
    {
      return;
    }
  }

} // void removeKey_



struct QgsProject::Imp
{
  QFile file;                 // current physical project file
  QgsPropertyKey properties_; // property hierarchy
  QString title;              // project title
  bool dirty;                 // project has been modified since it has been read or saved

  Imp()
      : title()
      , dirty( false )
  {                             // top property node is the root
    // "properties" that contains all plug-in
    // and extra property keys and values
    properties_.name() = "properties"; // root property node always this value
  }

  /** Clear project properties when a new project is started
   */
  void clear()
  {
    // QgsDebugMsg( "Clearing project properties Impl->clear();" );

    file.setFileName( QString() );
    properties_.clearKeys();
    title.clear();
    dirty = false;
  }

};  // struct QgsProject::Imp



QgsProject::QgsProject()
    : imp_( new QgsProject::Imp )
    , mBadLayerHandler( new QgsProjectBadLayerDefaultHandler() )
    , mRelationManager( new QgsRelationManager( this ) )
    , mRootGroup( new QgsLayerTreeGroup )
{
  clear();

  // bind the layer tree to the map layer registry.
  // whenever layers are added to or removed from the registry,
  // layer tree will be updated
  mLayerTreeRegistryBridge = new QgsLayerTreeRegistryBridge( mRootGroup, this );
} // QgsProject ctor



QgsProject::~QgsProject()
{
  delete mBadLayerHandler;
  delete mRelationManager;
  delete mRootGroup;

  // note that QScopedPointer automatically deletes imp_ when it's destroyed
} // QgsProject dtor



QgsProject *QgsProject::instance()
{
  if ( !theProject_ )
  {
    theProject_ = new QgsProject;
  }
  return theProject_;
} // QgsProject *instance()

void QgsProject::setTitle( const QString &title )
{
  imp_->title = title;

  dirty( true );
}


QString QgsProject::title() const
{
  return imp_->title;
} // QgsProject::title() const


bool QgsProject::isDirty() const
{
  return imp_->dirty;
} // bool QgsProject::isDirty()


void QgsProject::dirty( bool b )
{
  imp_->dirty = b;
} // bool QgsProject::isDirty()

void QgsProject::setDirty( bool b )
{
  dirty( b );
}



void QgsProject::setFileName( QString const &name )
{
  imp_->file.setFileName( name );

  dirty( true );
} // void QgsProject::setFileName( QString const &name )



QString QgsProject::fileName() const
{
  return imp_->file.fileName();
}

QFileInfo QgsProject::fileInfo() const
{
  return QFileInfo( imp_->file );
}

void QgsProject::clear()
{
  imp_->clear();
  mEmbeddedLayers.clear();
  mRelationManager->clear();

  mVisibilityPresetCollection.reset( new QgsVisibilityPresetCollection() );

  mRootGroup->removeAllChildren();

  // reset some default project properties
  // XXX THESE SHOULD BE MOVED TO STATUSBAR RELATED SOURCE
  writeEntry( "PositionPrecision", "/Automatic", true );
  writeEntry( "PositionPrecision", "/DecimalPlaces", 2 );
  writeEntry( "Paths", "/Absolute", false );

  setDirty( false );
}

// basically a debugging tool to dump property list values
static void dump_( QgsPropertyKey const &topQgsPropertyKey )
{
  QgsDebugMsg( "current properties:" );
  topQgsPropertyKey.dump();
} // dump_


/**

Restore any optional properties found in "doc" to "properties".

properties tags for all optional properties.  Within that there will be scope
tags.  In the following example there exist one property in the "fsplugin"
scope.  "layers" is a list containing three string values.

\code{.xml}
<properties>
  <fsplugin>
    <foo type="int" >42</foo>
    <baz type="int" >1</baz>
    <layers type="QStringList" >
      <value>railroad</value>
      <value>airport</value>
    </layers>
    <xyqzzy type="int" >1</xyqzzy>
    <bar type="double" >123.456</bar>
    <feature_types type="QStringList" >
       <value>type</value>
    </feature_types>
  </fsplugin>
</properties>
\endcode

@param doc xml document
@param project_properties should be the top QgsPropertyKey node.

*/
static
void
_getProperties( QDomDocument const &doc, QgsPropertyKey &project_properties )
{
  QDomNodeList properties = doc.elementsByTagName( "properties" );

  if ( properties.count() > 1 )
  {
    QgsDebugMsg( "there appears to be more than one ``properties'' XML tag ... bailing" );
    return;
  }
  else if ( properties.count() < 1 )  // no properties found, so we're done
  {
    return;
  }

  // item(0) because there should only be ONE "properties" node
  QDomNodeList scopes = properties.item( 0 ).childNodes();

  if ( scopes.count() < 1 )
  {
    QgsDebugMsg( "empty ``properties'' XML tag ... bailing" );
    return;
  }

  QDomNode propertyNode = properties.item( 0 );

  if ( ! project_properties.readXML( propertyNode ) )
  {
    QgsDebugMsg( "Project_properties.readXML() failed" );
  }
} // _getProperties




/**
   Get the project title

   XML in file has this form:
\code{.xml}
   <qgis projectname="default project">
   <title>a project title</title>
\endcode

   @todo XXX we should go with the attribute xor title, not both.
*/
static void _getTitle( QDomDocument const &doc, QString &title )
{
  QDomNodeList nl = doc.elementsByTagName( "title" );

  title = "";                 // by default the title will be empty

  if ( !nl.count() )
  {
    QgsDebugMsg( "unable to find title element" );
    return;
  }

  QDomNode titleNode = nl.item( 0 );  // there should only be one, so zeroth element ok

  if ( !titleNode.hasChildNodes() ) // if not, then there's no actual text
  {
    QgsDebugMsg( "unable to find title element" );
    return;
  }

  QDomNode titleTextNode = titleNode.firstChild();  // should only have one child

  if ( !titleTextNode.isText() )
  {
    QgsDebugMsg( "unable to find title element" );
    return;
  }

  QDomText titleText = titleTextNode.toText();

  title = titleText.data();

} // _getTitle


/** Return the version string found in the given Dom document

   @returns the version string or an empty string if none found
 */
static QgsProjectVersion _getVersion( QDomDocument const &doc )
{
  QDomNodeList nl = doc.elementsByTagName( "qgis" );

  if ( !nl.count() )
  {
    QgsDebugMsg( " unable to find qgis element in project file" );
    return QgsProjectVersion( 0, 0, 0, QString() );
  }

  QDomNode qgisNode = nl.item( 0 );  // there should only be one, so zeroth element ok

  QDomElement qgisElement = qgisNode.toElement(); // qgis node should be element
  QgsProjectVersion projectVersion( qgisElement.attribute( "version" ) );
  return projectVersion;
} // _getVersion



/**
   Read map layers from project file


   @returns \code QPair< bool, QList<QDomNode> > \endcode
        bool is true if function worked; else is false.
     list contains nodes corresponding to layers that couldn't be loaded

   @note XML of form:

\code{.xml}
   <maplayer type="vector">
      <layername>Hydrop</layername>
      <datasource>/data/usgs/city_shp/hydrop.shp</datasource>
      <zorder>0</zorder>
      <provider>ogr</provider>
      <singlesymbol>
         <renderitem>
            <value>blabla</value>
            <symbol>
               <outlinecolor red="85" green="0" blue="255" />
               <outlinestyle>SolidLine</outlinestyle>
               <outlinewidth>1</outlinewidth>
               <fillcolor red="0" green="170" blue="255" />
               <fillpattern>SolidPattern</fillpattern>
            </symbol>
            <label>blabla</label>
         </renderitem>
      </singlesymbol>
      <label>0</label>
      <labelattributes>
         <label text="Label" field="" />
         <family name="Sans Serif" field="" />
         <size value="12" units="pt" field="" />
         <bold on="0" field="" />
         <italic on="0" field="" />
         <underline on="0" field="" />
         <color red="0" green="0" blue="0" field="" />
         <x field="" />
         <y field="" />
         <offset  units="pt" x="0" xfield="" y="0" yfield="" />
         <angle value="0" field="" />
         <alignment value="center" field="" />
      </labelattributes>
   </maplayer>
\endcode
*/
QPair< bool, QList<QDomNode> > QgsProject::_getMapLayers( QDomDocument const &doc )
{
  // Layer order is set by the restoring the legend settings from project file.
  // This is done on the 'readProject( ... )' signal

  QDomNodeList nl = doc.elementsByTagName( "maplayer" );

  QList<QDomNode> brokenNodes; // a list of Dom nodes corresponding to layers
  // that we were unable to load; this could be
  // because the layers were removed or
  // re-located after the project was last saved

  // process the map layer nodes

  if ( 0 == nl.count() )      // if we have no layers to process, bail
  {
    return qMakePair( true, brokenNodes ); // Decided to return "true" since it's
    // possible for there to be a project with no
    // layers; but also, more imporantly, this
    // would cause the tests/qgsproject to fail
    // since the test suite doesn't currently
    // support test layers
  }

  bool returnStatus = true;

  emit layerLoaded( 0, nl.count() );

  // Collect vector layers with joins.
  // They need to refresh join caches and symbology infos after all layers are loaded
  QList< QPair< QgsVectorLayer*, QDomElement > > vLayerList;

  for ( int i = 0; i < nl.count(); i++ )
  {
    QDomNode node = nl.item( i );
    QDomElement element = node.toElement();

    QString name = node.namedItem( "layername" ).toElement().text();
    if ( !name.isNull() )
      emit loadingLayer( tr( "Loading layer %1" ).arg( name ) );

    if ( element.attribute( "embedded" ) == "1" )
    {
      createEmbeddedLayer( element.attribute( "id" ), readPath( element.attribute( "project" ) ), brokenNodes, vLayerList );
      continue;
    }
    else
    {
      if ( !addLayer( element, brokenNodes, vLayerList ) )
      {
        returnStatus = false;
      }
    }
    emit layerLoaded( i + 1, nl.count() );
  }

  // Update field map of layers with joins and create join caches if necessary
  // Needs to be done here once all dependent layers are loaded
  QList< QPair< QgsVectorLayer*, QDomElement > >::iterator vIt = vLayerList.begin();
  for ( ; vIt != vLayerList.end(); ++vIt )
  {
    vIt->first->createJoinCaches();
    vIt->first->updateFields();
  }

  QSet<QgsVectorLayer *> notified;
  for ( vIt = vLayerList.begin(); vIt != vLayerList.end(); ++vIt )
  {
    if ( notified.contains( vIt->first ) )
      continue;

    notified << vIt->first;
    emit readMapLayer( vIt->first, vIt->second );
  }



  return qMakePair( returnStatus, brokenNodes );
} // _getMapLayers

bool QgsProject::addLayer( const QDomElement &layerElem, QList<QDomNode> &brokenNodes, QList< QPair< QgsVectorLayer*, QDomElement > > &vectorLayerList )
{
  QString type = layerElem.attribute( "type" );
  QgsDebugMsg( "Layer type is " + type );
  QgsMapLayer *mapLayer = 0;

  if ( type == "vector" )
  {
    mapLayer = new QgsVectorLayer;
  }
  else if ( type == "raster" )
  {
    mapLayer = new QgsRasterLayer;
  }
  else if ( type == "plugin" )
  {
    QString typeName = layerElem.attribute( "name" );
    mapLayer = QgsPluginLayerRegistry::instance()->createLayer( typeName );
  }

  if ( !mapLayer )
  {
    QgsDebugMsg( "Unable to create layer" );

    return false;
  }

  Q_CHECK_PTR( mapLayer );

  // have the layer restore state that is stored in Dom node
  if ( mapLayer->readLayerXML( layerElem ) && mapLayer->isValid() )
  {
    // postpone readMapLayer signal for vector layers with joins
    QgsVectorLayer *vLayer = qobject_cast<QgsVectorLayer*>( mapLayer );
    if ( !vLayer || vLayer->vectorJoins().size() == 0 )
      emit readMapLayer( mapLayer, layerElem );
    else
      vectorLayerList.push_back( qMakePair( vLayer, layerElem ) );

    QList<QgsMapLayer *> myLayers;
    myLayers << mapLayer;
    QgsMapLayerRegistry::instance()->addMapLayers( myLayers );

    return true;
  }
  else
  {
    delete mapLayer;

    QgsDebugMsg( "Unable to load " + type + " layer" );
    brokenNodes.push_back( layerElem );
    return false;
  }
}


/**
   Please note that most of the contents were copied from qgsproject
*/
bool QgsProject::read( QFileInfo const &file )
{
  imp_->file.setFileName( file.filePath() );

  return read();
} // QgsProject::read



/**
   presuming that the caller has already reset the map canvas, map registry, and legend
 */
bool QgsProject::read()
{
  clearError();

  QScopedPointer<QDomDocument> doc( new QDomDocument( "qgis" ) );

  if ( !imp_->file.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    imp_->file.close();

    setError( tr( "Unable to open %1" ).arg( imp_->file.fileName() ) );

    return false;
  }

  // location of problem associated with errorMsg
  int line, column;
  QString errorMsg;

  if ( !doc->setContent( &imp_->file, &errorMsg, &line, &column ) )
  {
    // want to make this class as GUI independent as possible; so commented out
#if 0
    QMessageBox::critical( 0, tr( "Project File Read Error" ),
                           tr( "%1 at line %2 column %3" ).arg( errorMsg ).arg( line ).arg( column ) );
#endif

    QString errorString = tr( "Project file read error: %1 at line %2 column %3" )
                          .arg( errorMsg ).arg( line ).arg( column );

    QgsDebugMsg( errorString );

    imp_->file.close();

    setError( tr( "%1 for file %2" ).arg( errorString, imp_->file.fileName() ) );

    return false;
  }

  imp_->file.close();


  QgsDebugMsg( "Opened document " + imp_->file.fileName() );
  QgsDebugMsg( "Project title: " + imp_->title );

  // get project version string, if any
  QgsProjectVersion fileVersion =  _getVersion( *doc );
  QgsProjectVersion thisVersion( QGis::QGIS_VERSION );

  if ( thisVersion > fileVersion )
  {
    QgsLogger::warning( "Loading a file that was saved with an older "
                        "version of qgis (saved in " + fileVersion.text() +
                        ", loaded in " + QGis::QGIS_VERSION +
                        "). Problems may occur." );

    QgsProjectFileTransform projectFile( *doc, fileVersion );

    //! Shows a warning when an old project file is read.
    emit oldProjectVersionWarning( fileVersion.text() );
    QgsDebugMsg( "Emitting oldProjectVersionWarning(oldVersion)." );

    projectFile.updateRevision( thisVersion );
  }

  // start new project, just keep the file name
  QString fileName = imp_->file.fileName();
  clear();
  imp_->file.setFileName( fileName );

  // now get any properties
  _getProperties( *doc, imp_->properties_ );

  QgsDebugMsg( QString::number( imp_->properties_.count() ) + " properties read" );

  dump_( imp_->properties_ );

  // now get project title
  _getTitle( *doc, imp_->title );

  // read the layer tree from project file

  mRootGroup->setCustomProperty( "loading", 1 );

  QDomElement layerTreeElem = doc->documentElement().firstChildElement( "layer-tree-group" );
  if ( !layerTreeElem.isNull() )
  {
    mRootGroup->readChildrenFromXML( layerTreeElem );
  }
  else
  {
    QgsLayerTreeUtils::readOldLegend( mRootGroup, doc->documentElement().firstChildElement( "legend" ) );
  }

  QgsDebugMsg( "Loaded layer tree:\n " + mRootGroup->dump() );

  mLayerTreeRegistryBridge->setEnabled( false );

  // get the map layers
  QPair< bool, QList<QDomNode> > getMapLayersResults =  _getMapLayers( *doc );

  // review the integrity of the retrieved map layers
  bool clean = getMapLayersResults.first;

  if ( !clean )
  {
    QgsDebugMsg( "Unable to get map layers from project file." );

    if ( ! getMapLayersResults.second.isEmpty() )
    {
      QgsDebugMsg( "there are " + QString::number( getMapLayersResults.second.size() ) + " broken layers" );
    }

    // we let a custom handler to decide what to do with missing layers
    // (default implementation ignores them, there's also a GUI handler that lets user choose correct path)
    mBadLayerHandler->handleBadLayers( getMapLayersResults.second, *doc );
  }

  mLayerTreeRegistryBridge->setEnabled( true );

  // load embedded groups and layers
  loadEmbeddedNodes( mRootGroup );

  // make sure the are just valid layers
  QgsLayerTreeUtils::removeInvalidLayers( mRootGroup );

  mRootGroup->removeCustomProperty( "loading" );

  mVisibilityPresetCollection.reset( new QgsVisibilityPresetCollection() );
  mVisibilityPresetCollection->readXML( *doc );

  // read the project: used by map canvas and legend
  emit readProject( *doc );

  // if all went well, we're allegedly in pristine state
  if ( clean )
    dirty( false );

  return true;

} // QgsProject::read


void QgsProject::loadEmbeddedNodes( QgsLayerTreeGroup *group )
{
  Q_FOREACH ( QgsLayerTreeNode *child, group->children() )
  {
    if ( QgsLayerTree::isGroup( child ) )
    {
      QgsLayerTreeGroup *childGroup = QgsLayerTree::toGroup( child );
      if ( childGroup->customProperty( "embedded" ).toInt() )
      {
        // make sure to convert the path from relative to absolute
        QString projectPath = readPath( childGroup->customProperty( "embedded_project" ).toString() );
        childGroup->setCustomProperty( "embedded_project", projectPath );

        QgsLayerTreeGroup *newGroup = createEmbeddedGroup( childGroup->name(), projectPath, childGroup->customProperty( "embedded-invisible-layers" ).toStringList() );
        if ( newGroup )
        {
          QList<QgsLayerTreeNode*> clonedChildren;
          Q_FOREACH ( QgsLayerTreeNode *newGroupChild, newGroup->children() )
            clonedChildren << newGroupChild->clone();
          delete newGroup;

          childGroup->insertChildNodes( 0, clonedChildren );
        }
      }
      else
      {
        loadEmbeddedNodes( childGroup );
      }
    }
    else if ( QgsLayerTree::isLayer( child ) )
    {
      if ( child->customProperty( "embedded" ).toInt() )
      {
        QList<QDomNode> brokenNodes;
        QList< QPair< QgsVectorLayer*, QDomElement > > vectorLayerList;
        createEmbeddedLayer( QgsLayerTree::toLayer( child )->layerId(), child->customProperty( "embedded_project" ).toString(), brokenNodes, vectorLayerList );
      }
    }

  }
}


bool QgsProject::read( QDomNode &layerNode )
{
  QList<QDomNode> brokenNodes;
  QList< QPair< QgsVectorLayer*, QDomElement > > vectorLayerList;
  return addLayer( layerNode.toElement(), brokenNodes, vectorLayerList );
} // QgsProject::read( QDomNode &layerNode )



bool QgsProject::write( QFileInfo const &file )
{
  imp_->file.setFileName( file.filePath() );

  return write();
} // QgsProject::write( QFileInfo const &file )


bool QgsProject::write()
{
  clearError();

  // Create backup file
  if ( QFile::exists( fileName() ) )
  {
    QString backup = fileName() + '~';
    if ( QFile::exists( backup ) )
      QFile::remove( backup );
    QFile::rename( fileName(), backup );
  }

  // if we have problems creating or otherwise writing to the project file,
  // let's find out up front before we go through all the hand-waving
  // necessary to create all the Dom objects
  if ( !imp_->file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    imp_->file.close();         // even though we got an error, let's make
    // sure it's closed anyway

    setError( tr( "Unable to save to file %1" ).arg( imp_->file.fileName() ) );
    return false;
  }
  QFileInfo myFileInfo( imp_->file );
  if ( !myFileInfo.isWritable() )
  {
    // even though we got an error, let's make
    // sure it's closed anyway
    imp_->file.close();
    setError( tr( "%1 is not writable. Please adjust permissions (if possible) and try again." )
              .arg( imp_->file.fileName() ) );
    return false;
  }



  QDomImplementation DomImplementation;
  DomImplementation.setInvalidDataPolicy( QDomImplementation::DropInvalidChars );

  QDomDocumentType documentType =
    DomImplementation.createDocumentType( "qgis", "http://mrcc.com/qgis.dtd",
                                          "SYSTEM" );
  QScopedPointer<QDomDocument> doc( new QDomDocument( documentType ) );

  QDomElement qgisNode = doc->createElement( "qgis" );
  qgisNode.setAttribute( "projectname", title() );
  qgisNode.setAttribute( "version", QString( "%1" ).arg( QGis::QGIS_VERSION ) );

  doc->appendChild( qgisNode );

  // title
  QDomElement titleNode = doc->createElement( "title" );
  qgisNode.appendChild( titleNode );

  QDomText titleText = doc->createTextNode( title() );  // XXX why have title TWICE?
  titleNode.appendChild( titleText );

  // write layer tree - make sure it is without embedded subgroups
  QgsLayerTreeNode *clonedRoot = mRootGroup->clone();
  QgsLayerTreeUtils::replaceChildrenOfEmbeddedGroups( QgsLayerTree::toGroup( clonedRoot ) );
  QgsLayerTreeUtils::updateEmbeddedGroupsProjectPath( QgsLayerTree::toGroup( clonedRoot ) ); // convert absolute paths to relative paths if required
  clonedRoot->writeXML( qgisNode );
  delete clonedRoot;

  // let map canvas and legend write their information
  emit writeProject( *doc );

  // within top level node save list of layers
  const QMap<QString, QgsMapLayer*> &layers = QgsMapLayerRegistry::instance()->mapLayers();

  // Iterate over layers in zOrder
  // Call writeXML() on each
  QDomElement projectLayersNode = doc->createElement( "projectlayers" );
  projectLayersNode.setAttribute( "layercount", qulonglong( layers.size() ) );

  QMap<QString, QgsMapLayer*>::ConstIterator li = layers.constBegin();
  while ( li != layers.end() )
  {
    QgsMapLayer *ml = li.value();

    if ( ml )
    {
      QString externalProjectFile = layerIsEmbedded( ml->id() );
      QHash< QString, QPair< QString, bool> >::const_iterator emIt = mEmbeddedLayers.find( ml->id() );
      if ( emIt == mEmbeddedLayers.constEnd() )
      {
        // general layer metadata
        QDomElement maplayerElem = doc->createElement( "maplayer" );

        ml->writeLayerXML( maplayerElem, *doc );

        emit writeMapLayer( ml, maplayerElem, *doc );

        projectLayersNode.appendChild( maplayerElem );
      }
      else
      {
        // layer defined in an external project file
        // only save embedded layer if not managed by a legend group
        if ( emIt.value().second )
        {
          QDomElement mapLayerElem = doc->createElement( "maplayer" );
          mapLayerElem.setAttribute( "embedded", 1 );
          mapLayerElem.setAttribute( "project", writePath( emIt.value().first ) );
          mapLayerElem.setAttribute( "id", ml->id() );
          projectLayersNode.appendChild( mapLayerElem );
        }
      }
    }
    li++;
  }

  qgisNode.appendChild( projectLayersNode );

  // now add the optional extra properties

  dump_( imp_->properties_ );

  QgsDebugMsg( QString( "there are %1 property scopes" ).arg( static_cast<int>( imp_->properties_.count() ) ) );

  if ( !imp_->properties_.isEmpty() ) // only worry about properties if we
    // actually have any properties
  {
    imp_->properties_.writeXML( "properties", qgisNode, *doc );
  }

  mVisibilityPresetCollection->writeXML( *doc );

  // now wrap it up and ship it to the project file
  doc->normalize();             // XXX I'm not entirely sure what this does

  QTextStream projectFileStream( &imp_->file );

  doc->save( projectFileStream, 2 );  // save as utf-8
  imp_->file.close();

  // check if the text stream had no error - if it does
  // the user will get a message so they can try to resolve the
  // situation e.g. by saving project to a volume with more space
  if ( projectFileStream.pos() == -1  || imp_->file.error() != QFile::NoError )
  {
    setError( tr( "Unable to save to file %1. Your project "
                  "may be corrupted on disk. Try clearing some space on the volume and "
                  "check file permissions before pressing save again." )
              .arg( imp_->file.fileName() ) );
    return false;
  }

  dirty( false );               // reset to pristine state

  emit projectSaved();

  return true;
} // QgsProject::write



void QgsProject::clearProperties()
{
  clear();

  dirty( true );
} // QgsProject::clearProperties()



bool
QgsProject::writeEntry( QString const &scope, const QString &key, bool value )
{
  dirty( true );

  return addKey_( scope, key, &imp_->properties_, value );
} // QgsProject::writeEntry ( ..., bool value )


bool
QgsProject::writeEntry( QString const &scope, const QString &key,
                        double value )
{
  dirty( true );

  return addKey_( scope, key, &imp_->properties_, value );
} // QgsProject::writeEntry ( ..., double value )


bool
QgsProject::writeEntry( QString const &scope, const QString &key, int value )
{
  dirty( true );

  return addKey_( scope, key, &imp_->properties_, value );
} // QgsProject::writeEntry ( ..., int value )


bool
QgsProject::writeEntry( QString const &scope, const QString &key,
                        const QString &value )
{
  dirty( true );

  return addKey_( scope, key, &imp_->properties_, value );
} // QgsProject::writeEntry ( ..., const QString &value )


bool
QgsProject::writeEntry( QString const &scope, const QString &key,
                        const QStringList &value )
{
  dirty( true );

  return addKey_( scope, key, &imp_->properties_, value );
} // QgsProject::writeEntry ( ..., const QStringList &value )

QStringList
QgsProject::readListEntry( QString const &scope,
                           const QString &key,
                           const QStringList& def,
                           bool *ok ) const
{
  QgsProperty *property = findKey_( scope, key, imp_->properties_ );

  QVariant value;

  if ( property )
  {
    value = property->value();

    bool valid = QVariant::StringList == value.type();
    if ( ok )
      *ok = valid;

    if ( valid )
    {
      return value.toStringList();
    }
  }

  return def;
} // QgsProject::readListEntry


QString
QgsProject::readEntry( QString const &scope,
                       const QString &key,
                       const QString &def,
                       bool *ok ) const
{
  QgsProperty *property = findKey_( scope, key, imp_->properties_ );

  QVariant value;

  if ( property )
  {
    value = property->value();

    bool valid = value.canConvert( QVariant::String );
    if ( ok )
      *ok = valid;

    if ( valid )
      return value.toString();
  }

  return def;
} // QgsProject::readEntry


int
QgsProject::readNumEntry( QString const &scope, const QString &key, int def,
                          bool *ok ) const
{
  QgsProperty *property = findKey_( scope, key, imp_->properties_ );

  QVariant value;

  if ( property )
  {
    value = property->value();
  }

  bool valid = value.canConvert( QVariant::String );

  if ( ok )
  {
    *ok = valid;
  }

  if ( valid )
  {
    return value.toInt();
  }

  return def;
} // QgsProject::readNumEntry


double
QgsProject::readDoubleEntry( QString const &scope, const QString &key,
                             double def,
                             bool *ok ) const
{
  QgsProperty *property = findKey_( scope, key, imp_->properties_ );
  if ( property )
  {
    QVariant value = property->value();

    bool valid = value.canConvert( QVariant::Double );
    if ( ok )
      *ok = valid;

    if ( valid )
      return value.toDouble();
  }

  return def;
} // QgsProject::readDoubleEntry


bool
QgsProject::readBoolEntry( QString const &scope, const QString &key, bool def,
                           bool *ok ) const
{
  QgsProperty *property = findKey_( scope, key, imp_->properties_ );

  if ( property )
  {
    QVariant value = property->value();

    bool valid = value.canConvert( QVariant::Bool );
    if ( ok )
      *ok = valid;

    if ( valid )
      return value.toBool();
  }

  return def;
} // QgsProject::readBoolEntry


bool QgsProject::removeEntry( QString const &scope, const QString &key )
{
  removeKey_( scope, key, imp_->properties_ );

  dirty( true );

  return !findKey_( scope, key, imp_->properties_ );
} // QgsProject::removeEntry



QStringList QgsProject::entryList( QString const &scope, QString const &key ) const
{
  QgsProperty *foundProperty = findKey_( scope, key, imp_->properties_ );

  QStringList entries;

  if ( foundProperty )
  {
    QgsPropertyKey *propertyKey = dynamic_cast<QgsPropertyKey*>( foundProperty );

    if ( propertyKey )
      { propertyKey->entryList( entries ); }
  }

  return entries;
} // QgsProject::entryList


QStringList QgsProject::subkeyList( QString const &scope, QString const &key ) const
{
  QgsProperty *foundProperty = findKey_( scope, key, imp_->properties_ );

  QStringList entries;

  if ( foundProperty )
  {
    QgsPropertyKey *propertyKey = dynamic_cast<QgsPropertyKey*>( foundProperty );

    if ( propertyKey )
      { propertyKey->subkeyList( entries ); }
  }

  return entries;

} // QgsProject::subkeyList



void QgsProject::dumpProperties() const
{
  dump_( imp_->properties_ );
} // QgsProject::dumpProperties


// return the absolute path from a filename read from project file
QString QgsProject::readPath( QString src ) const
{
  if ( readBoolEntry( "Paths", "/Absolute", false ) )
  {
    return src;
  }

  // if this is a VSIFILE, remove the VSI prefix and append to final result
  QString vsiPrefix = qgsVsiPrefix( src );
  if ( ! vsiPrefix.isEmpty() )
  {
    // unfortunately qgsVsiPrefix returns prefix also for files like "/x/y/z.gz"
    // so we need to check if we really have the prefix
    if ( src.startsWith( "/vsi", Qt::CaseInsensitive ) )
      src.remove( 0, vsiPrefix.size() );
    else
      vsiPrefix.clear();
  }

  // relative path should always start with ./ or ../
  if ( !src.startsWith( "./" ) && !src.startsWith( "../" ) )
  {
#if defined(Q_OS_WIN)
    if ( src.startsWith( "\\\\" ) ||
         src.startsWith( "//" ) ||
         ( src[0].isLetter() && src[1] == ':' ) )
    {
      // UNC or absolute path
      return vsiPrefix + src;
    }
#else
    if ( src[0] == '/' )
    {
      // absolute path
      return vsiPrefix + src;
    }
#endif

    // so this one isn't absolute, but also doesn't start // with ./ or ../.
    // That means that it was saved with an earlier version of "relative path support",
    // where the source file had to exist and only the project directory was stripped
    // from the filename.
    QString home = homePath();
    if ( home.isNull() )
      return vsiPrefix + src;

    QFileInfo fi( home + '/' + src );

    if ( !fi.exists() )
    {
      return vsiPrefix + src;
    }
    else
    {
      return vsiPrefix + fi.canonicalFilePath();
    }
  }

  QString srcPath = src;
  QString projPath = fileName();

  if ( projPath.isEmpty() )
  {
    return vsiPrefix + src;
  }

#if defined(Q_OS_WIN)
  srcPath.replace( '\\', '/' );
  projPath.replace( '\\', '/' );

  bool uncPath = projPath.startsWith( "//" );
#endif

  QStringList srcElems = srcPath.split( '/', QString::SkipEmptyParts );
  QStringList projElems = projPath.split( '/', QString::SkipEmptyParts );

#if defined(Q_OS_WIN)
  if ( uncPath )
  {
    projElems.insert( 0, "" );
    projElems.insert( 0, "" );
  }
#endif

  // remove project file element
  projElems.removeLast();

  // append source path elements
  projElems << srcElems;
  projElems.removeAll( "." );

  // resolve ..
  int pos;
  while (( pos = projElems.indexOf( ".." ) ) > 0 )
  {
    // remove preceding element and ..
    projElems.removeAt( pos - 1 );
    projElems.removeAt( pos - 1 );
  }

#if !defined(Q_OS_WIN)
  // make path absolute
  projElems.prepend( "" );
#endif

  return vsiPrefix + projElems.join( "/" );
}

// return the absolute or relative path to write it to the project file
QString QgsProject::writePath( const QString& src, const QString& relativeBasePath ) const
{
  if ( readBoolEntry( "Paths", "/Absolute", false ) || src.isEmpty() )
  {
    return src;
  }

  QFileInfo srcFileInfo( src );
  QFileInfo projFileInfo( fileName() );
  QString srcPath = srcFileInfo.exists() ? srcFileInfo.canonicalFilePath() : src;
  QString projPath = projFileInfo.canonicalFilePath();

  if ( !relativeBasePath.isNull() )
  {
    projPath = relativeBasePath;
  }

  if ( projPath.isEmpty() )
  {
    return src;
  }

  // if this is a VSIFILE, remove the VSI prefix and append to final result
  QString vsiPrefix = qgsVsiPrefix( src );
  if ( ! vsiPrefix.isEmpty() )
  {
    srcPath.remove( 0, vsiPrefix.size() );
  }

#if defined( Q_OS_WIN )
  const Qt::CaseSensitivity cs = Qt::CaseInsensitive;

  srcPath.replace( '\\', '/' );

  if ( srcPath.startsWith( "//" ) )
  {
    // keep UNC prefix
    srcPath = "\\\\" + srcPath.mid( 2 );
  }

  projPath.replace( '\\', '/' );
  if ( projPath.startsWith( "//" ) )
  {
    // keep UNC prefix
    projPath = "\\\\" + projPath.mid( 2 );
  }
#else
  const Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif

  QStringList projElems = projPath.split( '/', QString::SkipEmptyParts );
  QStringList srcElems = srcPath.split( '/', QString::SkipEmptyParts );

  // remove project file element
  projElems.removeLast();

  projElems.removeAll( "." );
  srcElems.removeAll( "." );

  // remove common part
  int n = 0;
  while ( srcElems.size() > 0 &&
          projElems.size() > 0 &&
          srcElems[0].compare( projElems[0], cs ) == 0 )
  {
    srcElems.removeFirst();
    projElems.removeFirst();
    n++;
  }

  if ( n == 0 )
  {
    // no common parts; might not even by a file
    return src;
  }

  if ( projElems.size() > 0 )
  {
    // go up to the common directory
    for ( int i = 0; i < projElems.size(); i++ )
    {
      srcElems.insert( 0, ".." );
    }
  }
  else
  {
    // let it start with . nevertheless,
    // so relative path always start with either ./ or ../
    srcElems.insert( 0, "." );
  }

  return vsiPrefix + srcElems.join( "/" );
}

void QgsProject::setError( const QString& errorMessage )
{
  mErrorMessage = errorMessage;
}

QString QgsProject::error() const
{
  return mErrorMessage;
}

void QgsProject::clearError()
{
  setError( QString() );
}

void QgsProject::setBadLayerHandler( QgsProjectBadLayerHandler *handler )
{
  delete mBadLayerHandler;
  mBadLayerHandler = handler;
}

QString QgsProject::layerIsEmbedded( const QString &id ) const
{
  QHash< QString, QPair< QString, bool > >::const_iterator it = mEmbeddedLayers.find( id );
  if ( it == mEmbeddedLayers.constEnd() )
  {
    return QString();
  }
  return it.value().first;
}

bool QgsProject::createEmbeddedLayer( const QString &layerId, const QString &projectFilePath, QList<QDomNode> &brokenNodes,
                                      QList< QPair< QgsVectorLayer*, QDomElement > > &vectorLayerList, bool saveFlag )
{
  QgsDebugCall;

  static QString prevProjectFilePath;
  static QDomDocument projectDocument;

  if ( projectFilePath != prevProjectFilePath )
  {
    prevProjectFilePath.clear();

    QFile projectFile( projectFilePath );
    if ( !projectFile.open( QIODevice::ReadOnly ) )
    {
      return false;
    }

    if ( !projectDocument.setContent( &projectFile ) )
    {
      return false;
    }

    prevProjectFilePath = projectFilePath;
  }

  // does project store pathes absolute or relative?
  bool useAbsolutePathes = true;

  QDomElement propertiesElem = projectDocument.documentElement().firstChildElement( "properties" );
  if ( !propertiesElem.isNull() )
  {
    QDomElement absElem = propertiesElem.firstChildElement( "Paths" ).firstChildElement( "Absolute" );
    if ( !absElem.isNull() )
    {
      useAbsolutePathes = absElem.text().compare( "true", Qt::CaseInsensitive ) == 0;
    }
  }

  QDomElement projectLayersElem = projectDocument.documentElement().firstChildElement( "projectlayers" );
  if ( projectLayersElem.isNull() )
  {
    return false;
  }

  QDomNodeList mapLayerNodes = projectLayersElem.elementsByTagName( "maplayer" );
  for ( int i = 0; i < mapLayerNodes.size(); ++i )
  {
    // get layer id
    QDomElement mapLayerElem = mapLayerNodes.at( i ).toElement();
    QString id = mapLayerElem.firstChildElement( "id" ).text();
    if ( id == layerId )
    {
      // layer can be embedded only once
      if ( mapLayerElem.attribute( "embedded" ) == "1" )
      {
        return false;
      }

      mEmbeddedLayers.insert( layerId, qMakePair( projectFilePath, saveFlag ) );

      // change datasource path from relative to absolute if necessary
      // see also QgsMapLayer::readLayerXML
      if ( !useAbsolutePathes )
      {
        QString provider( mapLayerElem.firstChildElement( "provider" ).text() );
        QDomElement dsElem( mapLayerElem.firstChildElement( "datasource" ) );
        QString datasource( dsElem.text() );
        if ( provider == "spatialite" )
        {
          QgsDataSourceURI uri( datasource );
          QFileInfo absoluteDs( QFileInfo( projectFilePath ).absolutePath() + '/' + uri.database() );
          if ( absoluteDs.exists() )
          {
            uri.setDatabase( absoluteDs.absoluteFilePath() );
            datasource = uri.uri();
          }
        }
        else if ( provider == "ogr" )
        {
          QStringList theURIParts( datasource.split( '|' ) );
          QFileInfo absoluteDs( QFileInfo( projectFilePath ).absolutePath() + '/' + theURIParts[0] );
          if ( absoluteDs.exists() )
          {
            theURIParts[0] = absoluteDs.absoluteFilePath();
            datasource = theURIParts.join( "|" );
          }
        }
        else if ( provider == "gpx" )
        {
          QStringList theURIParts( datasource.split( '?' ) );
          QFileInfo absoluteDs( QFileInfo( projectFilePath ).absolutePath() + '/' + theURIParts[0] );
          if ( absoluteDs.exists() )
          {
            theURIParts[0] = absoluteDs.absoluteFilePath();
            datasource = theURIParts.join( "?" );
          }
        }
        else if ( provider == "delimitedtext" )
        {
          QUrl urlSource( QUrl::fromEncoded( datasource.toAscii() ) );

          if ( !datasource.startsWith( "file:" ) )
          {
            QUrl file( QUrl::fromLocalFile( datasource.left( datasource.indexOf( '?' ) ) ) );
            urlSource.setScheme( "file" );
            urlSource.setPath( file.path() );
          }

          QFileInfo absoluteDs( QFileInfo( projectFilePath ).absolutePath() + '/' + urlSource.toLocalFile() );
          if ( absoluteDs.exists() )
          {
            QUrl urlDest = QUrl::fromLocalFile( absoluteDs.absoluteFilePath() );
            urlDest.setQueryItems( urlSource.queryItems() );
            datasource = QString::fromAscii( urlDest.toEncoded() );
          }
        }
        else
        {
          QFileInfo absoluteDs( QFileInfo( projectFilePath ).absolutePath() + '/' + datasource );
          if ( absoluteDs.exists() )
          {
            datasource = absoluteDs.absoluteFilePath();
          }
        }

        dsElem.removeChild( dsElem.childNodes().at( 0 ) );
        dsElem.appendChild( projectDocument.createTextNode( datasource ) );
      }

      if ( addLayer( mapLayerElem, brokenNodes, vectorLayerList ) )
      {
        return true;
      }
      else
      {
        mEmbeddedLayers.remove( layerId );
        return false;
      }
    }
  }

  return false;
}


QgsLayerTreeGroup *QgsProject::createEmbeddedGroup( const QString &groupName, const QString &projectFilePath, const QStringList &invisibleLayers )
{
  // open project file, get layer ids in group, add the layers
  QFile projectFile( projectFilePath );
  if ( !projectFile.open( QIODevice::ReadOnly ) )
  {
    return 0;
  }

  QDomDocument projectDocument;
  if ( !projectDocument.setContent( &projectFile ) )
  {
    return 0;
  }

  // store identify disabled layers of the embedded project
  QSet<QString> embeddedIdentifyDisabledLayers;
  QDomElement disabledLayersElem = projectDocument.documentElement().firstChildElement( "properties" ).firstChildElement( "Identify" ).firstChildElement( "disabledLayers" );
  if ( !disabledLayersElem.isNull() )
  {
    QDomNodeList valueList = disabledLayersElem.elementsByTagName( "value" );
    for ( int i = 0; i < valueList.size(); ++i )
    {
      embeddedIdentifyDisabledLayers.insert( valueList.at( i ).toElement().text() );
    }
  }

  QgsLayerTreeGroup *root = new QgsLayerTreeGroup;

  QDomElement layerTreeElem = projectDocument.documentElement().firstChildElement( "layer-tree-group" );
  if ( !layerTreeElem.isNull() )
  {
    root->readChildrenFromXML( layerTreeElem );
  }
  else
  {
    QgsLayerTreeUtils::readOldLegend( root, projectDocument.documentElement().firstChildElement( "legend" ) );
  }

  QgsLayerTreeGroup *group = root->findGroup( groupName );
  if ( !group || group->customProperty( "embedded" ).toBool() )
  {
    // embedded groups cannot be embedded again
    delete root;
    return 0;
  }

  // clone the group sub-tree (it is used already in a tree, we cannot just tear it off)
  QgsLayerTreeGroup *newGroup = QgsLayerTree::toGroup( group->clone() );
  delete root;
  root = 0;

  newGroup->setCustomProperty( "embedded", 1 );
  newGroup->setCustomProperty( "embedded_project", projectFilePath );

  // set "embedded" to all children + load embedded layers
  mLayerTreeRegistryBridge->setEnabled( false );
  initializeEmbeddedSubtree( projectFilePath, newGroup );
  mLayerTreeRegistryBridge->setEnabled( true );

  // consider the layers might be identify disabled in its project
  Q_FOREACH ( const QString& layerId, newGroup->findLayerIds() )
  {
    if ( embeddedIdentifyDisabledLayers.contains( layerId ) )
    {
      QStringList thisProjectIdentifyDisabledLayers = QgsProject::instance()->readListEntry( "Identify", "/disabledLayers" );
      thisProjectIdentifyDisabledLayers.append( layerId );
      QgsProject::instance()->writeEntry( "Identify", "/disabledLayers", thisProjectIdentifyDisabledLayers );
    }

    QgsLayerTreeLayer *layer = newGroup->findLayer( layerId );
    if ( layer )
    {
      layer->setVisible( invisibleLayers.contains( layerId ) ? Qt::Unchecked : Qt::Checked );
    }
  }

  return newGroup;
}

void QgsProject::initializeEmbeddedSubtree( const QString &projectFilePath, QgsLayerTreeGroup *group )
{
  Q_FOREACH ( QgsLayerTreeNode *child, group->children() )
  {
    // all nodes in the subtree will have "embedded" custom property set
    child->setCustomProperty( "embedded", 1 );

    if ( QgsLayerTree::isGroup( child ) )
    {
      initializeEmbeddedSubtree( projectFilePath, QgsLayerTree::toGroup( child ) );
    }
    else if ( QgsLayerTree::isLayer( child ) )
    {
      // load the layer into our project
      QList<QDomNode> brokenNodes;
      QList< QPair< QgsVectorLayer*, QDomElement > > vectorLayerList;
      createEmbeddedLayer( QgsLayerTree::toLayer( child )->layerId(), projectFilePath, brokenNodes, vectorLayerList, false );
    }
  }
}

void QgsProject::setSnapSettingsForLayer( const QString &layerId, bool enabled, QgsSnapper::SnappingType  type, QgsTolerance::UnitType unit, double tolerance, bool avoidIntersection )
{
  QStringList layerIdList, enabledList, snapTypeList, toleranceUnitList, toleranceList, avoidIntersectionList;
  snapSettings( layerIdList, enabledList, snapTypeList, toleranceUnitList, toleranceList, avoidIntersectionList );
  int idx = layerIdList.indexOf( layerId );
  if ( idx != -1 )
  {
    layerIdList.removeAt( idx );
    enabledList.removeAt( idx );
    snapTypeList.removeAt( idx );
    toleranceUnitList.removeAt( idx );
    toleranceList.removeAt( idx );
    avoidIntersectionList.removeOne( layerId );
  }

  layerIdList.append( layerId );

  // enabled
  enabledList.append( enabled ? "enabled" : "disabled" );

  // snap type
  QString typeString;
  if ( type == QgsSnapper::SnapToSegment )
  {
    typeString = "to_segment";
  }
  else if ( type == QgsSnapper::SnapToVertexAndSegment )
  {
    typeString = "to_vertex_and_segment";
  }
  else
  {
    typeString = "to_vertex";
  }
  snapTypeList.append( typeString );

  // units
  toleranceUnitList.append( QString::number( unit ) );

  // tolerance
  toleranceList.append( QString::number( tolerance ) );

  // avoid intersection
  if ( avoidIntersection )
  {
    avoidIntersectionList.append( layerId );
  }

  writeEntry( "Digitizing", "/LayerSnappingList", layerIdList );
  writeEntry( "Digitizing", "/LayerSnappingEnabledList", enabledList );
  writeEntry( "Digitizing", "/LayerSnappingToleranceList", toleranceList );
  writeEntry( "Digitizing", "/LayerSnappingToleranceUnitList", toleranceUnitList );
  writeEntry( "Digitizing", "/LayerSnapToList", snapTypeList );
  writeEntry( "Digitizing", "/AvoidIntersectionsList", avoidIntersectionList );
  emit snapSettingsChanged();
}

bool QgsProject::snapSettingsForLayer( const QString &layerId, bool &enabled, QgsSnapper::SnappingType &type, QgsTolerance::UnitType &units, double &tolerance,
                                       bool &avoidIntersection ) const
{
  QStringList layerIdList, enabledList, snapTypeList, toleranceUnitList, toleranceList, avoidIntersectionList;
  snapSettings( layerIdList, enabledList, snapTypeList, toleranceUnitList, toleranceList, avoidIntersectionList );
  int idx = layerIdList.indexOf( layerId );
  if ( idx == -1 )
  {
    return false;
  }

  // make sure all lists are long enough
  int minListEntries = idx + 1;
  if ( layerIdList.size() < minListEntries || enabledList.size() < minListEntries || snapTypeList.size() < minListEntries ||
       toleranceUnitList.size() < minListEntries || toleranceList.size() < minListEntries )
  {
    return false;
  }

  // enabled
  enabled = enabledList.at( idx ) == "enabled";

  // snap type
  QString snapType = snapTypeList.at( idx );
  if ( snapType == "to_segment" )
  {
    type = QgsSnapper::SnapToSegment;
  }
  else if ( snapType == "to_vertex_and_segment" )
  {
    type = QgsSnapper::SnapToVertexAndSegment;
  }
  else // to vertex
  {
    type = QgsSnapper::SnapToVertex;
  }

  // units
  if ( toleranceUnitList.at( idx ) == "1" )
  {
    units = QgsTolerance::Pixels;
  }
  else if ( toleranceUnitList.at( idx ) == "2" )
  {
    units = QgsTolerance::ProjectUnits;
  }
  else
  {
    units = QgsTolerance::LayerUnits;
  }

  // tolerance
  tolerance = toleranceList.at( idx ).toDouble();

  // avoid intersection
  avoidIntersection = ( avoidIntersectionList.indexOf( layerId ) != -1 );

  return true;
}

void QgsProject::snapSettings( QStringList &layerIdList, QStringList &enabledList, QStringList &snapTypeList, QStringList &toleranceUnitList, QStringList &toleranceList,
                               QStringList &avoidIntersectionList ) const
{
  layerIdList = readListEntry( "Digitizing", "/LayerSnappingList" );
  enabledList = readListEntry( "Digitizing", "/LayerSnappingEnabledList" );
  toleranceList = readListEntry( "Digitizing", "/LayerSnappingToleranceList" );
  toleranceUnitList = readListEntry( "Digitizing", "/LayerSnappingToleranceUnitList" );
  snapTypeList = readListEntry( "Digitizing", "/LayerSnapToList" );
  avoidIntersectionList = readListEntry( "Digitizing", "/AvoidIntersectionsList" );
}

void QgsProject::setTopologicalEditing( bool enabled )
{
  QgsProject::instance()->writeEntry( "Digitizing", "/TopologicalEditing", ( enabled ? 1 : 0 ) );
  emit snapSettingsChanged();
}

bool QgsProject::topologicalEditing() const
{
  return ( QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 ) > 0 );
}

void QgsProjectBadLayerDefaultHandler::handleBadLayers( const QList<QDomNode>& /*layers*/, const QDomDocument& /*projectDom*/ )
{
  // just ignore any bad layers
}

QString QgsProject::homePath() const
{
  QFileInfo pfi( fileName() );
  if ( !pfi.exists() )
    return QString::null;

  return pfi.canonicalPath();
}

QgsRelationManager *QgsProject::relationManager() const
{
  return mRelationManager;
}

QgsLayerTreeGroup *QgsProject::layerTreeRoot() const
{
  return mRootGroup;
}

QgsVisibilityPresetCollection* QgsProject::visibilityPresetCollection()
{
  return mVisibilityPresetCollection.data();
}
