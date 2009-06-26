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

//#define DEBUG

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

	RC     rc, err;
	int    key, key2, count = 0, diff, pid, eid;     
	string value;
	bool keyCond = false;
	
	// check if there is at least one condition on the key
	for (int i = 0; i < cond.size(); i++)
	{
		// if there is a condition on the key
		if (cond[i].attr == 1)
			// set keyCond to true
			keyCond = true;
	}
	// if the query is "select COUNT(*)" and there is a where clause
	if ((attr == 4) && (cond.size() > 0))
		// set keyCond true
		keyCond = true;
	
	// if there is an index on the table
	string idx = table + ".idx";
	ifstream fin(idx.c_str());
	// and if there is a condition on a key attribute
	if (fin && keyCond)
	{
		#ifdef DEBUG
		cout << "The file exists, and is open for input" << endl;
		#endif
		// close the file stream
		fin.close();
		// variables
		BTreeIndex bti;				// B+ index
		RecordFile rf;				// Record File
		
		// open the file as a B+ Tree Index
		err = bti.open(table, 'r');
		if (err != 0) return err;
		
		// open the table file
		if ((rc = rf.open(table + ".tbl", 'r')) < 0)
		{
			// if it doesn't exist, report an error and return
			fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
			return rc;
		}
		
		// IDEA: get it to work with any one condition, then add support
		// for multiple conditions
		
		// check for = in the first selcond
		if (cond[0].comp == SelCond::EQ)
		{
			// get the key
			key = atoi(cond[0].value);
			// find the entry with the value >= the key
			err = bti.locate(key, pid, eid);
			if (err != 0) return err;
			
			// get the entry
			err = bti.readForward(pid, eid, key2, rid);
			if (err != 0) return err;
			
			// check if the key is equal
			if (key == key2)
			{
				// get the record
				err = rf.read(rid, key2, value);
				if (err != 0) return err;
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
			}
			// print matching tuple count if "select count(*)"
			if (attr == 4)
			{
				fprintf(stdout, "%d\n", count);
			}
		}
		
		// check for > or >=
		if ((cond[0].comp == SelCond::GT) || (cond[0].comp == SelCond::GE))
		{
			// bool to know if we should skip the first key if it's equal
			// to the search key
			bool skipFirst = false;
			if (cond[0].comp == SelCond::GT) skipFirst = true;
				
			// get the key
			key = atoi(cond[0].value);
			// find the entry with the value >= the key
			err = bti.locate(key, pid, eid);
			if (err != 0) return err;
			
			// get the entry
			err = bti.readForward(pid, eid, key2, rid);
			if (err != 0) return err;
			
			// if the key found is = to search key and we should skip
			if ((key2 == key) && skipFirst) 
			{
				// skip the first entry
				err = bti.readForward(pid, eid, key2, rid);
				if (err != 0) return err;
			}
			
			// get the record
			err = rf.read(rid, key2, value);
			if (err != 0) return err;
			
			// print the tuples
			while (err == 0)
			{
				// print the tuple 
				switch (attr)
				{
				case 1:  // SELECT key
					fprintf(stdout, "%d\n", key2);
					break;
				case 2:  // SELECT value
					fprintf(stdout, "%s\n", value.c_str());
					break;
				case 3:  // SELECT *
					fprintf(stdout, "%d '%s'\n", key2, value.c_str());
					break;
				}
				// get the next tuple
				err = bti.readForward(pid, eid, key2, rid);
				rf.read(rid, key2, value);
				// increase matching tuple counter
				count++;
			}
			// print matching tuple count if "select count(*)"
			if (attr == 4)
			{
				fprintf(stdout, "%d\n", count);
			}
		}
		
		// check for < or <=
		if ((cond[0].comp == SelCond::LT) || (cond[0].comp == SelCond::LE))
		{
			// bool to know if we should skip the last key if it's equal
			// to the search key
			bool skipLast = false;
			if (cond[0].comp == SelCond::LT) skipLast = true;
				
			// get the key
			key = atoi(cond[0].value);
			// find the entry with the smallest value
			err = bti.locate(-2147483648, pid, eid);
			if (err != 0) return err;
			
			// get the entry
			err = bti.readForward(pid, eid, key2, rid);
			if (err != 0) return err;
			
			// get the record
			err = rf.read(rid, key2, value);
			if (err != 0) return err;
			
			// print the tuples
			while (key2 < key)
			{
				// jump out of the loop if we're found the last entry
				//if (key == key2) break;
				// print the tuple 
				switch (attr)
				{
				case 1:  // SELECT key
					fprintf(stdout, "%d\n", key2);
					break;
				case 2:  // SELECT value
					fprintf(stdout, "%s\n", value.c_str());
					break;
				case 3:  // SELECT *
					fprintf(stdout, "%d '%s'\n", key2, value.c_str());
					break;
				}
				// get the next tuple
				err = bti.readForward(pid, eid, key2, rid);
				if (err != 0) break;
				rf.read(rid, key2, value);
				// increase matching tuple counter
				count++;
			}
			// see if we should print the last value
			// if the key found is = to search key and we shouldn't skip
			if ((key == key2) && !skipLast)
			{
				// print the last entry
				switch (attr)
				{
				case 1:  // SELECT key
					fprintf(stdout, "%d\n", key2);
					break;
				case 2:  // SELECT value
					fprintf(stdout, "%s\n", value.c_str());
					break;
				case 3:  // SELECT *
					fprintf(stdout, "%d '%s'\n", key2, value.c_str());
					break;
				}
				// get the next tuple
				err = bti.readForward(pid, eid, key2, rid);
				rf.read(rid, key2, value);
				
				// increment the count
				count++;
			}
			// print matching tuple count if "select count(*)"
			if (attr == 4)
			{
				fprintf(stdout, "%d\n", count);
			}
		}
		
		// close the B+ Tree Index and Record File
		bti.close();
		rf.close();
		// skip the normal select procedure
		goto exit_select;
	}
	// if there is no index on the table
	else
	{
		#ifdef DEBUG
		cout << "EPIC FAIL!" << endl; 
		#endif
	}
	
	
	
  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0)
  {
	  fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
	  return rc;
  }

  // scan the table file from the beginning
  rid.pid = rid.sid = 0;
  count = 0;
  while (rid < rf.endRid())
  {
    // read the tuple
    if ((rc = rf.read(rid, key, value)) < 0)
	{
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
		switch (cond[i].comp) 
		{
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

  // print matching tuple count if "select count(*)"
  if (attr == 4)
  {
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
	#ifdef DEBUG
	cout << "SqlEngine::load" << endl;
	#endif
	RC err;
	// dummy recordfile object to open table
	RecordFile rf;
	RecordId rid;
	// string input from file and value return from parse
	string s, v;
	// key return from parse and insert variables
	int k, pid, eid;
	// file stream
	ifstream fin;
	// index opened flag
	bool btiOpened = false;
	// instanciate a btreeindex object
	BTreeIndex bti;
	
	// try to open the file
	fin.open(loadfile.c_str(), ios::in);
	if (!fin) return -1001;
	
	// try to open the table
	err = rf.open(table + ".tbl", 'w');
	// check for error
	if (err != 0) return err;

	// while there is still input from the file
	while (fin)
	{
		// get a line from the file
		getline(fin, s);
		// parse the record data from the line
		err = parseLoadLine(s, k, v);
		// if that fails, most likely we're at eof
		if (err != 0) continue;
		
		// get the last rid
		rid = rf.endRid();
		// add the record to the record file
		err = rf.append(k,v,rid);
		// check for error
		if (err != 0) return err;
		
		// if there should be an index
		if (index)
		{
			// check if it's open
			if (!btiOpened)
			{
				// set the opened flag true
				btiOpened = true;
				// open it
				err = bti.open(table, 'w');
				// check for error
				if (err != 0) return err;
			}
			// debugging
			#ifdef DEBUG
			cout << "SqlEngine::load, line 183" << endl;
			#endif
			// insert the key and record into the index
			err = bti.insert(k, rid);
			// check for error
			if (err != 0)
			{
				#ifdef DEBUG
				cout << k << " "
					 << rid.pid << " "
					 << rid.sid  << "!!!!!!!!!!!!!!!!!!!!!" << endl;
				#endif
				goto insert_error;
			}
		}
	}
insert_error:
	// close the input file
	fin.close();
	// close the record file
	err = rf.close();
	// close the index if it exists
	if (index) err = bti.close();
	return err;
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
