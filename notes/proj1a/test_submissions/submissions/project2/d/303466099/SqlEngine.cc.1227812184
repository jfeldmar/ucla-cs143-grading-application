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
	int    key = 0;     
	string value;
	int    count;
	int    diff;

	// open the table file
	if ((rc = rf.open(table + ".tbl", 'r')) < 0) 
	{
		fprintf(stderr, "Error: table %s does not exist\n", 
			table.c_str());
		return rc;
	}

	// Count the number of conditions on the key column
	int keyConds = 0;
	for(int i = 0; i < cond.size(); i++)
	{
		if(cond[i].attr == 1 && cond[i].comp != SelCond::NE)
			keyConds++;
	}

	BTreeIndex index;

	RC rctest = index.open(table, 'r');

	// If index exists and there are condition(s) on the key column
	if(rctest >= 0 && keyConds > 0)
	{	
		vector<SelCond> conditions;
		int lowerBound = -1;
		int upperBound = -1;
		int searchKey = -1;
		bool satisfy = true;

		count = 0;

		for(int i = 0; i < cond.size(); i++) {
		  if(cond[i].attr == 1)
		  {
		    int val = atoi(cond[i].value);
		    
		    if(cond[i].comp == SelCond::EQ && searchKey == -1)
		      searchKey = val;
		    if(cond[i].comp == SelCond::GT && val > lowerBound)
		      lowerBound = val + 1;
		    else if(cond[i].comp == SelCond::GE && val > lowerBound)
		      lowerBound = val;
		    else if(cond[i].comp == SelCond::LT)
		    {
		      if(upperBound == -1 || val < upperBound)
		        upperBound = val - 1;
		    }
		    else if(cond[i].comp == SelCond::LE)
		    {
		      if(upperBound == -1 || val < upperBound)
		        upperBound = val;
		    }
		    else
		      continue;
		  }
		  else
		    conditions.push_back(cond[i]);
		}
		
		IndexCursor cursor;

		// If there exists an equality condition
		if(searchKey != -1)
		{
		  // Search for the key
		  index.locate(searchKey, cursor);
		  index.readForward(cursor, key, rid);
			
		  // Read the tuple
		  if ((rc = rf.read(rid, key, value)) < 0) 
		  {
		    fprintf(stderr, 
		      "Error: while reading a tuple from table %s\n", 
		      table.c_str());
		    goto exit_select;
    		  }

		  // Key is not found
		  if(searchKey != key)
		    satisfy = false;

		  // If there exists and upper and lower bound
		  if(lowerBound != -1 && upperBound != -1) {
		    if(key < lowerBound || key > upperBound)
		      satisfy = false;
		  }
		  else 
		  {
		    if(lowerBound != -1) // only lower bound exists
		    {
		      if(key < lowerBound)
		        satisfy = false;
		    }
		    if(upperBound != -1) // only upper bound exists
		    {
		      if(key > upperBound)
		        satisfy = false;
		    }
		  }

		  for(int i = 0; i < conditions.size(); i++) {
		    if((conditions[i].attr == 1)
		      && (conditions[i].comp == SelCond::NE)) {
		        if(key == searchKey)
		          satisfy = false;
		    }
		    if(conditions[i].attr == 2) {
		      switch(conditions[i].comp) {
		        case SelCond::EQ:
		          if(value.c_str() != conditions[i].value)
		            satisfy = false; break;
		        case SelCond::NE:
			  if(value.c_str() == conditions[i].value)
			    satisfy = false; break;
			case SelCond::LT:
			  if(value.c_str() >= conditions[i].value)
			    satisfy = false; break;
			case SelCond::GT:
			  if(value.c_str() <= conditions[i].value)
			    satisfy = false; break;
			case SelCond::LE:
			  if(value.c_str() > conditions[i].value)
			    satisfy = false; break;
			case SelCond::GE:
			  if(value.c_str() < conditions[i].value)
			    satisfy = false; break;
		      }
		    }
		  }

		  if(satisfy) {
		    count++;
				
		    // print the tuple 
		    switch (attr) {
		      case 1:  // SELECT key
		        fprintf(stdout, "%d\n", key); break;
                      case 2:  // SELECT value
		        fprintf(stdout, "%s\n", value.c_str()); break;
                      case 3:  // SELECT *
                        fprintf(stdout, "%d '%s'\n", key, value.c_str()); break;
		    }
		  }
		}

		else
		{
		  if(lowerBound != -1)
		    index.locate(lowerBound, cursor);
		  else
		    index.locate(0, cursor);

		  while(index.readForward(cursor, key, rid) != RC_END_OF_TREE)
		  {
		    satisfy = true;
		    // Read the tuple
		    if ((rc = rf.read(rid, key, value)) < 0) {
		      fprintf(stderr, 
		      "Error: while reading a tuple from table %s\n", 
		      table.c_str());
		      goto exit_select;
		    }

		    if((upperBound != -1 && key <= upperBound) 
		    || upperBound == -1) 
		    {
		      for(int i = 0; i < conditions.size(); i++) 
		      {
		        if((conditions[i].attr == 1)
			&& (conditions[i].comp == SelCond::NE)) 
			{
			  if(key == searchKey)
			    satisfy = false;
			}
			if(conditions[i].attr == 2) 
			{
			int diff = strcmp(value.c_str(), 
						conditions[i].value);
			  switch(conditions[i].comp) {
			    case SelCond::EQ:
			      if(diff != 0)
			        satisfy = false; break;
			    case SelCond::NE:
			      if(diff == 0)
			        satisfy = false; break;
			    case SelCond::LT:
			      if(diff >= 0)
				satisfy = false; break;
			    case SelCond::GT:
			      if(diff <= 0)
				satisfy = false; break;
			    case SelCond::LE:
			      if(diff > 0)
				satisfy = false; break;
			    case SelCond::GE:
			      if(diff < 0)
				satisfy = false; break;
			  }
			}
		      }
		    }
		    else
		      break;
		      
		    if(satisfy)
		    {
		      count++;
				
    		      // print the tuple 
    		      switch (attr) {
    		        case 1:  // SELECT key
      		        fprintf(stdout, "%d\n", key); break;
	    		case 2:  // SELECT value
      			fprintf(stdout, "%s\n", value.c_str()); break;
	    		case 3:  // SELECT *
      			fprintf(stdout, "%d '%s'\n", key, value.c_str()); break;
	    	      }
		    }
		  }
		}
		index.close();
	}
	else
	{
		// scan the table file from the beginning
		rid.pid = rid.sid = 0;
		count = 0;
		while (rid < rf.endRid()) 
		{
			// read the tuple
			if ((rc = rf.read(rid, key, value)) < 0) 
			{
				fprintf(stderr, 
				"Error: while reading a tuple from table %s\n", 
				table.c_str());
				goto exit_select;
    			}

    			// check the conditions on the tuple
    			for (unsigned i = 0; i < cond.size(); i++) 
			{
      				// compute the difference between 
      				// the tuple value 
      				// and the condition value
      				switch (cond[i].attr) 
				{
      					case 1:
					diff = key - atoi(cond[i].value);
					break;
      					case 2:
					diff = strcmp(value.c_str(), 
						cond[i].value);
					break;
      				}

      				// skip the tuple if any condition is not met
      				switch (cond[i].comp) 
				{
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
    			switch (attr) 
			{
    				case 1:  // SELECT key
      					fprintf(stdout, "%d\n", key);
	      				break;
    				case 2:  // SELECT value
      					fprintf(stdout, "%s\n", value.c_str());
      					break;
	    			case 3:  // SELECT *
      					fprintf(stdout, "%d '%s'\n", 
					key, value.c_str());
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
  	return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	RecordFile rf;
	RecordId   rid;
	FILE*      lf;

	char   tuple [256];
	int    key;
	string value;
	RC     rc;
 
	rc = rf.open(table + ".tbl", 'w');
	lf = fopen(loadfile.c_str(), "r");

	if(lf == NULL)
	{
		perror("File does not exist");
		rc = RC_FILE_OPEN_FAILED;
	}
	else
	{
		BTreeIndex bTree;
		if(index == true)
			bTree.open(table, 'w');
		while(fgets(tuple, 256, lf) != NULL)
		{
			parseLoadLine(tuple, key, value);
			rf.append(key, value, rid);
			if(index == true)
				bTree.insert(key, rid);
		}
		if(index == true)
			bTree.close();
		fclose(lf);
	} 

	rf.close();

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
