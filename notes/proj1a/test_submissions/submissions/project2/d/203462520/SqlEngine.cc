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
#include "BTreeNode.h"
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
  RecordId   rid, endrid;  // record cursor for table scanning
	BTreeIndex	bti; // BTreeIndex object

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
	endrid = rf.endRid();
	//fprintf(stderr, "endrid.pid: %d endrid.sid: %d\n", endrid.pid, endrid.sid);
	
	/* open the index if it exists; if it doesn't, skip
	 * to original brute-force method
	 */
	if ((rc = bti.open(table + ".idx", 'r')) >= 0) {

			// for doing locate searches in the tree
			IndexCursor cursor, tmpcursor;
			cursor.pid = tmpcursor.pid = 3; // first leaf node (always)
			cursor.eid = tmpcursor.eid = 0; // first eid (always)

			endrid.pid = 0; // don't stop until you hit the end
			endrid.sid = 0; // of the index file
			
			int searchKey = 0;
			int searchMin, searchMax;
			bool lowerbound, upperbound, no_strings;
			lowerbound = upperbound = false;
			no_strings = true;
			int akey;
	
		/* check each condition on the tuple. 
		 * use the conditions to build a starting and ending values
		 * for read forward to test against
		 */
		for (unsigned i = 0; i < cond.size(); i++) {
			tmpcursor.pid = 3;
			tmpcursor.eid = 0;
			// compute the difference between the tuple value and the condition value
			switch (cond[i].attr) {
				case 1:
					// key condition
					searchKey = atoi(cond[i].value);
					break;
				case 2:
					// value condition, just skip to next cond
					no_strings = false;
				  continue;
				}
				/* using the index as guide,  adjust rid or endrid 
				 * this allows us to search less of the table. this
				 * method would not work properly with OR conditions, since
				 * you could have "key = 5 OR key = 7". However, "key = 5
				 * AND key = 7" should obviously return nothing.
				 */
				switch (cond[i].comp) {
				case SelCond::EQ:
					if (
							((!lowerbound) && (!upperbound)) ||
							((searchMin < searchKey) && (searchMax > searchKey))
							 ) {
								
								if ((rc = bti.locate(searchKey, tmpcursor)) < 0) {
									goto exit_select;
								} else {
								/* the value was found and the TREE location is stored in 
								 * cursor, so we need to read the record from the tree
								 * will limit the search accordingly.
								 */

								/* cursor will hold the "starting point" in the indices, and
								 * endrid will hold the ending index.
								 * adjust endrid to be just *past* the cursor value; since this is
								 * an equality search, there should be no future compatible results
								 */
								endrid.pid = cursor.pid = tmpcursor.pid;
								cursor.eid = tmpcursor.eid;
								endrid.sid = cursor.eid + 1;
								searchMin = searchMax = searchKey;
								lowerbound = upperbound = true;
								}
					} else {
							/* there was an additional equality condition that was not the same
							 * as the last one -- this is a guaranteed "no match" situation
							 */
									goto out; 
					}

					/* the readforward section will test all the conditions again,
					 * including any extra (conflicting) EQUALITY conditions.
					 * hence, if we found an answer to an EQ search, we can return
					 * one pid/eid and be confident that the answer will be here, or
					 * it will be disqualified by the additional conditionals.
					 *
					 * this is the basic format of all these tests here -- shrink 
					 * the potential tuples by creating a range of potential indices
					 */
					break;
				case SelCond::NE:
					break;
				case SelCond::GT:
					/* a GT selection sets the start point as the first record
					 * which is greater than this entry. it does not set the 
					 * endrid at all.
					 */
					/* if there is an LT upper bound already, and the new GT 
					 * lower bound is HIGHER than the LTUB, FAIL.
					 * e.g., we already have the rule key < 10, and now we want
					 * to add a rule key > 11
					 */
					if ((upperbound) && (searchKey > searchMax)) {
									goto out;
					}
					if ((!lowerbound) ||
							(searchKey > searchMin)) {
						if ((rc = bti.locate(searchKey, tmpcursor)) < 0) {
									goto exit_select;
						} 
						searchMin = searchKey;
						lowerbound = true;
						cursor.pid = tmpcursor.pid;
						cursor.eid = tmpcursor.eid;
					}
					break;
				case SelCond::LT:
					/* an LT selection sets the ENDING point -- the point 
					 * at which further records will not match
					 */
					if ((lowerbound) && (searchKey < searchMin)) {
									goto out;
					}
					if ((!upperbound) ||
							(searchKey < searchMin)) {
						if ((rc = bti.locate(searchKey, tmpcursor)) < 0) {
									goto exit_select;
						} 
						searchMax = searchKey;
						upperbound = true;
						endrid.pid = tmpcursor.pid;
						endrid.sid = tmpcursor.eid;
					}
					break;
				case SelCond::GE:
					if ((upperbound) && (searchKey > searchMax)) {
									goto out;
					}
					if ((!lowerbound) ||
							(searchKey > searchMin)) {
						if ((rc = bti.locate(searchKey - 1, tmpcursor)) < 0) {
										goto exit_select;
						} 
						searchMin = searchKey;
						lowerbound = true;
						cursor.pid = tmpcursor.pid;
						cursor.eid = tmpcursor.eid;
					}
					break;
				case SelCond::LE:
					if ((lowerbound) && (searchKey < searchMin)) {
									goto out;
					}
					if ((!upperbound) ||
							(searchKey < searchMin)) {
						if ((rc = bti.locate(searchKey + 1, tmpcursor)) < 0) {
									goto exit_select;
						} 
						searchMax = searchKey;
						upperbound = true;
						endrid.pid = tmpcursor.pid;
						endrid.sid = tmpcursor.eid;
					}
					break;
				}
			} 
			
			/* we have now pared down the range of indices to read according to the 
			 * conjunction of the search terms. now that this is done, all we have left
			 * is to read them.
			 *
			 * if the select is a COUNT(*), we need to simply add the number of valid 
			 * entries in the range of indices.
			 *
			 * if the select is a select * or something else, we need to print the 
			 * resulting tuples.
			 */
			read_forward:
			
			while ((rc = bti.readForward(cursor, akey, rid)) >= 0 ) {
	    /* our first run was to reduce the range of indices to follow
			 * now we still need to test the AND of all conditions to cover
			 * for cases like multiple EQ conditions. These aren't all 
			 * necessary now, but I do need them for value tests, so I'll just
			 * leave them all here for the time being.
			 */
				
				// if this is a COUNT(*), and we don't need to compare strings
				// we can skip reading/writing and just count from the leaves
				if (attr == 4 && no_strings) {
								count++;
								goto next_index_tuple;
				}
		
		    // otherwise we have to read the tuple and examine
		    if ((rc = rf.read(rid, key, value)) < 0) {
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
					if (diff != 0) goto next_index_tuple;
					break;
	      case SelCond::NE:
					if (diff == 0) goto next_index_tuple;
					break;
	      case SelCond::GT:
					if (diff <= 0) goto next_index_tuple;
					break;
	      case SelCond::LT:
					if (diff >= 0) goto next_index_tuple;
					break;
	      case SelCond::GE:
					if (diff < 0) goto next_index_tuple;
					break;
	      case SelCond::LE:
					if (diff > 0) goto next_index_tuple;
					break;
	      }
	    }

		    // the condition is met for the tuple (thanks to us)
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
	
		    /* quit if we've hit the range endpoint
				 */
			next_index_tuple:
				if (((endrid.pid != 0) && (endrid.sid != 0)) && // dont stop til end
						(cursor.pid == endrid.pid) && (cursor.eid == endrid.sid)) {
								goto out;
				}
	  	}
			goto out;

	} else {

		/* there was no index, so we need to go through the whole 
		 * table!
		 */

	  while (rid < endrid) {
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
  
	out:
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
	/* Project 2A changes by Peter Peterson
	 * code largely taken from C++ tutorial (and elsewhere):
	 * http://www.cplusplus.com/doc/tutorial/files.html */
	ifstream infile (loadfile.c_str());
	if (infile.is_open())
	{
					RC rc;
					BTreeIndex bti;
					RecordId thisrid;
					RecordFile handle;
					handle.open(table + ".tbl", 'w');
					if (index) {
									if ((rc = bti.open(table + ".idx", 'w')) < 0)
													return rc;
					}
					string thisline;
					string thisvalue;
					int thiskey;
					while (!infile.eof())
					{
									// get record line
									getline(infile,thisline);

									// parse record to make sure its valid
									if (parseLoadLine(thisline, thiskey, thisvalue) == 0)
									
									{ // append input
											if (handle.append(thiskey, thisvalue, thisrid) != 0)
											{
  												fprintf(stderr, 
														"Error: unable to insert record key='%d' value='%s'.\n", thiskey, thisvalue.c_str());
											} /* implicit else */
													//fprintf(stderr, "Inserted record key='%d' value='%s'... ", thiskey, thisvalue.c_str());
													if (index) {
														if ((rc = bti.insert(thiskey, thisrid)) < 0) { 
																		//fprintf(stderr, "and index key... ");
																		return rc;
																		}
													}
													//fprintf(stderr, "\n");
									} else { // bad line, skip
													// fprintf(stderr, "Skipping line: '%s'\n", thisline.c_str());
													continue;
									}
					}
					bti.PrintTree();
					infile.close(); // close input file
					if (index) {
									if ((rc = bti.close()) < 0) return rc;
					}
					handle.close(); // close db table
	} else {
	  fprintf(stderr, "Error: unable to open file '%s'\n", table.c_str());
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

