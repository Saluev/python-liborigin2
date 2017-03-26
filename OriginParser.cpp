/***************************************************************************
    File                 : OriginParser.cpp
    --------------------------------------------------------------------
    Copyright            : (C) 2008 Alex Kargovsky
    Email (use @ for *)  : kargovsky*yumr.phys.msu.su
    Description          : Origin file parser base class

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

#include "OriginParser.h"
#include <algorithm>
#include <boost/algorithm/string.hpp> // for iequals

using namespace boost::algorithm;
using namespace Origin;

vector<Origin::SpreadSheet>::difference_type OriginParser::findSpreadByName(const string& name) const
{
	for (vector<SpreadSheet>::const_iterator it = speadSheets.begin(); it != speadSheets.end(); ++it)
	{
		if (iequals(it->name, name, locale())) return it - speadSheets.begin();
	}
	return -1;
}

vector<Origin::Excel>::difference_type OriginParser::findExcelByName(const string& name) const
{
	for (vector<Excel>::const_iterator it = excels.begin(); it != excels.end(); ++it)
	{
		if (iequals(it->name, name, locale())) return it - excels.begin();
	}
	return -1;
}

vector<Origin::SpreadColumn>::difference_type OriginParser::findSpreadColumnByName(vector<Origin::SpreadSheet>::size_type spread, const string& name) const
{
	for (vector<SpreadColumn>::const_iterator it = speadSheets[spread].columns.begin(); it != speadSheets[spread].columns.end(); ++it)
	{
		if (it->name == name) return it - speadSheets[spread].columns.begin();
	}
	return -1;
}

vector<Origin::SpreadColumn>::difference_type OriginParser::findExcelColumnByName(vector<Origin::Excel>::size_type excel, vector<Origin::SpreadSheet>::size_type sheet, const string& name) const
{
	for (vector<SpreadColumn>::const_iterator it = excels[excel].sheets[sheet].columns.begin(); 	it != excels[excel].sheets[sheet].columns.end(); ++it)
	{
		if (it->name == name) return it - excels[excel].sheets[sheet].columns.begin();
	}
	return -1;
}

vector<Origin::Matrix>::difference_type OriginParser::findMatrixByName(const string& name) const
{
	for (vector<Matrix>::const_iterator it = matrixes.begin(); it != matrixes.end(); ++it)
	{
		if (iequals(it->name, name, locale())) return it - matrixes.begin();
	}
	return -1;
}

vector<Origin::Function>::difference_type OriginParser::findFunctionByName(const string& name) const
{
	for (vector<Function>::const_iterator it = functions.begin(); it != functions.end(); ++it)
	{
		if (iequals(it->name, name, locale())) return it - functions.begin();
	}
	return -1;
}

pair<string, string> OriginParser::findDataByIndex(unsigned int index) const
{
	for(vector<SpreadSheet>::const_iterator it = speadSheets.begin(); it != speadSheets.end(); ++it)
	{
		for(vector<SpreadColumn>::const_iterator it1 = it->columns.begin(); it1 != it->columns.end(); ++it1)
		{
			if(it1->index == index)
				return make_pair("T_" + it->name, it1->name);
		}
	}

	for(vector<Matrix>::const_iterator it = matrixes.begin(); it != matrixes.end(); ++it)
	{
		for(vector<MatrixSheet>::const_iterator it1 = it->sheets.begin(); it1 != it->sheets.end(); ++it1)
		{
			if(it1->index == index)
				return make_pair("M_" + it->name, it1->name);
		}

	}


	for(vector<Excel>::const_iterator it = excels.begin(); it != excels.end(); ++it)
	{
		for(vector<SpreadSheet>::const_iterator it1 = it->sheets.begin(); it1 != it->sheets.end(); ++it1)
		{
			for(vector<SpreadColumn>::const_iterator it2 = it1->columns.begin(); it2 != it1->columns.end(); ++it2)
			{
				if(it2->index == index)
					return make_pair("E_" + it->name, it2->name);
			}
		}
	}

	for(vector<Function>::const_iterator it = functions.begin(); it != functions.end(); ++it)
	{
		if(it->index == index)
			return make_pair("F_" + it->name, it->name);
	}

	return pair<string, string>();
}

pair<ProjectNode::NodeType, string> OriginParser::findObjectByIndex(unsigned int index) const
{
	for(vector<SpreadSheet>::const_iterator it = speadSheets.begin(); it != speadSheets.end(); ++it)
	{
		if(it->objectID == (int)index)
			return make_pair(ProjectNode::SpreadSheet, it->name);
	}

	for(vector<Matrix>::const_iterator it = matrixes.begin(); it != matrixes.end(); ++it)
	{
		if(it->objectID == (int)index)
			return make_pair(ProjectNode::Matrix, it->name);
	}

	for(vector<Excel>::const_iterator it = excels.begin(); it != excels.end(); ++it)
	{
		if(it->objectID == (int)index)
			return make_pair(ProjectNode::Excel, it->name);
	}

	for(vector<Graph>::const_iterator it = graphs.begin(); it != graphs.end(); ++it)
	{
		if(it->objectID == (int)index){
			if (it->is3D)
				return make_pair(ProjectNode::Graph3D, it->name);
			else
				return make_pair(ProjectNode::Graph, it->name);
		}
	}

	return pair<ProjectNode::NodeType, string>();
}

void OriginParser::convertSpreadToExcel(vector<Origin::SpreadSheet>::size_type spread)
{
	//add new Excel sheet
	excels.push_back(Excel(speadSheets[spread].name, speadSheets[spread].label, speadSheets[spread].maxRows, speadSheets[spread].hidden, speadSheets[spread].loose));

	for(vector<SpreadColumn>::iterator it = speadSheets[spread].columns.begin(); it != speadSheets[spread].columns.end(); ++it)
	{
		unsigned int index = 0;
		int pos = it->name.find_last_of("@");
		if(pos != -1)
		{
			index = strtol(it->name.substr(pos + 1).c_str(), 0, 10) - 1;
			it->name = it->name.substr(0, pos);
		}

		if(excels.back().sheets.size() <= index)
			excels.back().sheets.resize(index + 1);

		excels.back().sheets[index].columns.push_back(*it);
	}

	speadSheets.erase(speadSheets.begin() + spread);
}

int OriginParser::findColumnByName(int spread, const string& name)
{
	unsigned int columns = speadSheets[spread].columns.size();
	for (unsigned int i = 0; i < columns; i++){
		string colName = speadSheets[spread].columns[i].name;
		if (colName.size() >= 11)
			colName.resize(11);

		if (name == colName)
			return i;
	}
	return -1;
}
