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

  BTreeIndex bTree;
  IndexCursor cursor;
  int maxCst, minCst;
  bool useIdx = false;
  bool skipTuple = false;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }
  
  maxCst = MAX_INT;
  minCst = MIN_INT;

  for (int j = 0; j < cond.size(); ++j) {

	  // check whether there exists a cond on "key"
	  if (cond[j].attr == 1) {
		  key = atoi(cond[j].value);

		  // useIdx = true, if there exists a non-negation condition
		  switch (cond[j].comp) {
			  case SelCond::EQ:					  
				  maxCst = (key < maxCst) ? key : maxCst;
				  minCst = (key > minCst) ? key : minCst;
				  useIdx = true;
				  break;
			  case SelCond::LT:
				  maxCst = (--key < maxCst) ? key : maxCst;
				  useIdx = true;
				  break;
			  case SelCond::LE:
				  maxCst = (key < maxCst) ? key : maxCst;
				  useIdx = true;
				  break;
			  case SelCond::GT:
				  minCst = (++key > minCst) ? key : minCst;
				  useIdx = true;
				  break;
			  case SelCond::GE:
				  minCst = (key > minCst) ? key : minCst;
				  useIdx = true;
				  break;
		  }
	  }
  }

  if (useIdx) {
	  // check if index exists
	  if (bTree.open(table, 'r'))
		  useIdx = false;
  }

  // if useIdx == true, use index to scan
  if (useIdx) {
  	count = 0;
  	bool valid_tuple;
  	key = minCst;
  	bTree.locate(key, cursor);
  	while (key <= maxCst) {
  		valid_tuple = true;
  		rc = bTree.readForward(cursor, key, rid);  		
  		if ((rc == RC_END_OF_TREE) || (key > maxCst)) break;
  		else if (rc < 0) {
		  fprintf(stderr, "Error: while reading from the index\n");
		  goto exit_select;  			
  		}
  		
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
				if (diff != 0) 
				valid_tuple = false;
				i = cond.size();
				break;
			  case SelCond::NE:
				if (diff == 0) 
				valid_tuple = false;
				i = cond.size();
				break;
			  case SelCond::GT:
				if (diff <= 0) 
				valid_tuple = false;
				i = cond.size();
				break;
			  case SelCond::LT:
				if (diff >= 0) 
				valid_tuple = false;
				i = cond.size();
				break;
			  case SelCond::GE:
				if (diff < 0) 
				valid_tuple = false;
				i = cond.size();
				break;
			  case SelCond::LE:
				if (diff > 0) 
				valid_tuple = false;
				i = cond.size();
				break;
		  }
		} // end of for

		if (valid_tuple) {

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
		}
  	} // end of while loop

	bTree.close();
  }
  else {
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
  }
  
  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table and index files and return
  exit_select:
  rf.close();

  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */

	int			key;
	char		delTuple[128];
	string		value;
	string		tblName = table + ".tbl";
	RecordId	endID;
	BTreeIndex	bTree;

	fstream delFile(loadfile.c_str(), fstream::in);
	RecordFile rf(tblName, 'w');

	if (index) {
		bTree.open(table, 'w');

		while (delFile.getline(delTuple, 128)) {
			parseLoadLine(delTuple, key, value);
			rf.append(key, value, endID);
			bTree.insert(key, endID);
		}

		bTree.close();
	}
	else {
		while (delFile.getline(delTuple, 128)) {
			parseLoadLine(delTuple, key, value);
			rf.append(key, value, endID);
		}
	}

	delFile.close();
	rf.close();

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
