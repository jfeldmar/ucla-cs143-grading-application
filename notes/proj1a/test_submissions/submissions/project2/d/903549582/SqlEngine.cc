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

int SqlEngine::getBestCond(const std::vector<SelCond>& cond){
//return an index of the best cond ( =, <, >,..)
	for(int i=0; i<cond.size(); i++){
		if( cond[i].attr == 2 ) continue; 
		if( cond[i].comp == SelCond::EQ ) return i;	
	}
	for(int i=0; i<cond.size(); i++){
		if( cond[i].comp == SelCond::LT ||
				cond[i].comp == SelCond::LE ||
				cond[i].comp == SelCond::GT ||
				cond[i].comp == SelCond::GE		) return i;	
	}
	return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning
  BTreeIndex bindex;

  RC     rc;
  int    key;     
  string value;
  int    count =0;
  int    diff;

	bool index;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

	if( cond.size() == 0 ){
		return selectNoIndex(attr, table, cond); //do select without index
	}
	
  // open the index file
  if ((rc = bindex.open(table + ".idx", 'r')) < 0) {
    //fprintf(stderr, "Error: index %s does not exist\n", table.c_str());
    //return rc;
    index = false;
		return selectNoIndex(attr, table, cond); //do select without index
  }
  else{
  	index = true;	
  }

	int condKey;
	int currentKey;
	IndexCursor currentCursor; // pid, eid
	bool done = false;

	int bestIndex = getBestCond(cond);
	if(SH_DEBUG){
		printf("index of the best cond: %d\n",bestIndex);
	}

	SelCond bestCond = cond[bestIndex];
	if(bestCond.comp == SelCond::NE){
		return selectNoIndex(attr, table, cond); //do select without index
	}
	else if(bestCond.attr == 2) //not key attribute
		return selectNoIndex(attr, table, cond); //do select without index

	condKey = atoi(bestCond.value);
	
	if(SH_DEBUG){
		printf("BestCond: CondKey:%d\n", condKey);
	}

	if( bestCond.comp == SelCond::LT || bestCond.comp == SelCond::LE )
		currentKey = 0; //search from the beginning..
	else
		currentKey = condKey;
		
	rc = bindex.locate(currentKey, currentCursor);
	if(rc != 0){
		if(SH_DEBUG)
			printf("BTreeIndex locate error\n");
		return rc; //not exist..
	}

	rc = bindex.readForward(currentCursor, key, rid);
	//printf("key:%d rid:[%d,%d]\n", key, rid.pid, rid.sid);
	if( rc == RC_INVALID_CURSOR){
		if(SH_DEBUG)
			printf("BTreeIndex readForward: read until the end...\n");
		done = true; //nothing to do..	
	}
	if( rc != RC_INVALID_CURSOR && rc != 0){
		if(SH_DEBUG)
			printf("BTreeIndex readForward error\n");
		return rc; //not exist..
	}

	rc = rf.read(rid, key, value);
	if(rc != 0){
		if(SH_DEBUG)
			printf("RecordFile read error\n");
		return rc; //not exist..
	}
	
	if(SH_DEBUG)
		printf("Data Record Key:%d, Value:%s\n", key, value.c_str());

	int cindex;
	int i=-1;
	cindex = bestIndex;
	diff = key - currentKey;

	if( bestCond.comp == SelCond::LT || bestCond.comp == SelCond::LE )
		diff = key - condKey;

	bool first = true;
	if(SH_DEBUG) printf("bestIndex:%d diff=%d\n", cindex, diff);
goto condition;			
					
	while( !done ){
		
/*		
			for(i=0; i<cond.size(); i++){
			cindex = i;
			condKey = atoi(cond[cindex].value);
				printf("size:%d i=%d\n", cond.size(), cindex);

			if(SH_DEBUG)
				printf("CondKey:%d\n", condKey);
*/

			rc = bindex.readForward(currentCursor, key, rid);
			if(SH_DEBUG) printf("key:%d rid:[%d,%d]\n", key, rid.pid, rid.sid);
			if( rc == RC_INVALID_CURSOR){
				if(SH_DEBUG)
					printf("BTreeIndex readForward: read until the end...\n");
				done = true; //nothing to do..	
			}
			if( rc != RC_INVALID_CURSOR && rc != 0){
				if(SH_DEBUG)
					printf("BTreeIndex readForward error\n");
				return rc; //not exist..
			}
		
			rc = rf.read(rid, key, value);
			if(rc != 0){
				if(SH_DEBUG)
					printf("RecordFile read error\n");
				return rc; //not exist..
			}
		
		/*
		*/
		if(SH_DEBUG)
			printf("Data Record Key:%d, Value:%s\n", key, value.c_str());

			for(i=0; i<cond.size(); i++){
			cindex = i;
			condKey = atoi(cond[cindex].value);
			
			if(SH_DEBUG)	printf("size:%d i=%d\n", cond.size(), cindex);

			if(SH_DEBUG)
				printf("CondKey:%d\n", condKey);


		switch (cond[cindex].attr) {
      case 1:
				diff = key - atoi(cond[cindex].value);
				break;
      case 2:
				diff = strcmp(value.c_str(), cond[cindex].value);
				break;
   	}
      
condition:
			if(SH_DEBUG) printf("i=%d\n", cindex);
			switch (cond[cindex].comp){
				case SelCond::EQ:
					//if(key == condKey){
					if(diff == 0){
						done = true;
					}
					else{
						goto next_tuple;
					}
					break;
				case SelCond::GT:
					if(SH_DEBUG) printf("GT, diff:%d\n", diff);
					//if(key <= condKey){
					if(diff <= 0){
						goto next_tuple;
					}
					break;
				case SelCond::LT:
					if(SH_DEBUG) printf("LT, diff:%d\n", diff);
					//if(key >= condKey){
					if(diff >= 0){
						goto next_tuple;
					}
					break;
				case SelCond::GE:
					if(SH_DEBUG) printf("GE, diff:%d\n", diff);
					//if(key < condKey){
					if(diff < 0){
						goto next_tuple;
					}
					break;
				case SelCond::LE:
					if(SH_DEBUG) printf("LE, diff:%d\n", diff);
					//if(key > condKey){
					if(diff > 0){
						goto next_tuple;
					}
					break;
				case SelCond::NE:
					if(SH_DEBUG) printf("NE, diff:%d\n", diff);
					//if(key == condKey){
					if(diff == 0){
						goto next_tuple;
					}
					break;
			}//switch
		if(SH_DEBUG) printf("after switch i=%d\n", cindex);
	}//for

		count++;
		if(SH_DEBUG) printf("after for i=%d\n", cindex);
		//print results
		switch(attr){
			case 1: 
				fprintf(stdout, "%d\n", key);
				break;
			case 2:	
				fprintf(stdout, "%s\n", value.c_str());
				break;
			case 3:
				fprintf(stdout, "%d '%s'\n", key, value.c_str());
				break;
		}

		
next_tuple:

		if(SH_DEBUG) printf("get next\n");
		if( cond[cindex].comp == SelCond::LT || cond[cindex].comp == SelCond::LE ){
			if( key > condKey ) done = true;
		}
		
}// while(done)

	if(attr == 4){
		fprintf(stdout, "%d\n", count);
	}

  rf.close();
  bindex.close();
  return rc;
}

RC SqlEngine::selectNoIndex(int attr, const std::string& table, const std::vector<SelCond>& cond){
	
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
  //
	string line;
	ifstream lf;
	
	string value;
	int key;
	
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning
	BTreeIndex bindex;

	char* file_name = new char[loadfile.length()];
	memcpy(file_name, loadfile.data(), loadfile.length());

	lf.open(file_name);
	rf.open( table+".tbl", 'w');
	
	if(index == true)
		bindex.open(table+".idx", 'w');
	
	while(true){
	  getline(lf, line);
	  if(lf.eof()) break;
	  parseLoadLine(line, key, value);
	  //open the file to record, mode is write
	  rf.append(key, value, rid);

		if(index){
			if(SH_DEBUG)
				printf("re rid:[%d,%d]\n", rid.pid, rid.sid);
	 		bindex.insert(key, rid);
		}
	 }
			
	bindex.close();
	rf.close();
	lf.close();	
		
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
