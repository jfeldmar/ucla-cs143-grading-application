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

	RecordId   sRid; // the starting rid to scan the table
	RecordId   eRid; // the ending rid to scan the table
	
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

	// Open the Index File if Available
	bool index = false;
	bool useIndex = false;
	BTreeIndex treeIndex;
	if ( treeIndex.open(table + ".idx", 'r') == 0 )
		index = true;

	// Create a Cursor
	IndexCursor cursor;
	treeIndex.getInitialCursor(cursor);

	// initialize the record id's
	rid.pid = sRid.pid = cursor.pid;
	rid.sid = sRid.sid = cursor.eid;
	eRid.pid = rf.endRid().pid;
	eRid.sid = rf.endRid().sid;

	// Go through each condition and set up the RANGE for the following while loop
	for (unsigned i = 0; index && i < cond.size(); i++)
	{
		// Check if we can use an index
		if ( cond[i].attr == 1 && cond[i].comp != SelCond::NE )
		{
			// Allow the use of the index
			useIndex = true;

			// Locate the value
			treeIndex.locate(atoi(cond[i].value), cursor);

			// Set the appropriate boundaries
			switch (cond[i].comp)
			{
				case SelCond::GT:
				case SelCond::GE:
					//cout << "GREATER THAN OR GREATER THAN OR EQUAL TO" << endl;
					if ( cursor.pid > sRid.pid || ( cursor.pid == sRid.pid && cursor.eid > sRid.sid ) )
					{
						sRid.pid = cursor.pid;
						sRid.sid = cursor.eid;
					}
					break;
				case SelCond::EQ:
					//cout << "EQUAL TO" << endl;
					sRid.pid = cursor.pid;
					sRid.sid = cursor.eid;
					eRid.pid = cursor.pid ;
					eRid.sid = cursor.eid;
					break;
				case SelCond::LT:
				case SelCond::LE:
					//cout << "LESS THAN OR LESS THAN OR EQUAL TO" << endl;
					if ( cursor.pid < sRid.pid || ( cursor.pid == sRid.pid && cursor.eid < sRid.sid ) )
					{
						eRid.pid = cursor.pid;
						eRid.sid = cursor.eid;
					}
					break;
			}

			// Set EQUAL TO to the highest precedence
			if ( cond[i].comp == SelCond::EQ )
				break;
		}
	}

	// Set up the cursor
	cursor.pid = sRid.pid;
	cursor.eid = sRid.sid;


	// If we're using the B+ Tree
	if ( useIndex )
	{
		// Now loop through the provided tuple set
		count = 0;
		while ( treeIndex.readForward(cursor, key, rid) == 0 )
		{
			// read the tuple
			if ((rc = rf.read(rid, key, value)) < 0) {
				fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
				goto exit_select;
			}
	
			// check the conditions on the tuple
			for (unsigned i = 0; i < cond.size(); i++) {
				// compute the difference between the tuple value and the condition value
				switch (cond[i].attr)
				{
					case 1:
						diff = key - atoi(cond[i].value);
						break;
					case 2:
						diff = strcmp(value.c_str(), cond[i].value);
						break;
				}
	
				// skip the tuple if any condition is not met
				switch (cond[i].comp)
				{
					case SelCond::EQ:
						if (diff != 0) goto next_tree_tuple;
						break;
					case SelCond::NE:
						if (diff == 0) goto next_tree_tuple;
						break;
					case SelCond::GT:
						if (diff <= 0) goto next_tree_tuple;
						break;
					case SelCond::LT:
						if (diff >= 0) goto next_tree_tuple;
						break;
					case SelCond::GE:
						if (diff < 0) goto next_tree_tuple;
						break;
					case SelCond::LE:
						if (diff > 0) goto next_tree_tuple;
						break;
				}
			}
	
			// the condition is met for the tuple. 
			// increase matching tuple counter
			count++;
	
			// print the tuple 
			switch (attr)
			{
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
			next_tree_tuple:
				continue;
		}
	}
	else // Scan it as usual
	{
		// scan the table file from the beginning
		rid.pid = rid.sid = 0;
		count = 0;
		while ( rid < rf.endRid() )
		{
			// read the tuple
			if ((rc = rf.read(rid, key, value)) < 0) {
				fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
				goto exit_select;
			}

			// check the conditions on the tuple
			for (unsigned i = 0; i < cond.size(); i++)
			{
				// compute the difference between the tuple value and the condition value
				switch (cond[i].attr)
				{
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
			switch (attr)
			{
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
	}

	// print matching tuple count if "select count(*)"
	if (attr == 4) {
		fprintf(stdout, "%d\n", count);
	}
	rc = 0;

	// close the table file and return
	exit_select:
		if ( index )
			treeIndex.close();

		rf.close();
		return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	// Create an index variable
	BTreeIndex treeIndex;

	// Table variables
	RecordFile tb_file;
	RecordId rid;

	// Input Stream
	ifstream ld_file( loadfile.c_str() );

	// Temporary variables
	string line;
	string value;
	int key;

	// Open the load file: read only
	if ( !ld_file.is_open() )
	{
		return RC_FILE_OPEN_FAILED;
	}

	// Open table file: append only (create it if not exist)
	if ( tb_file.open(table + ".tbl", 'w') != 0 )
	{
		return RC_FILE_OPEN_FAILED;
	}

	// Create the index file if necessary
	if ( index )
	{
		if ( treeIndex.open(table + ".idx", 'w') != 0 )
		{
			return RC_FILE_OPEN_FAILED;
		}
	}

	// Read from loadfile and write to table
	while ( ld_file.good() )
	{
		getline( ld_file, line);
		// don't need to check parseLoadLine since 
		// it's guarenteed to be well formed
		parseLoadLine( line, key, value);
		tb_file.append( key, value, rid);

		// Insert into the index, if necessary
		if ( index )
			treeIndex.insert(key, rid);
	}

	// Close both files
	tb_file.close();
	ld_file.close();

	// Close the index file, if necessary
	if ( index )
		treeIndex.close();

	// Return success
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