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
#include <stdio.h>
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
  IndexCursor icursor; //index cursor for index scanning
  BTreeIndex btindex; //the BTreeIndex node

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;
  bool   isIndexed; //this is a boolean parameter that checks to see whether or not there is an index
  int    newIntMin; //seeing as how INT_MIN is a constant, need this to check bounds
  int    newIntMax; //seeing as how INT_MAX is a constant, need this to check bounds
  
  newIntMin = INT_MIN;
  newIntMax = INT_MAX;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  // open the INDEX file
  if ((rc = btindex.open(table + ".idx", 'r')) < 0) {
    //if we cannot open the index file, this must mean there is no index for this table, and we continue accordingly
    isIndexed == false;
  } else {
    isIndexed == true;
  }

  // scan the table file from the beginning
  rid.pid = rid.sid = 0;
  // now we need to scan the table for the correct index to be used
  if(isIndexed){
    icursor.eid = icursor.pid = -1;

    // find the index on the tuple
    for (unsigned i = 0; i < cond.size(); i++) {
      if (cond[i].attr == 1){
     int intValue = atoi(cond[i].value);
	switch (cond[i].comp) {
	case SelCond::EQ:
	  btindex.locate(intValue,icursor);
	  newIntMin = intValue;
	  newIntMax = intValue;
	  break;
	case SelCond::NE:
	  //here, we don't want to use the index, as it is slower than not using it
	  break;
	case SelCond::GT:
	  if (icursor.eid == -1){
	    btindex.locate(intValue,icursor);
	  }
	  //checking to see if our value is greater than the integer minimum or not, and then updating the bound
	  if(intValue > newIntMin){
	    newIntMin = intValue;
btindex.locate(intValue,icursor);
}
	  break;
	case SelCond::LT:
	  if (icursor.eid == -1)
	    btindex.locate(INT_MIN, icursor);
	  //checking to see if our value is greater than the integer minimum or not, and then updating the bound
	  if(intValue < newIntMax){
	    newIntMax = intValue;
	btindex.locate(newIntMin, icursor);
}
	  break;
	case SelCond::GE:
	  if (icursor.eid == -1)
	    btindex.locate(intValue,icursor);
	  //checking to see if our value is greater than the integer minimum or not, and then updating the bound
	  if(intValue > newIntMin){
	    newIntMin = intValue;
 btindex.locate(intValue,icursor);
}
	  break;
	case SelCond::LE:
	  if (icursor.eid == -1)
	    btindex.locate(INT_MIN, icursor);
	  //checking to see if our value is greater than the integer minimum or not, and then updating the bound
	  if(intValue < newIntMax){
	    newIntMax = intValue;
btindex.locate(newIntMin, icursor);
}
	  break;
	}
      }
    }
  }

  if(icursor.pid == -1)
    isIndexed = false;

	//fprintf(stderr,"Bounds %d,%d\n", newIntMin,newIntMax);
  count = 0;
  int intValue = 0;
  while (rid < rf.endRid()) {
    if(isIndexed){
      btindex.readForward(icursor,intValue,rid);
      //Now we can check to see whether or not the key is actually within the right bounds
      if(intValue > newIntMax || intValue < newIntMin)
	break;
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
  btindex.close();
  rf.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	/* your code here */
	
	RecordFile rf;	// RecordFile containing the table
	RecordId  rid;	// record cursor for table scanning
	
	RC     rc;
	int    key;     
	string value;
	const int MAX_LINE_LENGTH = 500;
	char   line [MAX_LINE_LENGTH];
	int    count;
	FILE*  infile;
	BTreeIndex btindex;
	
	// open the table file
	if ((rc = rf.open(table + ".tbl", 'w')) < 0) {
		fprintf(stderr, "Error: table %s does not exist and failed to be created\n", table.c_str());
		return rc;
	}
	
	// start loading at the next available record
	count = 0;
	
	infile = fopen(loadfile.c_str(), "r");
	if (infile == NULL) {
		fprintf(stderr, "Error: failed to open file %s\n", loadfile.c_str());
		rf.close();
		return -1;
	}
	
	// open the index
	if ((rc = btindex.open(table + ".idx", 'w')) < 0) {
		fprintf(stderr, "Error: table %s does not exist and failed to create an index\n", table.c_str());
		return rc;
	}

	while (!ferror(infile) && !feof(infile) && fgets(line, MAX_LINE_LENGTH, infile) != NULL) {
		// parse the line to be loaded
		if((rc = parseLoadLine(line, key, value)) == RC_INVALID_FILE_FORMAT) {
			fprintf(stderr, "Error: failed to parse line from file %s\n", loadfile.c_str());
			goto exit_select;
		}
		
		// write the tuple to a record
		if ((rc = rf.append(key, value, rid)) < 0) {
			fprintf(stderr, "Error: failed to write a tuple to table %s\n", table.c_str());
			goto exit_select;
		}

		// write the tuple to the index
		if ((rc = btindex.insert(key, rid)) < 0) {
			fprintf(stderr, "Error: failed to write a tuple to table %s index\n", table.c_str());
			goto exit_select;
		}
		
		// increase successful loads counter
		count++;
	}
	
	// check for read errors
	if (ferror(infile)) {
		fprintf(stderr, "Error: failed to read from file %s\n", loadfile.c_str());
		rc = -1;
		goto exit_select;
	}
	
	//fprintf(stdout, "Successfully Loaded %d Records\n", count);
	rc = 0;

	// close the file and table file; return
	exit_select:
	fclose(infile);
	rf.close();
	btindex.close();
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
