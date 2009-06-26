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
  int    count = 0;
  int    diff;
  bool index;

  BTreeIndex *btree = new BTreeIndex();

  // open the table file
  	if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    		return rc;
  	}

	btree->close();
  	rc = btree->open(table + ".idx", 'r');

	if (rc == 0)
		index = true;
  	else
		index = false;

	bool keyCond = false;

    	for (unsigned i = 0; i < cond.size(); i++) 
	{
      		if (cond[i].attr == 1) 
		{
			keyCond = true;
			break;
      		}
      	}

if (index && keyCond)
{
	bool EqCond = false;	// Check Equality Condition First
	unsigned EqIndex = -1;

	bool RangeCond = false;
	unsigned RangeIndex = -1;

	bool LesserCond = false;

	bool flag = false;

	for (unsigned i = 0; i < cond.size(); i++) 
	{
		if (cond[i].comp == SelCond::NE)
			goto noindex;

      		if (cond[i].comp == SelCond::EQ && cond[i].attr == 1) 
		{
			EqCond = true;
			EqIndex = i;
			break;
      		}

		if ((cond[i].comp == SelCond::GE || cond[i].comp == SelCond::GT) && cond[i].attr == 1 && !RangeCond)
		{
			RangeCond = true;
			RangeIndex = i;
		}

		if ((cond[i].comp == SelCond::LE || cond[i].comp == SelCond::LT) && cond[i].attr == 1 && !LesserCond)
		{
			LesserCond = true;
		}
      	}

	if (EqCond)
	{
		IndexCursor *cursor = new IndexCursor();

		btree->locate(atoi(cond[EqIndex].value),*cursor);
		btree->readForward(*cursor,key,rid);
		
		if ((rc = rf.read(rid, key, value)) < 0) 
		{
      			fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
      			goto exit_select;
   		}

      		diff = key - atoi(cond[EqIndex].value);
		if (diff != 0) 
			goto exit_select;

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
  			case 4:
    				fprintf(stdout, "%d\n", count);
  				break;
    		}

		goto exit_select;	
	}

	if (RangeCond)
	{
		IndexCursor *cursor = new IndexCursor();

		btree->locate(atoi(cond[RangeIndex].value),*cursor);
		if(btree->readForward(*cursor,key,rid) < 0)
			goto endTree;
		
		while(1)
		{
			if ((rc = rf.read(rid, key, value)) < 0) 
			{
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
						if (diff != 0) goto next1;
						break;
     					case SelCond::NE:
						if (diff == 0) goto next1;
						break;
      					case SelCond::GT:
						if (diff <= 0) goto next1;
						break;
      					case SelCond::LT:
						if (diff >= 0 && cond[i].attr == 1) goto endTree;
						if (diff >= 0) goto next1;
						break;
      					case SelCond::GE:
						if (diff < 0) goto next1;
						break;
      					case SelCond::LE:
						if (diff > 0 && cond[i].attr == 1) goto endTree;
						if (diff > 0) goto next1;
						break;
      				}
    			}

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

			next1:
			if (flag)
				goto endTree;

			if(btree->readForward(*cursor,key,rid) < 0)
			{
				if (!flag)
					flag = true;
			}
		}

		endTree:
  		if (attr == 4)
    			fprintf(stdout, "%d\n", count);
  	
		goto exit_select;	
	}
	
	if (LesserCond)
	{
		IndexCursor *cursor = new IndexCursor();

		btree->locate(-9999999,*cursor);
		if(btree->readForward(*cursor,key,rid) < 0)
			goto endTree2;
		
		while(1)
		{
			if ((rc = rf.read(rid, key, value)) < 0) 
			{
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
						if (diff != 0) goto next2;
						break;
     					case SelCond::NE:
						if (diff == 0) goto next2;
						break;
      					case SelCond::GT:
						if (diff <= 0) goto next2;
						break;
      					case SelCond::LT:
						if (diff >= 0 && cond[i].attr == 1) goto endTree2;
						if (diff >= 0) goto next2;
						break;
      					case SelCond::GE:
						if (diff < 0) goto next2;
						break;
      					case SelCond::LE:
						if (diff > 0 && cond[i].attr == 1) goto endTree2;
						if (diff > 0) goto next2;
						break;
      				}
    			}

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

			next2:
			if (flag)
				goto endTree2;

			if(btree->readForward(*cursor,key,rid) < 0)
			{
				if (!flag)
					flag = true;
			}
		}

		endTree2:
  		if (attr == 4)
    			fprintf(stdout, "%d\n", count);
  	
		goto exit_select;	
	}
}

else
{
	noindex:
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
}

  // close the table file and return
  exit_select:
  rf.close();
  return rc;

}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */
	char toParse[300];
	int key = 0;
	string value;
	RecordId rid;
	RecordFile *rf = new RecordFile();

	rf->open(table + ".tbl",'w');
	
	FILE * ifile;
	ifile = fopen(loadfile.c_str(),"r");
	
	/*if(!ifile.is_open())
	{
		printf("ERROR: Unable to Open File");
		return -1;
	}

	else*/

	if (index)
	{
		BTreeIndex *btree = new BTreeIndex();
		btree->open(table + ".idx",'w');

		while(fgets(toParse,300,ifile) != NULL)
		{
			parseLoadLine(toParse,key,value);
			rf->append((int)key,(const string&)value,rid);
			btree->insert((int)key,rid);
		}

		btree->close();
	}

	else
	{
		while(fgets(toParse,300,ifile) != NULL)
		{
			parseLoadLine(toParse,key,value);
			rf->append((int)key,(const string&)value,rid);
		}
	}

	fclose(ifile);
	rf->close();	

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
