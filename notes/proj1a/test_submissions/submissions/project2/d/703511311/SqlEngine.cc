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

  BTreeIndex indexTree;

  RC     rc;
  int    key = 0;
  string value = "";
  int    count = 0;
  int    diff = 0;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  //------------------------------------------------------------------------------

//Go in if index exists
if ((rc = indexTree.open(table + ".idx", 'r')) == 0)
{
	int equal = INT_MIN; 				// check equal attribute
	int lower = INT_MIN; 				// lower bound
	int upper = INT_MIN; 				// upper bound

	int key = 0;
	int count = 0;
	IndexCursor ic;
	RecordId rid;

	// check all selection conditions
	for (int i = 0; i < cond.size(); i++)
	{
		if (cond[i].attr == 1) // numeric conditions
		{
			switch (cond[i].comp)
			{
				case SelCond::EQ:
					if (equal == INT_MIN)
						equal = atoi(cond[i].value);
					else
						goto to_normal;
					break;
				case SelCond::NE:
					goto to_normal;
					break;
				case SelCond::GT:
					if (atoi(cond[i].value) > lower)
						lower = atoi(cond[i].value) + 1;
					break;
				case SelCond::LT:
					if (atoi(cond[i].value) < upper || upper == INT_MIN)
						upper = atoi(cond[i].value)-1;
					break;
				case SelCond::GE:
					if (atoi(cond[i].value) > lower)
						lower = atoi(cond[i].value);
					break;
				case SelCond::LE:
					if (atoi(cond[i].value) < upper || upper == INT_MIN)
						upper = atoi(cond[i].value);
					break;
			}
		}
		else
		{
			goto to_normal;
		}
	}

	//printf("%d %d %d ", lower, equal, upper); cout << tValue << endl; indexTree.close(); rf.close(); return 0;

	if (equal != INT_MIN)
	{
		if (upper != INT_MIN && equal > upper) goto exit_idx;
		if (lower != INT_MIN && equal < lower) goto exit_idx;

		lower = equal;
	}

	if (lower == INT_MIN) lower = INT_MIN;

	indexTree.readFile();
	if (indexTree.locate(lower, ic) < 0) goto exit_idx;

	printf("\n");

	do
	{
		if (indexTree.readForward(ic, key, rid) < 0) goto exit_idx;
		if (attr != 4)
			if (rf.read(rid, key, value) < 0) goto exit_idx;

		if (upper != INT_MIN && key > upper) goto attr_count;

		if (equal != INT_MIN)
		{
			if (equal < key) goto exit_idx;

			if (equal == key)
			{
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
					case 4:	 // SELECT count(*)
						fprintf(stdout, "%d\n", 1);
						break;
				}

				goto exit_idx;
			}
		}
		else
		{
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
		count++;

	} while (ic.pid != -1);

	attr_count:
		if (attr == 4)
			fprintf(stdout, "%d\n", count);

	exit_idx:
		indexTree.close();
		rf.close();
		return 0;

	to_normal: // continue
		indexTree.close();
}

  //------------------------------------------------------------------------------


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

  // close the table file and return
  exit_select:
  rf.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */

	RecordFile rf;   // RecordFile containing the table
	RecordId   rid;  // record cursor for table scanning
	BTreeIndex indexTree;

	FILE * file;
	char temp[256];

	string tempTable = &table[0];
	string tempIndex = tempTable + ".idx";
	tempTable = tempTable + ".tbl";
	string tempFile = &loadfile[0];

	if (rf.open(tempTable, 'w') == 0)
	{
		file = fopen(&loadfile[0], "r");

		if (index)
		{
			indexTree.open(tempIndex, 'w');
		}

		if (file != NULL)
		{
			while(fgets(temp, 256, file) != NULL)
			{
				int key = 0;
				string value = "";

				parseLoadLine(temp, key, value);

				if (rf.append(key, value, rid) < 0)
				{
					printf("Error while loading\n");
					return 1;
				}

				if (index)
				{
					int tempError = indexTree.insert(key, rid);

					if (tempError < 0) return tempError;
				}
			}

			indexTree.writeFile();
			indexTree.close();

			printf("Successfully loaded\n");

			fclose(file);
		}
		else
		{
			printf("Could not open the file %s\n", &loadfile[0]);
			return 1;
		}
		rf.close();
	}
	else
	{
		printf("Could not open/create the file %s\n", &tempTable);
		return 1;
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
