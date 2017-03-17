from libcpp.pair cimport pair
from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp cimport bool
cdef extern from "<time.h>":
    ctypedef long time_t

cdef extern from "boost/variant.hpp" namespace "boost":
    cdef cppclass boostvariant "variant" [T1, T2]:
        pass

cdef extern from "tree.hh":
    cdef cppclass tree_node "tree_node_" [T]:
        pass
    
    cdef cppclass tree[T]:
        
        cppclass iterator_base:
            T& operator*() const
            tree_node[T] *node
            unsigned int number_of_children() const
            iterator_base()
            iterator_base(tree_node[T] *)
        
        cppclass leaf_iterator(iterator_base):
            bool    operator==(const leaf_iterator&) const
            bool    operator!=(const leaf_iterator&) const
            leaf_iterator&  operator++()
            leaf_iterator&  operator--()
            leaf_iterator   operator++(int)
            leaf_iterator   operator--(int)
            leaf_iterator(tree_node[T] *node): iterator_base(node)
        
        cppclass pre_order_iterator(iterator_base):
            bool    operator==(const pre_order_iterator&) const
            bool    operator!=(const pre_order_iterator&) const
            pre_order_iterator&  operator++()
            pre_order_iterator&  operator--()
            pre_order_iterator   operator++(int)
            pre_order_iterator   operator--(int)
        
        inline pre_order_iterator   begin() const
        inline pre_order_iterator   end() const
        leaf_iterator   begin_leaf() const
        leaf_iterator   end_leaf() const
        int size() const


####################################################################################################
############################################            ############################################
########################################### OriginObj.h ############################################
###########################################            #############################################
####################################################################################################
    
cdef enum Color_ColorType:
        ctNone, ctAutomatic, ctRegular, ctCustom, ctIncrement, ctIndexing, ctRGB, ctMapping
cdef enum Color_RegularColor:
        Black = 0, Red = 1, Green = 2, Blue = 3, Cyan = 4, Magenta = 5, Yellow = 6, DarkYellow = 7, Navy = 8,
        Purple = 9, Wine = 10, Olive = 11, DarkCyan = 12, Royal=  13, Orange = 14, Violet = 15, Pink = 16, White = 17,
        LightGray = 18, Gray = 19, LTYellow = 20, LTCyan = 21, LTMagenta = 22, DarkGray = 23#, Custom = 255
cdef enum Window_State:
        wsNormal, wsMinimized, wsMaximized
cdef enum Window_Title:
        wtName, wtLabel, wtBoth
cdef enum SpreadColumn_ColumnType:
        ctX, ctY, ctZ, ctXErr, ctYErr, ctLabel, ctNONE
cdef enum VectorProperties_VectorPosition:
        vpTail, vpMidpoint, vpHead
cdef enum Matrix_ViewType:
        vtDataView, vtImageView
cdef enum Matrix_HeaderViewType:
        hvtColumnRow, hvtXY
cdef enum Function_FunctionType:
        ftNormal, ftPolar
cdef enum TextProperties_Justify:
        jLeft, jCenter, jRight
cdef enum SurfaceProperties_Type: # WTF, it is not used
        ColorMap3D, ColorFill, WireFrame, Bars
cdef enum SurfaceProperties_Grids:
        gNone, X, Y, XY
# WTF, the following 3 enums are not used
cdef enum GraphCurve_Plot:
        pLine "Line" = 200, Scatter=201, LineSymbol=202, Column = 203, Area = 204, HiLoClose = 205, Box = 206,
        ColumnFloat = 207, Vector = 208, PlotDot = 209, Wall3D = 210, Ribbon3D = 211, Bar3D = 212, ColumnStack = 213,
        AreaStack = 214, Bar = 215, BarStack = 216, FlowVector = 218, Histogram = 219, MatrixImage = 220, Pie = 225,
        Contour = 226, Unknown = 230, ErrorBar = 231, TextPlot = 232, XErrorBar = 233, SurfaceColorMap = 236,
        SurfaceColorFill = 237, SurfaceWireframe = 238, SurfaceBars = 239, Line3D = 240, Text3D = 241, Mesh3D = 242,
        XYZContour = 243, XYZTriangular = 245, LineSeries = 246, YErrorBar = 254, XYErrorBar = 255, GraphScatter3D = 0x8AF0,
        GraphTrajectory3D = 0x8AF1, Polar = 0x00020000, SmithChart = 0x00040000, FillArea = 0x00800000
cdef enum GraphCurve_LineStyle:
        Solid = 0, Dash = 1, Dot = 2, DashDot = 3, DashDotDot = 4, ShortDash = 5, ShortDot = 6, ShortDashDot = 7
cdef enum GraphCurve_LineConnect:
        NoLine = 0, Straight = 1, TwoPointSegment = 2, ThreePointSegment = 3, BSpline = 8, Spline = 9,
        StepHorizontal = 11, StepVertical = 12, StepHCenter = 13, StepVCenter = 14, Bezier = 15
cdef enum GraphAxis_AxisPosition:
        apLeft = 0, apBottom, apRight, apTop, apFront, apBack
cdef enum GraphAxis_Scale: # WTF, this enum is not used
        Linear = 0, Log10 = 1, Probability = 2, Probit = 3, Reciprocal = 4,
        OffsetReciprocal = 5, Logit = 6, Ln = 7, Log2 = 8
cdef enum Figure_FigureType:
        Rectangle, Circle
cdef enum ProjectNode_NodeType:
        ntSpreadSheet "Spreadsheet", ntMatrix "Matrix", ntExcel "Excel",
        ntGraph "Graph", ntGraph3D "Graph3D", ntNote "Note", ntFolder "Folder"

cdef extern from "OriginObj.h" namespace "Origin":
    
    cdef enum ValueType:
         Numeric = 0, Text = 1, Time = 2, Date = 3,  Month = 4, Day = 5, ColumnHeading = 6,
         TickIndexedDataset = 7, TextNumeric = 9, Categorical = 10
    cdef enum NumericDisplayType:
         DefaultDecimalDigits = 0, DecimalPlaces = 1, SignificantDigits = 2
    cdef enum Attach:
         Frame = 0, Page = 1, Scale = 2
    cdef enum BorderType:
         BlackLine = 0, Shadow = 1, DarkMarble = 2, WhiteOut = 3, BlackOut = 4, btNone "None" = -1
    cdef enum FillPattern:
        NoFill, BDiagDense, BDiagMedium, BDiagSparse, FDiagDense, FDiagMedium, FDiagSparse, 
        DiagCrossDense, DiagCrossMedium, DiagCrossSparse, HorizontalDense, HorizontalMedium, HorizontalSparse, 
        VerticalDense, VerticalMedium, VerticalSparse, CrossDense, CrossMedium, CrossSparse
    
    cdef cppclass Color:
        Color_ColorType type
        
        unsigned char regular  #
        unsigned char *custom  # this is a union actually
        unsigned char starting #
        unsigned char column   #

    cdef cppclass Rect:
        short left
        short top
        short right
        short bottom
        Rect(short width = 0, short height = 0)
        int height() const
        int width() const
        bool isValid() const
    
    cdef cppclass ColorMapLevel:
        Color fillColor
        unsigned char fillPattern
        Color fillPatternColor
        double fillPatternLineWidth
        bool lineVisible
        Color lineColor
        unsigned char lineStyle
        double lineWidth
        bool labelVisible

    ctypedef vector[pair[double, ColorMapLevel]] ColorMapVector

    cdef cppclass ColorMap:
        bool fillEnabled
        ColorMapVector levels
    
    cdef cppclass Window:
        string name
        string label
        int objectID
        bool hidden
        Window_State state
        Window_Title title
        Rect frameRect
        time_t creationDate
        time_t modificationDate
        Window(const string& _name= "", const string& _label = "", bool _hidden = false)
    
    ctypedef boostvariant[double, string] variant
    const double *getDoubleFromVariant(const variant *v)
    const string *getStringFromVariant(const variant *v)
    
    cdef cppclass SpreadColumn:
        string name
        SpreadColumn_ColumnType type
        ValueType valueType
        int valueTypeSpecification
        int significantDigits
        int decimalPlaces
        NumericDisplayType numericDisplayType
        string command
        string comment
        int width
        unsigned int index
        unsigned int colIndex
        unsigned int sheet
        vector[variant] data
        SpreadColumn(const string& _name = "", unsigned int _index = 0)
    
    cdef cppclass SpreadSheet(Window):
        unsigned int maxRows
        bool loose
        unsigned int sheets
        vector[SpreadColumn] columns
        #SpreadSheet(const string& _name = "")
    
    cdef cppclass Excel(Window): # WTF, is this class necessary?
        unsigned int maxRows
        bool loose
        vector[SpreadSheet] sheets
        #Excel(const string& _name = "", const string& _label = "", int _maxRows = 0, bool _hidden = false, bool _loose = true)
    
    cdef cppclass MatrixSheet:
        string name
        unsigned short rowCount
        unsigned short columnCount
        int valueTypeSpecification
        int significantDigits
        int decimalPlaces
        NumericDisplayType numericDisplayType
        string command
        unsigned short width
        unsigned int index
        Matrix_ViewType view
        ColorMap colorMap
        vector[double] data
        vector[double] coordinates
        #Matrix(const string& _name = "", unsigned int _index = 0)
    
    cdef cppclass Matrix(Window):
        Matrix_HeaderViewType header
        unsigned int activeSheet
        vector[MatrixSheet] sheets

    cdef cppclass Function:
        string name
        Function_FunctionType type
        string formula
        double begin
        double end
        int totalPoints
        unsigned int index
        #Function(const string& _name = "", unsigned int _index = 0)

    cdef cppclass TextBox:
        string text
        Rect clientRect
        Color color
        unsigned short fontSize
        int rotation
        int tab
        BorderType borderType
        Attach attach
        #TextBox(const string& _text = "")
        #TextBox(const string& _text, const Rect& _clientRect, const Color& _color, unsigned short _fontSize, int _rotation, int _tab, BorderType _borderType, Attach _attach)
    
    cdef cppclass PieProperties:
        unsigned char viewAngle
        unsigned char thickness
        bool clockwiseRotation
        short rotation
        unsigned short radius
        unsigned short horizontalOffset
        unsigned long displacedSectionCount
        unsigned short displacement
        bool formatAutomatic
        bool formatValues
        bool formatPercentages
        bool formatCategories
        bool positionAssociate
        unsigned short distance
        PieProperties()
    
    cdef cppclass VectorProperties:
        Color color
        double width
        unsigned short arrowLenght
        unsigned char arrowAngle
        bool arrowClosed
        string endXColumnName
        string endYColumnName
        VectorProperties_VectorPosition position
        string angleColumnName
        string magnitudeColumnName
        float multiplier
        int constAngle
        int constMagnitude
        VectorProperties()

    cdef cppclass TextProperties:
        Color color
        bool fontBold
        bool fontItalic
        bool fontUnderline
        bool whiteOut
        TextProperties_Justify justify
        short rotation
        short xOffset
        short yOffset
        unsigned short fontSize
    
    cdef cppclass SurfaceProperties:
        cppclass SurfaceColoration:
            bool fill
            bool contour
            Color lineColor
            double lineWidth
        unsigned char type
        SurfaceProperties_Grids grids
        double gridLineWidth
        Color gridColor
        bool backColorEnabled
        Color frontColor
        Color backColor
        bool sideWallEnabled
        Color xSideWallColor
        Color ySideWallColor
        SurfaceColoration surface
        SurfaceColoration topContour
        SurfaceColoration bottomContour
        ColorMap colorMap

    cdef cppclass PercentileProperties:
        unsigned char maxSymbolType
        unsigned char p99SymbolType
        unsigned char meanSymbolType
        unsigned char p1SymbolType
        unsigned char minSymbolType
        Color symbolColor
        Color symbolFillColor
        unsigned short symbolSize
        unsigned char boxRange
        unsigned char whiskersRange
        double boxCoeff
        double whiskersCoeff
        bool diamondBox
        unsigned char labels
    
    cdef cppclass GraphCurve:
        bool hidden
        unsigned char type
        string dataName
        string xDataName
        string xColumnName
        string yColumnName
        string zColumnName
        Color lineColor
        unsigned char lineTransparency
        unsigned char lineStyle
        unsigned char lineConnect
        unsigned char boxWidth
        double lineWidth
        bool fillArea
        unsigned char fillAreaType
        unsigned char fillAreaPattern
        Color fillAreaColor
        unsigned char fillAreaTransparency
        bool fillAreaWithLineTransparency
        Color fillAreaPatternColor
        double fillAreaPatternWidth
        unsigned char fillAreaPatternBorderStyle
        Color fillAreaPatternBorderColor
        double fillAreaPatternBorderWidth
        unsigned short symbolType
        Color symbolColor
        Color symbolFillColor
        unsigned char symbolFillTransparency
        double symbolSize
        unsigned char symbolThickness
        unsigned char pointOffset
        bool connectSymbols
        PieProperties pie
        VectorProperties vector
        TextProperties text
        SurfaceProperties surface
        ColorMap colorMap

    cdef cppclass GraphAxisBreak:
        bool show
        bool log10
        double gabFrom "from"
        double to
        double position
        double scaleIncrementBefore
        double scaleIncrementAfter
        unsigned char minorTicksBefore
        unsigned char minorTicksAfter
        GraphAxisBreak()

    cdef cppclass GraphGrid:
        bool hidden
        unsigned char color
        unsigned char style
        double width

    cdef cppclass GraphAxisFormat:
        bool hidden
        unsigned char color
        double thickness
        double majorTickLength
        int majorTicksType
        int minorTicksType
        int axisPosition
        double axisPositionValue
        TextBox label
        string prefix
        string suffix
        string factor

    cdef cppclass GraphAxisTick:
        bool showMajorLabels
        unsigned char color
        ValueType valueType
        int valueTypeSpecification
        int decimalPlaces
        unsigned short fontSize
        bool fontBold
        string dataName
        string columnName
        int rotation

    cdef cppclass GraphAxis:
        GraphAxis_AxisPosition position
        bool zeroLine
        bool oppositeLine
        double min
        double max
        double step
        unsigned char majorTicks
        unsigned char minorTicks
        unsigned char scale
        GraphGrid majorGrid
        GraphGrid minorGrid
        GraphAxisFormat *formatAxis # this is actually a size-2 array
        GraphAxisTick *tickAxis # this is actually a size-2 array

    cdef cppclass Figure:
        Figure_FigureType type
        Rect clientRect
        Attach attach
        Color color
        unsigned char style
        double width
        Color fillAreaColor
        unsigned char fillAreaPattern
        Color fillAreaPatternColor
        double fillAreaPatternWidth
        bool useBorderColor
        Figure(Figure_FigureType _type = Rectangle)

    cdef cppclass LineVertex:
        unsigned char shapeType
        double shapeWidth
        double shapeLength
        double x
        double y
        LineVertex()

    cdef cppclass Line:
        Rect clientRect
        Color color
        Attach attach
        double width
        unsigned char style
        LineVertex begin
        LineVertex end

    cdef cppclass Bitmap:
        Rect clientRect
        Attach attach
        unsigned long size
        string windowName
        BorderType borderType
        unsigned char* data
        Bitmap(const string& _name = "")
        Bitmap(const Bitmap& bitmap)

    cdef cppclass ColorScale:
        bool visible
        bool reverseOrder
        unsigned short labelGap
        unsigned short colorBarThickness
        Color labelsColor

    cdef cppclass GraphLayer:
        Rect clientRect
        TextBox legend
        Color backgroundColor
        BorderType borderType
        GraphAxis xAxis
        GraphAxis yAxis
        GraphAxis zAxis
        GraphAxisBreak xAxisBreak
        GraphAxisBreak yAxisBreak
        GraphAxisBreak zAxisBreak
        double histogramBin
        double histogramBegin
        double histogramEnd
        PercentileProperties percentile
        ColorScale colorScale
        vector[TextBox] texts
        vector[TextBox] pieTexts
        vector[Line] lines
        vector[Figure] figures
        vector[Bitmap] bitmaps
        vector[GraphCurve] curves
        float xAngle
        float yAngle
        float zAngle
        float xLength
        float yLength
        float zLength
        int imageProfileTool
        double vLine
        double hLine
        bool isWaterfall
        int xOffset
        int yOffset
        bool gridOnTop
        bool exchangedAxes
        bool isXYY3D
        bool orthographic3D
        GraphLayer()
        #bool threeDimensional
        bool is3D() const

    cdef cppclass GraphLayerRange:
        double min
        double max
        double step
        GraphLayerRange(double _min = 0.0, double _max = 0.0, double _step = 0.0)

    cdef cppclass Graph(Window):
        vector[GraphLayer] layers
        unsigned short width
        unsigned short height
        bool is3D
        bool isLayout
        bool connectMissingData
        string templateName
        #Graph(const string& _name = "")

    cdef cppclass Note(Window):
        string text
        #Note(const string& _name = "")
    
    cdef cppclass ProjectNode:
        ProjectNode_NodeType type
        string name
        time_t creationDate
        time_t modificationDate
        bool active
        ProjectNode(const string& _name = "", ProjectNode_NodeType _type = Folder, const time_t _creationDate = time(NULL), const time_t _modificationDate = time(NULL))

cdef extern from "OriginObj.h":
    ctypedef void (*ProgressCallback)(double progress, void *user_data)

ctypedef SurfaceProperties.SurfaceColoration SurfaceColoration

####################################################################################################
###########################################             ############################################
########################################## OriginFile.h ############################################
##########################################             #############################################
####################################################################################################
cdef extern from "OriginFile.h":
    cdef cppclass OriginFile:
        OriginFile(const string& fileName) nogil

        bool parse(ProgressCallback callback, void *user_data) nogil
        double version() const
        
        int spreadCount() const
        int matrixCount() const
        int functionCount() const
        int graphCount() const
        int noteCount() const
        
        SpreadSheet& spread(int s) const
        Matrix& matrix(int m) const
        Function& function(int f) const
        Graph& graph(int g) const
        Note& note(int n) const
        
        int functionIndex(const string& name) const
        
        const tree[ProjectNode]* project() const

