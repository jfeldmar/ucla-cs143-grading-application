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
  BTreeIndex bTreeIndex;
  bool useIndex; //Use the Btree index if useIndex == true

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
  
  //Check if SELECT query has one or more conditions on the key and if the table has a Btree
  useIndex = false;
  if (bTreeIndex.open(table + ".idx", 'r') == 0) //true if it has index
  {
	int k;
	for (k=0; k < cond.size(); k++) //Check all atribbutes
	{
		if (cond.at(k).attr == 1)
		{
			useIndex = true;  //Found at least one condition on the key attribute.
			break;
		}
	}
  }
 // using index case
  
  if(useIndex)
  {
	  int iCount = 0;
	  // check for lower and upper limits
	  int tempLimit;
	  int lower = 0;
	  IndexCursor lowerCursor, upperCursor;
	  lowerCursor.pid = 1;
	  lowerCursor.eid = 0;
	  int upper = -1;
	  bool upperSet = false;
	  for (int k=0; k < cond.size(); k++) //Check all atribbutes
	  {
		  if((cond[k].comp == SelCond::GT || cond[k].comp == SelCond::GE || cond[k].comp == SelCond::EQ) && cond[k].attr == 1)
		  {
			  tempLimit = atoi(cond[k].value);
			  if(tempLimit > lower)
			  {
				  bTreeIndex.locate(tempLimit, lowerCursor);
				  lower = tempLimit;
				  
			  }
		  }
		  if((cond[k].comp == SelCond::LT || cond[k].comp == SelCond::LE) && cond[k].attr == 1)
		  {
			  tempLimit = atoi(cond[k].value);
			  if(tempLimit < upper || !upperSet)
			  {
				  upperSet = true;
				  upper = tempLimit;
				  bTreeIndex.locate(upper, upperCursor);
			  }
		  }
	  }//end for limits

	  IndexCursor iCursor = lowerCursor; // used to traverse our index
	  int xKey;	// the extracted key
	  RecordId xRid;  // the extracted rid
	  int iDiff;
	  string xValue;

	  //Read forward once to get the initial key to read from
	  bTreeIndex.readForward(iCursor, xKey, xRid);
	  if(!upperSet)
	  {
		  upperSet = true;
		  upper = 100000; //Max value of integer
	  }
	  while((xKey <= upper &&  xKey >= lower) &&  upperSet && xRid.pid >= 0)
	  {
		  if ((rc = rf.read(xRid, xKey, xValue)) < 0) {
			fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
			goto exit_select;
		  }
		  // Extract conditions
		  for (unsigned i = 0; i < cond.size(); i++)
		  {
			  switch (cond[i].attr) {
			  case 1:
				  int cKey;
				  cKey = atoi(cond[i].value);
				  iDiff = xKey - cKey;
				  break;
			  case 2:
				  iDiff = strcmp(xValue.c_str(), cond[i].value);
				  break;
			  }//end switch for attr
		
			   switch (cond[i].comp) {
				  case SelCond::EQ:
					  if (iDiff != 0) goto index_next_tuple;
						break;
				  case SelCond::NE:
					  if (iDiff == 0) goto index_next_tuple;
						break;
				  case SelCond::GT:
					  if (iDiff <= 0) goto index_next_tuple;
						break;
				  case SelCond::LT:
					  if (iDiff >= 0) goto index_next_tuple;
						break;
				  case SelCond::GE:
					  if (iDiff < 0) goto index_next_tuple;
						break;
				  case SelCond::LE:
					  if (iDiff > 0) goto index_next_tuple;
						break;
			   }  // end switch for comp
		  }// end for
		  iCount++;
		  
		  
		   // print the tuple 
			switch (attr) {
			case 1:  // SELECT key
			  fprintf(stdout, "%d\n", xKey);
			  break;
			case 2:  // SELECT value
			  fprintf(stdout, "%s\n", xValue.c_str());
			  break;
			case 3:  // SELECT *
			  fprintf(stdout, "%d '%s'\n", xKey, xValue.c_str());
			  break;
			}
		  
index_next_tuple:
		  // move iCursor
		  bTreeIndex.readForward(iCursor, xKey, xRid);
	  } // end while loop

	  // print matching tuple count if "select count(*)"
	  if (attr == 4) {
		fprintf(stdout, "%d\n", iCount);
	  }
	  rc = 0;

	  // close the table file, index file and return
	  bTreeIndex.close();
	  rf.close();
	  return rc;
  }// end use index
  
  //No index used
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
  // Variables:
  ifstream inputFileStream;
  string tupleLine; //To hold the lines read from the load file
  int tupleKey; //Will hold key from line read
  string tupleValue; //Will hold value from line read
  RC rc;
  RecordFile rf; //Record file to be created/appended to.
  RecordId rid;
  BTreeIndex bTreeIndex;
  
  // Creat the table file
  if ((rc = rf.open(table + ".tbl", 'w')) < 0) {
	  fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
	  return rc;
  }
  // Checking if index was requested
  if (index)
  {
	if (bTreeIndex.open(table + ".idx", 'w')<0)
		fprintf(stderr, "Error: Could not open index file %s\n", table.c_str());
  }
  
  // Open load data table, read its lines and load to recordfile
  inputFileStream.open(loadfile.c_str());
  if (inputFileStream.is_open())
  {
    while (getline (inputFileStream,tupleLine))
    {
	  parseLoadLine(tupleLine, tupleKey, tupleValue);
	  rf.append(tupleKey, tupleValue, rid);
	  //If index was requested
	  if (index)
	  {
		bTreeIndex.insert(tupleKey, rid);
	  }
    }
    inputFileStream.close();
  }
  else cout << "Unable to open file";	
  
  // Close the record file rc
  rf.close();
  if (index)
	bTreeIndex.close();
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
