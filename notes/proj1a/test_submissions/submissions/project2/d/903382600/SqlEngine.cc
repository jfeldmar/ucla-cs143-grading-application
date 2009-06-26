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
#include "BTreeNode.h"

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
 BTreeIndex index; // BTree Index

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;
  IndexCursor cursor;
  bool indexexists = false;
  bool eq = false;
  bool gt = false;
  bool ge = false;
  bool lt = false;
  bool le = false;
  int max = 999999999;
  int min = -999999999;
  int equal;
  int counter;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  // Attempt to open index
  if (index.open(table + ".idx", 'r') == 0) {
	  indexexists = true;
  }
  
  //See if there are any equality checks
  if (indexexists == true) {
	  for (counter = 0; counter < cond.size(); counter++) { //Go through each comparator
		  if (cond[counter].attr == 1) { //If it operates on Key
		      switch (cond[counter].comp) {
		      case SelCond::EQ:
				eq = true;
				equal = atoi(cond[counter].value);
				break;
		      case SelCond::GT:
				//Take the greatest lower bound
				if(atoi(cond[counter].value) >= min) {
					min = atoi(cond[counter].value);
					gt = true;
					ge = false;
				}
				break;
		      case SelCond::LT:
		        //take the smallest upper bound
				if (atoi(cond[counter].value) <= max) {
					max = atoi(cond[counter].value);
					lt = true;
					le = false;
				}
				break;
		      case SelCond::GE:
				if (atoi(cond[counter].value) > min) {
					min = atoi(cond[counter].value);
					gt = false;
					ge = true;
				}
				break;
		      case SelCond::LE:
				if (atoi(cond[counter].value) < max) {
					max = atoi(cond[counter].value);
					lt = false;
					le = true;
				}
				break;
			  default:
			    break;
		      }
	      }
      }
  }
      
  //Start doing equality checks in the following priority: ==, and then (>, <, <=, >=).
  if (eq == true) {
	  index.locate(equal, cursor);
	  index.readForward(cursor, key, rid);
	  if (key == equal) {
		  rf.read(rid, key, value);
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
	if (diff != 0) goto exit_eq;
	break;
      case SelCond::NE:
	if (diff == 0) goto exit_eq;
	break;
      case SelCond::GT:
	if (diff <= 0) goto exit_eq;
	break;
      case SelCond::LT:
	if (diff >= 0) goto exit_eq;
	break;
      case SelCond::GE:
	if (diff < 0) goto exit_eq;
	break;
      case SelCond::LE:
	if (diff > 0) goto exit_eq;
	break;
      }
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
    case 4: // SELECT COUNT(*)
      cout << 1 << endl;
      break;
    }   
    
    exit_eq:
    index.close();
    rf.close();
    return rc;
	}
	else { //No match
	index.close();
	rf.close();
	return rc;
	}
}

//Begin doing range scans
if(ge == true || gt == true || le == true || lt == true) {
	count = 0;
	index.locate(min, cursor);
	
	read_forward:
	rc = index.readForward(cursor, key, rid);
	if (rc < 0)
		goto exit_range;
	if (gt == true && key == min) //If the key == to min when "> min" is active, skip it
		index.readForward(cursor, key, rid);
	if (lt == true && key >= max) //If the key >= max when "< max" is active, exit
		goto exit_range;
	if (le == true && key > max) //if key > max when "<= max" is active, exit
		goto exit_range;
	rf.read(rid, key, value);
	
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
	if (diff != 0) goto read_forward;
	break;
      case SelCond::NE:
	if (diff == 0) goto read_forward;
	break;
      case SelCond::GT:
	if (diff <= 0) goto read_forward;
	break;
      case SelCond::LT:
	if (diff >= 0) goto read_forward;
	break;
      case SelCond::GE:
	if (diff < 0) goto read_forward;
	break;
      case SelCond::LE:
	if (diff > 0) goto read_forward;
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
	
  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
    
  	goto read_forward;	
	
	exit_range:
	index.close();
	rf.close();
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
  /* DEBUG CODE FOR 2B */
  if (table == "debug") {
}
  else{
  /* your code here */
 
  ifstream inStream;
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning
  int    key;     
  string value;
  string input;
 
  //Initialize curser to beginning of table
  
  //Open input streams 
  inStream.open(loadfile.c_str());
  if (inStream.fail())
    cout << "ERROR: Input file does not exist." << endl;
  else {
  //Create a B+Tree
  BTreeIndex btree;
  
  //Open the table file
    rf.open(table + ".tbl", 'w');
    
    //Open the index
    if (index == true) {
	    btree.open(table + ".idx", 'w');
    }	    	    
  
  //Initialize curser to latest empty slot
    rid = rf.endRid();
 
    getline(inStream, input);  //Read a line from input file to test for eof

    int counter = 0;
    //Read data from data file and insert tuples into RF
  while (! inStream.eof() ) {
    parseLoadLine(input, key, value); //Parse the line
    rf.append(key, value, rid); //Write it into RF
 
    if (index == true) { //Write into index
 		btree.insert(key, rid);
 		counter++;
	}
    
    ++rid;  //Increment the counter
    getline(inStream, input);  //Read a line from input file
  }
 
  //Close streams and files
  inStream.close();
  rf.close();
  if (index == true) {
	  cout << "Inputted " << counter << " files." << endl;
  	btree.close();
	}
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

