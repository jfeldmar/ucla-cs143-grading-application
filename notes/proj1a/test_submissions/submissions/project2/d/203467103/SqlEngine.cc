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
#include "SqlEngine.h"
#include "BTreeIndex.h"

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

	RC     rc;
	int    key;     
	string value;
	int    count;
	int    diff;
	BTreeIndex indexFile;
	bool index = false;

	// open the table file
	if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	}
	
	// check if an index exists
	rc = indexFile.open(table +".idx", 'r');
	if (!rc) // an index file exists
		index = true; // let the rest of the function know

	if (index) { // if there is an index, we gotta be selective
		// see if there's a not-equals or equals clause
		
		IndexCursor cursor;
		// get the low and high bounds for scanRange
		RecordId probe;
		IndexCursor low;
		low.pid = -1;
		low.eid = -1;
		IndexCursor high;
		high.pid = -1;
		high.eid = -1;
		bool lowFound = false;
		bool highFound = false;
		bool rangeScan = false;
		for (unsigned i = 0; i < cond.size(); i++) { // for each condition
			if (cond[i].attr == 1) { // check only keys at the moment
				// where are values checked? in the scan functions
				switch (cond[i].comp) {
					case SelCond::EQ: // locate single
						indexFile.locate(atoi(cond[i].value), cursor);
						// locateSingle
						indexFile.readForward(cursor, key, probe);
						locateSingle(rf, attr, table, cond, probe);
						// teleport
						goto exit_select;
						break;
					case SelCond::NE:
						// if there is only not equals OR there are no conditions, scan the whole table
						break;
					case SelCond::GT:
						indexFile.locate(atoi(cond[i].value)+1, cursor);
						// take the max of the comparisons
						if (lowFound) {
							if (low < cursor)
								low = cursor;
						} else {
							low = cursor;
							lowFound = true;
						}
						rangeScan = true;
						break;
					case SelCond::GE:
						indexFile.locate(atoi(cond[i].value), cursor);
						// take the max
						if (lowFound) {
							if (low < cursor)
								low = cursor;
						} else {
							low = cursor;
							lowFound = true;
						}
						rangeScan = true;
						break;
					case SelCond::LT:
						rc = indexFile.locate(atoi(cond[i].value), cursor);
						if (rc == RC_NO_SUCH_RECORD) {
							cursor.pid = INT_MAX;
							cursor.eid = INT_MAX;
						}
						
						// take the min
						if (highFound) {
							if (high > cursor)
								high = cursor;
						} else {
							high = cursor;
							highFound = true;
						}
						rangeScan = true;
						break;
					case SelCond::LE:
						rc = indexFile.locate(atoi(cond[i].value), cursor);
						// +1 taken care of in scanRange
						if (rc == RC_NO_SUCH_RECORD) {
							cursor.pid = INT_MAX;
							cursor.eid = INT_MAX;
						}
						// take the min
						if (highFound) {
							if (high > cursor)
								high = cursor;
						} else {
							high = cursor;
							highFound = true;
						}
						rangeScan = true;
						break;
				} // done with switch
			} // done with if (attr == key)
		} // done going through comparisons
		
		if (rangeScan) { // equality was taken care of awhile back
			// if high <= low, die
			if (low.pid < 1 && high.pid == INT_MAX)
				goto table_scan;
			low.pid = low.pid >= 1 ? low.pid : 1;
			low.eid = low.eid >= 0 ? low.eid : 0;
			high.pid = high.pid >= 0 ? high.pid : INT_MAX;
			high.eid = high.eid >= 0 ? high.eid : INT_MAX;
			
			//printf("Range scan [%d:%d,%d:%d)\n", low.pid, low.eid, high.pid, high.eid);
			rc = scanRange(indexFile, rf, attr, table, cond, low, high);
			// now exit!
			goto exit_select;
			
		} else {
table_scan:
			//printf("Table scan\n");
			rc = scanTable(rf, attr, table, cond);
			goto exit_select;
		}
		
	} else { // if there's no index, just scan the whole thing
		rc = scanTable(rf, attr, table, cond);
		if (rc) {
			goto exit_select;
		}
	}
	
	
	// close the table file and return
exit_select:
	rf.close();
	indexFile.close();
	return rc;
}

// function scanTable
// Scans the entire table. Does not use index.
// Prints out matching tuples.
RC SqlEngine::scanTable(RecordFile &rf, int attr, const string& table, const vector<SelCond>& cond)
{
	
	RecordId   rid;  // record cursor for table scanning

	RC     rc;
	int    key;
	string value;
	int    count;
	int    diff;
	BTreeIndex indexFile;

	// scan the table file from the beginning
	rid.pid = rid.sid = 0;
	count = 0;
	while (rid < rf.endRid()) {
		// read the tuple
		if ((rc = rf.read(rid, key, value)) < 0) {
			fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
			return -1; // -1 for exit-select
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
					if (diff != 0) goto next_tuple;
					break;
				case SelCond::NE:
					if (diff == 0) goto next_tuple;
					break;
				case SelCond::GT:
					if (diff <= 0) goto next_tuple;
					break;
				case SelCond::LT:
					if (diff >= 0) goto next_tuple;
					break;
				case SelCond::GE:
					if (diff < 0) goto next_tuple;
					break;
				case SelCond::LE:
					if (diff > 0) goto next_tuple;
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

		// move to the next tuple
next_tuple:
		++rid;
	}

	// print matching tuple count if "select count(*)"
	if (attr == 4) {
		fprintf(stdout, "%d\n", count);
	}
	rc = 0;
	
	return rc;
}

// scan a range of tuples from [low to high)
// NOTE: tuples may not be stored in key order!!! dammit!
RC SqlEngine::scanRange(BTreeIndex &indexFile, RecordFile &rf, int attr, const std::string& table, const std::vector<SelCond>& cond, IndexCursor& low, IndexCursor& high)
{
	RecordId rid;  // record cursor for table scanning

	RC     rc;
	int    key;
	string value;
	int    count;
	int    diff;

	bool endReached = false;
	
	// transfer indexCursor low values to rid values
	IndexCursor cursor;
	cursor.pid = low.pid;
	cursor.eid = low.eid;
	
	// rid will be filled in on the readForward
	count = 0;
	while (!endReached && rid < rf.endRid()) { // keep going until high or the end is reached
		// get the rid
		if ((rc = indexFile.readForward(cursor, key, rid)) < 0) {
			if (rc == RC_END_OF_TREE)
				return -1;
			fprintf(stderr, "Error: while reading an rid from table %s\n", table.c_str());
			return -1; // -1 for exit-select
		}
		
		// read the tuple
		if ((rc = rf.read(rid, key, value)) < 0) {
			fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
			return -1; // -1 for exit-select
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
					if (diff != 0) goto next_tuple;
					break;
				case SelCond::NE:
					if (diff == 0) goto next_tuple;
					break;
				case SelCond::GT:
					if (diff <= 0) goto next_tuple;
					break;
				case SelCond::LT:
					if (diff >= 0) {
						if (cond[i].attr == 2) {
							goto next_tuple;
						} else {
							endReached = true;
							goto next_tuple;
						}
					}
					break;
				case SelCond::GE:
					if (diff < 0) goto next_tuple;
					break;
				case SelCond::LE:
					if (diff > 0) {
						if (cond[i].attr == 2) {
							goto next_tuple;
						} else {
							endReached = true;
							goto next_tuple;
						}
					}
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

		// move to the next tuple
next_tuple:
		; // do nothing!
	}

	// print matching tuple count if "select count(*)"
	if (attr == 4) {
		fprintf(stdout, "%d\n", count);
	}
	rc = 0;
	
	return rc;
}

RC SqlEngine::locateSingle(RecordFile &rf, int attr, const std::string& table, const std::vector<SelCond>& cond, RecordId& rid)
{

	RC     rc;
	int    key;
	string value;
	int    count;
	int    diff;
	BTreeIndex indexFile;

	count = 0;
	if (rid < rf.endRid()) {
		// read the tuple
		if ((rc = rf.read(rid, key, value)) < 0) {
			fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
			return -1; // -1 for exit-select
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
					if (diff != 0) goto next_tuple;
					break;
				case SelCond::NE:
					if (diff == 0) goto next_tuple;
					break;
				case SelCond::GT:
					if (diff <= 0) goto next_tuple;
					break;
				case SelCond::LT:
					if (diff >= 0) goto next_tuple;
					break;
				case SelCond::GE:
					if (diff < 0) goto next_tuple;
					break;
				case SelCond::LE:
					if (diff > 0) goto next_tuple;
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

		// move to the next tuple
next_tuple:
		;
	}

	// print matching tuple count if "select count(*)"
	if (attr == 4) {
		fprintf(stdout, "%d\n", count);
	}
	rc = 0;
	
	return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	// LOAD:
	// inputs: 
	// table: the table to store data in
	// loadfile: the file to load values from
	// index: if true, Bruinbase creates the index on the
	//   key column of the table
	// outputs:
	// RC error code
	RC rc;

	fstream readfile (loadfile.c_str(), fstream::in);
	if (readfile.fail()) {
		fprintf(stderr, "Error: file %d does not exist\n", loadfile.c_str());
		rc = RC_FILE_OPEN_FAILED;
		return rc;
	}

	RecordFile recordFile;
	rc = recordFile.open(table+".tbl", 'w');
	if (rc) {
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	}

	BTreeIndex indexFile;
	//indexFile.setDebug(1);
	if (index) { // open the index file
		rc = indexFile.open(table+".idx", 'w');
		
		//if (rc) {
		//	fprintf(stderr, "Error, indexFile %s cannot be opened, rc %s\n", table.c_str(), rc);
		//}
	}
	string line;
	int key;
	string value;
	RecordId rid;
	while(getline(readfile, line)) {
		rc = parseLoadLine(line, key, value);
		if (rc) {
			fprintf(stderr, "Error: %s has invalid file format\n", loadfile.c_str());
			return rc;
		}

		// Append the record to the record, retrieving the rid
		rc = recordFile.append(key, value, rid);
		if (rc) {
			fprintf(stderr, "Error: append error \n", table.c_str());
			return rc;
		}
		// if it is WITH INDEX, then insert the tuple into the index
		if (index) { // insert the rid into the BTIndex
			rc = indexFile.insert(key, rid);
			if (rc) {
				fprintf(stderr, "Error: insert error %d \n", rc);
			}
		}
	} // Finished reading the provided file

	rc = recordFile.close();
	if (rc) {
		fprintf(stderr, "Error: Cannot close file %d!\n", (table+".tbl").c_str());
		return rc;
	}

	readfile.close();
	if (readfile) { 
		fprintf(stderr, "Error: Cannot close file %d\n", loadfile.c_str());
		rc = RC_FILE_CLOSE_FAILED;
		return rc;
	}
	
	if (index) {
		rc = indexFile.close();
		if (rc) {
			fprintf(stderr, "Error: Cannot close file %d\n", loadfile.c_str());
			rc = RC_FILE_CLOSE_FAILED;
			return rc;
		}
	}

	return 0;
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
