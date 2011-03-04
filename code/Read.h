/*
 	Ray
    Copyright (C)  2010  Sébastien Boisvert

	http://DeNovoAssembler.SourceForge.Net/

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You have received a copy of the GNU General Public License
    along with this program (COPYING).  
	see <http://www.gnu.org/licenses/>

*/

// TYPE: DATA

#ifndef _Read
#define _Read

#include<string>
#include<stdint.h>
#include<vector>
#include<common_functions.h>
#include<MyAllocator.h>
#include<PairedRead.h>
using namespace std;

#define TYPE_SINGLE_END 0
#define TYPE_LEFT_END 1
#define TYPE_RIGHT_END 2

/**
 * a read is represented as a char*
 * and a (possible) link to paired information.
 */
class Read{
	uint8_t *m_sequence;
	PairedRead m_pairedRead;// the read on the left
	uint16_t m_length;
	uint8_t m_type;

	char*trim(char*a,const char*b);
public:
	void constructor(const char*sequence,MyAllocator*seqMyAllocator,bool trim);
	void getSeq(char*buffer)const;
	int length()const;
	uint64_t getVertex(int pos,int w,char strand,bool color)const;
	bool hasPairedRead()const;
	PairedRead*getPairedRead();
	uint8_t*getRawSequence();
	int getRequiredBytes();
	void setRawSequence(uint8_t*seq,int length);
	void setRightType();
	void setLeftType();
	int getType();
	void setType(uint8_t type);
}
#ifdef FORCE_PACKING
__attribute__((__packed__))
#endif
;

#endif
