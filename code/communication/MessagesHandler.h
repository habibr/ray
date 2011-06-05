/*
 	Ray
    Copyright (C) 2010, 2011  Sébastien Boisvert

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

#ifndef _MessagesHandler
#define _MessagesHandler

#include<vector>
#include<MyAllocator.h>
#include<Message.h>
#include<common_functions.h>
#include<RingAllocator.h>
#include<map>
#include<StaticVector.h>
using namespace std;

/**
 * software layer to handler messages
 * it uses persistant communication
 */
class MessagesHandler{
	int m_ringSize;
	int m_head;
	MPI_Request*m_ring;
	char*m_buffers;
	int m_rank;
	int m_size;

	#ifdef COUNT_MESSAGES
	uint64_t*m_receivedMessages;
	uint64_t*m_allReceivedMessages;
	int*m_allCounts;
	map<int,map<int,int> > m_buckets;
	#endif

public:
	void constructor(int rank,int size);
	/**
 *  send a message or more
 */
	void sendMessages(StaticVector*outbox,int source);
	/**
 * receive one or zero message.
 * the others, if any, will be picked up in the next iteration
 */
	void receiveMessages(StaticVector*inbox,RingAllocator*inboxAllocator,int destination);

	#ifdef COUNT_MESSAGES
	bool isFinished(int rank);
	void showStats();
	uint64_t*getReceivedMessages();
	void addCount(int rank,uint64_t count);
	void writeStats(const char*file);
	bool isFinished();
	#endif
	void freeLeftovers();
};

#endif