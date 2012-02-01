/*
 	Ray
    Copyright (C) 2010, 2011, 2012 Sébastien Boisvert

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

#ifndef _ColorSet_h
#define _ColorSet_h

#include <plugin_Searcher/VirtualKmerColor.h>

#include <stdint.h>
#include <vector>
#include <map>
using namespace std;

// maximum value for a uint64_t:
// 18446744073709551615
// xxxx000yyyyyyyyyyyyy
//    10000000000000000
#define COLOR_NAMESPACE 10000000000000000

typedef uint32_t VirtualKmerColorHandle;

/** This class is a translation table for
 * allocated virtual colors. 
 *
 * This is the Flyweight design pattern.
 *
 * \author: Sébastien Boisvert
 *
 * Frédéric Raymond proposed the idea of using color namespaces.
 */
class ColorSet{

	int OPERATION_NO_VIRTUAL_COLOR_HAS_PHYSICAL_COLOR_CREATION;
	int OPERATION_NO_VIRTUAL_COLOR_HAS_COUNT_CREATION;
	int OPERATION_NO_VIRTUAL_COLOR_HAS_HASH_CREATION;
	int OPERATION_VIRTUAL_COLOR_HAS_COLORS_FETCH;
	int OPERATION_NO_VIRTUAL_COLOR_HAS_COLORS_CREATION;
	int  OPERATION_NEW_FROM_EMPTY;
	int OPERATION_NEW_FROM_SCRATCH;
	int  OPERATION_applyHashOperation;
	int OPERATION_getHash;
	int  OPERATION_getVirtualColorFrom;
	int OPERATION_createVirtualColorFrom ;
	int OPERATION_incrementReferences ;
	int OPERATION_decrementReferences ;
	int OPERATION_purgeVirtualColor ;
	int OPERATION_allocateVirtualColorHandle ;
	int OPERATION_DUMMY ;


	uint64_t m_operations[16];

/** a list of available handles **/
	set<VirtualKmerColorHandle> m_availableHandles;

/** the table of virtual colors **/
	vector<VirtualKmerColor> m_virtualColors;

/** a list of physical colors **/
	set<PhysicalKmerColor> m_physicalColors;

	map<PhysicalKmerColor,map<int,map<uint64_t,set<VirtualKmerColorHandle> > > > m_index;

	uint64_t m_collisions;



/** get the hash value for a set of colors **/
	uint64_t getHash(set<PhysicalKmerColor>*colors);
/** allocates a virtual color handle **/
	VirtualKmerColorHandle allocateVirtualColorHandle();

	VirtualKmerColor*getVirtualColor(VirtualKmerColorHandle handle);

/** get a handle to a virtual color */
	VirtualKmerColorHandle getVirtualColorHandle(set<PhysicalKmerColor>*colors);

	void purgeVirtualColor(VirtualKmerColorHandle handle);

	uint64_t applyHashOperation(uint64_t hashValue,PhysicalKmerColor color);

	VirtualKmerColorHandle createVirtualColorFrom(VirtualKmerColorHandle handle,PhysicalKmerColor color);

	bool virtualColorHasAllPhysicalColorsOf(VirtualKmerColorHandle toInvestigate,VirtualKmerColorHandle list);
public:
	
	ColorSet();

/** increment the references for a virtual color **/
	void incrementReferences(VirtualKmerColorHandle handle);

/** decrement the references for a virtual color **/
	void decrementReferences(VirtualKmerColorHandle handle);
	
	int getNumberOfReferences(VirtualKmerColorHandle handle);

	int getNumberOfPhysicalColors(VirtualKmerColorHandle handle);

	VirtualKmerColorHandle getVirtualColorFrom(VirtualKmerColorHandle handle,PhysicalKmerColor color);

	int getTotalNumberOfPhysicalColors();
	int getTotalNumberOfVirtualColors();

	void printSummary();
	void printColors();

	set<PhysicalKmerColor>*getPhysicalColors(VirtualKmerColorHandle handle);

	bool virtualColorHasPhysicalColor(VirtualKmerColorHandle handle,PhysicalKmerColor color);
};

#endif
