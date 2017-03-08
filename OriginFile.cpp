/***************************************************************************
    File                 : OriginFile.cpp
    --------------------------------------------------------------------
    Copyright            : (C) 2005-2008 Stefan Gerlach
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

#include "OriginFile.h"
#include <cstdio>  // for fprintf
#include <cstdlib> // for atoi
#include <fstream>
#include <string>

OriginFile::OriginFile(const string& fileName)
:	fileVersion(0)
{
	unsigned int ioret;  // return value of io functions
	ifstream file(fileName.c_str(), ios_base::binary);

	if (!file.is_open())
	{
		ioret = fprintf(stderr, "Could not open %s!\n", fileName.c_str());
		return;
	}

#ifndef NO_CODE_GENERATION_FOR_LOG
	FILE *logfile = NULL;
	logfile = fopen("./opjfile.log", "w");
	if (logfile == NULL)
	{
		ioret = fprintf(stderr, "Could not open opjfile.log !\n");
		return;
	}
#endif // NO_CODE_GENERATION_FOR_LOG

	string vers;
	getline(file, vers);
	unsigned int majorVersion = atoi(vers.substr(5,1).c_str());
	char locale_decpoint = vers[6];
	unsigned int buildVersion = atoi(vers.substr(7).c_str());
	unsigned int buildNumber = atoi(vers.substr(12).c_str());
	file.close();
	LOG_PRINT(logfile, "File: %s\n", fileName.c_str())

	// translate version
	if (majorVersion==3) {
		if (buildVersion < 830)
			fileVersion = 350;
		else
			fileVersion = 410;
	} else if (buildVersion >= 110 && buildVersion <= 141) // 4.1
		fileVersion = 410;
	else if (buildVersion <= 210) // 5.0
		fileVersion = 500;
	else if (buildVersion <= 2623) // 6.0
		fileVersion = 600;
	else if (buildVersion <= 2627) // 6.0 SR1
		fileVersion = 601;
	else if (buildVersion <2635) // 6.0 SR4
		fileVersion = 604;
	else if (buildVersion <2656) // 6.1
		fileVersion = 610;
	else if (buildVersion <2659) // 7.0 SR0 (2656-2658)
		fileVersion = 700;
	else if (buildVersion <2664) // 7.0 SR1 (2659-2663)
		fileVersion = 701;
	else if (buildVersion <2672) // 7.0 SR2 (2664-2671)
		fileVersion = 702;
	else if (buildVersion <2673) // 7.0 SR3 (2672-2672)
		fileVersion = 703;
	else if (buildVersion <2766) // 7.0 SR4 (2673-2765)
		fileVersion = 704;
	else if (buildVersion <2878) // 7.5 (2766-2877)
		fileVersion = 750;
	else if (buildVersion <2881) // 8.0 SR0 (2878-2880)
		fileVersion = 800;
	else if (buildVersion <2892) // 8.0 SR1,SR2,SR3 (2878-2891)
		fileVersion = 801;
	else if (buildVersion <2944) // 8.0 SR4, 8.1 SR1-SR4 (2891-2943)
		fileVersion = 810;
	else {
		fileVersion = 850; // 8.5 SR0 and newer (2944-)
		LOG_PRINT(logfile, "Found project version 8.5 or newer\n")
	}

	if (fileVersion != 850) {
		LOG_PRINT(logfile, "Found project version %.2f\n", fileVersion/100.0)
	}
	// Close logfile, will be reopened in parser routine.
	// There are ways to keep logfile open and pass it to parser routine,
	// but I choose to do the same as with 'file', close it and reopen in 'parse'
	// routines.
#ifndef NO_CODE_GENERATION_FOR_LOG
	fclose(logfile);
#endif // NO_CODE_GENERATION_FOR_LOG
	parser.reset(createOriginAnyParser(fileName));
	/*switch(fileVersion){
		case 810:
			parser.reset(createOrigin810Parser(fileName));
			break;
		case 800:
			parser.reset(createOrigin800Parser(fileName));
			break;
		case 750:
			parser.reset(createOrigin750Parser(fileName));
			break;
		case 700:
		case 703:
		case 704:
			parser.reset(createOrigin700Parser(fileName));
			break;
		case 610:
			parser.reset(createOrigin610Parser(fileName));
			break;
		case 600:
		case 601:
		case 604:
			parser.reset(createOrigin600Parser(fileName));
			break;
		default:
			parser.reset(createOriginDefaultParser(fileName));
			break;
	}*/
}

bool OriginFile::parse(ProgressCallback callback, void *user_data)
{
	parser->setBuildVersion(buildVersion);
	return parser->parse(callback, user_data);
}

double OriginFile::version() const
{
	return fileVersion/100.0;
}

const tree<Origin::ProjectNode>* OriginFile::project() const
{
	return &parser->projectTree;
}

vector<Origin::SpreadSheet>::size_type OriginFile::spreadCount() const
{
	return parser->speadSheets.size();
}

Origin::SpreadSheet& OriginFile::spread(vector<Origin::SpreadSheet>::size_type s) const
{
	return parser->speadSheets[s];
}

vector<Origin::Matrix>::size_type OriginFile::matrixCount() const
{
	return parser->matrixes.size();
}

Origin::Matrix& OriginFile::matrix(vector<Origin::Matrix>::size_type m) const
{
	return parser->matrixes[m];
}

vector<Origin::Function>::size_type OriginFile::functionCount() const
{
	return parser->functions.size();
}

vector<Origin::Function>::size_type OriginFile::functionIndex(const string& name) const
{
	return parser->findFunctionByName(name);
}

Origin::Function& OriginFile::function(vector<Origin::Function>::size_type f) const
{
	return parser->functions[f];
}

vector<Origin::Graph>::size_type OriginFile::graphCount() const
{
	return parser->graphs.size();
}

Origin::Graph& OriginFile::graph(vector<Origin::Graph>::size_type g) const
{
	return parser->graphs[g];
}

vector<Origin::Note>::size_type OriginFile::noteCount() const
{
	return parser->notes.size();
}

Origin::Note& OriginFile::note(vector<Origin::Note>::size_type n) const
{
	return parser->notes[n];
}

string OriginFile::resultsLogString() const
{
	return parser->resultsLog;
}
