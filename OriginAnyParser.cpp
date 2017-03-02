/*
 * OriginAnyParser.cpp
 *
 * Copyright 2017 Miquel Garriga <gbmiquel@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Parser for all versions. Based mainly on Origin750Parser.cpp
 */

#include "OriginAnyParser.h"

OriginAnyParser::OriginAnyParser(const string& fileName)
:	file(fileName.c_str(),ios::binary)
{
}

bool OriginAnyParser::parse(ProgressCallback callback, void *user_data)
{
	file.callback = callback;
	file.callback_user_data = user_data;

#ifndef NO_CODE_GENERATION_FOR_LOG
	// append progress in log file
	logfile = fopen("opjfile.log","a");
#endif // NO_CODE_GENERATION_FOR_LOG

	// get length of file:
	file.seekg (0, ios_base::end);
	d_file_size = file.tellg();
	file.seekg(0, ios_base::beg);

	LOG_PRINT(logfile, "File size: %d\n", d_file_size)

	// get file and program version, check it is a valid file
	string fileVersion;
	getline(file, fileVersion);

	if ((fileVersion.substr(0,4) != "CPYA") or (*fileVersion.rbegin() != '#')) {
		LOG_PRINT(logfile, "File, is not a valid opj file\n")
		exit(1);
	}
	LOG_PRINT(logfile, "File version string: %s\n", fileVersion.c_str())
	unsigned long curpos = 0;
	curpos = file.tellg();
	LOG_PRINT(logfile, "Now at %d [0x%X]\n", curpos, curpos)

	// get global header size
	unsigned int gh_size=0, gh_endmark=0;
	gh_size = readObjectSize();
	curpos = file.tellg();
	LOG_PRINT(logfile, "Global header size: %d [0x%X], starts at %d [0x%X],", gh_size, gh_size, curpos, curpos)

	// get global header data
	if (0) {
		// skip header
		file.seekg(gh_size+1, ios_base::cur);
	} else {
		// read it into a string
		string gh_data = readObjectAsString(gh_size);
	}
	curpos = file.tellg();
	LOG_PRINT(logfile, " ends at %d [0x%X]\n", curpos, curpos)

	// now read a zero size end mark
	gh_endmark = readObjectSize();
	if (gh_endmark != 0) {
		LOG_PRINT(logfile, "Wrong end of list mark %d at %d [0x%X]\n", gh_endmark, (unsigned long)file.tellg(), (unsigned long)file.tellg())
		exit(4);
	}

	return true;
}

OriginParser* createOriginAnyParser(const string& fileName)
{
	return new OriginAnyParser(fileName);
}

unsigned int OriginAnyParser::readObjectSize() {
	unsigned int obj_size=0;
	char c=0;
	file >> obj_size;
	file >> c;
	if (c != '\n') {
		LOG_PRINT(logfile, "Wrong delimiter %c at %d [0x%X]\n", c, (unsigned long)file.tellg(), (unsigned long)file.tellg())
		exit(2);
	}
	return obj_size;
}

string OriginAnyParser::readObjectAsString(unsigned int size) {
	char c;
	// read a size-byte blob of data followed by '\n'
	if (size > 0) {
		// get a string large enough to hold the result, initialize  it to all 0's
		string blob = string(size, '\0');
		// read data into that string
		file >> blob;
		// read the '\n'
		file >> c;
		if (c != '\n') {
			LOG_PRINT(logfile, "Wrong delimiter %c at %d [0x%X]\n", c, (unsigned long)file.tellg(), (unsigned long)file.tellg())
			exit(3);
		}
		return blob;
	}
	return string();
}
