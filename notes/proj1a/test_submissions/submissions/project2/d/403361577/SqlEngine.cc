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

  // initialize btree
  BTreeIndex bTree;
  // flag to indicate whether to use btree or not
  int isIndex = 0;
  // get the condition array size
  int condSize = cond.size();
  // initialize priority for each condition operator
	int prioritizer[condSize];
	// current position of priority array
	int pos = 0;
  // flag to indicate whether comparing value or not
  int cprValue = 0;

  count = 0;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  // check to see if we're comparing key
	for (int i = 0; i < condSize; i++) {
    if (cond[i].attr == 2)
        cprValue = 1;
  }

  // we will not be using btree if there are no condition operator or comparing value attr.
  if (condSize == 0 || cprValue == 1) {
      // indicate we're using normal operation for search
      isIndex = 0;
  }
  else {
      // indicate we're using index for search
      isIndex = 1;
    
      // open the index file
      // use normal operation if btree failed to open
      if ((rc = bTree.open(table, 'r')) != 0) {
          isIndex = 0;
      }
    
      // prioritize '=' to 1st priority
      for (int i = 0; i < condSize; i++) {
        if (cond[i].comp == SelCond::EQ) {
          prioritizer[pos] = i;
          pos++;
        }
      }

      // prioritize '>' and '>=' to 2nd priority
      for (int i = 0; i < condSize; i++) {
        if (cond[i].attr == 1 && (cond[i].comp == SelCond::GE || cond[i].comp == SelCond::GT)) {
          prioritizer[pos] = i;
          pos++;
        }
      }

      // prioritize '<' and '<=' to 3rd priority
      for (int i = 0; i < condSize; i++) {
        if (cond[i].attr == 1 && (cond[i].comp == SelCond::LE || cond[i].comp == SelCond::LT)) {
          prioritizer[pos] = i;
          pos++;
        }
      }
  }


  // BTREE INDEX OPERATION
  if (isIndex == 1) {
  
		IndexCursor cursor;
		IndexCursor lastLeaf;
		int firstKey;
		int notFound = 0;
		int isEq = 0;
    int printCount = 0;
    int MAX_PRINT_COUNT = 0;
    int isGt = 0;
    int gtKey = -1;
    int isLt = 0;
    int ltKey = -1;
    int temp_key = 0;

    // set starting key value
		if(cond[prioritizer[0]].comp == SelCond::LE || cond[prioritizer[0]].comp == SelCond::LT)
        firstKey = bTree.minKey();
		else
        firstKey = atoi(cond[prioritizer[0]].value);

		// output error if failed to locate the starting key
		if (firstKey < 0 || bTree.locate(firstKey, cursor) != 0) {
        //cout << "FAILED TO LOCATE FIRST KEY: " << firstKey << endl;
        goto exit_select;
		}

    // set flags to indicate which comparison operator we're dealing with
    for (int i = 0; i < condSize; i++) {
        switch (cond[prioritizer[i]].comp) {
          case SelCond::EQ:
            isEq = 1;
            break;
          case SelCond::GT:
          case SelCond::GE:
            isGt = 1;
            // set greater key to largest possible to improve performance
            temp_key = atoi(cond[prioritizer[i]].value);
            if (temp_key > gtKey)
                gtKey = temp_key;
            break;
          case SelCond::LT:
          case SelCond::LE:
            isLt = 1;
            // ensure ltKey gets set the first time
            if (ltKey == -1)
                ltKey = atoi(cond[prioritizer[i]].value);
            // set lesser key to smallest possible to improve performance
            temp_key = atoi(cond[prioritizer[i]].value);
            if (temp_key < ltKey)
                ltKey = temp_key;
            break;
        }
    }

    // get the maximum allowed range to search
    if (isGt == 1 && isLt == 1) {
        MAX_PRINT_COUNT = ltKey - gtKey + 1;
    }
    if (isGt == 0 && isLt == 1) {
        MAX_PRINT_COUNT = ltKey + 1;
    }

    // set last leaf page to check
    if (ltKey != -1) {
        bTree.locate(ltKey, lastLeaf);
    } else {
        lastLeaf.pid = 0;	
        lastLeaf.eid = 0;
    }

		while (bTree.readForward(cursor, key, rid) == 0 && 
            (notFound < MAX_PRINT_COUNT || MAX_PRINT_COUNT == 0)) {
//cout << "KEY IS = " << key << endl;
//cout << "rid.pid IS = " << rid.pid << endl;
//cout << "rid.sid IS = " << rid.sid << endl;
//cout << "cursor.eid IS = " << cursor.eid << endl;
//cout << "cursor.pid IS = " << cursor.pid << endl;
        
      // read the tuple
      if ((rc = rf.read(rid, key, value)) != 0) {
        fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
        goto exit_select;
      }

			for (int i = 0; i < condSize; i++) {
          //check conditions on tuple
          switch(cond[prioritizer[i]].attr){
            case 1:
              diff = key-atoi(cond[prioritizer[i]].value);
              break;
            case 2:
              diff = strcmp(value.c_str(), cond[i].value);
              break;
          }

          //skip the tuple if any condition is not met
          switch (cond[prioritizer[i]].comp) {
            case SelCond::EQ:
              if(diff != 0) {
                goto end_loop;
              }
              break;
            case SelCond::GT:
              if(diff <= 0) {
                goto end_loop;
              }
              break;
            case SelCond::GE:
              if(diff < 0) {
                goto end_loop;
              }
              break;
            case SelCond::LT:
              if(diff >= 0) {
                goto end_loop;
              }
              break;
            case SelCond::LE:
              if(diff > 0) {
                goto end_loop;
              }
              break;
          }
			}

			//the condition is met for the tuple
			//increase matching tuple counter
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

      // update print count
      printCount++;

      /*
      // in case we finished printing alrdy, break to improve performance
			if (MAX_PRINT_COUNT > 0 && printCount > MAX_PRINT_COUNT)
          break;
			*/
			
			// on equals, juss print 1
			if (isEq == 1)
          break;
			
      end_loop:
      notFound++;
      if (cursor.pid == lastLeaf.pid && cursor.eid == lastLeaf.eid)
          break;
		}
    
    // print matching tuple count if "select count(*)"
    if (attr == 4) {
      fprintf(stdout, "%d\n", count);
    }
		
		rc = 0;
		
  } 
  // NORMAL OPERATION
  else {

      // scan the table file from the beginning
      rid.pid = rid.sid = 0;
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
  // close the btree if it was opened
  //if (isIndex == 1) {
      bTree.close();
  //}
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */
  
  // create record file object
  RecordFile rf;
  // key and value from each line
  int key;
  string value;
  // max size of each line in loadfile
  int MAX_LINE_LENGTH = 200;
  // read line from loadfile
  char rdline[MAX_LINE_LENGTH];
  // table filename
  string tbf = table + ".tbl";
  // make an array pointer to point to the loadfile string
  const char *loadfile_ptr = loadfile.c_str();
  // get the load file from parameter
  FILE *load_file = fopen(loadfile_ptr, "r");
  // error flag
  bool errorFlag = 0;
  // initialize btree
  BTreeIndex bTree;

  // create or write to the table with no error
  if (load_file != NULL && rf.open(tbf, 'w') == 0) {
      // create btree when index is set to true
      if (index == true) {
          if (bTree.open(table, 'w') != 0)
              errorFlag = 1;
      }
      // read from the loadfile
      while (fgets(rdline, MAX_LINE_LENGTH, load_file) != NULL) {
          // parse the load line
          if (parseLoadLine(rdline, key, value) == 0) {
              // get the rid
              RecordId rid = rf.endRid();
              // append the record to rid
              if (rf.append(key, value, rid) == 0) {
                  if (index == true) {
                      // insert the key into btree on index == true
                      if (bTree.insert(key, rid) != 0)
                          errorFlag = 1;
                  }
              } else
                  errorFlag = 1;
          } else {
              errorFlag = 1;
          }
      }
      // close the btree
      if (index == true)
          bTree.close();
      // close the record file
      rf.close();
      // close the load file
      fclose(load_file);
  } else {
      errorFlag = 1;
  }


  if (errorFlag) {
    cout << "ERROR LOADING TABLE FROM FILE" << endl;
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
