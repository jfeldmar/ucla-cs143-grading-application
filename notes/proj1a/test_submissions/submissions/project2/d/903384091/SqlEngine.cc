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
  
  BTreeIndex idx;
  
  //printf("cond.size() is %d\n", cond.size());
  
  // open the index file
  // index file does not exist
  rc = idx.open(table + ".idx", 'r');
  //printf("rc is %d\n", rc);
  if (rc < 0 || cond.empty() || (!cond.empty() && cond[0].comp == SelCond::NE))
  {
	//printf("Index file does not exist!\n");
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
  // index file exists
  else
  {
	//printf("Index file exists!\n");
	IndexCursor cursor;
	count = 0;
	//printf("Begin locating\n");
	if (!cond.empty())
	{
		if (cond[0].comp == SelCond::LT || cond[0].comp == SelCond::LE)
		{
			//printf("Less than or less than or equal\n");
			rc = idx.locate(0, cursor);
		}
		else
		{
			//printf("Other\n");
			rc = idx.locate(atoi(cond[0].value), cursor);
			//printf("cursor.pid is %d, cursor.eid is %d\n", cursor.pid, cursor.eid);
		}
	}
	//printf("Done locating\n");
	if (rc != 0)
		return rc;
	nextcursor:
		rc = idx.readForward(cursor, key, rid);
		
	//printf("rc is %d, key is %d, rid.pid is %d, and rid.sid is %d\n", rc, key, rid.pid, rid.sid);
	while (key != 0 || rid.pid != 0 || rid.sid != 0)
	{
	    // read the tuple
	    if ((rc = rf.read(rid, key, value)) < 0) {
	      fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
	      goto exit_select;
	    }
		
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
				if (diff != 0) goto close;
				break;
			  case SelCond::NE:
				if (diff == 0) goto close;
				break;
			  case SelCond::GT:
				if (diff <= 0) goto close;
				break;
			  case SelCond::LT:
				if (diff >= 0) goto close;
				break;
			  case SelCond::GE:
				if (diff < 0) goto close;
				break;
			  case SelCond::LE:
				if (diff > 0) goto close;
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
			
		goto nextcursor;
	}
	close:
	// Close the index
	idx.close();
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
  /* your code here */
  
  // Open RecordFile table for writing
  RecordFile rf;
  RecordId rid;
  RC rc;
  
  // open the table file
  if ((rc = rf.open(table + ".tbl", 'w')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }
  
  // Create index if desired
  BTreeIndex idx;
  
  if (index)
  {
    //printf("Creating an index!\n");
    if (rc = idx.open(table + ".idx", 'w') < 0)
	{
      fprintf(stderr, "Error: index %s failed to open\n", table.c_str());
      return rc;	  
	}
  }
  //else
	//printf("NOT creating an index!\n");
  
  // Open load file for reading
  ifstream lf;
  lf.open(loadfile.c_str());
  if (lf.is_open())
  {
	int key;
	string value;
	string line;
	
	getline(lf, line);
	
	while (line != "")
	{
		// Parse the line
		parseLoadLine(line, key, value);
		
		// Insert the key and value into the record
		rc = rf.append(key, value, rid);
		
		if (index)
		{
			// RC insert(int key, const RecordId& rid);
			//printf("Inserting\n");
			idx.insert(key, rid);
		}
		
		getline(lf, line);
	}
	
	// Close files
	lf.close();
	rf.close();
	if (index)
		idx.close();
  }
  else
  {
    fprintf(stderr, "Error opening file for reading\n");
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
