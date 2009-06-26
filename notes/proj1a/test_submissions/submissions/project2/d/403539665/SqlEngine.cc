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




  // maximum length of the value field
  static const int MAX_VALUE_LENGTH = 100;  

  // number of record slots per page
  static const int RECORDS_PER_PAGE = (PageFile::PAGE_SIZE - sizeof(int))/ (sizeof(int) + MAX_VALUE_LENGTH);  

  bool newindex;


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
  RecordFile rf2;   // RecordFile containing the table
  RecordId   rid2;  // record cursor for table scanning

  RC     rc;
  RC     rc2;
  int    key; 
  int unusedkey;
  string value;
  string unusedvalue;
  int    count;
  int    diff;
  
  //if(cond[0].comp != SelCond::NE)
  //{
	  if(rc2 = rf2.open(table + ".idx", 'r') < 0) {
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	  }
	  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	  }
  //}
  // open the table file
  /*else
  {
	  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	  }
  }*/

  // scan the table file from the beginning
  /*rid.pid = rid.sid = 0;
  count = 0;
  while (rid < rf.endRid()) {
    // read the tuple
    if ((rc = rf.read(rid, key, value)) < 0) {
      fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
      goto exit_select;
    }*/

  rid.pid = rid.sid = 0;
  count = 0;

  while (rid < rf2.endRid()) {
    // read the tuple
    if(newindex)
	{
		if ((rc = rf2.read(rid, key, unusedvalue)) < 0) {
			fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
			goto exit_select;
		}
		if ((rc = rf.read(rid, key, value)) < 0) {
			fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
			goto exit_select;
		}
	}
	else if ((rc = rf.read(rid, unusedkey, value)) < 0) {
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
  /* your code here */
	newindex = index;
	
	char mystring [256];
	RecordFile rf;
	RecordFile rf2;
	FILE * file;
	FILE * file2;
	RC rc;
	RC rc2;
	int key;
	int testkey;
	string value;
	RecordId id;
	RecordId id2;
	BTreeIndex newBT;
	PageFile pf;
	const char * test;
	test = "";

	
	if(!index)
	{
		rc = rf.open(table + ".tbl", 'w');

		if(rc < 0)
			printf("ERROR, Cant open the file");

		file = fopen(loadfile.c_str(), "r");

		if(file==NULL)
			printf("ERROR, COULD NOT OPEN FILE");
		else {
		
			testkey = -1;
		
			for(int i = 0; i < 5000; i++)
			{
				fgets(mystring,256,file);
				parseLoadLine(mystring,key,value);
				if(testkey != key)
					rf.append(key,value,id);
				else
					break;
				testkey=key;
			}
			return 0;
		}
	}
	else
	{
			
		rc = rf.open(table + ".tbl", 'w');

		if(rc < 0)
			printf("ERROR, Cant open the file");

		file = fopen(loadfile.c_str(), "r");

		if(file==NULL)
			printf("ERROR, COULD NOT OPEN FILE");
		else {
		
			rc2 = rf2.open(table + ".idx", 'w');

			if(rc2 < 0)
				printf("ERROR, Cant open the file");


			testkey = -1;
			id2.sid = 0;
			id2.pid = 0;

			for(int i = 0; i < 5000; i++)
			{
				fgets(mystring,256,file);
				parseLoadLine(mystring,key,value);
				if(testkey != key)
				{
					rf.append(key,value,id);

					rf2.append(key,test, id2);

					id2.sid = id2.sid + 1;

					if(id2.sid > RECORDS_PER_PAGE)
					{
						id2.sid = 0;
						id2.pid = id2.pid + 1;
					}
				}
				else
					break;
				testkey=key;
			}
			return 0;
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
