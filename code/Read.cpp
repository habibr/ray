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

#include<assert.h>
#include<common_functions.h>
#include<Read.h>
#include<cstdlib>
#include<iostream>
#include<cstring>

using namespace  std;

// #define __READ_VERBOSITY

char*Read::trim(char*buffer,const char*sequence){
	int theLen=strlen(sequence);
	strcpy(buffer,sequence);
	for(int i=0;i<theLen;i++){
		if(buffer[i]=='a')
			buffer[i]='A';
		else if(buffer[i]=='t')
			buffer[i]='T';
		else if(buffer[i]=='c')
			buffer[i]='C';
		else if(buffer[i]=='g')
			buffer[i]='G';
	}
	// discard N at the beginning and end of the read.
	// find the first symbol that is a A,T,C or G
	int first=0;
	while(buffer[first]!='A' and buffer[first]!='T' and buffer[first]!='C' and buffer[first]!='G' and first<theLen){
		first++;
	}
	char*corrected=buffer+first;
	// find the last symbol that is a A,T,C, or G
	int last=0;
	for(int i=0;i<(int)strlen(corrected);i++){
		if(corrected[i]=='A' or corrected[i]=='T' or corrected[i]=='C' or corrected[i]=='G'){
			last=i;
		}
	}
	last++;
	// only junk awaits beyond <last>
	corrected[last]='\0';
	//cout<<"OUT="<<corrected<<endl;
	return corrected;
}

void Read::constructor(const char*sequence,MyAllocator*seqMyAllocator,bool trimFlag){
	#ifdef __READ_VERBOSITY
	cout<<"In="<<sequence<<endl;
	#endif
	if(trimFlag && strlen(sequence)<4096){
		char buffer[4096];
		sequence=trim(buffer,sequence);
	}
	int length=strlen(sequence);
	int requiredBits=2*length;
	int modulo=requiredBits%8;
	if(modulo!=0){
		int bitsToAdd=8-modulo;
		requiredBits+=bitsToAdd;
	}

	#ifdef ASSERT
	assert(requiredBits%8==0);
	#endif

	int requiredBytes=requiredBits/8;

	uint8_t workingBuffer[4096];
	for(int i=0;i<requiredBytes;i++){
		workingBuffer[i]=0;
	}

	for(int position=0;position<length;position++){
		char nucleotide=sequence[position];
		if(nucleotide!='A'&&nucleotide!='T'&&nucleotide!='C'&&nucleotide!='G'){
			nucleotide='A';
		}
		uint8_t code=charToCode(nucleotide);
		#ifdef __READ_VERBOSITY
		if(position%4==0){
			cout<<"|";
		}
		cout<<" "<<(int)code;
		#endif
		int positionInWorkingBuffer=position/4;
		int codePositionInWord=position%4;
		uint8_t wordToUpdate=workingBuffer[positionInWorkingBuffer];
		// shift the code and or with the word to update
		code=(code<<(codePositionInWord*2));
		wordToUpdate=wordToUpdate|code;
		workingBuffer[positionInWorkingBuffer]=wordToUpdate;
	}
	#ifdef __READ_VERBOSITY
	cout<<endl;
	for(int i=0;i<requiredBytes;i++){
		cout<<" "<<(int)workingBuffer[i];
	}

	cout<<endl;
	#endif

	m_length=length;

	m_sequence=(uint8_t*)seqMyAllocator->allocate(requiredBytes*sizeof(uint8_t));
	memcpy(m_sequence,workingBuffer,requiredBytes);
	m_pairedRead=NULL;

	#ifdef __READ_VERBOSITY
	cout<<"Out="<<getSeq()<<endl;
	cout<<endl;
	#endif
}

string Read::getSeq() const{
	char workingBuffer[4096];

	for(int position=0;position<m_length;position++){
		int positionInWorkingBuffer=position/4;
		uint8_t word=m_sequence[positionInWorkingBuffer];
		int codePositionInWord=position%4;
		uint8_t code=(word<<(6-codePositionInWord*2));//eliminate bits before
		code=(code>>6);
		char nucleotide=codeToChar(code);
		workingBuffer[position]=nucleotide;
	}
	workingBuffer[m_length]='\0';
	string aString(workingBuffer);

	return aString;
}

int Read::length()const{
	return getSeq().length();
}

/*                      
 *           -----------------------------------
 *           -----------------------------------
 *                     p p-1 p-2               0
 */
uint64_t Read::getVertex(int pos,int w,char strand,bool color) const {
	string seq=getSeq();
	return kmerAtPosition(seq.c_str(),pos,w,strand,color);
}

void Read::setPairedRead(PairedRead*t){
	m_pairedRead=t;
}

bool Read::hasPairedRead()const{
	return m_pairedRead!=NULL;
}

PairedRead*Read::getPairedRead()const{
	return m_pairedRead;
}

