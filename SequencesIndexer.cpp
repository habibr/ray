/*
 	Ray
    Copyright (C) 2010  Sébastien Boisvert

	http://DeNovoAssembler.SourceForge.Net/

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You have received a copy of the GNU General Public License
    along with this program (gpl-3.0.txt).  
	see <http://www.gnu.org/licenses/>

*/

#include<Parameters.h>
#include<DistributionData.h>
#include<Loader.h>
#include<SequencesIndexer.h>
#include<common_functions.h>
#include<Message.h>

void SequencesIndexer::attachReads(
	vector<Message>*m_outbox,
	int*m_distribution_file_id,
	int*m_distribution_sequence_id,
	int rank,
	vector<Read*>*m_distribution_reads,
	int size,
	MyAllocator*m_distributionAllocator,
	int*m_distribution_currentSequenceId,
int m_wordSize,
	DistributionData*m_disData,
	bool*m_mode_AttachSequences,
	Parameters*m_parameters,
	bool*m_colorSpaceMode,
	MyAllocator*m_outboxAllocator
){
	vector<string> allFiles=(*m_parameters).getAllFiles();
	if((*m_distribution_reads).size()>0 and (*m_distribution_sequence_id)>(int)(*m_distribution_reads).size()-1){
		(*m_distribution_file_id)++;
		(*m_distribution_sequence_id)=0;
		(*m_distribution_reads).clear();
	}
	if((*m_distribution_file_id)>(int)allFiles.size()-1){
		flushAttachedSequences(1,m_outbox,rank,size,m_disData,m_outboxAllocator);
		for(int i=0;i<size;i++){
			Message aMessage(NULL, 0, MPI_UNSIGNED_LONG_LONG, i, TAG_MASTER_IS_DONE_ATTACHING_READS,rank);
			(*m_outbox).push_back(aMessage);
		}
		(*m_distribution_reads).clear();
		(*m_distributionAllocator).clear();
		(*m_mode_AttachSequences)=false;
		return;
	}
	if((*m_distribution_reads).size()==0){
		Loader loader;
		(*m_distribution_reads).clear();
		(*m_distributionAllocator).clear();
		(*m_distributionAllocator).constructor(DISTRIBUTION_ALLOCATOR_CHUNK_SIZE);
		#ifdef SHOW_PROGRESS
		cout<<"Rank "<<rank<<" loads "<<allFiles[(*m_distribution_file_id)]<<"."<<endl;
		#else
		cout<<"\r"<<"Loading "<<allFiles[(*m_distribution_file_id)]<<""<<endl;
		#endif
		loader.load(allFiles[(*m_distribution_file_id)],&(*m_distribution_reads),&(*m_distributionAllocator),&(*m_distributionAllocator));
		
		#ifdef SHOW_PROGRESS
		cout<<"Rank "<<rank<<" "<<(*m_distribution_reads).size()<<" sequences to attach"<<endl;
		#else
		cout<<"Indexing sequences"<<endl;
		#endif
	}
	#ifndef SHOW_PROGRESS
	time_t tmp=time(NULL);
	if(tmp>m_lastTime){
		m_lastTime=tmp;
		showProgress(m_lastTime);
	}
	#endif

	for(int i=0;i<1;i++){
		if((*m_distribution_sequence_id)>(int)(*m_distribution_reads).size()-1){
			#ifdef SHOW_PROGRESS
			cout<<"Rank "<<rank<<" attaches sequences, "<<(*m_distribution_reads).size()<<"/"<<(*m_distribution_reads).size()<<endl;
			#endif
			break;
		}

		int destination=(*m_distribution_currentSequenceId)%size;
		int sequenceIdOnDestination=(*m_distribution_currentSequenceId)/size;

		if(destination<0 or destination>size-1){
			cout<<destination<<" is bad"<<endl;
		}

		#ifdef SHOW_PROGRESS
		if((*m_distribution_sequence_id)%1000000==0){
			cout<<"Rank "<<rank<<" attaches sequences, "<<(*m_distribution_sequence_id)<<"/"<<(*m_distribution_reads).size()<<endl;
		}
		#endif

		char*sequence=((*m_distribution_reads))[(*m_distribution_sequence_id)]->getSeq();
		if((int)strlen(sequence)<m_wordSize){
			(*m_distribution_currentSequenceId)++;
			(*m_distribution_sequence_id)++;
			return;
		}
		char vertexChar[100];
		memcpy(vertexChar,sequence,m_wordSize);
		vertexChar[m_wordSize]='\0';
		if(isValidDNA(vertexChar)){
			VERTEX_TYPE vertex=wordId(vertexChar);
			int sendTo=vertexRank(vertex,size);
			m_disData->m_attachedSequence.addAt(sendTo,vertex);
			m_disData->m_attachedSequence.addAt(sendTo,destination);
			m_disData->m_attachedSequence.addAt(sendTo,sequenceIdOnDestination);
			m_disData->m_attachedSequence.addAt(sendTo,(VERTEX_TYPE)'F');
		}
		memcpy(vertexChar,sequence+strlen(sequence)-m_wordSize,m_wordSize);
		vertexChar[m_wordSize]='\0';
		if(isValidDNA(vertexChar)){
			VERTEX_TYPE vertex=complementVertex(wordId(vertexChar),m_wordSize,(*m_colorSpaceMode));
			int sendTo=vertexRank(vertex,size);
			m_disData->m_attachedSequence.addAt(sendTo,vertex);
			m_disData->m_attachedSequence.addAt(sendTo,destination);
			m_disData->m_attachedSequence.addAt(sendTo,sequenceIdOnDestination);
			m_disData->m_attachedSequence.addAt(sendTo,(VERTEX_TYPE)'R');
		}

		flushAttachedSequences(MAX_UINT64_T_PER_MESSAGE,m_outbox,rank,size,m_disData,m_outboxAllocator);

		(*m_distribution_currentSequenceId)++;
		(*m_distribution_sequence_id)++;
	}

}

void SequencesIndexer::flushAttachedSequences(int threshold,
	vector<Message>*m_outbox,
	int rank,
	int size,
	DistributionData*m_disData,
	MyAllocator*m_outboxAllocator
){
	for(int rankId=0;rankId<size;rankId++){
		int sendTo=rankId;
		int count=m_disData->m_attachedSequence.size(rankId);
		if(count<threshold)
			continue;
		VERTEX_TYPE*message=(VERTEX_TYPE*)(*m_outboxAllocator).allocate(count*sizeof(VERTEX_TYPE));
		for(int j=0;j<count;j++)
			message[j]=m_disData->m_attachedSequence.getAt(rankId,j);
		Message aMessage(message,count, MPI_UNSIGNED_LONG_LONG,sendTo,TAG_ATTACH_SEQUENCE,rank);
		(*m_outbox).push_back(aMessage);
		m_disData->m_attachedSequence.reset(rankId);
	}
}