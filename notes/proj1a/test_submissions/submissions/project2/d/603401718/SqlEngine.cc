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

bool sel_cond_compare(SelCond a, SelCond b)
{
	return atoi(a.value) < atoi(b.value);
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

	////Separate key attributes and value attributes
	////Set up seperate SelCond vectors for each key comparator
	list<SelCond> equality;
	list<SelCond> values;
	list<SelCond> compare;
		
	for (int i = 0; i < cond.size(); ++i)	{
		//If value, do nothing
		if (cond[i].attr == 2)	values.push_back(cond[i]);
		//Else if it's a key
		else 	{
			switch (cond[i].comp)	{
			case SelCond::EQ:
				equality.push_back(cond[i]); break;
			case SelCond::NE:
				break;
			default:
				compare.push_back(cond[i]);	break;
			}
		}
	}

	//Check if indexing is enabled
	bool indexing;
	BTreeIndex tree;
	if (tree.open(table + ".idx",'r') == 0)	indexing = true;
	else indexing = false;

	//If index is enabled and there is a condition on the key column
	if (indexing == true && values.size() != cond.size()) {

		//fprintf(stderr,"Entering indexing\n");
		
		//Set up a vector of candidate rids
		vector<RecordId> candidate;
		
		//Markers to determine whether candidates exist
		bool eq = false;
		bool ra = false;		

		//Insert equality candidates
		if(!equality.empty())	{
			//fprintf(stderr,"Equality\n");
			eq = true;
			IndexCursor cursor;
			rc = tree.locate(atoi((equality.front()).value),cursor);	
			if (rc != 0){ 
				//fprintf(stderr, "Error with %d\n", atoi((equality.front()).value));
				return rc;	
			}			
			RecordId equal_rid;
			int equal_key = atoi((equality.front()).value);
			while (equal_key == atoi((equality.front()).value))	{
				tree.readForward(cursor,equal_key,equal_rid);
				if (atoi((equality.front()).value)== equal_key)	{
					candidate.push_back(equal_rid);
				}
			}
		}

		//Insert range candidates if necessary
		if (eq == false && !compare.empty())	{
			//fprintf(stderr,"Inequality\n");
			ra = true;
			//Sort the list
			compare.sort(sel_cond_compare);
			//Declare the bounds
			int lower_bound,upper_bound;
			bool bound_found = false;
			//Finding the upper and lower bounds
			if ((compare.front()).comp == SelCond::LT)	{
				lower_bound = INT_MIN;
				upper_bound = atoi((compare.front()).value);
				bound_found = true;
			}
			else if ((compare.front()).comp == SelCond::LE) {
				lower_bound = INT_MIN;
				upper_bound = atoi((compare.front()).value) + 1;
				bound_found = true;
			}
			else if ((compare.front()).comp == SelCond::GT || (compare.front()).comp == SelCond::GE) {
				//Loop to find the lower and upper bound
				while (!bound_found)	{
					//Get the lower bound then pop
					if ((compare.front()).comp == SelCond::GT)	lower_bound = atoi((compare.front()).value);
					else if ((compare.front()).comp == SelCond::GE)	lower_bound = atoi((compare.front()).value) - 1;
					compare.pop_front();
					//If there was only one comparison entry, the upper bound is max
					if (compare.empty())	{
						upper_bound = INT_MAX;
						bound_found = true;
					}
					//If next minimum is lt or le, get the upperbound
					else if ((compare.front()).comp == SelCond::LT )	{
						upper_bound = atoi((compare.front()).value);
						bound_found = true;
					}
					else if ((compare.front()).comp == SelCond::LE) {
						upper_bound = atoi((compare.front()).value) + 1;
						bound_found = true;
					}
					//Else, start the loop again but using the new front
					else	{
						continue;
					}
				}
			}
			//Gather the candidates and locate in the tree
			IndexCursor cursor;
			rc = tree.locate(lower_bound + 1, cursor);	if (rc != 0){ goto normalcase;	}
			//fprintf(stderr,"Lowerbound is %d and Upperbound is %d\n",lower_bound,upper_bound);
			//Reading forward
			RecordId equal_rid;
			int equal_key = lower_bound + 1;
			//Reading forward until equal_key >= upperbound
			while (equal_key < upper_bound && cursor.pid != RC_END_OF_TREE)	{
				tree.readForward(cursor,equal_key,equal_rid);
				if (equal_key < upper_bound)	{
					candidate.push_back(equal_rid);
				}
			}
		}

		//If no conditions exist, skip to non-indexing section
		if (eq == false && ra == false)
			goto normalcase;

		//If there were equality or range conditions, check all the candidates		
		///Then print out the matching tuples
		for (int i = 0; i < candidate.size(); i++)	{
			//fprintf(stderr, "Printing candidates\n");

			if ((rc = rf.read(candidate[i], key, value)) < 0) {
				fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
				goto exit_select;
			}

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
					if (diff != 0) goto skip;
					break;
				case SelCond::NE:
					if (diff == 0)goto skip;
					break;
				case SelCond::GT:
					if (diff <= 0) goto skip;
					break;
				case SelCond::LT:
					if (diff >= 0) goto skip;
					break;
				case SelCond::GE:
					if (diff < 0) goto skip;
					break;
				case SelCond::LE:
					if (diff > 0) goto skip;
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
			}

skip:
			continue;
		}
	}
		
		
	
	
	
	//Normal case
	else {
	normalcase:
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
	//Constructing the RecordFile
	RecordFile record(table + ".tbl", 'w');

	//Opening the file for reading
	FILE * fd;
	fd = fopen(loadfile.c_str(), "r");

	BTreeIndex tree;

	//To allow indexing, need to construct the tree
	if (index == true)	{
		tree.open(table + ".idx", 'w');
		//indexing = true;
	}


	//If invalid, return error
	if (fd == NULL) {
		perror("An error has occurred");
	}
	//Otherwise
	else {
		//Allocating a buffer of 200 (for now)
		char buffer[256];
		//While not end of file...
		int i = 0;
		while (!feof(fd)){
			//Clear the buffer
			for (int j = 0; j < 256; ++j)	{
				buffer[j] = 0;
			}

			//fprintf(stderr,"%d\n", i++);
			fgets(buffer,256,fd);	//Copy one line to buffer
			
			int key;	//Declare key, value, and rid as return parameters
			std::string value;
			RecordId rid;
			
			//parseLoadLine which will provide key, value
			if (parseLoadLine(string(buffer),key,value) == 0)	{
				//Then append
				record.append(key,value,rid);

				//If we enabled indexing
				if (index == true)	{
					tree.insert(key,rid);
				}
			}
		}		
	}

	if (index == true)	{
		tree.close();
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
