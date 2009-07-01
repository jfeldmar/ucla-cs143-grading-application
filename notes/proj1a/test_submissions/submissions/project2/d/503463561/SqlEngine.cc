/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "BTreeIndex.h"
#include "SqlEngine.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning
  BTreeIndex idx;  // B+tree index 

  RC     rc;
  int    key;     
  string value;
  int    count;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  // Open the corresponding index file, if it exists. The boolean
  // value index indicates whether we have an index file we can use.
  // We open for read-only, so this will fail if the file is not available.
  bool indexExists = (idx.open(table + ".idx",'r') == 0);

  // Boundaries for key interval
  int low, high;
  // Is this the first time setting boundaries?
  bool lowerBound = false;
  bool upperBound = false;
  // Is the range empty?
  bool emptyRange = false;
  
  // Using conditions narrow down interval for B+tree index
  // Open B-tree index only if a condition exists on the *key*, not the *value*
  for (unsigned i = 0; i < cond.size(); i++) {
  		// Skip any conditions on the *value* of the record; the B-tree
		// can only help us speed up selections with conditions on the *key* of
		// the record.
		if (cond[i].attr != 1)
			continue;
		key = atoi(cond[i].value); // Get the key as an integer
      // Figure out the condition and set interval based on it
      switch (cond[i].comp) {
			case SelCond::EQ:
				if ((lowerBound && key < low) || (upperBound && key > high))
					emptyRange = true;
				else {
					low = key;
					high = key;
					lowerBound = upperBound = true;
				}
				break;
			case SelCond::NE:
				// If an equality exists on the key, then the range is empty (TODO)
				break;
			case SelCond::GT:
				if (upperBound && key >= high)
					emptyRange = true;
				if (!lowerBound || key >= low) {
					low = key + 1;
					lowerBound = true;
					}
				break;
			case SelCond::LT:
				if (lowerBound && key <= low)
					emptyRange = true;
				if (!upperBound || key <= high) {
					high = key - 1;
					upperBound = true;
					}
				break;
			case SelCond::GE:
				if (upperBound && key > high)
					emptyRange = true;
				if (!lowerBound || key > low) {
					low = key;
					lowerBound = true;
					}
				break;
			case SelCond::LE:
				if (lowerBound && key < low)
					emptyRange = true;
				if (!upperBound || key < high) {
					high = key;
					upperBound = true;
					}
				break;
			}
		if (emptyRange) { // Stop if we know the range is empty
			//fprintf(stderr,"Empty range!\n");
			break;
			}
  		}
	
//  if (indexExists)
//  	  fprintf(stderr, "Index exists!\n");
//  if (lowerBound || upperBound)
//  	  fprintf(stderr, "Bounds are (%d,%d)\n",low,high);

  // Determine whether we should use B+tree index or not
  // We only want to use the B+tree index when it exists, and some lower, upper
  // boundary has been defined for the key to limit our search.
  bool usingIndex = indexExists && (lowerBound || upperBound);
  count = 0;

  // In the special case of SELECT COUNT(*) with no conditions
  // return the number of entries in our B+ tree
  if (indexExists && attr == 4 && cond.size() == 0)
  	  count = idx.getNumEntries();
  else if (emptyRange); // If we know there is no possible tuple that can satisfy key
  						 		// condition, we break
  else if (usingIndex) { // Iterate using the B+tree index along a limited range
  		IndexCursor lowCursor; // Cursor next to lower bound
		if (lowerBound) { // If a lower bound exists
  			rc = idx.locate(low,lowCursor); // Get the pid,rid of the low key
			if (rc < 0 && rc != RC_NO_SUCH_RECORD) return rc;
			}
		else { // Set the pid and eid of the low cursor to first entry in first leaf node
			lowCursor.pid = 1;
			lowCursor.eid = 0;
			}

		// Iterate along the B+ tree until either the end is reached or the key is 
		// greater than the upper bound (if one exists)
		while ( (idx.readForward(lowCursor,key,rid) == 0) 
		  	  && (!upperBound || key <= high) ) {
			if ( (rc = checkTuple(attr, rf, rid, count, cond)) < 0) {
				fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
				rf.close();
				idx.close();
				return rc;
				}
			}
  		}
  else { // Iterate from the beginning of the table to the end
  	  rid.pid = rid.sid = 0;
	  //fprintf(stderr, "Not using B+ tree\n");
	  while (rid < rf.endRid()) {
	 	 if ( (rc = checkTuple(attr, rf, rid, count, cond)) < 0) {
			fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
		 	rf.close();
			return rc;
		 	}
		 // move to the next tuple
		 ++rid;
	  }
  }

  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  rf.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */
	RecordFile rf;   // RecordFile containing the table
	RecordId   rid;
	BTreeIndex idx;

	RC     rc;
	int    key;  
	string line;
	string value;
	ifstream readdata(loadfile.c_str());

	// If index == true, then Bruinbase creates corresponding B+ tree
	// index on the key column of the table
	if (index)
		if ( (rc = idx.open(table + ".idx",'w')) < 0) return rc;

	// open the table file
	if ((rc = rf.open(table + ".tbl", 'w')) < 0) {
		fprintf(stderr, "Error: table %s is not writable\n", table.c_str());
		if (index) idx.close();
		return rc;
	}
	
	if (readdata.is_open()) {
		while (! readdata.eof() ) {
			getline (readdata,line);
			if ( parseLoadLine (line,key,value) < 0 ) break;
			if ((rc = rf.append(key, value, rid)) < 0) {
				fprintf(stderr, "Error: while writing a tuple to table %s\n", table.c_str());
				readdata.close();
				rf.close();
				if (index) idx.close();
				return rc;
			}
			if (index && (rc = idx.insert(key,rid)) < 0) {
				fprintf(stderr,"Error: while writing a tuple to B+ tree index");
				readdata.close();
				rf.close();
				idx.close();
				}
		}
		rf.close();
		readdata.close();
		if (index) idx.close();
		return 0;
	} else {
		fprintf(stderr, "Error: cannot open loadfile\n");
		rf.close();
		if (index) idx.close();
		return -1;
	}
	
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}

RC SqlEngine::checkTuple(int attr,	RecordFile &rf, RecordId &rid,
	int &count, const vector<SelCond>& cond)
{
	string value;
	int key;
	int diff;
	RC rc;
	// read the tuple
	if ((rc = rf.read(rid, key, value)) < 0) {
		return rc;
	}

	// check the conditions on the tuple
	for (unsigned i = 0; i < cond.size(); i++) {
		// compute the difference between the tuple value and the condition value
		switch (cond[i].attr) {
		case 1:
			diff = key - atoi(cond[i].value);
			break;
		case 2:
			diff = strcmp(value.c_str(), cond[i].value);
			break;
			}

		// skip the tuple if any condition is not met
		switch (cond[i].comp) {
		case SelCond::EQ:
			if (diff != 0)
				return 0;
			break;
		case SelCond::NE:
			if (diff == 0)
				return 0;
			break;
		case SelCond::GT:
			if (diff <= 0)
				return 0;
			break;
		case SelCond::LT:
			if (diff >= 0)
				return 0;
			break;
		case SelCond::GE:
			if (diff < 0)
				return 0;
			break;
		case SelCond::LE:
			if (diff > 0)
				return 0;
			break;
			}
	 }

	 // the condition is met for the tuple. 
	 // increase matching tuple counter
	 count++;

	 // print the tuple 
	 switch (attr) {
	 case 1:  // SELECT key
		fprintf(stdout, "%d\n", key);
		break;
	 case 2:  // SELECT value
		fprintf(stdout, "%s\n", value.c_str());
		break;
	 case 3:  // SELECT *
		fprintf(stdout, "%d '%s'\n", key, value.c_str());
		break;
	 }
	return 0;
}