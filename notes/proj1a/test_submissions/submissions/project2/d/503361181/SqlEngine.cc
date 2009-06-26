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
  bool useindex = false;
  BTreeIndex bt;
  rc = bt.open(table + ".idx", 'r');
  
  if(rc == 0)
  {
	if(cond.size())
		useindex = true;
	for(unsigned i = 0; i < cond.size(); i++)
	{
		if(cond[i].attr != 1 || cond[i].comp == SelCond::NE)
		{
			useindex = false; 
			break;
		}
	}
  }
  if(useindex)
  {
	IndexCursor index;
	if(cond.size()==1)
	{
		if(cond[0].comp == SelCond::EQ)
		{
			bt.locate(atoi(cond[0].value), index);
			bt.readForward(index, key, rid);
			rf.read(rid, key, value);
			if(key == atoi(cond[0].value))
			{
				count++;
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
			if(attr == 4)
			{
				fprintf(stdout, "%d\n", count);
			}
				
			goto exit_select;
		}
		else if(cond[0].comp == SelCond::GE || cond[0].comp == SelCond::GT)
		{
			bt.locate(atoi(cond[0].value), index);
			bt.readForward(index, key, rid);
			rf.read(rid, key, value);
			if(cond[0].comp == SelCond::GE && key == atoi(cond[0].value))
			{
			count++;
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
			while(bt.readForward(index, key, rid) == 0)
			{
				count++;
				rf.read(rid, key, value);
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
			if(attr == 4)
			{
				fprintf(stdout, "%d\n", count);
			}
				
			goto exit_select;
		}
		else
		{
			int x = -2147483647;
			bt.locate(x, index);
			//bt.readForward(index, key, rid);
			
			while(1)
			{
				bt.readForward(index, key, rid);
				rf.read(rid, key, value);
				if((cond[0].comp == SelCond::LE && key>atoi(cond[0].value)) || (cond[0].comp == SelCond::LT && key>=atoi(cond[0].value)))
					break;
				count++;
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
			if(attr == 4)
			{
				fprintf(stdout, "%d\n", count);
			}
			goto exit_select;
		}
	}
		else if(cond.size() == 2)
		{
			int lowerbound, upperbound;
			if(cond[0].comp == SelCond::GE || cond[0].comp == SelCond::GT)
			{
				lowerbound = atoi(cond[0].value);
				upperbound = atoi(cond[1].value);
			}
			else
			{
				lowerbound = atoi(cond[1].value);
				upperbound = atoi(cond[1].value);
			}
			
			bt.locate(lowerbound, index);
			bt.readForward(index, key, rid);
			rf.read(rid, key, value);
			if((cond[0].comp == SelCond::GE || cond[1].comp == SelCond::GE) && key == lowerbound)
			{
			count++;
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
			while(bt.readForward(index, key, rid) == 0)
			{
				rf.read(rid, key, value);
				if(((cond[0].comp == SelCond::LE || cond[1].comp == SelCond::LE) && key>upperbound) ||
					((cond[0].comp == SelCond::LT || cond[1].comp == SelCond::LT) && key>=upperbound))
					{
						break;
					}
					
					
				count++;
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
			if(attr == 4)
			{
				fprintf(stdout, "%d\n", count);
			}
				
			goto exit_select;

			
		}
  }

  else
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
  bt.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */
  
  ifstream fin;
  fin.open(loadfile.c_str(), ios::binary);
  
  if(fin.fail())
  {
		cerr<<"Could not open file"<<endl;
		exit(1);
  }
  
  RecordFile rf;
  RecordId rid;
  BTreeIndex bt;

  
  rf.open(table+".tbl", 'w');
  
  if(index)
  {
	bt.open(table+".idx", 'w');
  }
  
  char line[1024];
  
  while(fin.getline(line, 1024)) 
  {
	int key;
	string value;
	parseLoadLine(line, key, value);
	rf.append(key, value, rid);
	if(index)
	{
		bt.insert(key, rid);
	}
  }

  if(index)
  {
	bt.close();
  }
  rf.close();
  fin.close();

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
