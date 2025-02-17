"""Helper utilities for QGIS python unit tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Tim Sutton (tim@linfiniti.com)'
__date__ = '20/01/2011'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os
import sys
import glob
import platform
import tempfile

from PyQt.QtCore import QSize, QDir
from PyQt.QtWidgets import QWidget

from qgis.core import (
    QgsApplication,
    QgsCoordinateReferenceSystem,
    QgsVectorFileWriter,
    QgsMapLayerRegistry,
    QgsMapSettings,
    QgsMapRendererParallelJob,
    QgsMapRendererSequentialJob,
    QgsFontUtils
)
from qgis.gui import QgsMapCanvas
from qgis_interface import QgisInterface
import hashlib
import re
try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

import webbrowser
import subprocess

# Support python < 2.7 via unittest2 needed for expected failure decorator.
# Note that you should ignore unused import warnings here as these are imported
# from this module by other tests.
if sys.version_info[0:2] < (2, 7):
    try:
        from unittest2 import TestCase, expectedFailure
        import unittest2 as unittest
    except ImportError:
        print("You should install unittest2 to run the salt tests")
        sys.exit(0)
else:
    from unittest import TestCase, expectedFailure
    import unittest

QGISAPP = None  # Static variable used to hold hand to running QGis app
CANVAS = None
PARENT = None
IFACE = None
GEOCRS = 4326  # constant for EPSG:GEOCRS Geographic CRS id

FONTSLOADED = False


def assertHashesForFile(theHashes, theFilename):
    """Assert that a files has matches one of a list of expected hashes"""
    myHash = hashForFile(theFilename)
    myMessage = ('Unexpected hash'
                 '\nGot: %s'
                 '\nExpected: %s'
                 '\nPlease check graphics %s visually '
                 'and add to list of expected hashes '
                 'if it is OK on this platform.'
                 % (myHash, theHashes, theFilename))
    assert myHash in theHashes, myMessage


def assertHashForFile(theHash, theFilename):
    """Assert that a files has matches its expected hash"""
    myHash = hashForFile(theFilename)
    myMessage = ('Unexpected hash'
                 '\nGot: %s'
                 '\nExpected: %s' % (myHash, theHash))
    assert myHash == theHash, myMessage


def hashForFile(theFilename):
    """Return an md5 checksum for a file"""
    myPath = theFilename
    myData = open(myPath).read()
    myHash = hashlib.md5()
    myHash.update(myData)
    myHash = myHash.hexdigest()
    return myHash


def getQgisTestApp():
    """ Start one QGis application to test agaist

    Input
        NIL

    Output
        handle to qgis app


    If QGis is already running the handle to that app will be returned
    """

    global QGISAPP  # pylint: disable=W0603

    if QGISAPP is None:
        myGuiFlag = True  # All test will run qgis in gui mode

        # In python3 we need to conver to a bytes object (or should
        # QgsApplication accept a QString instead of const char* ?)
        try:
            argvb = list(map(os.fsencode, sys.argv))
        except AttributeError:
            argvb = sys.argv

        # Note: QGIS_PREFIX_PATH is evaluated in QgsApplication -
        # no need to mess with it here.
        QGISAPP = QgsApplication(argvb, myGuiFlag)

        QGISAPP.initQgis()
        s = QGISAPP.showSettings()
        print(s)

    global PARENT  # pylint: disable=W0603
    if PARENT is None:
        PARENT = QWidget()

    global CANVAS  # pylint: disable=W0603
    if CANVAS is None:
        CANVAS = QgsMapCanvas(PARENT)
        CANVAS.resize(QSize(400, 400))

    global IFACE  # pylint: disable=W0603
    if IFACE is None:
        # QgisInterface is a stub implementation of the QGIS plugin interface
        IFACE = QgisInterface(CANVAS)

    return QGISAPP, CANVAS, IFACE, PARENT


def unitTestDataPath(theSubdir=None):
    """Return the absolute path to the QGIS unit test data dir.

    Args:
       * theSubdir: (Optional) Additional subdir to add to the path
    """
    myPath = __file__
    tmpPath = os.path.split(os.path.dirname(myPath))
    myPath = os.path.split(tmpPath[0])
    if theSubdir is not None:
        myPath = os.path.abspath(os.path.join(myPath[0],
                                              'testdata',
                                              theSubdir))
    else:
        myPath = os.path.abspath(os.path.join(myPath[0], 'testdata'))
    return myPath


def svgSymbolsPath():
    return os.path.abspath(
        os.path.join(unitTestDataPath(), '..', '..', 'images', 'svg'))


def setCanvasCrs(theEpsgId, theOtfpFlag=False):
    """Helper to set the crs for the CANVAS before a test is run.

    Args:

        * theEpsgId  - Valid EPSG identifier (int)
        * theOtfpFlag - whether on the fly projections should be enabled
                        on the CANVAS. Default to False.
    """
    # Enable on-the-fly reprojection
    CANVAS.mapRenderer().setProjectionsEnabled(theOtfpFlag)

    # Create CRS Instance
    myCrs = QgsCoordinateReferenceSystem()
    myCrs.createFromId(theEpsgId, QgsCoordinateReferenceSystem.E)

    # Reproject all layers to WGS84 geographic CRS
    CANVAS.mapRenderer().setDestinationCrs(myCrs)


def writeShape(theMemoryLayer, theFileName):
    myFileName = os.path.join(str(QDir.tempPath()), theFileName)
    print(myFileName)
    # Explicitly giving all options, not really needed but nice for clarity
    myErrorMessage = ''
    myOptions = []
    myLayerOptions = []
    mySelectedOnlyFlag = False
    mySkipAttributesFlag = False
    myGeoCrs = QgsCoordinateReferenceSystem()
    myGeoCrs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
    myResult = QgsVectorFileWriter.writeAsVectorFormat(
        theMemoryLayer,
        myFileName,
        'utf-8',
        myGeoCrs,
        'ESRI Shapefile',
        mySelectedOnlyFlag,
        myErrorMessage,
        myOptions,
        myLayerOptions,
        mySkipAttributesFlag)
    assert myResult == QgsVectorFileWriter.NoError


def doubleNear(a, b, tol=0.0000000001):
    """
    Tests whether two floats are near, within a specified tolerance
    """
    return abs(float(a) - float(b)) < tol


def compareWkt(a, b, tol=0.000001):
    """
    Compares two WKT strings, ignoring allowed differences between strings
    and allowing a tolerance for coordinates
    """
    # ignore case
    a0 = a.lower()
    b0 = b.lower()

    # remove optional spaces before z/m
    r = re.compile("\s+([zm])")
    a0 = r.sub(r'\1', a0)
    b0 = r.sub(r'\1', b0)

    # spaces before brackets are optional
    r = re.compile("\s*\(\s*")
    a0 = r.sub('(', a0)
    b0 = r.sub('(', b0)
    # spaces after brackets are optional
    r = re.compile("\s*\)\s*")
    a0 = r.sub(')', a0)
    b0 = r.sub(')', b0)

    # compare the structure
    r0 = re.compile("-?\d+(?:\.\d+)?(?:[eE]\d+)?")
    r1 = re.compile("\s*,\s*")
    a0 = r1.sub(",", r0.sub("#", a0))
    b0 = r1.sub(",", r0.sub("#", b0))
    if a0 != b0:
        return False

    # compare the numbers with given tolerance
    a0 = r0.findall(a)
    b0 = r0.findall(b)
    if len(a0) != len(b0):
        return False

    for (a1, b1) in zip(a0, b0):
        if not doubleNear(a1, b1, tol):
            return False

    return True


def getTempfilePath(sufx='png'):
    """
    :returns: Path to empty tempfile ending in defined suffix
    Caller should delete tempfile if not used
    """
    tmp = tempfile.NamedTemporaryFile(
        suffix=".{0}".format(sufx), delete=False)
    filepath = tmp.name
    tmp.close()
    return filepath


def renderMapToImage(mapsettings, parallel=False):
    """
    Render current map to an image, via multi-threaded renderer
    :param QgsMapSettings mapsettings:
    :param bool parallel: Do parallel or sequential render job
    :rtype: QImage
    """
    if parallel:
        job = QgsMapRendererParallelJob(mapsettings)
    else:
        job = QgsMapRendererSequentialJob(mapsettings)
    job.start()
    job.waitForFinished()

    return job.renderedImage()


def mapSettingsString(ms):
    """
    :param QgsMapSettings mapsettings:
    :rtype: str
    """
    # fullExtent() causes extra call in middle of output flow; get first
    full_ext = ms.visibleExtent().toString()

    s = 'MapSettings...\n'
    s += '  layers(): {0}\n'.format(
        [QgsMapLayerRegistry.instance().mapLayer(i).name()
         for i in ms.layers()])
    s += '  backgroundColor(): rgba {0},{1},{2},{3}\n'.format(
        ms.backgroundColor().red(), ms.backgroundColor().green(),
        ms.backgroundColor().blue(), ms.backgroundColor().alpha())
    s += '  selectionColor(): rgba {0},{1},{2},{3}\n'.format(
        ms.selectionColor().red(), ms.selectionColor().green(),
        ms.selectionColor().blue(), ms.selectionColor().alpha())
    s += '  outputSize(): {0} x {1}\n'.format(
        ms.outputSize().width(), ms.outputSize().height())
    s += '  outputDpi(): {0}\n'.format(ms.outputDpi())
    s += '  mapUnits(): {0}\n'.format(ms.mapUnits())
    s += '  scale(): {0}\n'.format(ms.scale())
    s += '  mapUnitsPerPixel(): {0}\n'.format(ms.mapUnitsPerPixel())
    s += '  extent():\n    {0}\n'.format(
        ms.extent().toString().replace(' : ', '\n    '))
    s += '  visibleExtent():\n    {0}\n'.format(
        ms.visibleExtent().toString().replace(' : ', '\n    '))
    s += '  fullExtent():\n    {0}\n'.format(full_ext.replace(' : ', '\n    '))
    s += '  hasCrsTransformEnabled(): {0}\n'.format(
        ms.hasCrsTransformEnabled())
    s += '  destinationCrs(): {0}\n'.format(
        ms.destinationCrs().authid())
    s += '  flag.Antialiasing: {0}\n'.format(
        ms.testFlag(QgsMapSettings.Antialiasing))
    s += '  flag.UseAdvancedEffects: {0}\n'.format(
        ms.testFlag(QgsMapSettings.UseAdvancedEffects))
    s += '  flag.ForceVectorOutput: {0}\n'.format(
        ms.testFlag(QgsMapSettings.ForceVectorOutput))
    s += '  flag.DrawLabeling: {0}\n'.format(
        ms.testFlag(QgsMapSettings.DrawLabeling))
    s += '  flag.DrawEditingInfo: {0}\n'.format(
        ms.testFlag(QgsMapSettings.DrawEditingInfo))
    s += '  outputImageFormat(): {0}\n'.format(ms.outputImageFormat())
    return s


def getExecutablePath(exe):
    """
    :param exe: Name of executable, e.g. lighttpd
    :returns: Path to executable
    """
    exe_exts = []
    if (platform.system().lower().startswith('win') and
            "PATHEXT" in os.environ):
        exe_exts = os.environ["PATHEXT"].split(os.pathsep)

    for path in os.environ["PATH"].split(os.pathsep):
        exe_path = os.path.join(path, exe)
        if os.path.exists(exe_path):
            return exe_path
        for ext in exe_exts:
            if os.path.exists(exe_path + ext):
                return exe_path
    return ''


def getTestFontFamily():
    return QgsFontUtils.standardTestFontFamily()


def getTestFont(style='Roman', size=12):
    """Only Roman and Bold are loaded by default
    Others available: Oblique, Bold Oblique
    """
    if not FONTSLOADED:
        loadTestFonts()
    return QgsFontUtils.getStandardTestFont(style, size)


def loadTestFonts():
    if QGISAPP is None:
        getQgisTestApp()

    global FONTSLOADED  # pylint: disable=W0603
    if FONTSLOADED is False:
        QgsFontUtils.loadStandardTestFonts(['Roman', 'Bold'])
        msg = getTestFontFamily() + ' base test font styles could not be loaded'
        res = (QgsFontUtils.fontFamilyHasStyle(getTestFontFamily(), 'Roman')
               and QgsFontUtils.fontFamilyHasStyle(getTestFontFamily(), 'Bold'))
        assert res, msg
        FONTSLOADED = True


def openInBrowserTab(url):
    if sys.platform[:3] in ('win', 'dar'):
        webbrowser.open_new_tab(url)
    else:
        # some Linux OS pause execution on webbrowser open, so background it
        cmd = 'import webbrowser;' \
              'webbrowser.open_new_tab("{0}")'.format(url)
        subprocess.Popen([sys.executable, "-c", cmd],
                         stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)


def printImportant(info):
    """
    Prints important information to stdout and to a file which in the end
    should be printed on test result pages.
    :param info: A string to print
    """

    print(info)
    with open(os.path.join(tempfile.gettempdir(), 'ctest-important.log'), 'a+') as f:
        f.write(u'{}\n'.format(info))


class DoxygenParser():

    """
    Parses the XML files generated by Doxygen which describe the API docs
    """

    def __init__(self, path):
        """
        Initializes the parser.
        :param path: Path to Doxygen XML output
        """
        self.documentable_members = 0
        self.documented_members = 0
        self.undocumented_string = ''
        self.bindable_members = []
        self.parseFiles(path)

    def parseFiles(self, path):
        """ Parses all the Doxygen XML files in a folder
            :param path: Path to Doxygen XML output
        """
        for f in glob.glob(os.path.join(path, '*.xml')):
            self.parseFile(f)

    def parseFile(self, f):
        """ Parses a single Doxygen XML file
            :param f: XML file path
        """
        documentable_members = 0
        documented_members = 0

        # Wrap everything in a try, as sometimes Doxygen XML is malformed
        try:
            for event, elem in ET.iterparse(f):
                if event == 'end' and elem.tag == 'compounddef':
                    if self.elemIsPublicClass(elem):
                        # store documentation status
                        members, documented, undocumented, bindable = self.parseClassElem(elem)
                        documentable_members += members
                        documented_members += documented
                        class_name = elem.find('compoundname').text
                        if documented < members:
                            self.undocumented_string += "Class {}, {}/{} members documented\n".format(class_name, documented, members)
                            for u in undocumented:
                                self.undocumented_string += ' Missing: {}\n'.format(u)
                            self.undocumented_string += "\n"

                        # store bindable members
                        if self.classElemIsBindable(elem):
                            for m in bindable:
                                self.bindable_members.append(m)

                    elem.clear()
        except ET.ParseError as e:
            # sometimes Doxygen generates malformed xml (eg for < and > operators)
            line_num, col = e.position
            with open(f, 'r') as xml_file:
                for i, l in enumerate(xml_file):
                    if i == line_num - 1:
                        line = l
                        break
            caret = '{:=>{}}'.format('^', col)
            print('ParseError in {}\n{}\n{}\n{}'.format(f, e, line, caret))

        self.documentable_members += documentable_members
        self.documented_members += documented_members

    def elemIsPublicClass(self, elem):
        """ Tests whether an XML element corresponds to a public (or protected) class
            :param elem: XML element
        """

        # only looking for classes
        if not elem.get('kind') == 'class':
            return False

        # only looking for public or protected classes
        return elem.get('prot') in ('public', 'protected')

    def classElemIsBindable(self, elem):
        """ Tests whether a class should have SIP bindings
            :param elem: XML element corresponding to a class
        """
        try:
            # check for 'not available in python bindings' note in class docs
            detailed_sec = elem.find('detaileddescription')
            for p in detailed_sec.getiterator('para'):
                for s in p.getiterator('simplesect'):
                    for ps in s.getiterator('para'):
                        if 'not available in python bindings' in ps.text.lower():
                            return False
            return True
        except:
            return True

    def parseClassElem(self, e):
        """ Parses an XML element corresponding to a Doxygen class
            :param e: XML element
        """
        documentable_members = 0
        documented_members = 0
        undocumented_members = []
        bindable_members = []
        # loop through all members
        for m in e.getiterator('memberdef'):
            if self.elemIsBindableMember(m):
                bindable_member = [e.find('compoundname').text, m.find('name').text]
                if not bindable_member in bindable_members:
                    bindable_members.append(bindable_member)
            if self.elemIsDocumentableMember(m):
                documentable_members += 1
                if self.memberIsDocumented(m):
                    documented_members += 1
                else:
                    undocumented_members.append(self.memberSignature(m))
        return documentable_members, documented_members, undocumented_members, bindable_members

    def memberSignature(self, elem):
        """ Returns the signature for a member
            :param elem: XML element for a class member
        """
        a = elem.find('argsstring')
        try:
            if a is not None:
                return elem.find('name').text + a.text
            else:
                return elem.find('name').text
        except:
            return None

    def elemIsBindableMember(self, elem):
        """ Tests whether an member should be included in SIP bindings
            :param elem: XML element for a class member
        """

        # only public or protected members are bindable
        if not self.visibility(elem) in ('public', 'protected'):
            return False

        # property themselves are not bound, only getters and setters
        if self.isProperty(elem):
            return False

        # ignore friend classes
        if self.isFriendClass(elem):
            return False

        # ignore typedefs (can't test for them)
        if self.isTypeDef(elem):
            return False

        if self.isVariable(elem) and self.visibility(elem) == 'protected':
            # protected variables can't be bound in SIP
            return False

        # check for members with special python doc notes (probably 'not available' or renamed methods, either way
        # they should be safe to ignore as obviously some consideration has been given to Python bindings)
        try:
            detailed_sec = elem.find('detaileddescription')
            for p in detailed_sec.getiterator('para'):
                for s in p.getiterator('simplesect'):
                    for ps in s.getiterator('para'):
                        if ps.text and 'python' in ps.text.lower():
                            return False
        except:
            pass

        # ignore constructors and destructor, can't test for these
        if self.isDestructor(elem) or self.isConstructor(elem):
            return False

        # ignore operators, also can't test
        if self.isOperator(elem):
            return False

        # ignore deprecated members
        if self.isDeprecated(elem):
            return False

        return True

    def elemIsDocumentableMember(self, elem):
        """ Tests whether an member should be included in Doxygen docs
            :param elem: XML element for a class member
        """

        # ignore variables (for now, eventually public/protected variables should be documented)
        if self.isVariable(elem):
            return False

        # only public or protected members should be documented
        if not self.visibility(elem) in ('public', 'protected'):
            return False

        # ignore reimplemented methods
        if self.isReimplementation(elem):
            return False

        # ignore friend classes
        if self.isFriendClass(elem):
            return False

        # ignore destructor
        if self.isDestructor(elem):
            return False

        # ignore constructors with no arguments
        if self.isConstructor(elem):
            try:
                if elem.find('argsstring').text == '()':
                    return False
            except:
                pass

        name = elem.find('name')

        # ignore certain obvious operators
        try:
            if name.text in ('operator=', 'operator=='):
                return False
        except:
            pass

        # ignore on_* slots
        try:
            if name.text.startswith('on_'):
                return False
        except:
            pass

        # ignore deprecated members
        if self.isDeprecated(elem):
            return False

        return True

    def visibility(self, elem):
        """ Returns the visibility of a class or member
            :param elem: XML element for a class or member
        """
        try:
            return elem.get('prot')
        except:
            return ''

    def isVariable(self, member_elem):
        """ Tests whether an member is a variable
            :param member_elem: XML element for a class member
        """
        try:
            if member_elem.get('kind') == 'variable':
                return True
        except:
            pass

        return False

    def isProperty(self, member_elem):
        """ Tests whether an member is a property
            :param member_elem: XML element for a class member
        """
        try:
            if member_elem.get('kind') == 'property':
                return True
        except:
            pass

        return False

    def isDestructor(self, member_elem):
        """ Tests whether an member is a destructor
            :param member_elem: XML element for a class member
        """
        try:
            name = member_elem.find('name').text
            if name.startswith('~'):
                # destructor
                return True
        except:
            pass
        return False

    def isConstructor(self, member_elem):
        """ Tests whether an member is a constructor
            :param member_elem: XML element for a class member
        """
        try:
            definition = member_elem.find('definition').text
            name = member_elem.find('name').text
            if definition == '{}::{}'.format(name, name):
                return True
        except:
            pass

        return False

    def isOperator(self, member_elem):
        """ Tests whether an member is an operator
            :param member_elem: XML element for a class member
        """
        try:
            name = member_elem.find('name').text
            if re.match('^operator\W.*', name):
                return True
        except:
            pass

        return False

    def isFriendClass(self, member_elem):
        """ Tests whether an member is a friend class
            :param member_elem: XML element for a class member
        """
        try:
            if member_elem.get('kind') == 'friend':
                return True
        except:
            pass
        return False

    def isTypeDef(self, member_elem):
        """ Tests whether an member is a type def
            :param member_elem: XML element for a class member
        """
        try:
            if member_elem.get('kind') == 'typedef':
                return True
        except:
            pass
        return False

    def isReimplementation(self, member_elem):
        """ Tests whether an member is a reimplementation
            :param member_elem: XML element for a class member
        """

        # use two different tests, as Doxygen will not detect reimplemented Qt methods
        try:
            if member_elem.find('reimplements') is not None:
                return True
            if ' override' in member_elem.find('argsstring').text:
                return True
        except:
            pass

        return False

    def isDeprecated(self, member_elem):
        """ Tests whether an member is deprecated
            :param member_elem: XML element for a class member
        """

        type_elem = member_elem.find('type')
        try:
            if 'Q_DECL_DEPRECATED' in type_elem.text:
                return True
        except:
            pass

        return False

    def memberIsDocumented(self, member_elem):
        """ Tests whether an member has documentation
            :param member_elem: XML element for a class member
        """
        for doc_type in ('inbodydescription', 'briefdescription', 'detaileddescription'):
            doc = member_elem.find(doc_type)
            if doc is not None and list(doc):
                return True
        return False
