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
	//attr=1 means "key" attribute, 
	//attr=2 means "value" attribute, 
	//attr=3 means "*", and 
	//attr=4 means "COUNT(*)"
	
	/*
	struct SelCond {
		int attr;       // 1 means "key" attribute. 2 means "value" attribute.
		enum Comparator { EQ, NE, LT, GT, LE, GE } comp;
		char* value;    // the value to compare
	};
	*/
	
	RecordFile rf;   // RecordFile containing the table
	BTreeIndex bti;	//BTreeIndex containing the table
	RecordId   rid;  // record cursor for table scanning

	RC     rc;
	int    key;     
	string value;
	int    count;
	int    diff;
	bool   index=false;
	//int i=0;	//position in conditions
	int left = cond.size();

	// open the table file
	if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	}
	if ((rc = bti.open(table + ".idx", 'r')) == 0) index = true;	//there is an index for this table

	//use INDEX if there exists a single equality condition on KEY (but NOT value)
	//goes through the equality conditions
		//it there is a VALUE attr or NE comp then jump to no_index
	if (!index || cond.size() > 1) {
		goto no_index;
	}
	else {
		has_index:
		IndexCursor cursor;
		int idxKey;
		RecordId idxRid;
		int i=0;
		//while (i < cond.size()) {
		
			if (cond[i].attr != 1 || cond[i].comp == SelCond::NE) {
				//is not a key attr, use recordfile
				goto no_index;	//compare string values
				//break;
			}
			else {
				//it is a key attr, so use index
				switch (cond[i].comp) {
					case SelCond::EQ:
						//search for key in bti that is == to value
						if (bti.locate(atoi(cond[i].value), cursor) < 0) {
							//no result
							cout << "locate error" << endl;
							goto exit_select;
						}
						//there is a result, find location in recordfile
						if (bti.readForward(cursor, idxKey, idxRid) < 0) {
							cout << "readForward error" << endl;
							goto exit_select;
						}
						//read data from recordfile at that rid
						if (rf.read(idxRid, key, value) < 0) {
							cout << "recordFile read error" << endl;
							goto exit_select;
						}
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
						//delete current condition from vector
						//cond.erase(i);
						left--;
						break;
					case SelCond::GT:
						//search for key in bti that is > to value
						if (bti.locate(atoi(cond[i].value), cursor) < 0) {
							//no result
							cout << "locate error" << endl;
							goto exit_select;
						}
						//there is a result, find location in recordfile
						
						while (bti.readForward(cursor, idxKey, idxRid) != -1) {
							if (idxKey == atoi(cond[i].value)) {
								goto skip_read;
							}
							if (cursor.pid == -1) {
								break;
							}
							//read data from recordfile at that rid
							//increment eid in idxRid to start it off at value > key
							if (rc = rf.read(idxRid, key, value) < 0) {
								cout << "recordFile read error" << endl;
								goto exit_select;
							}
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
							skip_read:;
						}
						//delete current condition from vector
						left--;
						break;
					case SelCond::LT:
						//search for key in bti that is > to value
						if (bti.locate(atoi(cond[i].value), cursor) < 0) {
							//no result
							cout << "locate error" << endl;
							goto exit_select;
						}
						//there is a result, find location in recordfile
						//start at pid=1 and continue on until it hits the cursor position
						IndexCursor tempCursor;
						tempCursor.pid = 1;
						tempCursor.eid = 0;
						if (tempCursor.pid == cursor.pid && tempCursor.eid == cursor.eid) {
							//no results
							left--;
							break;
						}
						while (bti.readForward(tempCursor, idxKey, idxRid) != -1) {
							//read data from recordfile at that rid
							//increment eid in idxRid to start it off at value > key
							if (rc = rf.read(idxRid, key, value) < 0) {
								cout << "recordFile read error" << endl;
								goto exit_select;
							}
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
							if (tempCursor.pid == cursor.pid && tempCursor.eid == cursor.eid) {
								break;
							}
						}
						left--;
						break;
					case SelCond::GE:
						//search for key in bti that is > to value
						if (bti.locate(atoi(cond[i].value), cursor) < 0) {
							//no result
							cout << "locate error" << endl;
							goto exit_select;
						}
						//there is a result, find location in recordfile
						
						while (bti.readForward(cursor, idxKey, idxRid) != -1) {
							if (cursor.pid == -1) {
								break;
							}
							//read data from recordfile at that rid
							//increment eid in idxRid to start it off at value > key
							if (rc = rf.read(idxRid, key, value) < 0) {
								cout << "recordFile read error" << endl;
								goto exit_select;
							}
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
						}
						//delete current condition from vector
						left--;
						break;
					case SelCond::LE:
						//search for key in bti that is > to value
						if (bti.locate(atoi(cond[i].value), cursor) < 0) {
							//no result
							cout << "locate error" << endl;
							goto exit_select;
						}
						//there is a result, find location in recordfile
						//start at pid=1 and continue on until it hits the cursor position
						tempCursor.pid = 1;
						tempCursor.eid = 0;
						if (tempCursor.pid == cursor.pid && tempCursor.eid == cursor.eid) {
							//no results
							left--;
							break;
						}
						while (bti.readForward(tempCursor, idxKey, idxRid) != -1) {
							//read data from recordfile at that rid
							//increment eid in idxRid to start it off at value > key
							if (rc = rf.read(idxRid, key, value) < 0) {
								cout << "recordFile read error" << endl;
								goto exit_select;
							}
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
							if (tempCursor.pid == cursor.pid && tempCursor.eid == cursor.eid) {
								//print the last one since it is less than or EQUAL to
								bti.readForward(tempCursor, idxKey, idxRid);
								if (rc = rf.read(idxRid, key, value) < 0) {
									cout << "recordFile read error" << endl;
									goto exit_select;
								}
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
								break;
							}
						}
						left--;
						break;
				}
			}
			//i++;
		//}
	}
	
	if (left == 0) goto exit_select;
	
	//either there is no index created or current attr is 2, 3, or 4
	no_index:
	// scan the table file from the beginning
	rid.pid = rid.sid = 0;
	count = 0;
	while (rid < rf.endRid()) {
		// read the tuple
		if ((rc = rf.read(rid, key, value)) < 0) {
		  fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
		  goto exit_select;
		}
		
		//attr=1 means "key" attribute, 
		//attr=2 means "value" attribute, 
		//attr=3 means "*", and 
		//attr=4 means "COUNT(*)"
		
		/*
		struct SelCond {
			int attr;       // 1 means "key" attribute. 2 means "value" attribute.
			enum Comparator { EQ, NE, LT, GT, LE, GE } comp;
			char* value;    // the value to compare
		};
		*/
		
		// check the conditions on the tuple
		//while (i < cond.size()) {
		for (int i=0; i<cond.size(); i++) {
			// compute the difference between the tuple value and the condition value
			switch (cond[i].attr) {
				case 1:
					diff = key - atoi(cond[i].value);	//if diff = 0 keys are equivalent
					break;
				case 2:
					diff = strcmp(value.c_str(), cond[i].value);	//compare string values
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
			
			//i++;
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

	// close the table file and return
	exit_select:
	rf.close();
	if (index) bti.close();	//close index file if there was one
	return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	RC rc;
	RecordId rid;
	int key;
	string line, value;
	
	//BEGIN RECORDFILE/BTREEINDEX CREATION
	RecordFile rf;
	BTreeIndex bti;	//only create index if index is true
	
	ifstream file(loadfile.c_str());	//open file
	
	//if RecordFile table.tbl does not exist yet, create RecordFile table.tbl, open it, and load tuples line by line into the table
	if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
		rf.open(table + ".tbl", 'w'); 		//table does not yet exist, so create it
		if (index) bti.open(table + ".idx", 'w');	//if RC doesn't exist, neither does index
		while (getline(file, line)) {
			parseLoadLine(line, key, value); //parse each line of input file and get key and value
			rf.append(key, value, rid);	//append key and value to table
			if (index) bti.insert(key, rid);
			
		}
	}
	
	//if table already exists, open RecordFile table.tbl and append tuples line by line  onto table.tbl
	else {
		//table already exists, so append instead of write over
		if (index) bti.open(table + ".idx", 'w');	//if RC does exist, index MIGHT exist
		while (getline(file, line)) {
			parseLoadLine(line, key, value); //parse each line of input file and get key and value
			rf.append(key, value, rid);	//append key and value to table
			if (index) bti.insert(key, rid);
		}
	}
	rf.close();
	if (index) bti.close();
	//END RECORDFILE/BTREEINDEX CREATION
	
	return rc;
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
