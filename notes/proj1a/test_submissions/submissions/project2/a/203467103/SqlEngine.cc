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
	RecordFile rf;   // RecordFile containing the table
	RecordId   rid;  // record cursor for table scanning

	RC     rc;
	int    key;     
	string value;
	int    count;
	int    diff;

	// open the table file
	if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	}

	// scan the table file from the beginning
	rid.pid = rid.sid = 0;
	count = 0;
	while (rid < rf.endRid()) {
		// read the tuple
		if ((rc = rf.read(rid, key, value)) < 0) {
			fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
			goto exit_select;
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

	// close the table file and return
exit_select:
	rf.close();
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
		// WITH INDEX does something
		if (index) {
			; // do something later
		}

		// Append the record to the record, retrieving the rid
		rc = recordFile.append(key, value, rid);
		if (rc) {
			fprintf(stderr, "Error: append error \n", table.c_str());
			return rc;
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
