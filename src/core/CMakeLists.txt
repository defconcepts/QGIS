#############################################################
# sources

SET(QGIS_CORE_SRCS
  gps/qgsgpsconnection.cpp
  gps/qgsgpsconnectionregistry.cpp
  gps/qgsnmeaconnection.cpp
  gps/qgsgpsdconnection.cpp
  gps/qgsgpsdetector.cpp
  gps/parse.c
  gps/sentence.c
  gps/info.c
  gps/time.c
  gps/gmath.c
  gps/tok.c
  gps/context.c

  symbology-ng/qgssymbolv2.cpp
  symbology-ng/qgssymbollayerv2.cpp
  symbology-ng/qgssymbollayerv2registry.cpp
  symbology-ng/qgssymbollayerv2utils.cpp
  symbology-ng/qgslegendsymbolitemv2.cpp
  symbology-ng/qgslinesymbollayerv2.cpp
  symbology-ng/qgsmarkersymbollayerv2.cpp
  symbology-ng/qgsfillsymbollayerv2.cpp
  symbology-ng/qgsrendererv2.cpp
  symbology-ng/qgsrendererv2registry.cpp
  symbology-ng/qgssinglesymbolrendererv2.cpp
  symbology-ng/qgscategorizedsymbolrendererv2.cpp
  symbology-ng/qgsgraduatedsymbolrendererv2.cpp
  symbology-ng/qgsrulebasedrendererv2.cpp
  symbology-ng/qgsheatmaprenderer.cpp
  symbology-ng/qgsinvertedpolygonrenderer.cpp
  symbology-ng/qgsvectorcolorrampv2.cpp
  symbology-ng/qgscptcityarchive.cpp
  symbology-ng/qgsstylev2.cpp
  symbology-ng/qgssymbologyv2conversion.cpp
  symbology-ng/qgssvgcache.cpp
  symbology-ng/qgsellipsesymbollayerv2.cpp
  symbology-ng/qgspointdisplacementrenderer.cpp
  symbology-ng/qgsvectorfieldsymbollayer.cpp
  symbology-ng/qgscolorbrewerpalette.cpp

  diagram/qgsdiagram.cpp
  diagram/qgspiediagram.cpp
  diagram/qgstextdiagram.cpp
  diagram/qgshistogramdiagram.cpp

  effects/qgsimageoperation.cpp
  effects/qgspainteffect.cpp
  effects/qgseffectstack.cpp
  effects/qgsblureffect.cpp
  effects/qgsshadoweffect.cpp
  effects/qgsgloweffect.cpp
  effects/qgspainteffectregistry.cpp
  effects/qgstransformeffect.cpp
  effects/qgscoloreffect.cpp

  layertree/qgslayertreegroup.cpp
  layertree/qgslayertreelayer.cpp
  layertree/qgslayertreemodel.cpp
  layertree/qgslayertreemodellegendnode.cpp
  layertree/qgslayertreenode.cpp
  layertree/qgslayertreeregistrybridge.cpp
  layertree/qgslayertreeutils.cpp

  auth/qgsauthcertutils.cpp
  auth/qgsauthconfig.cpp
  auth/qgsauthcrypto.cpp
  auth/qgsauthmanager.cpp
  auth/qgsauthmethodmetadata.cpp
  auth/qgsauthmethodregistry.cpp

  qgis.cpp
  qgsapplication.cpp
  qgsattributeaction.cpp
  qgsbrowsermodel.cpp
  qgscachedfeatureiterator.cpp
  qgscacheindex.cpp
  qgscacheindexfeatureid.cpp
  qgsclipper.cpp
  qgscolorscheme.cpp
  qgscolorschemeregistry.cpp
  qgscontexthelp.cpp
  qgscontexthelp_texts.cpp
  qgscoordinatereferencesystem.cpp
  qgscoordinatetransform.cpp
  qgsconditionalstyle.cpp
  qgscredentials.cpp
  qgsdartmeasurement.cpp
  qgscrscache.cpp
  qgsdatadefined.cpp
  qgsdatasourceuri.cpp
  qgsdataitem.cpp
  qgsdataitemprovider.cpp
  qgsdataitemproviderregistry.cpp
  qgsdatumtransformstore.cpp
  qgsdbfilterproxymodel.cpp
  qgsdiagramrendererv2.cpp
  qgsdistancearea.cpp
  qgseditformconfig.cpp
  qgserror.cpp
  qgsexpression.cpp
  qgsexpressioncontext.cpp
  qgsexpression_texts.cpp
  qgsexpressionfieldbuffer.cpp
  qgsfeature.cpp
  qgsfeatureiterator.cpp
  qgsfeaturerequest.cpp
  qgsfeaturestore.cpp
  qgsfield.cpp
  qgsfontutils.cpp
  qgsgeometrycache.cpp
  qgsgeometrysimplifier.cpp
  qgsgeometryvalidator.cpp
  qgsgml.cpp
  qgsgmlschema.cpp
  qgshistogram.cpp
  qgslayerdefinition.cpp
  qgslabel.cpp
  qgslabelattributes.cpp
  qgslabelingenginev2.cpp
  qgslabelsearchtree.cpp
  qgslegacyhelpers.cpp
  qgslegendrenderer.cpp
  qgslegendsettings.cpp
  qgslogger.cpp
  qgsmaphittest.cpp
  qgsmaplayer.cpp
  qgsmaplayerlegend.cpp
  qgsmaplayerregistry.cpp
  qgsmaplayerstylemanager.cpp
  qgsmaprenderer.cpp
  qgsmaprenderercache.cpp
  qgsmaprenderercustompainterjob.cpp
  qgsmaprendererjob.cpp
  qgsmaprendererparalleljob.cpp
  qgsmaprenderersequentialjob.cpp
  qgsmapsettings.cpp
  qgsmaptopixel.cpp
  qgsmaptopixelgeometrysimplifier.cpp
  qgsmessagelog.cpp
  qgsmessageoutput.cpp
  qgsmimedatautils.cpp
  qgsmultirenderchecker.cpp
  qgsnetworkaccessmanager.cpp
  qgsnetworkcontentfetcher.cpp
  qgsnetworkreplyparser.cpp
  qgsobjectcustomproperties.cpp
  qgsofflineediting.cpp
  qgsogcutils.cpp
  qgsowsconnection.cpp
  qgspaintenginehack.cpp
  qgspallabeling.cpp
  qgspluginlayer.cpp
  qgspluginlayerregistry.cpp
  qgspoint.cpp
  qgspointlocator.cpp
  qgsproject.cpp
  qgsprojectfiletransform.cpp
  qgsprojectproperty.cpp
  qgsprojectversion.cpp
  qgsprovidercountcalcevent.cpp
  qgsproviderextentcalcevent.cpp
  qgsprovidermetadata.cpp
  qgsproviderregistry.cpp
  qgspythonrunner.cpp
  qgsrelation.cpp
  qgsrelationmanager.cpp
  qgsrenderchecker.cpp
  qgsrendercontext.cpp
  qgsrectangle.cpp
  qgsrulebasedlabeling.cpp
  qgsrunprocess.cpp
  qgsscalecalculator.cpp
  qgsscaleexpression.cpp
  qgsscaleutils.cpp
  qgssimplifymethod.cpp
  qgssnapper.cpp
  qgssnappingutils.cpp
  qgsspatialindex.cpp
  qgssqlexpressioncompiler.cpp
  qgsstatisticalsummary.cpp
  qgsstringutils.cpp
  qgstransaction.cpp
  qgstolerance.cpp
  qgsvectordataprovider.cpp
  qgsvectorfilewriter.cpp
  qgsvectorlayer.cpp
  qgsvectorlayercache.cpp
  qgsvectorlayerdiagramprovider.cpp
  qgsvectorlayereditbuffer.cpp
  qgsvectorlayereditpassthrough.cpp
  qgsvectorlayereditutils.cpp
  qgsvectorlayerfeatureiterator.cpp
  qgsvectorlayerimport.cpp
  qgsvectorlayerjoinbuffer.cpp
  qgsvectorlayerlabeling.cpp
  qgsvectorlayerlabelprovider.cpp
  qgsvectorlayerrenderer.cpp
  qgsvectorlayerundocommand.cpp
  qgsvectorsimplifymethod.cpp
  qgsvisibilitypresetcollection.cpp
  qgsxmlutils.cpp
  qgsslconnect.cpp
  qgslocalec.cpp

  composer/qgsaddremoveitemcommand.cpp
  composer/qgsaddremovemultiframecommand.cpp
  composer/qgscomposermultiframecommand.cpp
  composer/qgscomposerarrow.cpp
  composer/qgscomposerframe.cpp
  composer/qgscomposermousehandles.cpp
  composer/qgscomposerobject.cpp
  composer/qgscomposerutils.cpp
  composer/qgscomposeritem.cpp
  composer/qgscomposeritemcommand.cpp
  composer/qgscomposeritemgroup.cpp
  composer/qgscomposerlabel.cpp
  composer/qgscomposerlegenditem.cpp
  composer/qgscomposerpicture.cpp
  composer/qgscomposermap.cpp
  composer/qgscomposermapitem.cpp
  composer/qgscomposermapgrid.cpp
  composer/qgscomposermapoverview.cpp
  composer/qgscomposertable.cpp
  composer/qgscomposertablev2.cpp
  composer/qgscomposertablecolumn.cpp
  composer/qgscomposerattributetable.cpp
  composer/qgscomposerattributetablev2.cpp
  composer/qgscomposerattributetablemodel.cpp
  composer/qgscomposerattributetablemodelv2.cpp
  composer/qgscomposertexttable.cpp
  composer/qgscomposerscalebar.cpp
  composer/qgscomposershape.cpp
  composer/qgscomposereffect.cpp
  composer/qgsatlascomposition.cpp
  composer/qgslegendmodel.cpp
  composer/qgscomposerlegend.cpp
  composer/qgscomposerlegendstyle.cpp
  composer/qgspaperitem.cpp
  composer/qgsscalebarstyle.cpp
  composer/qgsdoubleboxscalebarstyle.cpp
  composer/qgsnumericscalebarstyle.cpp
  composer/qgssingleboxscalebarstyle.cpp
  composer/qgsticksscalebarstyle.cpp
  composer/qgscomposerhtml.cpp
  composer/qgscomposermultiframe.cpp
  composer/qgscomposermodel.cpp
  composer/qgscomposition.cpp

  dxf/qgsdxfexport.cpp
  dxf/qgsdxfpaintdevice.cpp
  dxf/qgsdxfpaintengine.cpp
  dxf/qgsdxfpallabeling.cpp

  pal/costcalculator.cpp
  pal/feature.cpp
  pal/geomfunction.cpp
  pal/labelposition.cpp
  pal/layer.cpp
  pal/pal.cpp
  pal/palstat.cpp
  pal/pointset.cpp
  pal/priorityqueue.cpp
  pal/problem.cpp
  pal/util.cpp
  pal/rtree.hpp

  raster/qgscliptominmaxenhancement.cpp
  raster/qgsraster.cpp
  raster/qgsrasterblock.cpp
  raster/qgscolorrampshader.cpp
  raster/qgscontrastenhancement.cpp
  raster/qgscontrastenhancementfunction.cpp
  raster/qgsrasterdataprovider.cpp
  raster/qgslinearminmaxenhancement.cpp
  raster/qgslinearminmaxenhancementwithclip.cpp
  raster/qgspseudocolorshader.cpp
  raster/qgsrasterprojector.cpp
  raster/qgsrasterchecker.cpp
  raster/qgsrasteridentifyresult.cpp
  raster/qgsrasterinterface.cpp
  raster/qgsrasteriterator.cpp
  raster/qgsrasterlayer.cpp
  raster/qgsrasterlayerrenderer.cpp
  raster/qgsrasternuller.cpp
  raster/qgsrastertransparency.cpp
  raster/qgsrasterpipe.cpp
  raster/qgsrasterrange.cpp
  raster/qgsrastershader.cpp
  raster/qgsrastershaderfunction.cpp

  raster/qgsrasterdrawer.cpp
  raster/qgsrasterfilewriter.cpp
  raster/qgsrasterresamplefilter.cpp
  raster/qgsrasterrendererregistry.cpp
  raster/qgsrasterrenderer.cpp
  raster/qgsbilinearrasterresampler.cpp
  raster/qgscubicrasterresampler.cpp
  raster/qgspalettedrasterrenderer.cpp
  raster/qgsmultibandcolorrenderer.cpp
  raster/qgssinglebandcolordatarenderer.cpp
  raster/qgssinglebandgrayrenderer.cpp
  raster/qgssinglebandpseudocolorrenderer.cpp
  raster/qgsbrightnesscontrastfilter.cpp
  raster/qgshuesaturationfilter.cpp

  geometry/qgsabstractgeometryv2.cpp
  geometry/qgscircularstringv2.cpp
  geometry/qgscompoundcurvev2.cpp
  geometry/qgscurvev2.cpp
  geometry/qgscurvepolygonv2.cpp
  geometry/qgsgeometry.cpp
  geometry/qgsgeometrycollectionv2.cpp
  geometry/qgsgeometryeditutils.cpp
  geometry/qgsgeometryfactory.cpp
  geometry/qgsgeometryutils.cpp
  geometry/qgsgeos.cpp
  geometry/qgspointv2.cpp
  geometry/qgspolygonv2.cpp
  geometry/qgslinestringv2.cpp
  geometry/qgsmulticurvev2.cpp
  geometry/qgsmultilinestringv2.cpp
  geometry/qgsmultipointv2.cpp
  geometry/qgsmultipolygonv2.cpp
  geometry/qgsmultisurfacev2.cpp
  geometry/qgswkbptr.cpp
  geometry/qgswkbtypes.cpp
)

FILE(GLOB JSON_HELP_FILES "${CMAKE_SOURCE_DIR}/resources/function_help/json/*")
STRING(REPLACE "$" "$$" JSON_HELP_FILES "${JSON_HELP_FILES}")
STRING(REPLACE "\(" "\\(" JSON_HELP_FILES "${JSON_HELP_FILES}")
STRING(REPLACE "\)" "\\)" JSON_HELP_FILES "${JSON_HELP_FILES}")
ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/qgsexpression_texts.cpp
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/scripts/process_function_template.py ${CMAKE_CURRENT_BINARY_DIR}/qgsexpression_texts.cpp.temp
        COMMAND ${CMAKE_COMMAND} -DSRC=${CMAKE_CURRENT_BINARY_DIR}/qgsexpression_texts.cpp.temp -DDST=${CMAKE_CURRENT_SOURCE_DIR}/qgsexpression_texts.cpp -P ${CMAKE_SOURCE_DIR}/cmake/CopyIfChanged.cmake
    DEPENDS ${JSON_HELP_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

IF(ENABLE_MODELTEST)
  SET(MODELTEST_SRCS
    ../../tests/qt_modeltest/modeltest.cpp
    ../../tests/qt_modeltest/dynamictreemodel.cpp
  )
  SET(MODELTEST_MOC_HDRS
    ../../tests/qt_modeltest/modeltest.h
    ../../tests/qt_modeltest/dynamictreemodel.h
  )
  QT4_WRAP_CPP(MODELTEST_MOC_SRCS ${MODELTEST_MOC_HDRS})
  SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
    ${MODELTEST_SRCS} ${MODELTEST_MOC_SRCS}
  )
ENDIF(ENABLE_MODELTEST)

IF (NOT QT5_BUILD)
  SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
    qgshttptransaction.cpp
  )
ENDIF (NOT QT5_BUILD)

IF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # spatialindex headers produce warnings
  SET_SOURCE_FILES_PROPERTIES(qgsspatialindex.cpp PROPERTIES COMPILE_FLAGS -Wno-overloaded-virtual)
ENDIF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")

IF (WITH_INTERNAL_QEXTSERIALPORT)
  SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
    gps/qextserialport/qextserialport.cpp
    gps/qextserialport/qextserialenumerator.cpp
  )
  IF(WIN32)
    SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
      gps/qextserialport/win_qextserialport.cpp
    )
    ADD_DEFINITIONS(-D_TTY_WIN_)
  ELSE(WIN32)
    SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
      gps/qextserialport/posix_qextserialport.cpp
    )
    ADD_DEFINITIONS(-D_TTY_POSIX_)
  ENDIF(WIN32)
ELSE (WITH_INTERNAL_QEXTSERIALPORT)
  INCLUDE_DIRECTORIES(${QEXTSERIALPORT_INCLUDE_DIR})
ENDIF (WITH_INTERNAL_QEXTSERIALPORT)

IF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)
  SET(QGIS_CORE_SRCS ${QGIS_CORE_SRCS}
    gps/qgsqtlocationconnection.cpp
  )
ENDIF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)

ADD_FLEX_FILES(QGIS_CORE_SRCS qgsexpressionlexer.ll)

ADD_BISON_FILES(QGIS_CORE_SRCS qgsexpressionparser.yy)

SET(QGIS_CORE_MOC_HDRS

  qgsapplication.h
  qgsbrowsermodel.h
  qgscontexthelp.h
  qgscoordinatetransform.h
  qgsdataitem.h
  qgsdataprovider.h
  qgseditformconfig.h
  qgsgml.h
  qgsgmlschema.h
  qgsmaplayer.h
  qgsmaplayerlegend.h
  qgsmaplayerregistry.h
  qgsmaplayerstylemanager.h
  qgsmaprenderer.h
  qgsmaprenderercache.h
  qgsmaprenderercustompainterjob.h
  qgsmaprendererjob.h
  qgsmaprendererparalleljob.h
  qgsmaprenderersequentialjob.h
  qgsmessageoutput.h
  qgsmessagelog.h
  qgsnetworkreplyparser.h
  qgsnetworkcontentfetcher.h
  qgsofflineediting.h
  qgscredentials.h
  qgspluginlayer.h
  qgspointlocator.h
  qgsproject.h
  qgsrunprocess.h
  qgsrelationmanager.h
  qgssnappingutils.h
  qgsvectorlayer.h
  qgsvectorlayereditpassthrough.h
  qgsvectorlayereditbuffer.h
  qgsnetworkaccessmanager.h
  qgsvectordataprovider.h
  qgsvectorlayercache.h
  qgsvectorlayerjoinbuffer.h
  qgsvisibilitypresetcollection.h
  qgsgeometryvalidator.h
  qgswebview.h

  auth/qgsauthmanager.h
  auth/qgsauthmethod.h

  composer/qgsaddremoveitemcommand.h
  composer/qgsatlascomposition.h
  composer/qgscomposerattributetable.h
  composer/qgscomposerattributetablemodel.h
  composer/qgscomposerattributetablemodelv2.h
  composer/qgscomposerattributetablev2.h
  composer/qgscomposereffect.h
  composer/qgscomposerframe.h
  composer/qgscomposerhtml.h
  composer/qgscomposeritemgroup.h
  composer/qgscomposeritem.h
  composer/qgscomposerlabel.h
  composer/qgscomposerlegend.h
  composer/qgscomposermapgrid.h
  composer/qgscomposermap.h
  composer/qgscomposermapitem.h
  composer/qgscomposermapoverview.h
  composer/qgscomposermodel.h
  composer/qgscomposermousehandles.h
  composer/qgscomposermultiframe.h
  composer/qgscomposerobject.h
  composer/qgscomposerpicture.h
  composer/qgscomposerscalebar.h
  composer/qgscomposershape.h
  composer/qgscomposertablecolumn.h
  composer/qgscomposertable.h
  composer/qgscomposertablev2.h
  composer/qgscomposertexttable.h
  composer/qgscomposition.h
  composer/qgslegendmodel.h

  raster/qgsrasterlayer.h
  raster/qgsrasterdataprovider.h

  gps/qgsgpsconnection.h
  gps/qgsgpsdetector.h
  gps/qgsnmeaconnection.h
  gps/qgsgpsdconnection.h

  symbology-ng/qgscptcityarchive.h
  symbology-ng/qgssvgcache.h
  symbology-ng/qgsstylev2.h

  layertree/qgslayertreegroup.h
  layertree/qgslayertreelayer.h
  layertree/qgslayertreemodel.h
  layertree/qgslayertreemodellegendnode.h
  layertree/qgslayertreenode.h
  layertree/qgslayertreeregistrybridge.h
)

IF (NOT WITH_QTWEBKIT)
  SET(QGIS_CORE_MOC_HDRS ${QGIS_CORE_MOC_HDRS}
    qgswebpage.h
    qgswebframe.h
  )
ENDIF(NOT WITH_QTWEBKIT)

IF (NOT QT5_BUILD)
  SET(QGIS_CORE_MOC_HDRS ${QGIS_CORE_MOC_HDRS}
    qgshttptransaction.h
  )
ENDIF (NOT QT5_BUILD)

IF (WITH_INTERNAL_QEXTSERIALPORT)
  SET(QGIS_CORE_MOC_HDRS ${QGIS_CORE_MOC_HDRS}
    gps/qextserialport/qextserialport.h
    gps/qextserialport/qextserialenumerator.h
  )
ENDIF (WITH_INTERNAL_QEXTSERIALPORT)

IF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)
  SET(QGIS_CORE_MOC_HDRS ${QGIS_CORE_MOC_HDRS}
    gps/qgsqtlocationconnection.h
  )
ENDIF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)

QT4_WRAP_CPP(QGIS_CORE_MOC_SRCS ${QGIS_CORE_MOC_HDRS})

IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_CORE_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4512 /wd4996" )
ELSE(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_CORE_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
  IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" AND (CMAKE_BUILD_TYPE MATCHES Release OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo))
    SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/qgsexpression_texts.cpp PROPERTIES COMPILE_FLAGS "-O1")
  ENDIF ()
ENDIF(MSVC)

# install headers
# install qgsconfig.h and plugin.h here so they can get into
# the OS X framework target

SET(QGIS_CORE_HDRS
  ${CMAKE_BINARY_DIR}/qgsconfig.h
  ../plugins/qgisplugin.h

  qgis.h
  qgsattributeaction.h
  qgscachedfeatureiterator.h
  qgscacheindex.h
  qgscacheindexfeatureid.h
  qgsclipper.h
  qgscolorscheme.h
  qgscolorschemeregistry.h
  qgsconnectionpool.h
  qgscontexthelp.h
  qgsconditionalstyle.h
  qgscoordinatereferencesystem.h
  qgscrscache.h
  qgscsexception.h
  qgsdartmeasurement.h
  qgsdatadefined.h
  qgsdatadefined_p.h
  qgsdataitem.h
  qgsdataitemprovider.h
  qgsdataitemproviderregistry.h
  qgsdatasourceuri.h
  qgsdatumtransformstore.h
  qgsdbfilterproxymodel.h
  qgsdiagramrendererv2.h
  qgsdistancearea.h
  qgseditorwidgetconfig.h
  qgsfeaturefilterprovider.h
  qgserror.h
  qgsexception.h
  qgsexpression.h
  qgsexpressioncontext.h
  qgsexpressionfieldbuffer.h
  qgsfeature.h
  qgsfeature_p.h
  qgsfeatureiterator.h
  qgsfeaturerequest.h
  qgsfeaturestore.h
  qgsfield.h
  qgsfield_p.h
  qgsfontutils.h
  qgsgeometrycache.h
  qgshistogram.h
  qgslayerdefinition.h
  qgslabel.h
  qgslabelattributes.h
  qgslabelingenginev2.h
  qgslabelsearchtree.h
  qgslegacyhelpers.h
  qgslegendrenderer.h
  qgslegendsettings.h
  qgslogger.h
  qgsmaphittest.h
  qgsmaplayerrenderer.h
  qgsmaplayerstylemanager.h
  qgsmapsettings.h
  qgsmaptopixel.h
  qgsmaptopixelgeometrysimplifier.h
  qgsmapunitscale.h
  qgsmimedatautils.h
  qgsmultirenderchecker.h
  qgsobjectcustomproperties.h
  qgsogcutils.h
  qgsowsconnection.h
  qgspaintenginehack.h
  qgspalgeometry.h
  qgspallabeling.h
  qgspluginlayerregistry.h
  qgspoint.h
  qgspointlocator.h
  qgsproject.h
  qgsprojectfiletransform.h
  qgsprojectproperty.h
  qgsprojectversion.h
  qgsprovidercountcalcevent.h
  qgsproviderextentcalcevent.h
  qgsprovidermetadata.h
  qgsproviderregistry.h
  qgspythonrunner.h
  qgsrectangle.h
  qgsrelation.h
  qgsrenderchecker.h
  qgsrendercontext.h
  qgsscalecalculator.h
  qgsscaleexpression.h
  qgsscaleutils.h
  qgssimplifymethod.h
  qgssnapper.h
  qgssnappingutils.h
  qgsspatialindex.h
  qgssqlexpressioncompiler.h
  qgsstatisticalsummary.h
  qgsstringutils.h
  qgstolerance.h
  qgstransaction.h
  qgsvectordataprovider.h
  qgsvectorlayercache.h
  qgsvectorfilewriter.h
  qgsvectorlayerdiagramprovider.h
  qgsvectorlayereditutils.h
  qgsvectorlayerfeatureiterator.h
  qgsvectorlayerimport.h
  qgsvectorlayerlabelprovider.h
  qgsvectorlayerrenderer.h
  qgsvectorlayerundocommand.h
  qgsvectorsimplifymethod.h
  qgsvisibilitypresetcollection.h
  qgsxmlutils.h
  qgswebpage.h
  qgswebframe.h
  qgswebview.h
  qgslocalec.h

  auth/qgsauthcertutils.h
  auth/qgsauthconfig.h
  auth/qgsauthcrypto.h
  auth/qgsauthmanager.h
  auth/qgsauthmethod.h
  auth/qgsauthmethodmetadata.h
  auth/qgsauthmethodregistry.h

  diagram/qgsdiagram.h
  diagram/qgspiediagram.h
  diagram/qgstextdiagram.h
  diagram/qgshistogramdiagram.h

  effects/qgsimageoperation.h
  effects/qgspainteffect.h
  effects/qgseffectstack.h
  effects/qgsblureffect.h
  effects/qgsshadoweffect.h
  effects/qgsgloweffect.h
  effects/qgspainteffectregistry.h
  effects/qgstransformeffect.h
  effects/qgscoloreffect.h

  composer/qgsaddremovemultiframecommand.h
  composer/qgscomposerarrow.h
  composer/qgscomposerframe.h
  composer/qgscomposeritemcommand.h
  composer/qgscomposerlegenditem.h
  composer/qgscomposerlegendstyle.h
  composer/qgscomposermultiframecommand.h
  composer/qgscomposertable.h
  composer/qgscomposertexttable.h
  composer/qgsdoubleboxscalebarstyle.h
  composer/qgsnumericscalebarstyle.h
  composer/qgspaperitem.h
  composer/qgsscalebarstyle.h
  composer/qgssingleboxscalebarstyle.h
  composer/qgsticksscalebarstyle.h

  raster/qgsbilinearrasterresampler.h
  raster/qgsbrightnesscontrastfilter.h
  raster/qgscliptominmaxenhancement.h
  raster/qgscolorrampshader.h
  raster/qgscontrastenhancement.h
  raster/qgscontrastenhancementfunction.h
  raster/qgscubicrasterresampler.h
  raster/qgshuesaturationfilter.h
  raster/qgslinearminmaxenhancement.h
  raster/qgslinearminmaxenhancementwithclip.h
  raster/qgsmultibandcolorrenderer.h
  raster/qgspalettedrasterrenderer.h
  raster/qgspseudocolorshader.h
  raster/qgsraster.h
  raster/qgsrasterbandstats.h
  raster/qgsrasterblock.h
  raster/qgsrasterchecker.h
  raster/qgsrasterdrawer.h
  raster/qgsrasterfilewriter.h
  raster/qgsrasterhistogram.h
  raster/qgsrasteridentifyresult.h
  raster/qgsrasterinterface.h
  raster/qgsrasteriterator.h
  raster/qgsrasternuller.h
  raster/qgsrasterpipe.h
  raster/qgsrasterprojector.h
  raster/qgsrasterpyramid.h
  raster/qgsrasterrange.h
  raster/qgsrasterrenderer.h
  raster/qgsrasterresamplefilter.h
  raster/qgsrasterresampler.h
  raster/qgsrastershader.h
  raster/qgsrastershaderfunction.h
  raster/qgsrastertransparency.h
  raster/qgsrasterviewport.h
  raster/qgssinglebandcolordatarenderer.h
  raster/qgssinglebandgrayrenderer.h
  raster/qgssinglebandpseudocolorrenderer.h

  symbology-ng/qgscategorizedsymbolrendererv2.h
  symbology-ng/qgscolorbrewerpalette.h
  symbology-ng/qgsellipsesymbollayerv2.h
  symbology-ng/qgsfillsymbollayerv2.h
  symbology-ng/qgsgraduatedsymbolrendererv2.h
  symbology-ng/qgslegendsymbolitemv2.h
  symbology-ng/qgslinesymbollayerv2.h
  symbology-ng/qgsmarkersymbollayerv2.h
  symbology-ng/qgspointdisplacementrenderer.h
  symbology-ng/qgsrendererv2.h
  symbology-ng/qgsrendererv2registry.h
  symbology-ng/qgsrulebasedrendererv2.h
  symbology-ng/qgssinglesymbolrendererv2.h
  symbology-ng/qgsheatmaprenderer.h
  symbology-ng/qgsinvertedpolygonrenderer.h
  symbology-ng/qgssymbollayerv2.h
  symbology-ng/qgssymbollayerv2registry.h
  symbology-ng/qgssymbollayerv2utils.h
  symbology-ng/qgssymbologyv2conversion.h
  symbology-ng/qgssymbolv2.h
  symbology-ng/qgsvectorcolorrampv2.h
  symbology-ng/qgsvectorfieldsymbollayer.h

  layertree/qgslayertree.h
  layertree/qgslayertreeutils.h

  geometry/qgsgeometry.h
  geometry/qgsabstractgeometryv2.h
  geometry/qgswkbtypes.h
  geometry/qgspointv2.h
)

IF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)
  SET(QGIS_CORE_HDRS ${QGIS_CORE_HDRS}
    gps/qgsqtlocationconnection.h
    gps/qgsgpsconnectionregistry.h
  )
ENDIF (QT_MOBILITY_LOCATION_FOUND OR Qt5Positioning_FOUND)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  auth
  composer
  dxf
  effects
  geometry
  layertree
  pal
  raster
  renderer
  symbology-ng
  gps/qextserialport
)
INCLUDE_DIRECTORIES(SYSTEM
  ${SPATIALINDEX_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${EXPAT_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${SPATIALITE_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
)

IF (NOT MSVC)
  ADD_DEFINITIONS("-UCORE_EXPORT")
  IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
  ENDIF()
ENDIF (NOT MSVC)
ADD_DEFINITIONS("\"-DCORE_EXPORT=${DLLEXPORT}\"")


#for PAL classes
IF (WIN32)
    ADD_DEFINITIONS("-D_HAVE_WINDOWS_H_")
ELSE (WIN32)
    ADD_DEFINITIONS("-D_HAVE_PTHREAD_")
ENDIF (WIN32)

# Test data dir for QgsRenderChecker
ADD_DEFINITIONS(-DTEST_DATA_DIR="\\"${TEST_DATA_DIR}\\"")

#############################################################
# qgis_core library

ADD_LIBRARY(qgis_core SHARED ${QGIS_CORE_SRCS} ${QGIS_CORE_MOC_SRCS} ${QGIS_CORE_HDRS} ${QGIS_CORE_MOC_HDRS})

IF(ENABLE_MODELTEST)
  TARGET_LINK_LIBRARIES(qgis_core ${QT_QTTEST_LIBRARY})
ENDIF(ENABLE_MODELTEST)

IF(NOT APPLE)
  INSTALL(FILES ${QGIS_CORE_HDRS} ${QGIS_CORE_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ELSE(NOT APPLE)
  SET_TARGET_PROPERTIES(qgis_core PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis2_core
    BUILD_WITH_INSTALL_RPATH TRUE
    PUBLIC_HEADER "${QGIS_CORE_HDRS};${QGIS_CORE_MOC_HDRS}"
    LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}"
  )
ENDIF(NOT APPLE)

#generate unversioned libs for android
IF (NOT ANDROID)
  SET_TARGET_PROPERTIES(qgis_core PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT ANDROID)

# make sure to create qgsversion.h before compiling
ADD_DEPENDENCIES(qgis_core version)

# because of htonl
IF (WIN32)
  FIND_LIBRARY(SETUPAPI_LIBRARY NAMES setupapi PATHS $ENV{LIB})
  TARGET_LINK_LIBRARIES(qgis_core wsock32 ${SETUPAPI_LIBRARY})
ENDIF (WIN32)

IF(APPLE)
  TARGET_LINK_LIBRARIES(qgis_core "-framework CoreFoundation -framework IOKit")
ENDIF(APPLE)

IF (NOT WITH_INTERNAL_QEXTSERIALPORT)
  TARGET_LINK_LIBRARIES(qgis_core ${QEXTSERIALPORT_LIBRARY})
ENDIF (NOT WITH_INTERNAL_QEXTSERIALPORT)

IF (QT_MOBILITY_LOCATION_FOUND)
  TARGET_LINK_LIBRARIES(qgis_core ${QT_MOBILITY_LOCATION_LIBRARY})
ENDIF (QT_MOBILITY_LOCATION_FOUND)

TARGET_LINK_LIBRARIES(qgis_core
  ${QT_QTMAIN_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTSVG_LIBRARY}
  ${QT_QTWEBKIT_LIBRARY}
  ${QT_QTSQL_LIBRARY}
  ${QCA_LIBRARY}

  ${PROJ_LIBRARY}
  ${GEOS_LIBRARY}
  ${GDAL_LIBRARY}
  ${SPATIALINDEX_LIBRARY}
  ${EXPAT_LIBRARY}
  ${SQLITE3_LIBRARY}
  ${SPATIALITE_LIBRARY}
)

IF (Qt5Positioning_FOUND)
  TARGET_LINK_LIBRARIES(qgis_core
    Qt5::Positioning
  )
ENDIF (Qt5Positioning_FOUND)

# install

INSTALL(TARGETS qgis_core
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

# Mac dev frameworks

IF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
  INSTALL(TARGETS qgis_core FRAMEWORK DESTINATION ${QGIS_MACAPP_DEV_PREFIX})
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -id \"${QGIS_MACAPP_DEV_PREFIX}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_core.framework/qgis_core\")")
ENDIF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
