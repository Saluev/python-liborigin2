/***************************************************************************
    File                 : OriginObj.h
    --------------------------------------------------------------------
    Copyright            : (C) 2005-2007 Stefan Gerlach
                           (C) 2007-2008 Alex Kargovsky, Ion Vasilief
    Email (use @ for *)  : kargovsky*yumr.phys.msu.su, ion_vasilief*yahoo.fr
    Description          : Origin internal object classes

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/


#ifndef ORIGIN_OBJ_H
#define ORIGIN_OBJ_H

#include <cstring>
#include <ctime>
#include <vector>
#include "boost/variant.hpp"

using namespace std;

#define	_ONAN		(-1.23456789E-300)

namespace Origin
{

    typedef void (*ParsingProgressCallback)(double progress, void *user_data);

	enum ValueType {Numeric = 0, Text = 1, Time = 2, Date = 3,  Month = 4, Day = 5, ColumnHeading = 6, TickIndexedDataset = 7, TextNumeric = 9, Categorical = 10};
	enum NumericDisplayType {DefaultDecimalDigits = 0, DecimalPlaces = 1, SignificantDigits = 2};
	enum Attach {Frame = 0, Page = 1, Scale = 2};
	enum BorderType {BlackLine = 0, Shadow = 1, DarkMarble = 2, WhiteOut = 3, BlackOut = 4, None = -1};
	enum FillPattern {NoFill, BDiagDense, BDiagMedium, BDiagSparse, FDiagDense, FDiagMedium, FDiagSparse, 
		DiagCrossDense, DiagCrossMedium, DiagCrossSparse, HorizontalDense, HorizontalMedium, HorizontalSparse, 
		VerticalDense, VerticalMedium, VerticalSparse, CrossDense, CrossMedium, CrossSparse};

	struct Color
	{
		enum ColorType {None, Automatic, Regular, Custom, Increment, Indexing, RGB, Mapping};
		enum RegularColor {Black = 0, Red = 1, Green = 2, Blue = 3, Cyan = 4, Magenta = 5, Yellow = 6, DarkYellow = 7, Navy = 8,
			Purple = 9, Wine = 10, Olive = 11, DarkCyan = 12, Royal=  13, Orange = 14, Violet = 15, Pink = 16, White = 17,
			LightGray = 18, Gray = 19, LTYellow = 20, LTCyan = 21, LTMagenta = 22, DarkGray = 23/*, Custom = 255*/};

		ColorType type;
		union
		{
			unsigned char regular;
			unsigned char custom[3];
			unsigned char starting;
			unsigned char column;
		};
	};

	struct Rect
	{
		short left;
		short top;
		short right;
		short bottom;

		Rect(short width = 0, short height = 0)
		:	left(0)
		,	top(0)
		,	right(width)
		,	bottom(height)
		{
		};

		int height() const
		{
			return bottom - top;
		};

		int width() const
		{
			return right - left;
		};

		bool isValid() const
		{
			return height() > 0 && width() > 0;
		}
	};

	struct ColorMapLevel
	{
		Color fillColor;
		unsigned char fillPattern;
		Color fillPatternColor;
		double fillPatternLineWidth;

		bool lineVisible;
		Color lineColor;
		unsigned char lineStyle;
		double lineWidth;

		bool labelVisible;
	};

	typedef vector<pair<double, ColorMapLevel> > ColorMapVector;

	struct ColorMap
	{
		bool fillEnabled;
		ColorMapVector levels;
	};

	struct Window
	{
		enum State {Normal, Minimized, Maximized};
		enum Title {Name, Label, Both};

		string name;
		string label;
		int objectID;
		bool hidden;
		State state;
		Title title;
		Rect frameRect;
		time_t creationDate;
		time_t modificationDate;

		Window(const string& _name= "", const string& _label = "", bool _hidden = false)
		:	name(_name)
		,	label(_label)
		,	objectID(-1)
		,	hidden(_hidden)
		,	state(Normal)
		,	title(Both)
		{};
	};

	typedef boost::variant<double, string> variant;
	/* Two following functions are necessary because
	   Boost is way too difficult for Cython.
	   BTW, I had to add new .cpp file for their implementation. :[ */
	const double *getDoubleFromVariant(const variant *v);
	const string *getStringFromVariant(const variant *v);

	struct SpreadColumn
	{
		enum ColumnType {X, Y, Z, XErr, YErr, Label, NONE};

		string name;
		ColumnType type;
		ValueType valueType;
		int valueTypeSpecification;
		int significantDigits;
		int decimalPlaces;
		NumericDisplayType numericDisplayType;
		string command;
		string comment;
		int width;
		unsigned int index;
		unsigned int colIndex;
		unsigned int sheet;
		vector<variant> data;

		SpreadColumn(const string& _name = "", unsigned int _index = 0)
		:	name(_name)
		,	valueType(Numeric)
		,	valueTypeSpecification(0)
		,	significantDigits(6)
		,	decimalPlaces(6)
		,	numericDisplayType(DefaultDecimalDigits)
		,	command("")
		,	comment("")
		,	width(8)
		,	index(_index)
		,	colIndex(0)
		,	sheet(0)
		{};
	};

	struct SpreadSheet : public Window
	{
		unsigned int maxRows;
		bool loose;
		unsigned int sheets;
		vector<SpreadColumn> columns;

		SpreadSheet(const string& _name = "")
		:	Window(_name)
		,	loose(true)
		,	sheets(1)
		{};
	};

	struct Excel : public Window
	{
		unsigned int maxRows;
		bool loose;
		vector<SpreadSheet> sheets;

		Excel(const string& _name = "", const string& _label = "", int _maxRows = 0, bool _hidden = false, bool _loose = true)
		:	Window(_name, _label, _hidden)
		,	maxRows(_maxRows)
		,	loose(_loose)
		{
		};
	};

	struct MatrixSheet
	{
		enum ViewType {DataView, ImageView};

		string name;
		unsigned short rowCount;
		unsigned short columnCount;
		int valueTypeSpecification;
		int significantDigits;
		int decimalPlaces;
		NumericDisplayType numericDisplayType;
		string command;
		unsigned short width;
		unsigned int index;
		ViewType view;
		ColorMap colorMap;
		vector<double> data;
		vector<double> coordinates;

		MatrixSheet(const string& _name = "", unsigned int _index = 0)
		:	name(_name)
		,	valueTypeSpecification(0)
		,	significantDigits(6)
		,	decimalPlaces(6)
		,	numericDisplayType(DefaultDecimalDigits)
		,	command("")
		,	width(8)
		,	index(_index)
		,	view(DataView)
		{coordinates.push_back(10.0);coordinates.push_back(10.0);coordinates.push_back(1.0);coordinates.push_back(1.0);};
	};

	struct Matrix : public Window
	{
		enum HeaderViewType {ColumnRow, XY};

		unsigned int activeSheet;
		HeaderViewType header;
		vector<MatrixSheet> sheets;

		Matrix(const string& _name = "")
		:	Window(_name)
		,	activeSheet(0)
		,	header(ColumnRow)
		{};
	};

	struct Function
	{
		enum FunctionType {Normal, Polar};

		string name;
		FunctionType type;
		string formula;
		double begin;
		double end;
		int totalPoints;
		unsigned int index;

		Function(const string& _name = "", unsigned int _index = 0)
		:	name(_name)
		,	type(Normal)
		,	formula("")
		,	begin(0.0)
		,	end(0.0)
		,	totalPoints(0)
		,	index(_index)
		{};
	};


	struct TextBox
	{
		string text;
		Rect clientRect;
		Color color;
		unsigned short fontSize;
		int rotation;
		int tab;
		BorderType borderType;
		Attach attach;

		TextBox(const string& _text = "")
		:	text(_text)
		{};

		TextBox(const string& _text, const Rect& _clientRect, const Color& _color, unsigned short _fontSize, int _rotation, int _tab, BorderType _borderType, Attach _attach)
		:	text(_text)
		,	clientRect(_clientRect)
		,	color(_color)
		,	fontSize(_fontSize)
		,	rotation(_rotation)
		,	tab(_tab)
		,	borderType(_borderType)
		,	attach(_attach)
		{};
	};

	struct PieProperties
	{
		unsigned char viewAngle;
		unsigned char thickness;
		bool clockwiseRotation;
		short rotation;
		unsigned short radius;
		unsigned short horizontalOffset;
		unsigned long displacedSectionCount; // maximum - 32 sections
		unsigned short displacement;
		
		//labels
		bool formatAutomatic;
		bool formatValues;
		bool formatPercentages;
		bool formatCategories;
		bool positionAssociate;
		unsigned short distance;

		PieProperties()
		:	clockwiseRotation(false)
		,	formatAutomatic(false)
		,	formatValues(false)
		,	formatPercentages(false)
		,	formatCategories(false)
		,	positionAssociate(false)
		{};
	};

	struct VectorProperties
	{
		enum VectorPosition {Tail, Midpoint, Head};

		Color color;
		double width;
		unsigned short arrowLenght;
		unsigned char arrowAngle;
		bool arrowClosed;
		string endXColumnName;
		string endYColumnName;

		VectorPosition position;
		string angleColumnName;
		string magnitudeColumnName;
		float multiplier;
		int constAngle;
		int constMagnitude;

		VectorProperties()
		:	arrowClosed(false)
		,	position(Tail)
		,	multiplier(1.0)
		,	constAngle(0)
		,	constMagnitude(0)
		{};
	};

	struct TextProperties
	{
		enum Justify {Left, Center, Right};

		Color color;
		bool fontBold;
		bool fontItalic;
		bool fontUnderline;
		bool whiteOut;
		Justify justify;

		short rotation;
		short xOffset;
		short yOffset;
		unsigned short fontSize;
	};

	struct SurfaceProperties
	{
		struct SurfaceColoration
		{
			bool fill;
			bool contour;
			Color lineColor;
			double lineWidth;
		};

		enum Type {ColorMap3D, ColorFill, WireFrame, Bars};
		enum Grids {None, X, Y, XY};

		unsigned char type;
		Grids grids;
		double gridLineWidth;
		Color gridColor;

		bool backColorEnabled;
		Color frontColor;
		Color backColor;

		bool sideWallEnabled;
		Color xSideWallColor;
		Color ySideWallColor;

		SurfaceColoration surface;
		SurfaceColoration topContour;
		SurfaceColoration bottomContour;

		ColorMap colorMap;
	};

	struct PercentileProperties
	{
		unsigned char maxSymbolType;
		unsigned char p99SymbolType;
		unsigned char meanSymbolType;
		unsigned char p1SymbolType;
		unsigned char minSymbolType;
		Color symbolColor;
		Color symbolFillColor;
		unsigned short symbolSize;
		unsigned char boxRange;
		unsigned char whiskersRange;
		double boxCoeff;
		double whiskersCoeff;
		bool diamondBox;
		unsigned char labels;
	};

	struct GraphCurve
	{
		enum Plot {Line = 200, Scatter=201, LineSymbol=202, Column = 203, Area = 204, HiLoClose = 205, Box = 206,
			ColumnFloat = 207, Vector = 208, PlotDot = 209, Wall3D = 210, Ribbon3D = 211, Bar3D = 212, ColumnStack = 213,
			AreaStack = 214, Bar = 215, BarStack = 216, FlowVector = 218, Histogram = 219, MatrixImage = 220, Pie = 225,
			Contour = 226, Unknown = 230, ErrorBar = 231, TextPlot = 232, XErrorBar = 233, SurfaceColorMap = 236,
			SurfaceColorFill = 237, SurfaceWireframe = 238, SurfaceBars = 239, Line3D = 240, Text3D = 241, Mesh3D = 242,
			XYZContour = 243, XYZTriangular = 245, LineSeries = 246, YErrorBar = 254, XYErrorBar = 255, GraphScatter3D = 0x8AF0,
			GraphTrajectory3D = 0x8AF1, Polar = 0x00020000, SmithChart = 0x00040000, FillArea = 0x00800000};
		enum LineStyle {Solid = 0, Dash = 1, Dot = 2, DashDot = 3, DashDotDot = 4, ShortDash = 5, ShortDot = 6, ShortDashDot = 7};
		enum LineConnect {NoLine = 0, Straight = 1, TwoPointSegment = 2, ThreePointSegment = 3, BSpline = 8, Spline = 9, StepHorizontal = 11, StepVertical = 12, StepHCenter = 13, StepVCenter = 14, Bezier = 15};

		bool hidden;
		unsigned char type;
		string dataName;
		string xDataName;
		string xColumnName;
		string yColumnName;
		string zColumnName;
		Color lineColor;
		unsigned char lineTransparency;
		unsigned char lineStyle;
		unsigned char lineConnect;
		unsigned char boxWidth;
		double lineWidth;

		bool fillArea;
		unsigned char fillAreaType;
		unsigned char fillAreaPattern;
		Color fillAreaColor;
		unsigned char fillAreaTransparency;
		bool fillAreaWithLineTransparency;
		Color fillAreaPatternColor;
		double fillAreaPatternWidth;
		unsigned char fillAreaPatternBorderStyle;
		Color fillAreaPatternBorderColor;
		double fillAreaPatternBorderWidth;

		unsigned short symbolType;
		Color symbolColor;
		Color symbolFillColor;
		unsigned char symbolFillTransparency;
		double symbolSize;
		unsigned char symbolThickness;
		unsigned char pointOffset;

		bool connectSymbols;

		//pie
		PieProperties pie;

		//vector
		VectorProperties vector;

		//text
		TextProperties text;

		//surface
		SurfaceProperties surface;

		//contour
		ColorMap colorMap;
	};

	struct GraphAxisBreak
	{
		bool show;

		bool log10;
		double from;
		double to;
		double position;

		double scaleIncrementBefore;
		double scaleIncrementAfter;

		unsigned char minorTicksBefore;
		unsigned char minorTicksAfter;

		GraphAxisBreak()
		:	show(false)
		{};
	};

	struct GraphGrid
	{
		bool hidden;
		unsigned char color;
		unsigned char style;
		double width;
	};

	struct GraphAxisFormat
	{
		bool hidden;
		unsigned char color;
		double thickness;
		double majorTickLength;
		int majorTicksType;
		int minorTicksType;
		int axisPosition;
		double axisPositionValue;
		TextBox label;
		string prefix;
		string suffix;
		string factor;
	};

	struct GraphAxisTick
	{
		bool showMajorLabels;
		unsigned char color;
		ValueType valueType;
		int valueTypeSpecification; 
		int decimalPlaces;
		unsigned short fontSize;
		bool fontBold;
		string dataName;
		string columnName;
		int rotation;
	};

	struct GraphAxis
	{
		enum AxisPosition {Left = 0, Bottom, Right, Top, Front, Back};
		enum Scale {Linear = 0, Log10 = 1, Probability = 2, Probit = 3, Reciprocal = 4, OffsetReciprocal = 5, Logit = 6, Ln = 7, Log2 = 8};

		AxisPosition position;
		bool zeroLine;
		bool oppositeLine;
		double min;
		double max;
		double step;
		unsigned char majorTicks;
		unsigned char minorTicks;
		unsigned char scale;
		GraphGrid majorGrid;
		GraphGrid minorGrid;
		GraphAxisFormat formatAxis[2];
		GraphAxisTick tickAxis[2]; //bottom-top, left-right
	};

	struct Figure
	{
		enum FigureType {Rectangle, Circle};

		FigureType type;
		Rect clientRect;
		Attach attach;
		Color color;
		unsigned char style;
		double width;
		Color fillAreaColor;
		unsigned char fillAreaPattern;
		Color fillAreaPatternColor;
		double fillAreaPatternWidth;
		bool useBorderColor;

		Figure(FigureType _type = Rectangle)
		:	type(_type)
		{
		};
	};

	struct LineVertex
	{
		unsigned char shapeType;
		double shapeWidth;
		double shapeLength;
		double x;
		double y;

		LineVertex()
		:	shapeType(0)
		,	shapeWidth(0.0)
		,	shapeLength(0.0)
		,	x(0.0)
		,	y(0.0)
		{};
	};

	struct Line
	{
		Rect clientRect;
		Color color;
		Attach attach;
		double width;
		unsigned char style;
		LineVertex begin;
		LineVertex end;
	};

	struct Bitmap
	{
		Rect clientRect;
		Attach attach;
		unsigned long size;
		string windowName;
		BorderType borderType;
		unsigned char* data;

		Bitmap(const string& _name = "")
		:	size(0)
		,	windowName(_name)
		,	borderType(Origin::None)
		,	data(0)
		{
		};

		Bitmap(const Bitmap& bitmap)
		:	clientRect(bitmap.clientRect)
		,	attach(bitmap.attach)
		,	size(bitmap.size)
		,	windowName(bitmap.windowName)
		,	borderType(bitmap.borderType)
		{
			if(size > 0)
			{
				data = new unsigned char[size];
				memcpy(data, bitmap.data, size);
			}
		};

		~Bitmap()
		{
			if(size > 0)
				delete data;
		};
	};

	struct ColorScale
	{
		bool visible;
		bool reverseOrder;
		unsigned short labelGap;
		unsigned short colorBarThickness;
		Color labelsColor;
	};

	struct GraphLayer
	{
		Rect clientRect;
		TextBox legend;
		Color backgroundColor;
		BorderType borderType;

		GraphAxis xAxis;
		GraphAxis yAxis;
		GraphAxis zAxis;

		GraphAxisBreak xAxisBreak;
		GraphAxisBreak yAxisBreak;
		GraphAxisBreak zAxisBreak;

		double histogramBin;
		double histogramBegin;
		double histogramEnd;

		PercentileProperties percentile;
		ColorScale colorScale;
		ColorMap colorMap;

		vector<TextBox> texts;
		vector<TextBox> pieTexts;
		vector<Line> lines;
		vector<Figure> figures;
		vector<Bitmap> bitmaps;
		vector<GraphCurve> curves;

		float xAngle;
		float yAngle;
		float zAngle;

		float xLength;
		float yLength;
		float zLength;

		int imageProfileTool;
		double vLine;
		double hLine;

		bool isWaterfall;
		int xOffset;
		int yOffset;

		bool gridOnTop;
		bool exchangedAxes;
		bool isXYY3D;
		bool orthographic3D;

		GraphLayer()
		:	imageProfileTool(0)
		,	isWaterfall(false)
		,	gridOnTop(false)
		,	exchangedAxes(false)
		,	isXYY3D(false)
		,	orthographic3D(false)
		{colorScale.visible = false;};

		//bool threeDimensional;
		bool is3D() const
		{
			for (vector<GraphCurve>::const_iterator it = curves.begin(); it != curves.end(); ++it)
			{
				if (it->type == GraphCurve::Line3D) return true;
				if (it->type == GraphCurve::Mesh3D) return true;
			}
		return false;
		}
	};

	struct GraphLayerRange
	{
		double min;
		double max;
		double step;

		GraphLayerRange(double _min = 0.0, double _max = 0.0, double _step = 0.0)
		:	min(_min)
		,	max(_max)
		,	step(_step)
		{};
	};

	struct Graph : public Window
	{
		vector<GraphLayer> layers;
		unsigned short width;
		unsigned short height;
		bool is3D;
		bool isLayout;
		bool connectMissingData;
		string templateName;

		Graph(const string& _name = "")
		:	Window(_name)
		,	is3D(false)
		,	isLayout(false)
		,	connectMissingData(false)
		,	templateName("")
		{};
	};

	struct Note : public Window
	{
		string text;
		Note(const string& _name = "")
		:	Window(_name)
		{};
	};

	struct ProjectNode
	{
		enum NodeType {SpreadSheet, Matrix, Excel, Graph, Graph3D, Note, Folder};

		NodeType type;
		string name;
		time_t creationDate;
		time_t modificationDate;
		bool active;

		ProjectNode(const string& _name = "", NodeType _type = Folder, const time_t _creationDate = time(NULL), const time_t _modificationDate = time(NULL), bool _active = false)
		:	type(_type)
		,	name(_name)
		,	creationDate(_creationDate)
		,	modificationDate(_modificationDate)
		,	active(_active)
		{};
	};
}

typedef Origin::ParsingProgressCallback ProgressCallback;

#endif // ORIGIN_OBJ_H
