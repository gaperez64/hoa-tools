/**************************************************************************
 * Copyright (c) 2019- Guillermo A. Perez
 * 
 * This file is part of HOATOOLS.
 * 
 * HOATOOLS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * HOATOOLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with HOATOOLS. If not, see <http://www.gnu.org/licenses/>.
 * 
 * Guillermo A. Perez
 * University of Antwerp
 * guillermoalberto.perez@uantwerpen.be
 *************************************************************************/

#ifndef _SIMPLEHOA_H
#define _SIMPLEHOA_H

#include <stdio.h>
#include <stdbool.h>

/* All tree-like constructs will be BTrees, for binary tree. The
 * following enumerate structure gives us the types of nodes 
 * that could appear in such trees.
 */
typedef enum {
    NT_BOOL, NT_AND, NT_OR, NT_FIN,
    NT_INF, NT_NOT, NT_SET,
    NT_AP, NT_ALIAS
} NodeType;

typedef struct BTree BTree;
struct BTree {
    BTree* left;
    BTree* right;
    char* alias;
    int id;
    NodeType type;
};

/* Boiler-plate singly linked lists structures
 * for information in the HOA files
 */
typedef struct StringList StringList;
struct StringList {
    char* str;
    StringList* next;
};

typedef struct IntList IntList;
struct IntList {
    int i;
    IntList* next;
};

typedef struct TransList TransList;
struct TransList {
    BTree* label;
    IntList* successors;
    IntList* accSig;
    TransList* next;
};

typedef struct StateList StateList;
struct StateList {
    int id;
    char* name;
    BTree* label;
    IntList* accSig;
    TransList* transitions;
    StateList* next;
};

typedef struct AliasList AliasList;
struct AliasList {
    char* alias;
    BTree* labelExpr;
    AliasList* next;
};

/* The centralized data structure for all data collected
 * from the file is the following.
 */
typedef struct HoaData {
    int noStates;
    StringList* aps;
    StringList* accNameParameters;
    StringList* properties;
    StateList* states;
    AliasList* aliases;
    IntList* start;
    IntList* cntAPs;
    int noAccSets;
    int noAPs;
    BTree* acc;
    char* version;
    char* accNameID;
    char* toolName;
    char* toolVersion;
    char* name;
} HoaData;

// This function returns 0 if and only if parsing was successful
int parseHoa(FILE*, HoaData*);

// Defaults and destructor for centralized data structure
void defaultsHoa(HoaData*);
void deleteHoa(HoaData*);

// list management functions
StateList* newStateNode(int, char*, BTree*, IntList*);
StateList* prependStateNode(StateList*, StateList*, TransList*);
TransList* prependTransNode(TransList*, BTree*, IntList*, IntList*);
IntList* newIntNode(int);
IntList* prependIntNode(IntList*, int);
StringList* prependStrNode(StringList*, char*);
AliasList* prependAliasNode(AliasList*, char*, BTree*);
StringList* concatStrLists(StringList*, StringList*);
IntList* concatIntLists(IntList*, IntList*);

// tree management functions
BTree* boolBTree(bool);
BTree* andBTree(BTree*, BTree*);
BTree* orBTree(BTree*, BTree*);
BTree* notBTree(BTree*);
BTree* aliasBTree(char*);
BTree* accidBTree(NodeType, int, bool);
BTree* apBTree(int);

// For debugging purposes, this prints all data in human-readable form
void printHoa(const HoaData*);

#endif
