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
  BTreeIndex idx;  // BTreeIndex containing the index
  IndexCursor ic;  // index cursors for index scanning

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;
  bool   index;   // true if index exist
  int    ubound = INT_MAX;
  int    lbound = INT_MIN;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  
  // open the index file
  if((rc = idx.open(table + ".idx", 'r')) < 0) {
    index = false;
  }
  else {
    index = true;
  }

  // scan the table file from the beginning
  rid.pid = rid.sid = 0;
  
  // initialize all index cursors
  ic.pid = ic.eid = -1;
  if(index){
    for (unsigned i = 0; i < cond.size(); i++) {
      if (cond[i].attr == 1) {
	key = atoi(cond[i].value);
	switch (cond[i].comp) {
	case SelCond::EQ:
	  idx.locate(key,ic);
	  lbound = ubound = key;
	  goto begin_iteration;
	  break;
	case SelCond::NE:
	  break;
	case SelCond::GT:
	case SelCond::GE:
	  if (key > lbound){
	    lbound = key;
	    idx.locate(key,ic);
	  }
	  break;
	case SelCond::LT:
	case SelCond::LE:
	  if (key < ubound){
	    ubound = key;
	    idx.locate(lbound, ic);
	  }
	  break;
	}
      }
    }
  }

 begin_iteration:
  if(ic.pid == -1)
    index = false;

  count = 0;
  while (rid < rf.endRid()) {
    if(index){
      idx.readForward(ic,key,rid);
      if(key > ubound || key < lbound){
	goto exit_select;
      }
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
    if(!index)
      ++rid;
    else if(ic.pid == RC_END_OF_TREE)
      goto exit_select;
  }

  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  exit_select:
  rf.close();
  idx.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // RecordId of newly appended record
  ifstream   ifs;  // File to load the data from
  BTreeIndex idx; // Index that will be created for the table
  
  RC         rc;
  //const int  N = 256;
  //char       buf[N];
  string     line; // line read from the data file
  int        key;
  string     value;

  // open the load file
  ifs.open(loadfile.c_str(), ifstream::in);
  if(!ifs || !ifs.good()){
    fprintf(stderr, "Error: input file %s could not be opened\n", loadfile.c_str());
    return RC_FILE_OPEN_FAILED;
  }

  // open the table file
  rc = 0;
  if ((rc = rf.open(table + ".tbl", 'w')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return RC_FILE_OPEN_FAILED;
  }

  // open the index file
  //printf("Index = %s, rc = %d\n", (index ? "TRUE" : "FALSE"), rc);
  if(index){
    if((rc = idx.open(table + ".idx", 'w')) < 0) {
      fprintf(stderr, "Error: Index %s does not exist with error %d\n", table.c_str(),rc);
      return RC_FILE_OPEN_FAILED;
    }
  }

  do{
    getline(ifs,line);
    if(line == "") continue;
    if(parseLoadLine(line,key,value) != 0){
      break;
    }
    if(((rc = rf.append(key,value,rid) != 0)) || (index && ((rc = idx.insert(key,rid) !=0)))){
      fprintf(stderr, "Error: error appending record to %s table or index\n", table.c_str());
      break;
    }
  } while(!ifs.eof());

  // close the table file and return
  exit_select:
  rf.close();
  if(index){
    //idx.printIndex();
    idx.close();
  }
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
