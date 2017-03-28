/***************************************************************************
	File                 : endianfstream.hh
	--------------------------------------------------------------------
	Copyright            : (C) 2008 Alex Kargovsky
						   Email (use @ for *)  : kargovsky*yumr.phys.msu.su
	Description          : Endianless file stream class

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

#ifndef ENDIAN_FSTREAM_H
#define ENDIAN_FSTREAM_H

#include <fstream>
#include "OriginObj.h"

namespace std
{
	class iendianfstream : public ifstream
	{
	    
	public:
	    
	    ProgressCallback callback;
	    void *callback_user_data;
	
		iendianfstream(const char *_Filename, ios_base::openmode _Mode = ios_base::in)
			:	ifstream(_Filename, _Mode), callback(NULL), callback_user_data(NULL)
		{
			short word = 0x4321;
			bigEndian = (*(char*)& word) != 0x21;
			
			// get file size
			ifstream tmp(_Filename, ios::binary | ios::ate);
			size = tmp.tellg();
			tmp.close();
			
			lastProgress = 0;
		};

		iendianfstream& operator>>(bool& value)
		{
			char c;
			get(c);
			value = (c != 0);
			tell();
			return *this;
		}

		iendianfstream& operator>>(char& value)
		{
			get(value);
			tell();
			return *this;
		}

		iendianfstream& operator>>(unsigned char& value)
		{
			get(reinterpret_cast<char&>(value));
			tell();
			return *this;
		}

		iendianfstream& operator>>(short& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			tell();
			return *this;
		}

		iendianfstream& operator>>(unsigned short& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			tell();
			return *this;
		}

		iendianfstream& operator>>(int& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			tell();
			return *this;
		}

		iendianfstream& operator>>(unsigned int& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			tell();
			return *this;
		}

		iendianfstream& operator>>(long& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			tell();
			return *this;
		}

		iendianfstream& operator>>(unsigned long& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			tell();
			return *this;
		}

		iendianfstream& operator>>(float& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			tell();
			return *this;
		}

		iendianfstream& operator>>(double& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			tell();
			return *this;
		}

		iendianfstream& operator>>(long double& value)
		{
			read(reinterpret_cast<char*>(&value), sizeof(value));
			if(bigEndian)
				swap_bytes(reinterpret_cast<unsigned char*>(&value), sizeof(value));

			tell();
			return *this;
		}

		iendianfstream& operator>>(string& value)
		{
			read(reinterpret_cast<char*>(&value[0]), value.size());
			string::size_type pos = value.find_first_of('\0');
			if(pos != string::npos)
				value.resize(pos);

			tell();
			return *this;
		}

		iendianfstream& operator>>(Origin::Color& value)
		{
			unsigned char color[4];
			read(reinterpret_cast<char*>(&color), sizeof(color));
			switch(color[3])
			{
			case 0:
				if(color[0] < 0x64)
				{
					value.type = Origin::Color::Regular;
					value.regular = color[0];
				}
				else
				{
					switch(color[2])
					{
					case 0:
						value.type = Origin::Color::Indexing;
						break;
					case 0x40:
						value.type = Origin::Color::Mapping;
						break;
					case 0x80:
						value.type = Origin::Color::RGB;
						break;
					}

					value.column = color[0] - 0x64;
				}
				
				break;
			case 1:
				value.type = Origin::Color::Custom;
				for(int i = 0; i < 3; ++i)
					value.custom[i] = color[i];
				break;
			case 0x20:
				value.type = Origin::Color::Increment;
				value.starting = color[1];
				break;
			case 0xFF:
				if(color[0] == 0xFC)
					value.type = Origin::Color::None;
				else if(color[0] == 0xF7)
					value.type = Origin::Color::Automatic;

				break;

			default:
				value.type = Origin::Color::Regular;
				value.regular = color[0];
				break;

			}

			tell();
			return *this;
		}

	private:
		bool bigEndian;
		long size;
		void swap_bytes(unsigned char* data, int size)
		{
			register int i = 0;
			register int j = size - 1;
			while(i < j)
			{
				std::swap(data[i], data[j]);
				++i, --j;
			}
		}
		int lastProgress;
		inline void tell(void)
		{
		    long pos = tellg();
		    if(callback && callback_user_data)// && pos % 10240 /*x 10KB*/ == 0)
		    {
		        double progress = pos / (double)size;
		        int currProgress = 100 * progress;
		        if(currProgress > lastProgress)
		        {
		            cout << "\rliborigin: " << currProgress << "% done  ";
		            cout.flush();
		            callback(0.9 * progress, callback_user_data);
		        }
		        lastProgress = currProgress;
		    }
		}
	};
}

#endif // ENDIAN_FSTREAM_H
