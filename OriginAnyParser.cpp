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
	readFileVersion();
	unsigned long curpos = 0;
	curpos = file.tellg();
	LOG_PRINT(logfile, "Now at %d [0x%X]\n", curpos, curpos)

	// get global header
	readGlobalHeader();
	curpos = file.tellg();
	LOG_PRINT(logfile, "Now at %d [0x%X]\n", curpos, curpos)

	// get dataset list
	unsigned int dataset_list_size = 0;

	LOG_PRINT(logfile, "Reading Data sets ...\n")
	while (true) {
		if (!readDataSetElement()) break;
		dataset_list_size++;
	}
	LOG_PRINT(logfile, " ... done. Data sets: %d\n", dataset_list_size)

	return true;
}

OriginParser* createOriginAnyParser(const string& fileName)
{
	return new OriginAnyParser(fileName);
}

unsigned int OriginAnyParser::readObjectSize() {
	unsigned int obj_size = 0;
	char c = 0;
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
		// get a string large enough to hold the result, initialize it to all 0's
		string blob = string(size, '\0');
		// read data into that string
		// cannot use '>>' operator because iendianfstream truncates it at first '\0'
		file.read(&blob[0], size);
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

void OriginAnyParser::readFileVersion() {
	// get file and program version, check it is a valid file
	string fileVersion;
	getline(file, fileVersion);

	if ((fileVersion.substr(0,4) != "CPYA") or (*fileVersion.rbegin() != '#')) {
		LOG_PRINT(logfile, "File, is not a valid opj file\n")
		exit(1);
	}
	LOG_PRINT(logfile, "File version string: %s\n", fileVersion.c_str())
}

void OriginAnyParser::readGlobalHeader() {
	// get global header size
	unsigned int gh_size = 0, gh_endmark = 0;
	gh_size = readObjectSize();
	unsigned long curpos = file.tellg();
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
}

bool OriginAnyParser::readDataSetElement() {
	/* get info and values of a DataSet (worksheet column, matrix sheet, ...)
	 * return true if a DataSet is found, otherwise return false */
	unsigned int dse_header_size = 0, dse_data_size = 0, dse_mask_size = 0;
	unsigned long curpos = 0, dsh_start = 0, dsd_start = 0, dsm_start = 0;
	string dse_header;

	dse_header_size = readObjectSize();
	if (dse_header_size == 0) return false;

	curpos = file.tellg();
	dsh_start = curpos;
	LOG_PRINT(logfile, "Column found: header size %d [0x%X], starts at %d [0x%X]: ", dse_header_size, dse_header_size, curpos, curpos)
	dse_header = readObjectAsString(dse_header_size);

	// get known info
	string name(25,0);
	name = dse_header.substr(0x58,25);
	LOG_PRINT(logfile, "%s\n", name.c_str())

	// go to end of dataset header, get data size
	file.seekg(dsh_start+dse_header_size+1, ios_base::beg);
	dse_data_size = readObjectSize();
	dsd_start = file.tellg();
	string dse_data = readObjectAsString(dse_data_size);
	curpos = file.tellg();
	LOG_PRINT(logfile, "data size %d [0x%X], starts at %d [0x%X], ends at %d [0x%X]\n", dse_data_size, dse_data_size, dsd_start, dsd_start, curpos, curpos)

	// get data values

	// go to end of data values, get mask size (often zero)
	file.seekg(dsd_start+dse_data_size+1, ios_base::beg);
	dse_mask_size = readObjectSize();
	dsm_start = file.tellg();
	if (dse_mask_size > 0) LOG_PRINT(logfile, "mask size %d [0x%X], starts at %d [0x%X]", dse_mask_size, dse_mask_size, dsm_start, dsm_start)
	string dse_mask = readObjectAsString(dse_mask_size);

	// get mask values
	if (dse_mask_size > 0) {
		curpos = file.tellg();
		LOG_PRINT(logfile, ", ends at %d [0x%X]\n", curpos, curpos)
		// TODO: extract mask values from dse_mask
		// go to end of dataset mask
		file.seekg(dsm_start+dse_mask_size+1, ios_base::beg);
	}
	curpos = file.tellg();
	LOG_PRINT(logfile, "column ends at %d [0x%X]\n", curpos, curpos)

	return true;
}
