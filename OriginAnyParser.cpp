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

	return true;
}

OriginParser* createOriginAnyParser(const string& fileName)
{
	return new OriginAnyParser(fileName);
}
