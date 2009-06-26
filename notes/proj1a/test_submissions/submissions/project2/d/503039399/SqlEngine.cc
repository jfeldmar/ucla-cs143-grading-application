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
#include <algorithm>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);

bool operator< (const SelCond& c1, const SelCond& c2)
{
	if (c1.attr != c2.attr)
		return c1.attr < c2.attr;
	else if (c1.attr == 2)
		return false;
	else
	{
		int c1sortOrder;
		int c2sortOrder;

		switch (c1.comp)
		{
			case SelCond::EQ:
				c1sortOrder = 0;
				break;
			case SelCond::LT:
				c1sortOrder = 1;
				break;
			case SelCond::LE:
				c1sortOrder = 2;
				break;
			case SelCond::GT:
				c1sortOrder = 3;
				break;
			case SelCond::GE:
				c1sortOrder = 4;
				break;
			case SelCond::NE:
				c1sortOrder = 5;
				break;
		}
			
		switch (c2.comp)
		{
			case SelCond::EQ:
				c2sortOrder = 0;
				break;
			case SelCond::LT:
				c2sortOrder = 1;
				break;
			case SelCond::LE:
				c2sortOrder = 2;
				break;
			case SelCond::GT:
				c2sortOrder = 3;
				break;
			case SelCond::GE:
				c2sortOrder = 4;
				break;
			case SelCond::NE:
				c2sortOrder = 5;
				break;
		}
		if (c1.comp != c2.comp)
			return (c1sortOrder < c2sortOrder);
		else
		{
			int c1value = atoi(c1.value);
			int c2value = atoi(c2.value);
			if (c1.comp == SelCond::LT || c1.comp == SelCond::LE)
				return (c1value < c2value);
			else if (c1.comp == SelCond::GT || c1.comp == SelCond::GE)
				return (c1value > c2value);
			else
				return false;	
		}
	}
}

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
  bool index;
  int KEY_MIN = std::numeric_limits<int>::min();
  int KEY_MAX = std::numeric_limits<int>::max();

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

  // open the index if it exists

	BTreeIndex tableIndex;

	if ((rc = tableIndex.open(table + ".idx", 'r')) < 0)
		index = false;
	else
		index = true;
	
	
	if (index)
	{		
		vector<SelCond> condSorted = cond; // Copy the vector of SelConds so that they can be sorted
		
		sort(condSorted.begin(), condSorted.end());
		
		/* At this point, conSorted is sorted by attr and condition.  Our table is indexed by key, so we only care 
		about conditions on the key, were attr==1.  The next step is to remove redundant conditions, i.e.
		key > 2 AND key > 3 ==> key > 3.  For now, we will ignore <> conditions.
		*/
		bool lb = false;
		bool ub = false;
		bool eq = false;
		int eqValue;
		int lowerbound;
		int upperbound;

		for (unsigned int i = 0; i < condSorted.size(); i++) 
		{
			if (condSorted[i].attr == 2)
				break;
			
			int curValue = atoi(condSorted[i].value);
			
			if (condSorted[i].comp == SelCond::EQ)
			{
				// If this is the first equals condition, store the value and set eq to true
				if (!eq)
				{
					eq = true;
					eqValue = curValue;
				}
				else if (eqValue != curValue) // If we've already seen a different equals value, exit
				{		
					goto exit_select;
				}
			}		
			else if (condSorted[i].comp == SelCond::LT)
			{
				if (!ub)
				{
					ub = true;
					upperbound = curValue-1;
				}
			}
			else if (condSorted[i].comp == SelCond::LE)
			{
				if (!ub)
				{
					ub = true;
					upperbound = curValue;
				}
				else if (curValue < upperbound)
				{
					upperbound = curValue;
				}					
			}
			else if (condSorted[i].comp == SelCond::GT)
			{
				if (!lb)
				{
					lb = true;
					lowerbound = curValue+1;
				}
			}
			else if (condSorted[i].comp == SelCond::GE)
			{
				if (!lb)
				{
					lb = true;
					lowerbound = curValue;
				}
				else if (curValue > lowerbound)
				{
					lowerbound = curValue;
				}					
			}
	
		
		}

		// At this point, lowerbound, upperbound, and eq are set based on the conditions given.
		// The next step is to check the tuples within this rage and check to see if they meet
		// the rest of the condtions in condSorted (!= and conditions on value)
		
		if (!lb)
			lowerbound = KEY_MIN;
		if (!ub)
			upperbound = KEY_MAX;
		// Check to see if there are any valid keys in the range
		if (lowerbound > upperbound) // Check that lowerbound is less than upperbound
			goto exit_select;
		if (eq && eqValue < lowerbound) // Check that the equals condition is greater than lowerbound
			goto exit_select;
		if (eq && eqValue > upperbound)
			goto exit_select;
		
		IndexCursor ic;
		count = 0;
		
		if (eq)
		{
			lowerbound = eqValue;
			upperbound = eqValue;
		}
		
		int curKey;
		RecordId curRid;
		tableIndex.locate(lowerbound, ic);
		
		while (true)
		{
				if (tableIndex.readForward(ic, curKey, curRid) == RC_END_OF_TREE)
				{
					goto exit_select;
				}
				if (curKey > upperbound)
					goto exit_select;
				// read the tuple
				
			    if ((rc = rf.read(curRid, key, value)) < 0) 
				{
			   		fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
			   		goto exit_select;
			   	}			
			
	
				if (key > upperbound)
					goto exit_select;
					
				for(unsigned int i = 0; i<condSorted.size(); i++)
				{
					if (cond[i].attr == 1)
						continue;
					// compute the difference between the tuple value and the condition value

					diff = strcmp(value.c_str(), cond[i].value);

			   		// skip the tuple if any condition is not met
			   		switch (cond[i].comp) 
					{
			   			case SelCond::EQ:
							if (diff != 0) goto next_tuple_index;
							break;
			      		case SelCond::NE:
							if (diff == 0) goto next_tuple_index;
							break;
			    		case SelCond::GT:
							if (diff <= 0) goto next_tuple_index;
							break;
			    		case SelCond::LT:
							if (diff >= 0) goto next_tuple_index;
							break;
			    		case SelCond::GE:
							if (diff < 0) goto next_tuple_index;
							break;
			    		case SelCond::LE:
							if (diff > 0) goto next_tuple_index;
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
				next_tuple_index:
				continue;
			} // End of while loop	
			
	  			// close the table file and return

			goto exit_select;
		
		}

	// If no index exists, just scan the entire table
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
      		fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
      		goto exit_select;
    	}

    	// check the conditions on the tuple
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
      			fprintf(stdout, "%d '%s'\n", key, value.c_str());
      			break;
    	}

    	// move to the next tuple
    	next_tuple:
    	++rid;
  	}

  // print matching tuple count if "select count(*)"

	}
  	// close the table file and return
  	exit_select:
	if (attr == 4)
 	{
    	fprintf(stdout, "%d\n", count);
  	}
  	rc = 0;
  	rf.close();
  	return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	ifstream in; // Create an input file stream
	string line; // A buffer to store the current line in the file
	int key;
	string value;
	RecordFile rf;
	BTreeIndex tableIndex;
	RC rc;
	RecordId rid;
	
	in.open(loadfile.c_str()); // Open the file 'loadfile'
	if (!in)
	{
		fprintf(stderr, "Error: file %s does not exist\n", loadfile.c_str());
		return RC_FILE_OPEN_FAILED;
	}
	
	// open the table file
  	if ((rc = rf.open(table + ".tbl", 'w')) < 0) 
	{
    	fprintf(stderr, "Error: could not open table %s. %i\n", table.c_str(), rc);
    	return rc;
  	}


	
	// open the index file if 'index' is true
	if (index)
	{
		if ((rc = tableIndex.open(table + ".idx", 'w')) < 0)
		{
			fprintf(stderr, "Error: could not open table index %s. %i\n", table.c_str(), rc);
	    	return rc;
		}
	}
	

	
	while (!in.eof())
	{
		getline (in,line); // Read the current line of the input file
		
		if (parseLoadLine(line, key, value) == 0) // Parse the file and store key and value into their temp variables
		{
			if ((rc = rf.append(key, value, rid)) < 0) // call the append() function to store the tuplet
			{
				fprintf(stderr, "Error: cannot add %s, %i to table\n", value.c_str(), key);
				return rc;
			}
			// If indexing, store the tuple in the index as well
			if (index)
				tableIndex.insert(key, rid);
		}
	}
	
	if (index)
	{
		if ((rc = tableIndex.close()) < 0) // close the index file
		{
			fprintf(stderr, "Error: cannot close the index %s. %i\n", table.c_str(), rc);
			return rc;
			
		}
	}
	
		
	if ((rc = rf.close()) < 0) // close the record file
	{
		fprintf(stderr, "Error: cannot close the table %s. %i\n", table.c_str(), rc);
		return rc;
	}
	

	
	in.close(); // close the input file
	

	
	
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
