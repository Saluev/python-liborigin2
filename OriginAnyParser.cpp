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

	// get window list
	unsigned int window_list_size = 0;

	LOG_PRINT(logfile, "Reading Windows ...\n")
	while (true) {
		if (!readWindowElement()) break;
		window_list_size++;
	}
	LOG_PRINT(logfile, " ... done. Windows: %d\n", window_list_size)

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

	// get dataset header size
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

bool OriginAnyParser::readWindowElement() {
	/* get general info and details of a window
	 * return true if a Window is found, otherwise return false */
	unsigned int wde_header_size = 0;
	unsigned long curpos = 0, wdh_start = 0;

	// get window header size
	wde_header_size = readObjectSize();
	if (wde_header_size == 0) return false;

	curpos = file.tellg();
	wdh_start = curpos;
	LOG_PRINT(logfile, "Window found: header size %d [0x%X], starts at %d [0x%X]: ", wde_header_size, wde_header_size, curpos, curpos)
	string wde_header = readObjectAsString(wde_header_size);

	// get known info
	string name(25,0);
	name = wde_header.substr(0x02,25);
	LOG_PRINT(logfile, "%s\n", name.c_str())

	// go to end of window header
	file.seekg(wdh_start+wde_header_size+1, ios_base::beg);

	// get layer list
	unsigned int layer_list_size = 0;

	LOG_PRINT(logfile, " Reading Layers ...\n")
	while (true) {
		if (!readLayerElement()) break;
		layer_list_size++;
	}
	LOG_PRINT(logfile, "  ... done. Layers: %d\n", layer_list_size)
	curpos = file.tellg();
	LOG_PRINT(logfile, "window ends at %d [0x%X]\n", curpos, curpos)

	return true;
}

bool OriginAnyParser::readLayerElement() {
	/* get general info and details of a layer
	 * return true if a Layer is found, otherwise return false */
	unsigned int lye_header_size = 0;
	unsigned long curpos = 0, lyh_start = 0;

	// get layer header size
	lye_header_size = readObjectSize();
	if (lye_header_size == 0) return false;

	curpos = file.tellg();
	lyh_start = curpos;
	LOG_PRINT(logfile, "  Layer found: header size %d [0x%X], starts at %d [0x%X]: ", lye_header_size, lye_header_size, curpos, curpos)
	string lye_header = readObjectAsString(lye_header_size);

	// get known info
	string name(25,0);
	name = lye_header.substr(0x02,25);
	LOG_PRINT(logfile, "%s\n", name.c_str())

	// go to end of layer header
	file.seekg(lyh_start+lye_header_size+1, ios_base::beg);

	// get annotation list
	unsigned int annotation_list_size = 0;

	LOG_PRINT(logfile, " Reading Annotations ...\n")
	/* Some annotations can be groups of annotations. We need a recursive function for those cases */
	annotation_list_size = readAnnotationList();
	LOG_PRINT(logfile, "  ... done. Annotations: %d\n", annotation_list_size)

	// get curve list
	unsigned int curve_list_size = 0;

	LOG_PRINT(logfile, " Reading Curves ...\n")
	while (true) {
		if (!readCurveElement()) break;
		curve_list_size++;
	}
	LOG_PRINT(logfile, "  ... done. Curves: %d\n", curve_list_size)

	// get axisbreak list
	unsigned int axisbreak_list_size = 0;

	LOG_PRINT(logfile, " Reading Axis breaks ...\n")
	while (true) {
		if (!readAxisBreakElement()) break;
		axisbreak_list_size++;
	}
	LOG_PRINT(logfile, "  ... done. Axis breaks: %d\n", axisbreak_list_size)

	// get x axisparameter list
	unsigned int axispar_x_list_size = 0;

	LOG_PRINT(logfile, " Reading x-Axis parameters ...\n")
	while (true) {
		if (!readAxisParameterElement(1)) break;
		axispar_x_list_size++;
	}
	LOG_PRINT(logfile, "  ... done. x-Axis parameters: %d\n", axispar_x_list_size)

	// get y axisparameter list
	unsigned int axispar_y_list_size = 0;

	LOG_PRINT(logfile, " Reading y-Axis parameters ...\n")
	while (true) {
		if (!readAxisParameterElement(2)) break;
		axispar_y_list_size++;
	}
	LOG_PRINT(logfile, "  ... done. y-Axis parameters: %d\n", axispar_y_list_size)

	// get z axisparameter list
	unsigned int axispar_z_list_size = 0;

	LOG_PRINT(logfile, " Reading z-Axis parameters ...\n")
	while (true) {
		if (!readAxisParameterElement(3)) break;
		axispar_z_list_size++;
	}
	LOG_PRINT(logfile, "  ... done. z-Axis parameters: %d\n", axispar_z_list_size)

	curpos = file.tellg();
	LOG_PRINT(logfile, "layer ends at %d [0x%X]\n", curpos, curpos)

	return true;
}

unsigned int OriginAnyParser::readAnnotationList() {
	/* Purpose of this function is to allow recursive call for groups of annotation elements. */
	unsigned int annotation_list_size = 0;

	while (true) {
		if (!readAnnotationElement()) break;
		annotation_list_size++;
	}
	return annotation_list_size;
}

bool OriginAnyParser::readAnnotationElement() {
	/* get general info and details of an Annotation
	 * return true if an Annotation is found, otherwise return false */
	unsigned int ane_header_size = 0;
	unsigned long curpos = 0, anh_start = 0;

	// get annotation header size
	ane_header_size = readObjectSize();
	if (ane_header_size == 0) return false;

	curpos = file.tellg();
	anh_start = curpos;
	LOG_PRINT(logfile, "  Annotation found: header size %d [0x%X], starts at %d [0x%X]: ", ane_header_size, ane_header_size, curpos, curpos)
	string ane_header = readObjectAsString(ane_header_size);

	// get known info
	string name(41,0);
	name = ane_header.substr(0x46,41);
	LOG_PRINT(logfile, "%s\n", name.c_str())

	// go to end of annotation header
	file.seekg(anh_start+ane_header_size+1, ios_base::beg);

	// data of an annotation element is divided in three blocks
	// first block
	unsigned int ane_data_1_size = 0, andt1_start = 0;
	ane_data_1_size = readObjectSize();

	andt1_start = file.tellg();
	string andt1_data = readObjectAsString(ane_data_1_size);

	// TODO: get known info

	// go to end of first data block
	file.seekg(andt1_start+ane_data_1_size+1, ios_base::beg);

	// second block
	unsigned int ane_data_2_size = 0, andt2_start = 0;
	ane_data_2_size = readObjectSize();
	andt2_start = file.tellg();
	string andt2_data;

	// check for group of annotations
	if ((ane_data_1_size == 0x5e) and (ane_data_2_size == 0x04)) {
		unsigned int angroup_size = 0;
		curpos = file.tellg();
		LOG_PRINT(logfile, "Annotation group found at %d [0x%X] ...\n", curpos, curpos)
		angroup_size = readAnnotationList();
		curpos = file.tellg();
		LOG_PRINT(logfile, "... group end at %d [0x%X]. Annotations: %d\n", curpos, curpos, angroup_size)
		andt2_data = string("");
	} else {
		andt2_data = readObjectAsString(ane_data_2_size);
		// TODO: get known info
		// go to end of second data block
		file.seekg(andt2_start+ane_data_2_size+1, ios_base::beg);
	}

	// third block
	unsigned int ane_data_3_size = 0, andt3_start = 0;
	ane_data_3_size = readObjectSize();

	andt3_start = file.tellg();
	string andt3_data = readObjectAsString(ane_data_3_size);

	curpos = file.tellg();
	LOG_PRINT(logfile, "annotation ends at %d [0x%X]\n", curpos, curpos)

	return true;
}

bool OriginAnyParser::readCurveElement() {
	/* get general info and details of a Curve
	 * return true if a Curve is found, otherwise return false */
	unsigned int cve_header_size = 0, cve_data_size = 0;
	unsigned long curpos = 0, cvh_start = 0, cvd_start = 0;

	// get curve header size
	cve_header_size = readObjectSize();
	if (cve_header_size == 0) return false;

	curpos = file.tellg();
	cvh_start = curpos;
	LOG_PRINT(logfile, "  Curve found: header size %d [0x%X], starts at %d [0x%X]: ", cve_header_size, cve_header_size, curpos, curpos)
	string cve_header = readObjectAsString(cve_header_size);

	// TODO: get known info from curve header

	// go to end of header, get curve data size
	file.seekg(cvh_start+cve_header_size+1, ios_base::beg);
	cve_data_size = readObjectSize();
	cvd_start = file.tellg();
	string cve_data = readObjectAsString(cve_data_size);

	// TODO: get known info from curve data

	// go to end of data
	file.seekg(cvd_start+cve_data_size+1, ios_base::beg);

	curpos = file.tellg();
	LOG_PRINT(logfile, "curve ends at %d [0x%X]\n", curpos, curpos)

	return true;
}
