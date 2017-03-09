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
#include <sstream>

/* define a macro to get an int (or uint) from a istringstream in binary mode */
#define GET_INT(iss, ovalue) {iss.read(reinterpret_cast<char *>(&ovalue), 4);};
#define GET_SHORT(iss, ovalue) {iss.read(reinterpret_cast<char *>(&ovalue), 2);};
#define GET_FLOAT(iss, ovalue) {iss.read(reinterpret_cast<char *>(&ovalue), 4);};
#define GET_DOUBLE(iss, ovalue) {iss.read(reinterpret_cast<char *>(&ovalue), 8);};

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
	curpos = file.tellg();
	LOG_PRINT(logfile, "Now at %d [0x%X], filesize %d\n", curpos, curpos, d_file_size)

	// get window list
	unsigned int window_list_size = 0;

	LOG_PRINT(logfile, "Reading Windows ...\n")
	while (true) {
		if (!readWindowElement()) break;
		window_list_size++;
	}
	LOG_PRINT(logfile, " ... done. Windows: %d\n", window_list_size)
	curpos = file.tellg();
	LOG_PRINT(logfile, "Now at %d [0x%X], filesize %d\n", curpos, curpos, d_file_size)

	// get parameter list
	unsigned int parameter_list_size = 0;

	LOG_PRINT(logfile, "Reading Parameters ...\n")
	while (true) {
		if (!readParameterElement()) break;
		parameter_list_size++;
	}
	LOG_PRINT(logfile, " ... done. Parameters: %d\n", parameter_list_size)
	curpos = file.tellg();
	LOG_PRINT(logfile, "Now at %d [0x%X], filesize %d\n", curpos, curpos, d_file_size)

	// Note windows were added between version >4.141 and 4.210,
	// i.e., with Release 5.0
	if (curpos >= d_file_size) {
		LOG_PRINT(logfile, "Now at end of file\n")
		return true;
	}

	// get note windows list
	unsigned int note_list_size = 0;

	LOG_PRINT(logfile, "Reading Note windows ...\n")
	while (true) {
		if (!readNoteElement()) break;
		note_list_size++;
	}
	LOG_PRINT(logfile, " ... done. Note windows: %d\n", note_list_size)
	curpos = file.tellg();
	LOG_PRINT(logfile, "Now at %d [0x%X], filesize %d\n", curpos, curpos, d_file_size)

	// Project Tree was added between version >4.210 and 4.2616,
	// i.e., with Release 6.0
	if (curpos >= d_file_size) {
		LOG_PRINT(logfile, "Now at end of file\n")
		return true;
	}

	// get project tree
	readProjectTree();
	curpos = file.tellg();
	LOG_PRINT(logfile, "Now at %d [0x%X], filesize %d\n", curpos, curpos, d_file_size)

	// Attachments were added between version >4.2673_558 and 4.2764_623,
	// i.e., with Release 7.0
	if (curpos >= d_file_size) {
		LOG_PRINT(logfile, "Now at end of file\n")
		return true;
	}

	readAttachmentList();
	curpos = file.tellg();
	LOG_PRINT(logfile, "Now at %d [0x%X], filesize %d\n", curpos, curpos, d_file_size)
	if (curpos >= d_file_size) LOG_PRINT(logfile, "Now at end of file\n")

	return true;
}

string toLowerCase(string str){
	for (unsigned int i = 0; i < str.length(); i++)
		if (str[i] >= 0x41 && str[i] <= 0x5A)
			str[i] = str[i] + 0x20;
	return str;
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
	string sFileVersion;
	getline(file, sFileVersion);

	if ((sFileVersion.substr(0,4) != "CPYA") or (*sFileVersion.rbegin() != '#')) {
		LOG_PRINT(logfile, "File, is not a valid opj file\n")
		exit(1);
	}
	LOG_PRINT(logfile, "File version string: %s\n", sFileVersion.c_str())
}

void OriginAnyParser::readGlobalHeader() {
	// get global header size
	unsigned int gh_size = 0, gh_endmark = 0;
	gh_size = readObjectSize();
	unsigned long curpos = file.tellg();
	LOG_PRINT(logfile, "Global header size: %d [0x%X], starts at %d [0x%X],", gh_size, gh_size, curpos, curpos)

	// get global header data
	string gh_data;
	gh_data = readObjectAsString(gh_size);

	curpos = file.tellg();
	LOG_PRINT(logfile, " ends at %d [0x%X]\n", curpos, curpos)

	// when gh_size > 0x1B, a double with fileVersion/100 can be read at gh_data[0x1B:0x23]
	if (gh_size > 0x1B) {
		istringstream stmp;
		stmp.str(gh_data.substr(0x1B));
		double dFileVersion;
		GET_DOUBLE(stmp, dFileVersion)
		if (dFileVersion > 8.5) {
			fileVersion = (unsigned int)trunc(dFileVersion*100.);
		} else {
			fileVersion = 10*(unsigned int)trunc(dFileVersion*10.);
		}
		LOG_PRINT(logfile, "Project version as read from header: %.2f (%.6f)\n", fileVersion/100.0, dFileVersion)
	}

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
	LOG_PRINT(logfile, "Column: header size %d [0x%X], starts at %d [0x%X], ", dse_header_size, dse_header_size, curpos, curpos)
	dse_header = readObjectAsString(dse_header_size);

	// get known info
	string name(25,0);
	name = dse_header.substr(0x58,25);

	// go to end of dataset header, get data size
	file.seekg(dsh_start+dse_header_size+1, ios_base::beg);
	dse_data_size = readObjectSize();
	dsd_start = file.tellg();
	string dse_data = readObjectAsString(dse_data_size);
	curpos = file.tellg();
	LOG_PRINT(logfile, "data size %d [0x%X], from %d [0x%X] to %d [0x%X],", dse_data_size, dse_data_size, dsd_start, dsd_start, curpos, curpos)

	// get data values
	getColumnInfoAndData(dse_header, dse_header_size, dse_data, dse_data_size);

	// go to end of data values, get mask size (often zero)
	file.seekg(dsd_start+dse_data_size, ios_base::beg); // dse_data_size can be zero
	if (dse_data_size > 0) file.seekg(1, ios_base::cur);
	dse_mask_size = readObjectSize();
	dsm_start = file.tellg();
	if (dse_mask_size > 0) LOG_PRINT(logfile, "\nmask size %d [0x%X], starts at %d [0x%X]", dse_mask_size, dse_mask_size, dsm_start, dsm_start)
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
	LOG_PRINT(logfile, " ends at %d [0x%X]: ", curpos, curpos)
	LOG_PRINT(logfile, "%s\n", name.c_str())

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
	LOG_PRINT(logfile, " ... done. Layers: %d\n", layer_list_size)
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
	LOG_PRINT(logfile, "  Layer found: header size %d [0x%X], starts at %d [0x%X]\n", lye_header_size, lye_header_size, curpos, curpos)
	string lye_header = readObjectAsString(lye_header_size);

	// get known info

	// go to end of layer header
	file.seekg(lyh_start+lye_header_size+1, ios_base::beg);

	// get annotation list
	unsigned int annotation_list_size = 0;

	LOG_PRINT(logfile, "   Reading Annotations ...\n")
	/* Some annotations can be groups of annotations. We need a recursive function for those cases */
	annotation_list_size = readAnnotationList();
	LOG_PRINT(logfile, "   ... done. Annotations: %d\n", annotation_list_size)

	// get curve list
	unsigned int curve_list_size = 0;

	LOG_PRINT(logfile, "   Reading Curves ...\n")
	while (true) {
		if (!readCurveElement()) break;
		curve_list_size++;
	}
	LOG_PRINT(logfile, "   ... done. Curves: %d\n", curve_list_size)

	// get axisbreak list
	unsigned int axisbreak_list_size = 0;

	LOG_PRINT(logfile, "   Reading Axis breaks ...\n")
	while (true) {
		if (!readAxisBreakElement()) break;
		axisbreak_list_size++;
	}
	LOG_PRINT(logfile, "   ... done. Axis breaks: %d\n", axisbreak_list_size)

	// get x axisparameter list
	unsigned int axispar_x_list_size = 0;

	LOG_PRINT(logfile, "   Reading x-Axis parameters ...\n")
	while (true) {
		if (!readAxisParameterElement(1)) break;
		axispar_x_list_size++;
	}
	LOG_PRINT(logfile, "   ... done. x-Axis parameters: %d\n", axispar_x_list_size)

	// get y axisparameter list
	unsigned int axispar_y_list_size = 0;

	LOG_PRINT(logfile, "   Reading y-Axis parameters ...\n")
	while (true) {
		if (!readAxisParameterElement(2)) break;
		axispar_y_list_size++;
	}
	LOG_PRINT(logfile, "   ... done. y-Axis parameters: %d\n", axispar_y_list_size)

	// get z axisparameter list
	unsigned int axispar_z_list_size = 0;

	LOG_PRINT(logfile, "   Reading z-Axis parameters ...\n")
	while (true) {
		if (!readAxisParameterElement(3)) break;
		axispar_z_list_size++;
	}
	LOG_PRINT(logfile, "   ... done. z-Axis parameters: %d\n", axispar_z_list_size)

	curpos = file.tellg();
	LOG_PRINT(logfile, "  layer ends at %d [0x%X]\n", curpos, curpos)

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
	LOG_PRINT(logfile, "    Annotation found: header size %d [0x%X], starts at %d [0x%X]: ", ane_header_size, ane_header_size, curpos, curpos)
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
	LOG_PRINT(logfile, "     block 1 size %d [0x%X] at %d [0x%X]\n", ane_data_1_size, ane_data_1_size, andt1_start, andt1_start)
	string andt1_data = readObjectAsString(ane_data_1_size);

	// TODO: get known info

	// go to end of first data block
	file.seekg(andt1_start+ane_data_1_size+1, ios_base::beg);

	// second block
	unsigned int ane_data_2_size = 0, andt2_start = 0;
	ane_data_2_size = readObjectSize();
	andt2_start = file.tellg();
	LOG_PRINT(logfile, "     block 2 size %d [0x%X] at %d [0x%X]\n", ane_data_2_size, ane_data_2_size, andt2_start, andt2_start)
	string andt2_data;

	// check for group of annotations
	if ((ane_data_1_size == 0x5e) and (ane_data_2_size == 0x04)) {
		unsigned int angroup_size = 0;
		curpos = file.tellg();
		LOG_PRINT(logfile, "  Annotation group found at %d [0x%X] ...\n", curpos, curpos)
		angroup_size = readAnnotationList();
		curpos = file.tellg();
		LOG_PRINT(logfile, "  ... group end at %d [0x%X]. Annotations: %d\n", curpos, curpos, angroup_size)
		andt2_data = string("");
	} else {
		andt2_data = readObjectAsString(ane_data_2_size);
		// TODO: get known info
		// go to end of second data block
		file.seekg(andt2_start+ane_data_2_size, ios_base::beg);
		if (ane_data_2_size > 0) file.seekg(1, ios_base::cur);
	}

	// third block
	unsigned int ane_data_3_size = 0, andt3_start = 0;
	ane_data_3_size = readObjectSize();

	andt3_start = file.tellg();
	LOG_PRINT(logfile, "     block 3 size %d [0x%X] at %d [0x%X]\n", ane_data_3_size, ane_data_3_size, andt3_start, andt3_start)
	string andt3_data = readObjectAsString(ane_data_3_size);

	curpos = file.tellg();
	LOG_PRINT(logfile, "    annotation ends at %d [0x%X]\n", curpos, curpos)

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
	LOG_PRINT(logfile, "    Curve: header size %d [0x%X], starts at %d [0x%X], ", cve_header_size, cve_header_size, curpos, curpos)
	string cve_header = readObjectAsString(cve_header_size);

	// TODO: get known info from curve header
	string name = cve_header.substr(0x12,12);

	// go to end of header, get curve data size
	file.seekg(cvh_start+cve_header_size+1, ios_base::beg);
	cve_data_size = readObjectSize();
	cvd_start = file.tellg();
	LOG_PRINT(logfile, "data size %d [0x%X], from %d [0x%X]", cve_data_size, cve_data_size, cvd_start, cvd_start)
	string cve_data = readObjectAsString(cve_data_size);

	// TODO: get known info from curve data

	// go to end of data
	file.seekg(cvd_start+cve_data_size, ios_base::beg);
	if (cve_data_size > 0) file.seekg(1, ios_base::cur);

	curpos = file.tellg();
	LOG_PRINT(logfile, "to %d [0x%X]: %s\n", curpos, curpos, name.c_str())

	return true;
}

bool OriginAnyParser::readAxisBreakElement() {
	/* get info of Axis breaks
	 * return true if an Axis break, otherwise return false */
	unsigned int abe_data_size = 0;
	unsigned long curpos = 0, abd_start = 0;

	// get axis break data size
	abe_data_size = readObjectSize();
	if (abe_data_size == 0) return false;

	curpos = file.tellg();
	abd_start = curpos;
	string abd_data = readObjectAsString(abe_data_size);

	// get known info

	// go to end of axis break data
	file.seekg(abd_start+abe_data_size+1, ios_base::beg);

	return true;
}

bool OriginAnyParser::readAxisParameterElement(unsigned int naxis) {
	/* get info of Axis parameters for naxis-axis (x,y,z) = (1,2,3)
	 * return true if an Axis break is found, otherwise return false */
	unsigned int ape_data_size = 0;
	unsigned long curpos = 0, apd_start = 0;

	// get axis break data size
	ape_data_size = readObjectSize();
	if (ape_data_size == 0) return false;

	curpos = file.tellg();
	apd_start = curpos;
	string apd_data = readObjectAsString(ape_data_size);

	// get known info

	// go to end of axis break data
	file.seekg(apd_start+ape_data_size+1, ios_base::beg);

	return true;
}

bool OriginAnyParser::readParameterElement() {
	// get parameter name
	unsigned int par_start = 0, curpos = 0;
	string par_name;
	char c;

	par_start = file.tellg();
	getline(file, par_name);
	if (par_name[0] == '\0') {
		unsigned int eof_parameters_mark = readObjectSize();
		return false;
	}
	LOG_PRINT(logfile, " %s:", par_name.c_str())
	// get value
	double value;
	file >> value;
	LOG_PRINT(logfile, " %g\n", value)
	// read the '\n'
	file >> c;
	if (c != '\n') {
		LOG_PRINT(logfile, "Wrong delimiter %c at %d [0x%X]\n", c, (unsigned long)file.tellg(), (unsigned long)file.tellg())
		exit(3);
	}

	return true;
}

bool OriginAnyParser::readNoteElement() {
	/* get info of Note windows, including "Results Log"
	 * return true if a Note window is found, otherwise return false */
	unsigned int nwe_header_size = 0, nwe_label_size = 0, nwe_contents_size = 0;
	unsigned long curpos = 0, nwh_start = 0, nwl_start = 0, nwc_start = 0;

	// get note header size
	nwe_header_size = readObjectSize();
	if (nwe_header_size == 0) return false;

	curpos = file.tellg();
	nwh_start = curpos;
	LOG_PRINT(logfile, "  Note window found: header size %d [0x%X], starts at %d [0x%X]\n", nwe_header_size, nwe_header_size, curpos, curpos)
	string nwe_header = readObjectAsString(nwe_header_size);

	// TODO: get known info from header

	// go to end of header
	file.seekg(nwh_start+nwe_header_size+1, ios_base::beg);

	// get label size
	nwe_label_size = readObjectSize();
	nwl_start = file.tellg();
	string nwe_label = readObjectAsString(nwe_label_size);
	LOG_PRINT(logfile, "  label at %d [0x%X]: %s\n", nwl_start, nwl_start, nwe_label.c_str())

	// go to end of label
	file.seekg(nwl_start+nwe_label_size, ios_base::beg);
	if (nwe_label_size > 0) file.seekg(1, ios_base::cur);

	// get contents size
	nwe_contents_size = readObjectSize();
	nwc_start = file.tellg();
	string nwe_contents = readObjectAsString(nwe_contents_size);
	LOG_PRINT(logfile, "  contents at %d [0x%X]: \n%s\n", nwc_start, nwc_start, nwe_contents.c_str())

	return true;
}

void OriginAnyParser::readProjectTree() {
	unsigned int pte_depth = 0;

	// first preamble size and data (usually 4)
	unsigned int pte_pre1_size = readObjectSize();
	string pte_pre1 = readObjectAsString(pte_pre1_size);

	// second preamble size and data (usually 16)
	unsigned int pte_pre2_size = readObjectSize();
	string pte_pre2 = readObjectAsString(pte_pre2_size);

	// root element and children
	unsigned int rootfolder = readFolderTree(pte_depth);

	// epilogue (should be zero)
	unsigned int pte_post_size = readObjectSize();

	return;
}

unsigned int OriginAnyParser::readFolderTree(unsigned int depth) {
	unsigned int fle_header_size = 0, fle_eofh_size = 0, fle_name_size = 0, fle_prop_size = 0;
	unsigned long curpos = 0;

	// folder header size, data, end mark
	fle_header_size = readObjectSize();
	string fle_header = readObjectAsString(fle_header_size);
	fle_eofh_size = readObjectSize(); // (usually 0)

	// folder name size
	fle_name_size = readObjectSize();
	curpos = file.tellg();
	string fle_name = readObjectAsString(fle_name_size);
	LOG_PRINT(logfile, "Folder name at %d [0x%X]: %s\n", curpos, curpos, fle_name.c_str());

	// additional properties
	fle_prop_size = readObjectSize();
	for (unsigned int i = 0; i < fle_prop_size; i++) {
		unsigned int obj_size = readObjectSize();
		string obj_data = readObjectAsString(obj_size);
	}

	// file entries
	unsigned int number_of_files_size = 0;

	number_of_files_size = readObjectSize(); // should be 4 as number_of_files is an integer
	curpos = file.tellg();
	LOG_PRINT(logfile, "Number of files at %d [0x%X] ", curpos, curpos);
	string fle_nfiles = readObjectAsString(number_of_files_size);

	istringstream stmp(ios_base::binary);
	stmp.str(fle_nfiles);
	unsigned int number_of_files = 0;
	stmp.read(reinterpret_cast<char *>(&number_of_files), number_of_files_size);
	LOG_PRINT(logfile, "%d\n", number_of_files)

	for (unsigned int i=0; i < number_of_files; i++) {
		readProjectLeave();
	}

	// subfolder entries
	unsigned int number_of_folders_size = 0;

	number_of_folders_size = readObjectSize(); // should be 4 as number_of_subfolders is an integer
	curpos = file.tellg();
	LOG_PRINT(logfile, "Number of subfolders at %d [0x%X] ", curpos, curpos);
	string fle_nfolders = readObjectAsString(number_of_folders_size);

	stmp.str(fle_nfolders);
	unsigned int number_of_folders = 0;
	stmp.read(reinterpret_cast<char *>(&number_of_folders), number_of_folders_size);
	LOG_PRINT(logfile, "%d\n", number_of_folders)

	for (unsigned int i=0; i < number_of_folders; i++) {
		depth++;
		unsigned int subfolder = readFolderTree(depth);
		depth--;
	}

	return number_of_files;
}

void OriginAnyParser::readProjectLeave() {
	unsigned long curpos = 0;

	// preamble size (usually 0) and data
	unsigned int ptl_pre_size = readObjectSize();
	string ptl_pre = readObjectAsString(ptl_pre_size);

	// file data size (usually 8) and data
	unsigned int ptl_data_size = readObjectSize();
	curpos = file.tellg();
	string ptl_data = readObjectAsString(ptl_data_size);

	// get info from file data
	istringstream stmp(ios_base::binary);
	stmp.str(ptl_data);
	unsigned int file_type = 0, file_object_id = 0;
	GET_INT(stmp, file_type);
	GET_INT(stmp, file_object_id);

	LOG_PRINT(logfile, "File at %d [0x%X], type %d, object_id %d\n", curpos, curpos, file_type, file_object_id)

	// epilogue (should be zero)
	unsigned int ptl_post_size = readObjectSize();

	return;
}

void OriginAnyParser::readAttachmentList() {
	/* Attachments are divided in two groups (which can be empty)
	 first group is preceeded by two integers: 4096 (0x1000) and number_of_attachments followed as usual by a '\n' mark
	 second group is a series of (header, name, data) triplets without the '\n' mark.
	*/

	// figure out if first group is not empty. In this case we will read integer=8 at current file position
	unsigned int att_1st_empty = 0;
	file >> att_1st_empty;
	file.seekg(-4, ios_base::cur);

	istringstream stmp(ios_base::binary);
	string att_header;
	unsigned long curpos = 0;
	if (att_1st_empty == 8) {
		// first group
		unsigned int att_list1_size = 0;

		// get two integers
		// next line fails if first attachment group is empty: readObjectSize exits as there is no '\n' after 4 bytes for uint
		att_list1_size = readObjectSize(); // should be 8 as we expect two integer values
		curpos = file.tellg();
		string att_list1 = readObjectAsString(att_list1_size);
		LOG_PRINT(logfile, "First attachment group at %d [0x%X]", curpos, curpos)

		stmp.str(att_list1);

		unsigned int att_mark = 0, number_of_atts = 0, iattno = 0, att_data_size = 0;
		GET_INT(stmp, att_mark) // should be 4096
		GET_INT(stmp, number_of_atts)
		LOG_PRINT(logfile, " with %d attachments.\n", number_of_atts)

		for (unsigned int i=0; i < number_of_atts; i++) {
			/* Header is a group of 7 integers followed by \n
			1st  attachment mark (4096: 0x00 0x10 0x00 0x00)
			2nd  attachment number ( <num_of_att)
			3rd  attachment size
			4th .. 7th ???
			*/
			// get header
			att_header = readObjectAsString(7*4);
			stmp.str(att_header);
			GET_INT(stmp, att_mark) // should be 4096
			GET_INT(stmp, iattno)
			GET_INT(stmp, att_data_size)
			curpos = file.tellg();
			LOG_PRINT(logfile, "Attachment no %d (%d) at %d [0x%X], size %d\n", i, iattno, curpos, curpos, att_data_size)

			// get data
			string att_data = readObjectAsString(att_data_size);
			// even if att_data_size is zero, we get a '\n' mark
			if (att_data_size == 0) file.seekg(1, ios_base::cur);
		}
	} else {
		LOG_PRINT(logfile, "First attachment group is empty\n")
	}

	/* Second group is a series of (header, name, data) triplets
	   There is no number of attachments. It ends when we reach EOF. */
	curpos = file.tellg();
	LOG_PRINT(logfile, "Second attachment group starts at %d [0x%X], file size %d\n", curpos, curpos, d_file_size)
	/* Header is a group of 3 integers, with no '\n' at end
		1st attachment header+name size including itself
		2nd attachment type (0x59 0x04 0xCA 0x7F for excel workbooks)
		3rd size of data */

	// get header
	att_header = string(12,0);
	while (true) {
		// check for eof
		if ((file.tellg() == d_file_size) || (file.eof())) break;
		// cannot use readObjectAsString: there is no '\n' at end
		file.read(reinterpret_cast<char*>(&att_header[0]), 12);

		if (file.gcount() != 12) break;
		// get header size, type and data size
		unsigned int att_header_size=0, att_type=0, att_size=0;
		stmp.str(att_header);
		GET_INT(stmp, att_header_size)
		GET_INT(stmp, att_type)
		GET_INT(stmp, att_size)

		// get name and data
		unsigned int name_size = att_header_size - 3*4;
		string att_name = string(name_size, 0);
		file.read(&att_name[0], name_size);
		curpos = file.tellg();
		string att_data = string(att_size, 0);
		file.read(&att_data[0], att_size);
		LOG_PRINT(logfile, "attachment at %d [0x%X], type 0x%X, size %d [0x%X]: %s\n", curpos, curpos, att_type, att_size, att_size, att_name.c_str())
	}

	return;
}

bool OriginAnyParser::getColumnInfoAndData(string col_header, unsigned int col_header_size, string col_data, unsigned int col_data_size) {
	istringstream stmp(ios_base::binary);
	static unsigned int dataIndex=0;
	short data_type;
	char data_type_u;
	char valuesize;
	string name(25,0), column_name;

	stmp.str(col_header.substr(0x16));
	GET_SHORT(stmp, data_type);

	data_type_u = col_header[0x3F];
	valuesize = col_header[0x3D];
	if(valuesize == 0) {
		LOG_PRINT(logfile, "	WARNING : found strange valuesize of %d\n", (int)valuesize);
		valuesize = 10;
	}

	name = col_header.substr(0x58,25);
	string::size_type colpos = name.find_last_of("_");

	if(colpos != string::npos){
		column_name = name.substr(colpos + 1);
		name.resize(colpos);
	}

	LOG_PRINT(logfile, "\n  data_type 0x%.4X, data_type_u 0x%.2X, valuesize %d [0x%X], %s [%s]\n", data_type, data_type_u, valuesize, valuesize, name.c_str(), column_name.c_str());

	unsigned short signature;
	if (col_header_size > 0x72) {
		stmp.str(col_header.substr(0x71));
		GET_SHORT(stmp, signature);

		int total_rows, first_row, last_row;
		stmp.str(col_header.substr(0x19));
		GET_INT(stmp, total_rows);
		GET_INT(stmp, first_row);
		GET_INT(stmp, last_row);
		LOG_PRINT(logfile, "  total %d, first %d, last %d rows\n", total_rows, first_row, last_row)
	} else {
		LOG_PRINT(logfile, "  NOTE: alternative signature determination\n")
		signature = col_header[0x18];
	}
	LOG_PRINT(logfile, "  signature %d [0x%X], valuesize %d size %d ", signature, signature, valuesize, col_data_size)


	unsigned int current_col = 1;//, nr = 0, nbytes = 0;
	static unsigned int col_index = 0;
	unsigned int current_sheet = 0;
	int spread = 0;

	if (column_name.empty()) { // Matrix or function
		if (valuesize == col_data_size) { // only one row: Function
			functions.push_back(Function(name, dataIndex));
			++dataIndex;
			Origin::Function &f = functions.back();
			f.formula = toLowerCase(col_data.c_str());

			stmp.str(col_header.substr(0x0A));
			short t;
			GET_SHORT(stmp, t)
			if (t == 0x1194)
				f.type = Function::Polar;

			stmp.str(col_header.substr(0x21));
			GET_INT(stmp, f.totalPoints)
			GET_DOUBLE(stmp, f.begin)
			double d;
			GET_DOUBLE(stmp, d)
			f.end = f.begin + d*(f.totalPoints - 1);

			LOG_PRINT(logfile, "\n NEW FUNCTION: %s = %s", f.name.c_str(), f.formula.c_str());
			LOG_PRINT(logfile, ". Range [%g : %g], number of points: %d\n", f.begin, f.end, f.totalPoints);

		} else { // Matrix
			int mIndex = -1;
			string::size_type pos = name.find_first_of("@");
			if (pos != string::npos){
				string sheetName = name;
				name.resize(pos);
				mIndex = findMatrixByName(name);
				if (mIndex != -1){
					LOG_PRINT(logfile, "\n  NEW MATRIX SHEET\n");
					matrixes[mIndex].sheets.push_back(MatrixSheet(sheetName, dataIndex));
				}
			} else {
				LOG_PRINT(logfile, "\n  NEW MATRIX\n");
				matrixes.push_back(Matrix(name));
				matrixes.back().sheets.push_back(MatrixSheet(name, dataIndex));
			}
			++dataIndex;
			getMatrixValues(col_data, col_data_size, data_type, data_type_u, valuesize, mIndex);
		}
	} else {
		if(speadSheets.size() == 0 || findSpreadByName(name) == -1) {
			LOG_PRINT(logfile, "\n  NEW SPREADSHEET\n");
			current_col = 1;
			speadSheets.push_back(SpreadSheet(name));
			spread = speadSheets.size() - 1;
			speadSheets.back().maxRows = 0;
			current_sheet = 0;
		} else {
			spread = findSpreadByName(name);
			current_col = speadSheets[spread].columns.size();
			if(!current_col)
				current_col = 1;
			++current_col;
		}
		speadSheets[spread].columns.push_back(SpreadColumn(column_name, dataIndex));
		speadSheets[spread].columns.back().colIndex = ++col_index;

		string::size_type sheetpos = speadSheets[spread].columns.back().name.find_last_of("@");
		if(sheetpos != string::npos){
			unsigned int sheet = strtol(column_name.substr(sheetpos + 1).c_str(), 0, 10);
			if( sheet > 1){
				speadSheets[spread].columns.back().name = column_name;

				if (current_sheet != (sheet - 1))
					current_sheet = sheet - 1;

				speadSheets[spread].columns.back().sheet = current_sheet;
				if (speadSheets[spread].sheets < sheet)
					speadSheets[spread].sheets = sheet;
			}
		}
		++dataIndex;
		LOG_PRINT(logfile, "  data index %d, valuesize %d, ", dataIndex, valuesize)

		unsigned int nr = col_data_size / valuesize;
		LOG_PRINT(logfile, "n. of rows = %d\n\n", nr)

		speadSheets[spread].maxRows<nr ? speadSheets[spread].maxRows=nr : 0;
		stmp.str(col_data);
		for(unsigned int i = 0; i < nr; ++i)
		{
			double value;
			if(valuesize <= 8)	// Numeric, Time, Date, Month, Day
			{
				GET_DOUBLE(stmp, value)
				if ((i < 5) or (i > (nr-5))) {
					LOG_PRINT(logfile, "%g ", value)
				} else if (i == 5) {
					LOG_PRINT(logfile, "... ")
				}
				speadSheets[spread].columns[(current_col-1)].data.push_back(value);
			}
			else if((data_type & 0x100) == 0x100) // Text&Numeric
			{
				unsigned char c = col_data[i*valuesize];
				stmp.seekg(i*valuesize+2, ios_base::beg);
				if(c != 1) //value
				{
					GET_DOUBLE(stmp, value);
					if ((i < 5) or (i > (nr-5))) {
						LOG_PRINT(logfile, "%g ", value)
					} else if (i == 5) {
						LOG_PRINT(logfile, "... ")
					}
					speadSheets[spread].columns[(current_col-1)].data.push_back(value);
				}
				else //text
				{
					string svaltmp = col_data.substr(i*valuesize+2, valuesize-2).c_str();
					// TODO: check if this test is still needed
					if(svaltmp.find(0x0E) != string::npos) { // try find non-printable symbol - garbage test
						svaltmp = string();
						LOG_PRINT(logfile, "Non printable symbol found, place 1 for i=%d\n", i)
					}
					if ((i < 5) or (i > (nr-5))) {
						LOG_PRINT(logfile, "\"%s\" ", svaltmp.c_str())
					} else if (i == 5) {
						LOG_PRINT(logfile, "... ")
					}
					speadSheets[spread].columns[(current_col-1)].data.push_back(svaltmp);
				}
			}
			else //text
			{
				string svaltmp = col_data.substr(i*valuesize, valuesize).c_str();
				// TODO: check if this test is still needed
				if(svaltmp.find(0x0E) != string::npos) { // try find non-printable symbol - garbage test
					svaltmp = string();
					LOG_PRINT(logfile, "Non printable symbol found, place 2 for i=%d\n", i)
				}
				if ((i < 5) or (i > (nr-5))) {
					LOG_PRINT(logfile, "\"%s\" ", svaltmp.c_str())
				} else if (i == 5) {
					LOG_PRINT(logfile, "... ")
				}
				speadSheets[spread].columns[(current_col-1)].data.push_back(svaltmp);
			}
		}
		LOG_PRINT(logfile, "\n\n")
	}

	return true;
}

void OriginAnyParser::getMatrixValues(string col_data, unsigned int col_data_size, short data_type, char data_type_u, char valuesize, int mIndex)
{
	if (matrixes.empty())
		return;

	istringstream stmp;
	stmp.str(col_data);

	if (mIndex < 0)
		mIndex = matrixes.size() - 1;

	int size = col_data_size/valuesize;
	bool logValues = true;
	switch(data_type){
		case 0x6001://double
			for(unsigned int i = 0; i < size; ++i){
				double value;
				GET_DOUBLE(stmp, value)
				matrixes[mIndex].sheets.back().data.push_back((double)value);
			}
			break;
		case 0x6003://float
			for(unsigned int i = 0; i < size; ++i){
				float value;
				GET_FLOAT(stmp, value)
				matrixes[mIndex].sheets.back().data.push_back((double)value);
			}
			break;
		case 0x6801://int
			if (data_type_u == 8){//unsigned
				for(unsigned int i = 0; i < size; ++i){
					unsigned int value;
					GET_INT(stmp, value)
					matrixes[mIndex].sheets.back().data.push_back((double)value);
				}
			} else {
				for(unsigned int i = 0; i < size; ++i){
					int value;
					GET_INT(stmp, value)
					matrixes[mIndex].sheets.back().data.push_back((double)value);
				}
			}
			break;
		case 0x6803://short
			if (data_type_u == 8){//unsigned
				for(unsigned int i = 0; i < size; ++i){
					unsigned short value;
					GET_SHORT(stmp, value)
					matrixes[mIndex].sheets.back().data.push_back((double)value);
				}
			} else {
				for(unsigned int i = 0; i < size; ++i){
					short value;
					GET_SHORT(stmp, value)
					matrixes[mIndex].sheets.back().data.push_back((double)value);
				}
			}
			break;
		case 0x6821://char
			if (data_type_u == 8){//unsigned
				for(unsigned int i = 0; i < size; ++i){
					unsigned char value;
					value = col_data[i];
					matrixes[mIndex].sheets.back().data.push_back((double)value);
				}
			} else {
				for(unsigned int i = 0; i < size; ++i){
					char value;
					value = col_data[i];
					matrixes[mIndex].sheets.back().data.push_back((double)value);
				}
			}
			break;
		default:
			LOG_PRINT(logfile, "	UNKNOWN MATRIX DATATYPE: %02X SKIP DATA\n", data_type);
			matrixes.pop_back();
			logValues = false;
	}

	if (logValues){
		LOG_PRINT(logfile, "	FIRST 10 CELL VALUES: ");
		for(unsigned int i = 0; i < 10 && i < matrixes[mIndex].sheets.back().data.size(); ++i)
			LOG_PRINT(logfile, "%g\t", matrixes[mIndex].sheets.back().data[i]);
	}
}
