
cimport objects
from objects cimport bool, string, vector, tree, tree_node, OriginFile
cimport cython.operator
from cython.operator import dereference as deref
from PyQt4 import QtCore, QtGui
Qt = QtCore.Qt

####################################################################################################
##########################################               ###########################################
######################################### Object classes ###########################################
#########################################               ############################################
####################################################################################################

cdef class Window:
    
    cdef public string name, label
    cdef public int objectID
    cdef public bool hidden
    cdef public int state, title # enums
    cdef public object frameRect
    cdef public long creationDate, modificationDate
    
    cdef void copy(self, const objects.Window *wnd):
        self.name = wnd.name
        self.label = wnd.label
        self.objectID = wnd.objectID
        self.hidden = wnd.hidden
        self.state = <int>wnd.state
        self.title = <int>wnd.title
        self.frameRect = makeRect(wnd.frameRect)
        self.creationDate = wnd.creationDate
        self.modificationDate = wnd.modificationDate

cdef class SpreadColumn:
    
    cdef public string name, command, comment
    cdef public int type, valueType, numericDisplayType # enums
    cdef public int valueTypeSpecification, significantDigits, decimalPlaces, width, index, sheet
    cdef public object data
    
    cdef void copy(self, const objects.SpreadColumn *col):
        cdef const double *doublePointer
        cdef const string *stringPointer
        cdef double doubleValue
        cdef string stringValue
        self.name = col.name
        self.type = <int>col.type
        self.valueType = <int>col.valueType
        self.valueTypeSpecification = col.valueTypeSpecification
        self.significantDigits = col.significantDigits
        self.decimalPlaces = col.decimalPlaces
        self.numericDisplayType = <int>col.numericDisplayType
        self.command = col.command
        self.comment = col.comment
        self.width = col.width
        self.index = col.index
        self.sheet = col.sheet
        self.data = []
        # data copying is... umm... complicated.
        # Because of FUCKING BOOST::VARIANT!11
        dataSize = col.data.size()
        for i in range(dataSize):
            doublePointer = objects.getDoubleFromVariant(&(col.data[i]))
            stringPointer = objects.getStringFromVariant(&(col.data[i]))
            if doublePointer:
                doubleValue = deref(doublePointer)
                self.data.append(doubleValue)
            else:
                stringValue = deref(stringPointer)
                self.data.append(stringValue)

cdef class SpreadSheet(Window):
    
    cdef public int maxRows, sheets
    cdef public bool loose, multisheet
    cdef public object columns
    
    cdef void copy(self, const objects.Window *wnd):
        Window.copy(self, wnd)
        cdef const objects.SpreadSheet *sht = <objects.SpreadSheet*>wnd
        self.maxRows = sht.maxRows
        self.loose = sht.loose
        self.multisheet = sht.multisheet
        self.sheets = sht.sheets
        self.columns = [makeSpreadColumn(sht.columns[i]) for i in xrange(sht.columns.size())]

cdef class Note(Window):
    
    cdef public string text
    
    cdef void copy(self, const objects.Window *wnd):
        Window.copy(self, wnd)
        cdef const objects.Note *nt = <objects.Note*>wnd
        self.text = nt.text

cdef class ProjectNode:
    
    cdef public int type # enum
    cdef public string name
    cdef public int creationDate, modificationDate
    
    cdef void copy(self, const objects.ProjectNode *pn):
        self.type = <int>pn.type
        self.name = pn.name
        self.creationDate = pn.creationDate
        self.modificationDate = pn.modificationDate

####################################################################################################
##########################################              ############################################
######################################### Object makers ############################################
#########################################              #############################################
####################################################################################################

cdef makeColor(const objects.Color &clr):
    if clr.cType == objects.ctRegular: # TODO: implement commented colors
        return [Qt.black, Qt.red, Qt.green, Qt.blue, Qt.cyan, Qt.magenta, Qt.yellow, Qt.darkYellow,
               "Qt.navy", "Qt.purple", "Qt.wine", "Qt.olive", Qt.darkCyan, "Qt.royal", "Qt.orange",
               "Qt.violet", "Qt.pink", Qt.white, Qt.lightGray, Qt.gray, "Qt.lightYellow",
               "Qt.lightCyan", "Qt.lightMagenta", Qt.darkGray][clr.custom[0]] # TODO: check which byte is used
    elif clr.cType == objects.ctRGB:
        return QtCore.QColor(clr.custom[0], clr.custom[1], clr.custom[2])
    else: # TODO: other color objects
        return QtCore.QColor()

cdef makeRect(const objects.Rect &rect):
    return QtCore.QRect(QtCore.QPoint(rect.left, rect.top), QtCore.QPoint(rect.right, rect.bottom))

cdef makeSpreadColumn(const objects.SpreadColumn &col):
    result = SpreadColumn()
    result.copy(&col)
    return result

cdef makeSpreadSheet(const objects.SpreadSheet &sht):
    print "Making spreadsheet..."
    result = SpreadSheet()
    result.copy(&sht)
    return result

cdef makeNote(const objects.Note &nt):
    print "Making note..."
    result = Note()
    result.copy(&nt)
    return result

cdef makeProjectNode(const objects.ProjectNode &pn):
    result = ProjectNode()
    result.copy(&pn)
    return result

cdef makeTreeNode(tree_node[objects.ProjectNode] *nd):
    cdef tree[objects.ProjectNode].leaf_iterator *it = new tree[objects.ProjectNode].leaf_iterator(nd)
    cdef tree[objects.ProjectNode].iterator_base *tmp = new tree[objects.ProjectNode].iterator_base(nd)
    numel = tmp.number_of_children()
    del tmp
    children = []
    for i in xrange(numel):
        child = makeProjectNode(deref(deref(it)))
        print child.name
        children.append(child)
        cython.operator.preincrement(it)
    del it
    return children

cdef makeTree(const tree[objects.ProjectNode] *tr):
    cdef tree[objects.ProjectNode].pre_order_iterator it = tr.begin()
    print "Going to pythonify tree with %d nodes." % tr.size()
    return makeTreeNode(it.node)

cdef getNodes(OriginFile *originFile):
    spreadCount = originFile.spreadCount()
    matrixCount = originFile.matrixCount()
    functionCount = originFile.functionCount()
    graphCount = originFile.graphCount()
    noteCount = originFile.noteCount()
    print spreadCount, matrixCount, functionCount, graphCount, noteCount
    spreads   = [makeSpreadSheet(originFile.spread  (i)) for i in xrange(originFile.spreadCount())]
    #matrices  = [originFile.matrix  (i) for i in xrange(originFile.matrixCount())]
    #functions = [originFile.function(i) for i in xrange(originFile.spreadCount())]
    #graphs    = [originFile.graph   (i) for i in xrange(originFile.graphCount ())]
    notes     = [makeNote       (originFile.note    (i)) for i in xrange(originFile.noteCount  ())]
    cdef const tree[objects.ProjectNode] *project = originFile.project()
    tree = makeTree(project)
    

def parseOriginFile(filename):
    cdef OriginFile *originFile = new OriginFile(filename)
    result = originFile.parse()
    getNodes(originFile)
    return result




