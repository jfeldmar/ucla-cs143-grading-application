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
#include <istream>
#include <iomanip>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"
#ifndef _WIN32
#include <sys/time.h>
#endif
#include <time.h>

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);
#ifdef _WIN32
struct timeval {
             time_t  tv_sec;     /* seconds */
             clock_t tv_usec;    /* microseconds */
         };
struct timespec {
               time_t   tv_sec;        /* seconds */
               long     tv_nsec;       /* nanoseconds */
       };
extern int gettimeofday(struct timeval *tv, void *);
// CLOCK_REALTIME
extern int clock_gettime(int, struct timespec *tp);
#endif


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}
void SqlEngine::printTuple(int key,string value,int attr,int count)
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
		case 4:
			fprintf(stdout, "%d\n", count);
			break;
		}
}
RC SqlEngine::selectWithIndex(RecordFile& rf,BTreeIndex& bt,const vector<SelCond>& cond,int attr)
{
	bool	skip=false,done=false;
	RecordId   rid;  // record cursor for table scanning
	RC     rc=0;
	int    key=0,findkey,keyg=0,keyl=0;
	string value;
	int    count=0;
	int    diff;
	IndexCursor	ic;

	// check the conditions on the tuple
	/* condition cases:
	key > x == findkey=x
	key < x -- findkey=1
	key > x and key <  y -- findkey=x
	key > x and key <> y -- findkey=x
	key < x and key <> y -- findkey=1
	key = x and key = y  -- findkey=x, findkey=y
	*/
	for (unsigned i = 0; i < cond.size(); i++)
	{
		switch (cond[i].comp)
		{
		case SelCond::EQ:
			key = atoi(cond[i].value);
			break;
		case SelCond::GT:
		case SelCond::GE:
			keyg = atoi(cond[i].value);
			break;
		case SelCond::LT:
		case SelCond::LE:
			keyl = atoi(cond[i].value);
			break;
		} // end switch condition
	} // end for check cond
	if (keyl && keyg)
		 findkey=keyg;
	else if (keyg)
		 findkey=keyg;
	else if (keyl)
		 findkey=1;
//fprintf(stdout, "keyg %d keyl %d key %d findkey %d size %d\n",keyg,keyl,key,findkey,cond.size());

	if (key)
	{
		done=true; // any equal condition overrides everything else
		for (unsigned i = 0; i < cond.size(); i++){
			switch (cond[i].comp)
			{
			 case SelCond::EQ:
				 findkey=atoi(cond[i].value);
				 rc = bt.locate(findkey,ic);
				 done=false;
				 break;
			 default:
				 break;
			}
			while(!done)
			{
				rc=bt.readForward(ic,key,rid);
				diff = key - findkey;
				switch (cond[i].comp)
				{
				 case SelCond::EQ:
					if (diff == 0)
					{
					 if ((rc = rf.read(rid, key, value)) < 0) {
						fprintf(stderr, "selectAll Error: while reading a tuple %d %d\n", rid.pid,rid.sid);
						return rc;
					 }
					 printTuple(key,value,attr,count);
					}
					else if (diff > 0) 
					 done=true;
				 break;
				 default:
				 break;
				} // end switch
			}; // end while
		} // endfor
		return rc;
	} // endif EQ key

	rc = bt.locate(findkey,ic);
	done=false;
	do
	{
		rc=bt.readForward(ic,key,rid);

     for (unsigned i = 0; i < cond.size(); i++)
	 {
		findkey=atoi(cond[i].value);
		diff = key - findkey;

		switch (cond[i].comp)
		{
		  case SelCond::NE:
			if (diff == 0) skip=true;
			break;
		  case SelCond::GT:
			if (diff <= 0) skip=true;
			break;
		  case SelCond::LT:
			if (diff >= 0) skip=true;
			if (diff > 0) done=true;
			break;
		  case SelCond::GE:
			if (diff < 0) skip=true;
			break;
		  case SelCond::LE:
			if (diff > 0) skip=true;
			if (diff > 0) done=true;
			break;
		} // end switch condition
	 } // end for i

	 if (!skip)
	 {
		count++;
		skip=false;
		// print the tuple 
		if ((rc = rf.read(rid, key, value)) < 0) {
			fprintf(stderr, "selectAll Error: while reading a tuple %d %d\n", rid.pid,rid.sid);
			return rc;
		 }
		printTuple(key,value,attr,count);
	 } // endif not skip
	} while(!done);
	return rc;
}
RC SqlEngine::selectAll(RecordFile& rf,const vector<SelCond>& cond,int attr)
{
	  RecordId   rid;  // record cursor for table scanning
	  RC     rc;
	  int    key;     
	  string value;
	  int    count;
	  int    diff;

	// scan the table file from the beginning
  rid.pid = rid.sid = 0;
  count = 0;
  while (rid < rf.endRid()) {
    // read the tuple
    if ((rc = rf.read(rid, key, value)) < 0) {
      fprintf(stderr, "selectAll Error: while reading a tuple %d %d\n", rid.pid,rid.sid);
      return rc;
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
      } // end switch
    } // endfor of condition

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
  } // end while

  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  return rc;
}
RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
	RecordFile rf;   // RecordFile containing the table
	RC	rc;
	bool	haveIdx=false;

	// open the table file
	if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	}
  	BTreeIndex bb;
	rc = bb.open(table+".idx",'r');
	if (rc == 0) haveIdx=true;
	// setup timing statistics
	struct timeval tm,tmend;
	gettimeofday(&tm,NULL);

	// if no conditions pass to all tuple search
	if (cond.size() == 0) haveIdx=false;
	else if (cond.size() == 1) {
		// if request is not equal then all tuple search
		switch (cond[0].comp)
		{
		 case SelCond::NE:
			haveIdx=false;
			break;
		 default:
			break;
		} // end switch condition
	}
	// the request is nonkey or value based then all tuple search
	for (unsigned i = 0; i < cond.size(); i++) {
		if (cond[i].attr == 2) haveIdx=false;
	} // end for check

	if (haveIdx)
		selectWithIndex(rf,bb,cond,attr);
	else
		selectAll(rf,cond,attr);

	// close the table file and return
	gettimeofday(&tmend,NULL);
	time_t tt = tmend.tv_usec - tm.tv_usec;
//	fprintf(stdout, "Total time %d us\n", tt);
	rf.close();
	if (haveIdx) bb.close();
	return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	// open loadfile if not exit return error
	const int linesize=100;
	std::ifstream infile;
	infile.open(loadfile.c_str());
	if (infile.fail())
	{
		fprintf(stdout, "Unable to open file: %s\n",loadfile.c_str());
		return RC_FILE_OPEN_FAILED;
	}
	// if exist then
	string datafile = table + ".tbl";
	string indexfile = table + ".idx";
	// open new database RecordFile(const std::string& filename, char mode);
	RecordFile outfile = RecordFile(datafile, 'w');
	string line;		// string version of raw data
	char nline[linesize];	// raw line of data
	int key;		// returned key value
	string value;		// returned value string
	RC retcode;		// internal return code
	RecordId rid;		// returned record
	BTreeIndex bb;
	if (index) bb.open(indexfile,'w');
	do
	{
		// read line
		infile.getline(nline,linesize);
		if (!infile.eof())
		{
		line.assign(nline);
		// parse line parseLoadLine(const string& line, int& key, string& value)
		retcode = parseLoadLine( line, key, value);
		//printf("%s\nkey %d %s\n",nline,key,value.c_str());
		if (retcode != 0) return retcode;
		// append line RC append(int key, const std::string& value, RecordId& rid);
		retcode = outfile.append(key, value, rid);
		if (retcode != 0) return retcode;
		if (index) retcode = bb.insert(key,rid);
		
		}
	} while(!infile.eof());	// end while
	// if end of loadfile, RC close();
	outfile.close();
	infile.close();
	if (index) bb.close();
	
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
