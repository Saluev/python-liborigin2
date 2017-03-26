/***************************************************************************
    File                 : OriginFile.h
    --------------------------------------------------------------------
    Copyright            : (C) 2005-2007 Stefan Gerlach
                           (C) 2007-2008 Alex Kargovsky, Ion Vasilief
    Email (use @ for *)  : kargovsky*yumr.phys.msu.su, ion_vasilief*yahoo.fr
    Description          : Origin file import class

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

#ifndef ORIGIN_FILE_H
#define ORIGIN_FILE_H

#include "config.h"
#include "OriginObj.h"
#include "OriginParser.h"
#include <memory>

using namespace std;

class OriginFile
{
public:
	OriginFile(const string& fileName);

	bool parse(ProgressCallback callback = NULL, void *user_data = NULL);				//!< parse Origin file
	double version() const;																//!< get version of Origin file

	vector<Origin::SpreadSheet>::size_type spreadCount() const;							//!< get number of spreadsheets
	Origin::SpreadSheet& spread(vector<Origin::SpreadSheet>::size_type s) const;		//!< get spreadsheet s

	vector<Origin::Matrix>::size_type matrixCount() const;								//!< get number of matrices
	Origin::Matrix& matrix(vector<Origin::Matrix>::size_type m) const;					//!< get matrix m

	vector<Origin::Function>::size_type functionCount() const;							//!< get number of functions
	vector<Origin::Function>::size_type functionIndex(const string& name) const;		//!< get name of function s
	Origin::Function& function(vector<Origin::Function>::size_type f) const;			//!< get function f

	vector<Origin::Graph>::size_type graphCount() const;								//!< get number of graphs
	Origin::Graph& graph(vector<Origin::Graph>::size_type g) const;						//!< get graph g
	
	vector<Origin::Note>::size_type noteCount() const;									//!< get number of notes
	Origin::Note& note(vector<Origin::Note>::size_type n) const;						//!< get note n

	const tree<Origin::ProjectNode>* project() const;									//!< get project tree
	string resultsLogString() const;													//!< get Results Log

private:
	unsigned int fileVersion, buildVersion;
	unique_ptr<OriginParser> parser;
};

#endif // ORIGIN_FILE_H
