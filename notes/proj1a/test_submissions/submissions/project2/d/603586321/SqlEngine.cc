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
	int bt_error;
	BTreeIndex bt;

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

	// Check if the index file exists
	bool used_index = false;
	bt_error = bt.open(table+".idx", 'r');
	if(bt_error == 0) {
  		// Find the min and max keys to use
  		int min_key = INT_MIN;
  		int max_key = INT_MAX;
		for (unsigned i = 0; i < cond.size(); i++) {
			// Only check conditions on key
			if(cond[i].attr == 1) {
				// skip the tuple if any condition is not met
				if(cond[i].comp == SelCond::EQ) {
					min_key = atoi(cond[i].value);
					max_key = atoi(cond[i].value);
					break;
				} else if(cond[i].comp == SelCond::GT) {
					min_key = max(atoi(cond[i].value)+1, min_key);
				} else if(cond[i].comp == SelCond::LT) {
					max_key = min(atoi(cond[i].value)-1, max_key);
				} else if(cond[i].comp == SelCond::GE) {
					min_key = max(atoi(cond[i].value), min_key);
				} else if(cond[i].comp == SelCond::LE) {
					max_key = min(atoi(cond[i].value), max_key);
				}
			}
		}
		// Check if index file should be used or not
		if(min_key != INT_MIN || max_key != INT_MAX) {
			// Get cursor
			IndexCursor cursor;
			bt.locate(min_key, cursor);

			count = 0;
			while(cursor.eid != -1 && cursor.pid != -1) {
				bt.readForward(cursor, key, rid);
				if(key > max_key) {
					break;
				}

				// check the conditions on key
				for (unsigned i = 0; i < cond.size(); i++) {
					if(cond[i].attr == 1) {
						diff = key - atoi(cond[i].value);

						// skip the tuple if any condition is not met
						switch (cond[i].comp) {
							case SelCond::EQ:
								if (diff != 0) goto next_index;
								break;
							case SelCond::NE:
								if (diff == 0) goto next_index;
								break;
							case SelCond::GT:
								if (diff <= 0) goto next_index;
								break;
							case SelCond::LT:
								if (diff >= 0) goto next_index;
								break;
							case SelCond::GE:
								if (diff < 0) goto next_index;
								break;
							case SelCond::LE:
								if (diff > 0) goto next_index;
								break;
						}
					}
				}
				if ((rc = rf.read(rid, key, value)) < 0) {
					fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
					goto exit_select;
				}
				// check the conditions on value
				for (unsigned i = 0; i < cond.size(); i++) {
					if(cond[i].attr == 2) {
						diff = strcmp(value.c_str(), cond[i].value);

						// skip the tuple if any condition is not met
						switch (cond[i].comp) {
							case SelCond::EQ:
								if (diff != 0) goto next_index;
								break;
							case SelCond::NE:
								if (diff == 0) goto next_index;
								break;
							case SelCond::GT:
								if (diff <= 0) goto next_index;
								break;
							case SelCond::LT:
								if (diff >= 0) goto next_index;
								break;
							case SelCond::GE:
								if (diff < 0) goto next_index;
								break;
							case SelCond::LE:
								if (diff > 0) goto next_index;
								break;
						}
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

				next_index:
					continue;
			}

			used_index = true;
		}

		// Close index file
		bt_error = bt.close();
		if(bt_error != 0) {
			perror ("Error closing file %b");
			return -1;
		}
	}
	if(!used_index) {
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
	FILE* fp;
	RecordFile rf;
	int rf_error, bt_error;
	const int max_entry_length = 200;
	char entry[max_entry_length];
	int key;
  	string value;
	RecordId rid;
	BTreeIndex bt;

	// Open the loadfile
	fp = fopen(loadfile.c_str(), "r");
  	if(!fp) {
		perror ("Error opening file");
		return -1;
	}

	// Open the record file to store the entries
	rf_error = rf.open(table+".tbl", 'w');
  	if(rf_error != 0) {
    		return rf_error;
  	}

	// If index = true then open the index file
	if(index) {
	  	bt_error = bt.open(table+".idx", 'w');
	  	if(bt_error != 0) {
			perror ("Error opening file");
			return -1;
	  	}
	}
	// Read the lines in loadfile and store them as entries in the record file
	while(fgets(entry, max_entry_length , fp)) {
		rf_error = parseLoadLine(entry, key, value);
		if(rf_error != 0) {
      		perror ("Error reading line");
			return -1;
		}
    	rf.append(key, value, rid);

		// If index = true then insert the key into the index file
		if(index) {
			bt.insert(key, rid);
		}
  	}
	// If index = true then close the index file
	if(index) {
		bt_error = bt.close();
  		if(bt_error != 0) {
			perror ("Error closing file");
			return -1;
		}
	}

	// Close record file
	rf_error = rf.close();
  	if(rf_error != 0) {
		return rf_error;
	}

	// Close loadfile
  	if(fclose(fp) != 0) {
		perror ("Error closing file");
		return -1;
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
