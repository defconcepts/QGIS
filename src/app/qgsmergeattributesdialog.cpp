/***************************************************************************
                         qgsmergeattributesdialog.cpp
                         ----------------------------
    begin                : May 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsmergeattributesdialog.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsfield.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsattributeeditor.h"
#include "qgsstatisticalsummary.h"

#include <limits>
#include <QComboBox>
#include <QSettings>

QList< QgsStatisticalSummary::Statistic > QgsMergeAttributesDialog::mDisplayStats =
  QList< QgsStatisticalSummary::Statistic > () << QgsStatisticalSummary::Count
  << QgsStatisticalSummary::Sum
  << QgsStatisticalSummary::Mean
  << QgsStatisticalSummary::Median
  << QgsStatisticalSummary::StDev
  << QgsStatisticalSummary::StDevSample
  << QgsStatisticalSummary::Min
  << QgsStatisticalSummary::Max
  << QgsStatisticalSummary::Range
  << QgsStatisticalSummary::Minority
  << QgsStatisticalSummary::Majority
  << QgsStatisticalSummary::Variety
  << QgsStatisticalSummary::FirstQuartile
  << QgsStatisticalSummary::ThirdQuartile
  << QgsStatisticalSummary::InterQuartileRange;

QgsMergeAttributesDialog::QgsMergeAttributesDialog( const QgsFeatureList &features, QgsVectorLayer *vl, QgsMapCanvas *canvas, QWidget *parent, Qt::WindowFlags f )
    : QDialog( parent, f )
    , mFeatureList( features )
    , mVectorLayer( vl )
    , mMapCanvas( canvas )
    , mSelectionRubberBand( 0 )
{
  setupUi( this );
  createTableWidgetContents();

  QHeaderView* verticalHeader = mTableWidget->verticalHeader();
  if ( verticalHeader )
  {
    QObject::connect( mTableWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( selectedRowChanged() ) );
  }
  mTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  mTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  mFromSelectedPushButton->setIcon( QgsApplication::getThemeIcon( "mActionFromSelectedFeature.png" ) );
  mRemoveFeatureFromSelectionButton->setIcon( QgsApplication::getThemeIcon( "mActionRemoveSelectedFeature.png" ) );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/MergeAttributes/geometry" ).toByteArray() );

  connect( mSkipAllButton, SIGNAL( clicked() ), this, SLOT( setAllToSkip() ) );
}

QgsMergeAttributesDialog::QgsMergeAttributesDialog()
    : QDialog()
    , mVectorLayer( NULL )
    , mMapCanvas( NULL )
    , mSelectionRubberBand( NULL )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/MergeAttributes/geometry" ).toByteArray() );
}

QgsMergeAttributesDialog::~QgsMergeAttributesDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/MergeAttributes/geometry", saveGeometry() );

  delete mSelectionRubberBand;
}

void QgsMergeAttributesDialog::createTableWidgetContents()
{
  //get information about attributes from vector layer
  if ( !mVectorLayer )
  {
    return;
  }

  //combo box row, attributes titles, feature values and current merge results
  mTableWidget->setRowCount( mFeatureList.size() + 2 );

  //create combo boxes and insert attribute names
  const QgsFields& fields = mVectorLayer->fields();
  QSet<int> pkAttrList = mVectorLayer->pkAttributeList().toSet();

  int col = 0;
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    if ( mVectorLayer->editFormConfig()->widgetType( idx ) == "Hidden" ||
         mVectorLayer->editFormConfig()->widgetType( idx ) == "Immutable" )
      continue;

    mTableWidget->setColumnCount( col + 1 );

    QComboBox *cb = createMergeComboBox( fields[idx].type() );
    if ( pkAttrList.contains( idx ) )
    {
      cb->setCurrentIndex( cb->findData( "skip" ) );
    }
    mTableWidget->setCellWidget( 0, col, cb );

    QTableWidgetItem *item = new QTableWidgetItem( fields[idx].name() );
    item->setData( Qt::UserRole, idx );
    mTableWidget->setHorizontalHeaderItem( col++, item );
  }

  //insert the attribute values
  QStringList verticalHeaderLabels; //the id column is in the
  verticalHeaderLabels << tr( "Id" );

  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    verticalHeaderLabels << FID_TO_STRING( mFeatureList[i].id() );

    QgsAttributes attrs = mFeatureList[i].attributes();

    for ( int j = 0; j < mTableWidget->columnCount(); j++ )
    {
      int idx = mTableWidget->horizontalHeaderItem( j )->data( Qt::UserRole ).toInt();

      QTableWidgetItem* attributeValItem = new QTableWidgetItem( attrs.at( idx ).toString() );
      attributeValItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
      mTableWidget->setItem( i + 1, j, attributeValItem );
      mTableWidget->setCellWidget( i + 1, j, QgsAttributeEditor::createAttributeEditor( mTableWidget, NULL, mVectorLayer, idx, attrs.at( idx ) ) );
    }
  }

  //merge
  verticalHeaderLabels << tr( "Merge" );
  mTableWidget->setVerticalHeaderLabels( verticalHeaderLabels );

  //insert currently merged values
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    refreshMergedValue( i );
  }
}

QComboBox *QgsMergeAttributesDialog::createMergeComboBox( QVariant::Type columnType ) const
{
  QComboBox *newComboBox = new QComboBox();
  //add items for feature
  QgsFeatureList::const_iterator f_it = mFeatureList.constBegin();
  for ( ; f_it != mFeatureList.constEnd(); ++f_it )
  {
    newComboBox->addItem( tr( "Feature %1" ).arg( f_it->id() ), QString( "f%1" ).arg( f_it->id() ) );
  }

  if ( columnType == QVariant::Double || columnType == QVariant::Int )
  {
    Q_FOREACH ( QgsStatisticalSummary::Statistic stat, mDisplayStats )
    {
      newComboBox->addItem( QgsStatisticalSummary::displayName( stat ) , stat );
    }
  }
  else if ( columnType == QVariant::String )
  {
    newComboBox->addItem( tr( "Concatenation" ), "concat" );
  }

  newComboBox->addItem( tr( "Skip attribute" ), "skip" );

  QObject::connect( newComboBox, SIGNAL( currentIndexChanged( const QString& ) ),
                    this, SLOT( comboValueChanged( const QString& ) ) );
  return newComboBox;
}

int QgsMergeAttributesDialog::findComboColumn( QComboBox* c ) const
{
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    if ( mTableWidget->cellWidget( 0, i ) == c )
    {
      return i;
    }
  }
  return -1;
}

void QgsMergeAttributesDialog::comboValueChanged( const QString &text )
{
  Q_UNUSED( text );
  QComboBox *senderComboBox = qobject_cast<QComboBox *>( sender() );
  if ( !senderComboBox )
  {
    return;
  }
  int column = findComboColumn( senderComboBox );
  if ( column < 0 )
    return;

  refreshMergedValue( column );
}

void QgsMergeAttributesDialog::selectedRowChanged()
{
  //find out selected row
  QList<QTableWidgetItem *> selectionList = mTableWidget->selectedItems();
  if ( selectionList.size() < 1 )
  {
    delete mSelectionRubberBand;
    mSelectionRubberBand = 0;
    return;
  }

  int row = selectionList[0]->row();

  if ( !mTableWidget || !mMapCanvas || !mVectorLayer || row < 1 || row >= mTableWidget->rowCount() )
  {
    return;
  }

  //read the feature id
  QTableWidgetItem* idItem = mTableWidget->verticalHeaderItem( row );
  if ( !idItem )
  {
    return;
  }

  bool conversionSuccess = false;
  int featureIdToSelect = idItem->text().toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    //the merge result row was selected
    delete mSelectionRubberBand;
    mSelectionRubberBand = 0;
    return;
  }
  createRubberBandForFeature( featureIdToSelect );
}

void QgsMergeAttributesDialog::refreshMergedValue( int col )
{
  QComboBox* comboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, col ) );
  if ( !comboBox )
  {
    return;
  }

  //evaluate behaviour (feature value or min / max / mean )
  QString mergeBehaviourString = comboBox->itemData( comboBox->currentIndex() ).toString();
  QVariant mergeResult; // result to show in the merge result field
  if ( mergeBehaviourString == "concat" )
  {
    mergeResult = concatenationAttribute( col );
  }
  else if ( mergeBehaviourString == "skip" )
  {
    mergeResult = tr( "Skipped" );
  }
  else if ( mergeBehaviourString.startsWith( "f" ) )
  {
    //an existing feature value - TODO should be QgsFeatureId, not int
    int featureId = mergeBehaviourString.mid( 1 ).toInt();
    mergeResult = featureAttribute( featureId, col );
  }
  else
  {
    //numerical statistic
    QgsStatisticalSummary::Statistic stat = ( QgsStatisticalSummary::Statistic )( comboBox->itemData( comboBox->currentIndex() ).toInt() );
    mergeResult = calcStatistic( col, stat );
  }

  //insert string into table widget
  QTableWidgetItem* newTotalItem = new QTableWidgetItem();
  newTotalItem->setData( Qt::DisplayRole, mergeResult );
  newTotalItem->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mTableWidget->setItem( mTableWidget->rowCount() - 1, col, newTotalItem );
}

QVariant QgsMergeAttributesDialog::featureAttribute( int featureId, int col )
{
  int idx = mTableWidget->horizontalHeaderItem( col )->data( Qt::UserRole ).toInt();

  int i;
  for ( i = 0; i < mFeatureList.size() && mFeatureList[i].id() != featureId; i++ )
    ;

  QVariant value;
  if ( i < mFeatureList.size() &&
       QgsAttributeEditor::retrieveValue( mTableWidget->cellWidget( i + 1, col ), mVectorLayer, idx, value ) )
  {
    return value;
  }
  else
  {
    return QVariant( mVectorLayer->fields().at( col ).type() );
  }
}


QVariant QgsMergeAttributesDialog::calcStatistic( int col, QgsStatisticalSummary::Statistic stat )
{
  QgsStatisticalSummary summary( stat );

  bool conversion = false;
  QList<double> values;
  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    double currentValue = mTableWidget->item( i + 1, col )->text().toDouble( &conversion );
    if ( conversion )
    {
      values << currentValue;
    }
  }

  if ( values.isEmpty() )
  {
    return QVariant( mVectorLayer->fields()[col].type() );
  }

  summary.calculate( values );

  return QVariant::fromValue( summary.statistic( stat ) );
}

QVariant QgsMergeAttributesDialog::concatenationAttribute( int col )
{
  QStringList concatString;
  concatString.reserve( mFeatureList.size() );
  for ( int i = 0; i < mFeatureList.size(); ++i )
  {
    concatString << mTableWidget->item( i + 1, col )->text();
  }
  return concatString.join( "," ); //todo: make separator user configurable
}

void QgsMergeAttributesDialog::on_mFromSelectedPushButton_clicked()
{
  //find the selected feature
  if ( !mVectorLayer )
  {
    return;
  }

  //find out feature id of selected row
  QList<QTableWidgetItem *> selectionList = mTableWidget->selectedItems();
  if ( selectionList.size() < 1 )
  {
    return;
  }

  //assume all selected items to be in the same row
  QTableWidgetItem* selectedItem = selectionList[0];
  int selectedRow = selectedItem->row();
  QTableWidgetItem* selectedHeaderItem = mTableWidget->verticalHeaderItem( selectedRow );
  if ( !selectedHeaderItem )
  {
    return;
  }

  bool conversionSuccess;
  int featureId = selectedHeaderItem->text().toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    return;
  }

  QSet<int> pkAttributes = mVectorLayer->pkAttributeList().toSet();
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    if ( pkAttributes.contains( i ) )
    {
      continue;
    }
    QComboBox* currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( currentComboBox )
    {
      currentComboBox->setCurrentIndex( currentComboBox->findData( QString::number( featureId ) ) );
    }
  }
}

void QgsMergeAttributesDialog::on_mRemoveFeatureFromSelectionButton_clicked()
{
  if ( !mVectorLayer )
  {
    return;
  }

  //find out feature id of selected row
  QList<QTableWidgetItem *> selectionList = mTableWidget->selectedItems();
  if ( selectionList.size() < 1 )
  {
    return;
  }

  //assume all selected items to be in the same row
  QTableWidgetItem* selectedItem = selectionList[0];
  int selectedRow = selectedItem->row();
  QTableWidgetItem* selectedHeaderItem = mTableWidget->verticalHeaderItem( selectedRow );
  if ( !selectedHeaderItem )
  {
    return;
  }

  bool conversionSuccess;
  int featureId = selectedHeaderItem->text().toInt( &conversionSuccess );
  if ( !conversionSuccess )
  {
    selectedRowChanged();
    return;
  }

  mTableWidget->removeRow( selectedRow );
  selectedRowChanged();

  //remove feature from the vector layer selection
  QgsFeatureIds selectedIds = mVectorLayer->selectedFeaturesIds();
  selectedIds.remove( featureId );
  mVectorLayer->setSelectedFeatures( selectedIds );
  mMapCanvas->repaint();

  //remove feature option from the combo box (without altering the current merge values)
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    QComboBox* currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( !currentComboBox )
      continue;

    currentComboBox->blockSignals( true );
    currentComboBox->removeItem( currentComboBox->findData( QString::number( featureId ) ) );
    currentComboBox->blockSignals( false );
  }

  //finally remove the feature from mFeatureList
  for ( QgsFeatureList::iterator f_it = mFeatureList.begin();
        f_it != mFeatureList.end();
        ++f_it )
  {
    if ( f_it->id() == featureId )
    {
      mFeatureList.erase( f_it );
      break;
    }
  }
}

void QgsMergeAttributesDialog::createRubberBandForFeature( int featureId )
{
  //create rubber band to highlight the feature
  delete mSelectionRubberBand;
  mSelectionRubberBand = new QgsRubberBand( mMapCanvas, mVectorLayer->geometryType() == QGis::Polygon );
  mSelectionRubberBand->setColor( QColor( 255, 0, 0, 65 ) );
  QgsFeature featureToSelect;
  mVectorLayer->getFeatures( QgsFeatureRequest().setFilterFid( featureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( featureToSelect );
  mSelectionRubberBand->setToGeometry( featureToSelect.constGeometry(), mVectorLayer );
}

QgsAttributes QgsMergeAttributesDialog::mergedAttributes() const
{
  if ( mFeatureList.size() < 1 )
  {
    return QgsAttributes();
  }

  QgsAttributes results( mTableWidget->columnCount() );
  for ( int i = 0; i < mTableWidget->columnCount(); i++ )
  {
    int idx = mTableWidget->horizontalHeaderItem( i )->data( Qt::UserRole ).toInt();

    QComboBox *comboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( !comboBox )
      continue;

    QTableWidgetItem *currentItem = mTableWidget->item( mFeatureList.size() + 1, i );
    if ( !currentItem )
      continue;

    if ( idx >= results.count() )
      results.resize( idx + 1 ); // make sure the results vector is long enough (maybe not necessary)

    if ( comboBox->itemData( comboBox->currentIndex() ).toString() != "skip" )
    {
      results[idx] = currentItem->data( Qt::DisplayRole );
    }
    else if ( mVectorLayer->dataProvider() )
    {
      results[idx] = mVectorLayer->dataProvider()->defaultValue( idx );
    }
  }

  return results;
}

QSet<int> QgsMergeAttributesDialog::skippedAttributeIndexes() const
{
  QSet<int> skipped;
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    QComboBox *comboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( !comboBox )
    {
      //something went wrong, better skip this attribute
      skipped << i;
      continue;
    }

    if ( comboBox->itemData( comboBox->currentIndex() ).toString() == "skip" )
    {
      skipped << i;
    }
  }

  return skipped;
}

void QgsMergeAttributesDialog::setAllToSkip()
{
  for ( int i = 0; i < mTableWidget->columnCount(); ++i )
  {
    QComboBox* currentComboBox = qobject_cast<QComboBox *>( mTableWidget->cellWidget( 0, i ) );
    if ( currentComboBox )
    {
      currentComboBox->setCurrentIndex( currentComboBox->findData( "skip" ) );
    }
  }
}
