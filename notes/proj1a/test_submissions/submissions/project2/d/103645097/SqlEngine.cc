/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

#define MIN_INT -2147483647
#define MAX_INT 2147483647

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

  BTreeIndex tree;
  bool index = tree.open(table + ".idx", 'r') == 0; // use index?

  // scan the table file from the beginning
  rid.pid = rid.sid = 0;
  count = 0;

  bool usesKey = false;
  for(unsigned i = 0; i < cond.size(); i++)
  {
	  if(cond[i].attr == 1 && cond[i].comp != SelCond::NE)
	  {
		  usesKey = true;
		  break;
	  }
  }

  if(index && usesKey)
  {

	  bool ok = true;

	  // calculate allowed ranges of keys (since only AND is allowed, it should be the MOST restrictive)
	  int keyMin = MIN_INT;
	  int keyMax = MAX_INT;

	  for(unsigned i = 0; i < cond.size(); i++)
	  {
		  if(cond[i].attr == 2 || cond[i].comp == SelCond::NE)
			  continue;

		  int val = atoi(cond[i].value);

		  switch(cond[i].comp)
		  {
		    case SelCond::EQ:
		    	keyMin = keyMax = val;
			    break;
		    case SelCond::GE:
		    	keyMin = val > keyMin ? val : keyMin;
			    break;
		    case SelCond::GT:
		    	val++;
		    	keyMin = val > keyMin ? val : keyMin;
			    break;
		    case SelCond::LE:
		    	keyMax = val < keyMax ? val : keyMax;
		    	break;
		    case SelCond::LT:
		    	val--;
		    	keyMax = val < keyMax ? val : keyMax;
		    	break;
		  }

		  if(keyMin > keyMax || keyMax < keyMin)
			  break;
	  }

	  checkRange:;
	  if(keyMin > keyMax || keyMax < keyMin)
		  ok = false;

	  IndexCursor cursor;
	  tree.locate(keyMin, cursor);

	  while(ok)
	  {
		  if(tree.readForward(cursor, key, rid) != 0)
			  break;
		  if(key > keyMax)
			  break;

			  if ((rc = rf.read(rid, key, value)) < 0) {
					  fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
					  goto exit_select;
					}
		  for(unsigned i = 0; i < cond.size(); i++)
		  {
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
			if (diff != 0) goto next_tuple2;
			break;
			  case SelCond::NE:
			if (diff == 0) goto next_tuple2;
			break;
			  case SelCond::GT:
			if (diff <= 0) goto next_tuple2;
			break;
			  case SelCond::LT:
			if (diff >= 0) goto next_tuple2;
			break;
			  case SelCond::GE:
			if (diff < 0) goto next_tuple2;
			break;
			  case SelCond::LE:
			if (diff > 0) goto next_tuple2;
			break;
			  }
			}

			// the condition is met for the tuple.
			// increase matching tuple counter
			count++;

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

		  next_tuple2:;
	  }
  }
  else // no index, or query doesn't mention key value at all
  {
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

  // close the table file and return
  exit_select:
  rf.close();
  if(index)
	  tree.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  ifstream dataFile(loadfile.c_str());

  string line;
  int    key;
  string value;
  RecordId rid;

  string tableFileName(table);
  tableFileName += ".tbl";

  RecordFile record(tableFileName, 'w');

  BTreeIndex tree;
  if(index)
  {
	  string indexFileName(table);
	  indexFileName += ".idx";
	  tree.open(indexFileName, 'w');
  }

  while(!dataFile.eof())
  {
	  getline(dataFile, line);

	  if(parseLoadLine(line, key, value) != 0)
		  break;

	  record.append(key, value, rid);

	  if(index)
		  tree.insert(key, rid);
  }

  dataFile.close();

  if(index)
	  tree.close();

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
