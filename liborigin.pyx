
cimport objects
from objects cimport bool, string, vector, pair, tree, tree_node, OriginFile
cimport cython.operator
from cython.operator import dereference as deref
#from PyQt4 import QtCore, QtGui
#Qt = QtCore.Qt

####################################################################################################
##########################################               ###########################################
######################################### Object classes ###########################################
#########################################               ############################################
####################################################################################################

cdef class Color:
    
    cdef public int type # enum
    cdef public int regular, starting, column
    cdef public object custom
    
    cdef void copy(self, const objects.Color *arg):
        self.type = <int>arg.type
        self.regular = arg.regular
        self.starting = arg.starting
        self.column = arg.column

cdef class Rect:
    
    cdef public int left, top, right, bottom
    
    cdef void copy(self, const objects.Rect *arg):
        self.left = arg.left
        self.top = arg.top
        self.right = arg.right
        self.bottom = arg.bottom

cdef class ColorMapLevel:
    
    cdef public object fillColor, fillPatternColor, lineColor
    cdef public int fillPattern, lineStyle
    cdef public float fillPatternLineWidth, lineWidth
    cdef public bool lineVisible, labelVisible
    
    cdef void copy(self, const objects.ColorMapLevel *lvl):
        self.fillColor = makeColor(lvl.fillColor)
        self.fillPatternColor = makeColor(lvl.fillPatternColor)
        self.lineColor = makeColor(lvl.lineColor)
        self.fillPattern = lvl.fillPattern
        self.lineStyle = lvl.lineStyle
        self.fillPatternLineWidth = lvl.fillPatternLineWidth
        self.lineWidth = lvl.lineWidth
        self.lineVisible = lvl.lineVisible
        self.labelVisible = lvl.labelVisible

cdef class ColorMap:
    
    cdef public bool fillEnabled
    cdef public object levels
    
    cdef void copy(self, const objects.ColorMap *mp):
        self.fillEnabled = mp.fillEnabled
        self.levels = []
        for i in xrange(mp.levels.size()):
            self.levels.append((mp.levels[i].first, makeColorMapLevel(mp.levels[i].second)))

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
                self.data.append(float(doubleValue))
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

cdef class Matrix(Window):
    
    cdef public int rowCount, columnCount, valueTypeSpecification, significantDigits, decimalPlaces
    cdef public int numericDisplayType, view, header # enums
    cdef public string command
    cdef public int width, index, sheets
    cdef public object colorMap, data, coordinates
    
    cdef void copy(self, const objects.Window *wnd):
        Window.copy(self, wnd)
        cdef const objects.Matrix *mtx = <objects.Matrix*>wnd
        self.rowCount = mtx.rowCount
        self.columnCount = mtx.columnCount
        self.valueTypeSpecification = mtx.valueTypeSpecification
        self.significantDigits = mtx.significantDigits
        self.decimalPlaces = mtx.decimalPlaces
        self.numericDisplayType = <int>mtx.numericDisplayType
        self.command = mtx.command
        self.width = mtx.width
        self.index = mtx.index
        self.sheets = mtx.sheets
        self.view = <int>mtx.view
        self.header = <int>mtx.header
        self.colorMap = makeColorMap(mtx.colorMap)
        self.data = [mtx.data[i] for i in xrange(mtx.data.size())]
        self.coordinates = [mtx.coordinates[i] for i in xrange(mtx.coordinates.size())]

cdef class Function:
    
    cdef public string name, formula
    cdef public int type # enum
    cdef public float begin, end
    cdef public int totalPoints, index
    
    cdef void copy(self, const objects.Function *f):
        self.name = f.name
        self.formula = f.formula
        self.type = <int>f.type
        self.begin = f.begin
        self.end = f.end
        self.totalPoints = f.totalPoints
        self.index = f.index








cdef class TextBox:

    cdef public string text
    cdef public object clientRect, color
    cdef public int fontSize, rotation, tab
    cdef public int borderType, attach #enums
    
    cdef void copy(self, const objects.TextBox *tb):
        self.text = tb.text
        self.clientRect = makeRect(tb.clientRect)
        self.color = makeColor(tb.color)
        self.fontSize = tb.fontSize
        self.rotation = tb.rotation
        self.tab = tb.tab
        self.borderType = <int>tb.borderType
        self.attach = <int>tb.attach

cdef class PieProperties:
    
    cdef public int  viewAngle, thickness, rotation, radius, \
                     horizontalOffset, displacedSectionCount, displacement, distance
    cdef public bool clockwiseRotation, formatAutomatic, formatValues, formatPercentages, \
                     formatCategories, positionAssociate
    
    cdef void copy(self, const objects.PieProperties *arg):
        self.clockwiseRotation = arg.clockwiseRotation
        self.formatAutomatic = arg.formatAutomatic
        self.formatValues = arg.formatValues
        self.formatPercentages = arg.formatPercentages
        self.formatCategories = arg.formatCategories
        self.positionAssociate = arg.positionAssociate


cdef class TextProperties:
    
    cdef public object color # Color
    cdef public bool fontBold, fontItalic, fontUnderline, whiteOut
    cdef public int justify # enum
    cdef public int rotation, xOffset, yOffset, fontSize
    
    cdef void copy(self, const objects.TextProperties *arg):
        self.color = makeColor(arg.color)
        self.fontBold = arg.fontBold
        self.fontItalic = arg.fontItalic
        self.fontUnderline = arg.fontUnderline
        self.whiteOut = arg.whiteOut
        self.justify = <int>arg.justify
        self.rotation = arg.rotation
        self.xOffset = arg.xOffset
        self.yOffset = arg.yOffset
        self.fontSize = arg.fontSize


cdef class VectorProperties:

    cdef public object color # Color
    cdef public float width, multiplier
    cdef public int arrowLength, arrowAngle, constAngle, constMagnitude # original arrowLength has a typo
    cdef public bool arrowClosed
    cdef public string endXColumnName, endYColumnName, angleColumnName, magnitudeColumnName
    
    cdef void copy(self, const objects.VectorProperties *arg):
        self.color = makeColor(arg.color)
        self.width = arg.width
        self.multiplier = arg.multiplier
        self.arrowClosed = arg.arrowClosed
        self.endXColumnName = arg.endXColumnName
        self.endYColumnName = arg.endYColumnName
        self.angleColumnName = arg.angleColumnName
        self.magnitudeColumnName = arg.magnitudeColumnName


cdef class SurfaceColoration:
    
    cdef public bool fill, contour
    cdef public object lineColor # Color
    cdef public float lineWidth
    
    cdef void copy(self, const objects.SurfaceColoration *arg):
        self.fill = arg.fill
        self.contour = arg.contour
        self.lineColor = makeColor(arg.lineColor)
        self.lineWidth = arg.lineWidth


cdef class SurfaceProperties:
    
    cdef public int type
    cdef public int grids # enum
    cdef public float gridLineWidth
    cdef public object gridColor, frontColor, backColor, xSideWallColor, ySideWallColor # Color
    cdef public object surface, topContour, bottomContour # SurfaceColoration
    cdef public object colorMap # ColorMap
    cdef public bool backColorEnabled, sideWallEnabled
    
    cdef void copy(self, const objects.SurfaceProperties *arg):
        self.type = arg.type
        self.grids = <int>arg.grids
        self.gridLineWidth = arg.gridLineWidth
        self.gridColor = makeColor(arg.gridColor)
        self.frontColor = makeColor(arg.frontColor)
        self.backColor = makeColor(arg.backColor)
        self.xSideWallColor = makeColor(arg.xSideWallColor)
        self.ySideWallColor = makeColor(arg.ySideWallColor)
        self.surface = makeSurfaceColoration(arg.surface)
        self.topContour = makeSurfaceColoration(arg.topContour)
        self.bottomContour = makeSurfaceColoration(arg.bottomContour)
        self.colorMap = makeColorMap(arg.colorMap)
        self.backColorEnabled = arg.backColorEnabled
        self.sideWallEnabled = arg.sideWallEnabled


cdef class PercentileProperties:
    
    cdef public int maxSymbolType, p99SymbolType, meanSymbolType, p1SymbolType, minSymbolType, \
                    symbolSize, boxRange, whiskersRange
    cdef public object symbolColor, symbolFillColor # Color
    cdef public float boxCoeff, whiskersCoeff
    cdef public bool diamondBox
    
    cdef void copy(self, const objects.PercentileProperties *arg):
        self.maxSymbolType = arg.maxSymbolType
        self.p99SymbolType = arg.p99SymbolType
        self.meanSymbolType = arg.meanSymbolType
        self.p1SymbolType = arg.p1SymbolType
        self.minSymbolType = arg.minSymbolType
        self.boxRange = arg.boxRange
        self.whiskersRange = arg.whiskersRange
        self.symbolColor = makeColor(arg.symbolColor)
        self.symbolFillColor = makeColor(arg.symbolFillColor)
        self.boxCoeff = arg.boxCoeff
        self.whiskersCoeff = arg.whiskersCoeff
        self.diamondBox = arg.diamondBox


cdef class GraphCurve:
    
    cdef public int type, lineStyle, lineConnect, boxWidth, fillAreaType, fillAreaPattern, \
                    fillAreaPatternBorderStyle, symbolType, symbolThickness, pointOffset
    cdef public string dataName, xColumnName, yColumnName, zColumnName
    cdef public object lineColor, fillAreaColor, fillAreaPatternColor, fillAreaPatternBorderColor, \
                       symbolColor, symbolFillColor # Color
    cdef public object pie, vector, text, surface, colorMap
    cdef public float lineWidth, fillAreaPatternWidth, fillAreaPatternBorderWidth, symbolSize
    cdef public bool fillArea, connectSymbols
    
    cdef void copy(self, const objects.GraphCurve *arg):
        self.type = arg.type
        self.lineStyle = arg.lineStyle
        self.lineConnect = arg.lineConnect
        self.boxWidth = arg.boxWidth
        self.fillAreaType = arg.fillAreaType
        self.fillAreaPattern = arg.fillAreaPattern
        self.symbolThickness = arg.symbolThickness
        self.pointOffset = arg.pointOffset
        self.dataName = arg.dataName
        self.xColumnName = arg.xColumnName
        self.yColumnName = arg.yColumnName
        self.zColumnName = arg.zColumnName
        self.lineColor = makeColor(arg.lineColor)
        self.fillAreaColor = makeColor(arg.fillAreaColor)
        self.fillAreaPatternColor = makeColor(arg.fillAreaPatternColor)
        self.fillAreaPatternBorderColor = makeColor(arg.fillAreaPatternBorderColor)
        self.symbolColor = makeColor(arg.symbolColor)
        self.symbolFillColor = makeColor(arg.symbolFillColor)
        self.pie = makePieProperties(arg.pie)
        self.vector = makeVectorProperties(arg.vector)
        self.text = makeTextProperties(arg.text)
        self.surface = makeSurfaceProperties(arg.surface)
        self.colorMap = makeColorMap(arg.colorMap)
        self.lineWidth = arg.lineWidth
        self.fillAreaPatternWidth = arg.fillAreaPatternWidth
        self.fillAreaPatternBorderWidth = arg.fillAreaPatternBorderWidth
        self.symbolSize = arg.symbolSize
        self.fillArea = arg.fillArea
        self.connectSymbols = arg.connectSymbols


cdef class GraphAxisBreak:
    
    cdef public bool show, log10
    cdef public float gabFrom, to, position, scaleIncrementBefore, scaleIncrementAfter
    cdef public int minorTicksBefore, minorTicksAfter
    
    cdef void copy(self, const objects.GraphAxisBreak *arg):
        self.show = arg.show
        self.log10 = arg.log10
        self.gabFrom = arg.gabFrom
        self.to = arg.to
        self.position = arg.position
        self.scaleIncrementBefore = arg.scaleIncrementBefore
        self.scaleIncrementAfter = arg.scaleIncrementAfter
        self.minorTicksBefore = arg.minorTicksBefore
        self.minorTicksAfter = arg.minorTicksAfter


cdef class GraphGrid:
    
    cdef public bool hidden
    cdef public int color, style
    cdef public float width
    
    cdef void copy(self, const objects.GraphGrid *arg):
        self.hidden = arg.hidden
        self.color = arg.color
        self.style = arg.style
        self.width = arg.width


cdef class GraphAxisFormat:
    
    cdef public bool hidden
    cdef public int color, majorTicksType, minorTicksType, axisPosition
    cdef public float thickness, majorTickLength, axisPositionValue
    cdef public object label # TextBox
    cdef public string prefix, suffix
    
    cdef void copy(self, const objects.GraphAxisFormat *arg):
        self.hidden = arg.hidden
        self.color = arg.color
        self.majorTicksType = arg.majorTicksType
        self.minorTicksType = arg.minorTicksType
        self.axisPosition = arg.axisPosition
        self.thickness = arg.thickness
        self.majorTickLength = arg.majorTickLength
        self.axisPositionValue = arg.axisPositionValue
        self.label = makeTextBox(arg.label)
        self.prefix = arg.prefix
        self.suffix = arg.suffix


cdef class GraphAxisTick:
    
    cdef public bool hidden, fontBold
    cdef public int color, valueTypeSpecification, decimalPlaces, fontSize, rotation
    cdef public int valueType # enum
    cdef public string dataName, columnName
    
    cdef void copy(self, const objects.GraphAxisTick *arg):
        self.hidden = arg.hidden
        self.fontBold = arg.fontBold
        self.color = arg.color
        self.valueTypeSpecification = arg.valueTypeSpecification
        self.decimalPlaces = arg.decimalPlaces
        self.fontSize = arg.fontSize
        self.rotation = arg.rotation
        self.valueType = <int>arg.valueType
        self.dataName = arg.dataName
        self.columnName = arg.columnName


cdef class GraphAxis:
    
    cdef public int position # enum
    cdef public float min, max, step
    cdef public int majorTicks, minorTicks, scale
    cdef public object majorGrid, minorGrid # GraphGrid
    cdef public object formatAxis # GraphAxisFormat x 2
    cdef public object tickAxis   # GraphAxisTick x 2
    
    cdef void copy(self, const objects.GraphAxis *arg):
        self.position = <int>arg.position
        self.min = arg.min
        self.max = arg.max
        self.step = arg.step
        self.majorTicks = arg.majorTicks
        self.minorTicks = arg.minorTicks
        self.scale = arg.scale
        self.majorGrid = makeGraphGrid(arg.majorGrid)
        self.minorGrid = makeGraphGrid(arg.minorGrid)
        self.formatAxis = [makeGraphAxisFormat(arg.formatAxis[0]), makeGraphAxisFormat(arg.formatAxis[1])]
        self.tickAxis = [makeGraphAxisTick(arg.tickAxis[0]), makeGraphAxisTick(arg.tickAxis[1])]
        


cdef class Figure:
    
    cdef public int type, attach # enums
    cdef public object clientRect # Rect
    cdef public object color, fillAreaColor, fillAreaPatternColor # Color
    cdef public int style, fillAreaPattern, 
    cdef public float width, fillAreaPatternWidth
    cdef public bool useBorderColor
    
    cdef void copy(self, const objects.Figure *arg):
        self.type = <int>arg.type
        self.attach = <int>arg.attach
        self.clientRect = makeRect(arg.clientRect)
        self.color = makeColor(arg.color)
        self.fillAreaColor = makeColor(arg.fillAreaColor)
        self.fillAreaPatternColor = makeColor(arg.fillAreaPatternColor)
        self.width = arg.width
        self.fillAreaPatternWidth = arg.fillAreaPatternWidth
        self.useBorderColor = arg.useBorderColor


cdef class LineVertex:
    
    cdef public int shapeType
    cdef public float shapeWidth, shapeLength, x, y
    
    cdef void copy(self, const objects.LineVertex *arg):
        self.shapeType = arg.shapeType
        self.shapeWidth = arg.shapeWidth
        self.shapeLength = arg.shapeLength
        self.x = arg.x
        self.y = arg.y


cdef class Line:
    
    cdef public object clientRect # Rect
    cdef public object color # Color
    cdef public int attach # enum
    cdef public float width
    cdef public int style
    cdef public object begin, end # LineVertex
    
    cdef void copy(self, const objects.Line *arg):
        self.clientRect = makeRect(arg.clientRect)
        self.color = makeColor(arg.color)
        self.attach = <int>arg.attach
        self.width = arg.width
        self.style = arg.style
        self.begin = makeLineVertex(arg.begin)
        self.end = makeLineVertex(arg.end)


cdef class Bitmap:
    
    cdef public object clientRect # Rect
    cdef public int attach, borderType # enums
    cdef public int size
    cdef public string windowName
    cdef public object data
    
    cdef void copy(self, const objects.Bitmap *arg):
        self.clientRect = makeRect(arg.clientRect)
        self.attach = <int>arg.attach
        self.borderType = <int>arg.borderType
        self.size = arg.size
        self.windowName = arg.windowName
        self.data = [arg.data[i] for i in xrange(self.size)]


cdef class ColorScale:
    
    cdef public bool reverseOrder
    cdef public int labelGap, colorBarThickness
    cdef public object labelsColor # Color
    
    cdef void copy(self, const objects.ColorScale *arg):
        self.reverseOrder = arg.reverseOrder
        self.labelGap = arg.labelGap
        self.colorBarThickness = arg.colorBarThickness
        self.labelsColor = makeColor(arg.labelsColor)


cdef class GraphLayer:
    
    cdef public object clientRect # Rect
    cdef public object legend # TextBox
    cdef public object backgroundColor # Color
    cdef public int borderType # enum
    cdef public object xAxis, yAxis, zAxis # GraphAxis
    cdef public object xAxisBreak, yAxisBreak, zAxisBreak # GraphAxisBreak
    cdef public float histogramBin, histogramBegin, histogramEnd, \
                       xLength, yLength, zLength, vLine, hLine
    cdef public object percentile # PercentileProperties
    cdef public object colorScale # ColorScale
    cdef public object texts, pieTexts, lines, figures, bitmaps, curves
    cdef public bool imageProfileTool, isXYY3D
    
    cdef void copy(self, const objects.GraphLayer *arg):
        self.clientRect = makeRect(arg.clientRect)
        self.legend = makeTextBox(arg.legend)
        self.backgroundColor = makeColor(arg.backgroundColor)
        self.borderType = <int>arg.borderType
        self.xAxis = makeGraphAxis(arg.xAxis)
        self.yAxis = makeGraphAxis(arg.yAxis)
        self.zAxis = makeGraphAxis(arg.zAxis)
        self.xAxisBreak = makeGraphAxisBreak(arg.xAxisBreak)
        self.yAxisBreak = makeGraphAxisBreak(arg.yAxisBreak)
        self.zAxisBreak = makeGraphAxisBreak(arg.zAxisBreak)
        self.histogramBin = arg.histogramBin
        self.histogramBegin = arg.histogramBegin
        self.histogramEnd = arg.histogramEnd
        self.vLine = arg.vLine
        self.hLine = arg.hLine
        self.percentile = makePercentileProperties(arg.percentile)
        self.colorScale = makeColorScale(arg.colorScale)
        self.texts    = [makeTextBox   (arg.texts[i]   ) for i in xrange(arg.texts.size()   )]
        self.pieTexts = [makeTextBox   (arg.pieTexts[i]) for i in xrange(arg.pieTexts.size())]
        self.lines    = [makeLine      (arg.lines[i]   ) for i in xrange(arg.lines.size()   )]
        self.figures  = [makeFigure    (arg.figures[i] ) for i in xrange(arg.figures.size() )]
        self.bitmaps  = [makeBitmap    (arg.bitmaps[i] ) for i in xrange(arg.bitmaps.size() )]
        self.curves   = [makeGraphCurve(arg.curves[i]  ) for i in xrange(arg.curves.size()  )]
        self.imageProfileTool = arg.imageProfileTool
        self.isXYY3D = arg.isXYY3D


cdef class GraphLayerRange:
    
    cdef public float min, max, step
    
    cdef void copy(self, const objects.GraphLayerRange *arg):
        self.min = arg.min
        self.max = arg.max
        self.step = arg.step

cdef class Graph(Window):
    
    cdef public object layers
    cdef public int width, height
    cdef public bool is3D, isLayout
   
    cdef void copy(self, const objects.Window *wnd):
        Window.copy(self, wnd)
        cdef const objects.Graph *grph = <objects.Graph*>wnd
        self.layers = [makeGraphLayer(grph.layers[i]) for i in xrange(grph.layers.size())]
        self.width = grph.width
        self.height = grph.height
        self.is3D = grph.is3D
        self.isLayout = grph.isLayout

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

cdef makeColor(const objects.Color &arg):
    result = Color()
    result.copy(&arg)
    return result
    #if clr.cType == objects.ctRegular: # TODO: implement commented colors
    #    result = [Qt.black, Qt.red, Qt.green, Qt.blue, Qt.cyan, Qt.magenta, Qt.yellow, Qt.darkYellow,
    #           "Qt.navy", "Qt.purple", "Qt.wine", "Qt.olive", Qt.darkCyan, "Qt.royal", "Qt.orange",
    #           "Qt.violet", "Qt.pink", Qt.white, Qt.lightGray, Qt.gray, "Qt.lightYellow",
    #           "Qt.lightCyan", "Qt.lightMagenta", Qt.darkGray][clr.regular] # TODO: check which byte is used
    #elif clr.cType == objects.ctRGB:
    #    result = QtGui.QColor(clr.custom[0], clr.custom[1], clr.custom[2])
    #elif clr.cType == objects.ctNone:
    #    result = None
    #elif clr.cType == objects.ctAutomatic:
    #    result = "auto"
    #else: # TODO: other color objects
    #    print "Unimplemented color: %d" % <int>clr.cType
    #    result = QtGui.QColor()
    #return result

cdef makeRect(const objects.Rect &arg):
    result = Rect()
    result.copy(&arg)
    return result
    #return QtCore.QRect(QtCore.QPoint(arg.left, arg.top), QtCore.QPoint(arg.right, arg.bottom))

cdef makeColorMapLevel(const objects.ColorMapLevel &lvl):
    result = ColorMapLevel()
    result.copy(&lvl)
    return result

cdef makeColorMap(const objects.ColorMap &mp):
    result = ColorMap()
    result.copy(&mp)
    return result

cdef makeSpreadColumn(const objects.SpreadColumn &col):
    result = SpreadColumn()
    result.copy(&col)
    return result

cdef makeSpreadSheet(const objects.SpreadSheet &sht):
    result = SpreadSheet()
    result.copy(&sht)
    print "Loading book #%d(%s/%s) (%d sheets, %d columns)" % (result.objectID, result.name, result.label.replace(b"\n", b""), result.sheets, len(result.columns))
    return result

cdef makeMatrix(const objects.Matrix &mtx):
    result = Matrix()
    result.copy(&mtx)
    print "Loading matrix #%d(%s/%s)" % (result.objectID, result.name, result.label.replace(b"\n", b""))
    return result

cdef makeFunction(const objects.Function &f):
    result = Function()
    result.copy(&f)
    print "Loading function #%d(%d)" % (result.index, result.name)
    return result

cdef makeTextBox(const objects.TextBox &arg):
    result = TextBox()
    result.copy(&arg)
    return result

cdef makePieProperties(const objects.PieProperties &arg):
    result = PieProperties()
    result.copy(&arg)
    return result

cdef makeVectorProperties(const objects.VectorProperties &arg):
    result = VectorProperties()
    result.copy(&arg)
    return result

cdef makeTextProperties(const objects.TextProperties &arg):
    result = TextProperties()
    result.copy(&arg)
    return result
    
cdef makeSurfaceColoration(const objects.SurfaceColoration &arg):
    result = SurfaceColoration()
    result.copy(&arg)
    return result

cdef makeSurfaceProperties(const objects.SurfaceProperties &arg):
    result = SurfaceProperties()
    result.copy(&arg)
    return result

cdef makePercentileProperties(const objects.PercentileProperties &arg):
    result = PercentileProperties()
    result.copy(&arg)
    return result

cdef makeGraphCurve(const objects.GraphCurve &arg):
    result = GraphCurve()
    result.copy(&arg)
    return result

cdef makeGraphAxisBreak(const objects.GraphAxisBreak &arg):
    result = GraphAxisBreak()
    result.copy(&arg)
    return result

cdef makeGraphGrid(const objects.GraphGrid &arg):
    result = GraphGrid()
    result.copy(&arg)
    return result

cdef makeGraphAxisFormat(const objects.GraphAxisFormat &arg):
    result = GraphAxisFormat()
    result.copy(&arg)
    return result

cdef makeGraphAxisTick(const objects.GraphAxisTick &arg):
    result = GraphAxisTick()
    result.copy(&arg)
    return result

cdef makeGraphAxis(const objects.GraphAxis &arg):
    result = GraphAxis()
    result.copy(&arg)
    return result

cdef makeFigure(const objects.Figure &arg):
    result = Figure()
    result.copy(&arg)
    return result

cdef makeLineVertex(const objects.LineVertex &arg):
    result = LineVertex()
    result.copy(&arg)
    return result

cdef makeLine(const objects.Line &arg):
    result = Line()
    result.copy(&arg)
    return result

cdef makeBitmap(const objects.Bitmap &arg):
    result = Bitmap()
    result.copy(&arg)
    return result

cdef makeColorScale(const objects.ColorScale &arg):
    result = ColorScale()
    result.copy(&arg)
    return result

cdef makeGraphLayer(const objects.GraphLayer &arg):
    result = GraphLayer()
    result.copy(&arg)
    return result

cdef makeGraphLayerRange(const objects.GraphLayerRange &arg):
    result = GraphLayerRange()
    result.copy(&arg)
    return result

cdef makeGraph(const objects.Graph &arg):
    result = Graph()
    result.copy(&arg)
    print "Loading graph #%d(%s/%s) (%d layers, 3D: %s)" % (result.objectID, result.name, result.label.replace(b"\n", b""), len(result.layers), result.is3D)
    return result

cdef makeNote(const objects.Note &nt):
    result = Note()
    result.copy(&nt)
    print "Loading note #%d(%s/%s)" % (result.objectID, result.name, result.label.replace(b"\n", b""))
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


####################################################################################################
#######################################                 ############################################
###################################### Python interface ############################################
######################################                 #############################################
####################################################################################################

cdef void cProgressCallback(double progress, void *user_data):
    pyProgressCallback = <object>user_data
    if pyProgressCallback:
        pyProgressCallback(float(progress))


cdef void tell(int &objCount, int &objHandled, void *callback):
        cython.operator.preincrement(objHandled)
        if callback != NULL:
            pyCallback = <object>callback
            pyCallback(0.9 + 0.1 * (objHandled / float(objCount)))

cdef getNodes(OriginFile *originFile, pyCallback):
    
    print "Origin file version: %g" % originFile.version()
    
    spreadCount = originFile.spreadCount()
    matrixCount = originFile.matrixCount()
    functionCount = originFile.functionCount()
    graphCount = originFile.graphCount()
    noteCount = originFile.noteCount()
    print spreadCount, matrixCount, functionCount, graphCount, noteCount
    cdef int objectsCount = spreadCount + matrixCount + functionCount + graphCount + noteCount
    cdef int objectsHandled = 0
    
    spreads, matrices, functions, graphs, notes = [], [], [], [], []
    for i in xrange(spreadCount):
        spreads.append(makeSpreadSheet(originFile.spread(i)))
        tell(objectsCount, objectsHandled, <void*>pyCallback)
    matrices  = [makeMatrix     (originFile.matrix  (i)) for i in xrange(matrixCount  )]
    objectsHandled += matrixCount
    functions = [makeFunction   (originFile.function(i)) for i in xrange(functionCount)]
    objectsHandled += functionCount
    for i in xrange(graphCount):
        graphs.append(makeGraph(originFile.graph(i)))
        tell(objectsCount, objectsHandled, <void*>pyCallback)
    notes     = [makeNote       (originFile.note    (i)) for i in xrange(noteCount    )]
    objectsHandled += noteCount
    
    #spreads   = [makeSpreadSheet(originFile.spread  (i)) for i in xrange(spreadCount  )]
    #matrices  = [makeMatrix     (originFile.matrix  (i)) for i in xrange(matrixCount  )]
    #functions = [makeFunction   (originFile.function(i)) for i in xrange(functionCount)]
    #graphs    = [makeGraph      (originFile.graph   (i)) for i in xrange(graphCount   )]
    #notes     = [makeNote       (originFile.note    (i)) for i in xrange(noteCount    )]
    #cdef const tree[objects.ProjectNode] *project = originFile.project()
    #tree = makeTree(project)
    return {'functions': functions,
             'matrices': matrices,
              'spreads': spreads,
               'graphs': graphs,
                'notes': notes}
    

def parseOriginFile(filename, pyCallback=None):
    cdef OriginFile *originFile = new OriginFile(str.encode(filename))
    cdef objects.ProgressCallback cCallback = cProgressCallback
    cdef void *pyCallbackPtr = <void*>pyCallback
    result = originFile.parse(cCallback, pyCallbackPtr) # FIXME: something is wrong here
    return result and getNodes(originFile, pyCallback)




