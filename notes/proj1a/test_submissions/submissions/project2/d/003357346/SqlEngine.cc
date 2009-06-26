/*
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
#include <cassert>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

#define SIZ 1024

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);
void printRecord(int attr, int key, string value);

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


//enumerate the conditions	
enum {
	EQ,
	GT,
	GE,
	LT,
	LE
};


/* vars */
  int index = 0;
  int priority[5] = {-1, -1, -1, -1, -1}; 		//set up a priority array for different inequality
  int foundString = 0;
  string indexfile;			//set up the indexfile
  string str_val = "";
  BTreeIndex treeindex;			//Use btree index
  indexfile = table;
  indexfile.append(".idx"); 		//Append so we have table.idx

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  // scan the table file from the beginning
  rid.pid = rid.sid = 0;
  count = 0;

	int max_val = -1;
	int min_val = -1;
	int foundKey = 0;

  //Equal sign has the highest priority
  for(int i = 0;i<cond.size();i++)  {

	//if looking for string, break
	if(cond[i].attr != 1) {
		if(foundString == 1)
			goto exit_select;
		foundString = 1;	
		str_val = cond[i].value;
	}
	else {
		foundKey = 1;

	//mark down equality
	switch(cond[i].comp) {
		case SelCond::EQ: {
			//if found two different equality conditions, exit
			if(priority[EQ] != -1) {
				int comp_value = atoi(cond[priority[EQ]].value);
				if(comp_value != atoi(cond[i].value))
					goto exit_select;
			}
			//otherwise, set the equality cond
			priority[EQ] = i;
			index ++;
			break;
		}
		case SelCond::GT: {

			//fprintf(stdout, "comparing value %i with max %i\n",atoi(cond[i].value),  max_val);
			if(atoi(cond[i].value) > max_val) {
				priority[GT] = i;
				index ++;
				max_val = atoi(cond[i].value);
			}
			break;
		}

		case SelCond::GE: {
			if(atoi(cond[i].value) > max_val) {
				priority[GE] = i;
				index ++;
				max_val = atoi(cond[i].value);
			}
			break;
		}

		case SelCond::LT: {
			if(min_val == -1)
				min_val = atoi(cond[i].value);

			if(atoi(cond[i].value) <= min_val) {
				priority[LT] = i;
				index ++;
				min_val = atoi(cond[i].value);
			}
			break;
		}
		case SelCond::LE: {
			if(min_val == -1)
				min_val = atoi(cond[i].value);

			if(atoi(cond[i].value) <= min_val) {
				priority[LE] = i;
				index ++;
				min_val = atoi(cond[i].value);
			}
			break;
		}

		default:
			goto exit_select;
			break;
	}
	}
}

if(foundKey == 1 && cond.size() > 0) {
	if((rc = treeindex.open(indexfile, 'r')) >= 0)	{
		//printf("Choosing index!\n");
		goto index_search;
	}
}

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
//	if(foundString == 1 || (treeindex.open(indexfile,'r') < 0) || cond[i].comp == SelCond::NE) 	//check to see if table.idx exists
//		break;			    	//if it doens't exist, then we proceed normal search
//	else
//		goto index_search;	   	//if there is an index file then we search by index

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

	goto exit_select;
 

// look for the key using our index 
/* isearch */
index_search:

	//fprintf(stderr, "\nISEARCH\n");
	IndexCursor cursor; 			
	IndexCursor end;

	int eq,gt,ge,lt,le;
	eq = gt = ge = lt = le = -1;
	//check our priority list and ensure that it is logical
	if(priority[EQ] != -1)
		eq = atoi(cond[priority[EQ]].value); 
	if(priority[GT] != -1)
		gt = atoi(cond[priority[GT]].value); 
	if(priority[GE] != -1)
		ge = atoi(cond[priority[GE]].value); 
	if(priority[LT] != -1)
		lt = atoi(cond[priority[LT]].value); 
	if(priority[LE] != -1)
		le = atoi(cond[priority[LE]].value); 

	//fprintf(stdout, "EQ: %i GT: %i GE: %i LT: %i LE %i\n", eq, gt, ge, lt, le);
	//treeindex.printMeta();

	int greater;
	gt = (gt>=ge?gt:-1);
	ge = (ge>gt?ge:-1);
	greater = max(gt, ge);

	int less;
	le  = (le==lt?-1:le);
	less = min(lt, le);
	if(less == -1)
		less = max(lt,le);

	if(priority[EQ] != -1 && eq < 0)
		goto exit_select;
		
	//printf("Greater: %i Less: %i\n", greater, less);

	//if the following conditions are broken, nothing is returned
	if((eq != -1 && (greater != -1 && greater > eq) || (less != -1 && less < eq)) || (greater != -1 && less != -1 && greater > less)) 
		goto exit_select; 

	//if we have an equal cond, just find that key
	if(eq != -1) {
		if((rc = treeindex.locate(eq, cursor)) == -1) {
			goto exit_select;
		}

		treeindex.readForward(cursor, key, rid);
		rf.read(rid, key, value);
		if(foundString) {
			if(value == str_val)
				printRecord(attr, key, value);
		}
		else
			printRecord(attr, key, value);

		rc = 0;
		goto exit_select;
	}	


	count = 0;
	//find starting point for cursor		
	if(greater != -1) 
		treeindex.locate(greater, cursor);
	else  
		treeindex.findLeftMost(cursor);

	//print the first record
	if(gt != -1) {
		treeindex.readForward(cursor, key, rid);
		rf.read(rid, key, value);
		if(gt != key) {
			if(foundString) {
				if(value == str_val)
					printRecord(attr, key, value);
			}
			else 
				printRecord(attr, key, value);
			count ++;
		}
	}

	//find ending point for cursor
	if(less != -1)
		treeindex.locate(less, end);
	else  {
		end.pid = 0;	
		end.eid = 0;
	}

	//printf("Cursor pid: %i Cursor eid: %i -- End pid: %i End eid: %i\n", cursor.pid, cursor.eid, end.pid, end.eid);
	//
	//treeindex.printTree();
	//read all the relevant tuples
	while(!(cursor.pid == end.pid && cursor.eid == end.eid)) {
	//fprintf(stderr, "$$ LOOPING: Cursor at <pid,eid>: <%i,%i> \n", cursor.pid, cursor.eid);
		
		treeindex.readForward(cursor, key, rid);
		//if we've found an empty slot, continue
		if(key == 0 && rid.pid == 0 && rid.sid == 0)
			continue;

		count++;
		rf.read(rid, key, value);
		//printf("Key: %i Value: %s\n", key, value.c_str());
		if(foundString) {
			if(value == str_val)
				printRecord(attr, key, value);
		}
		else 
			printRecord(attr, key, value);

		//if we've reached the end, then decide whether we need to print the last tuple or not
		if(cursor.pid == end.pid && cursor.eid == end.eid) {
			if(le != -1 && cursor.pid != 0 && cursor.eid != 0) {
				treeindex.readForward(cursor, key, rid);
				rf.read(rid, key, value);
				if(le = key) {
					if(foundString) {
						if(value == str_val)
							printRecord(attr, key, value);
					}
					else 
						printRecord(attr, key, value);
				}
			}
			break;
		}
		//fprintf(stderr, "\n");
	}

  // print matching tuple count if "select count(*)"
 if (attr == 4) {
    fprintf(stdout, "%d\n", count);
    //fprintf(stderr, "%d\n", count);
 }



	//fprintf(stderr, "LOOPED %i TIMES!\n", i); 
	rc = 0;
	treeindex.close();

  // close the table file and return
  exit_select:
  rf.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	RC rc;
	BTreeIndex treeindex;
	RecordFile cf;
	RecordId rid;
	int key;
	string value;
	char buf[BUFSIZ];
	FILE * myfile;
	
	//Append the file to have .idx extension
	string indexfile;
	indexfile = table;
	indexfile.append(".idx");		

	//printf("the sring is %s\n",loadfile.c_str());
	//the file we try to read does exist!
	if((myfile = fopen(loadfile.c_str(),"r")) != NULL){
		if(index)
			if((rc = treeindex.open(indexfile,'w')) != 0)
				return rc;	
	
		if(cf.open(table + ".tbl", 'w') == 0)
			{
			while(fgets(buf,sizeof(buf),myfile))
				{
				if(parseLoadLine(buf,key,value) == 0)
					{
					rid = cf.endRid();
					if(cf.append(key,value,rid)!=0)
						printf("ERROR: Cannot append to the record!\n");
					if(index)
						treeindex.insert(key,rid);
					}
				 
				else 
					printf("ERROR: file error\n");
				}
			cf.close();
			printf("Load Successfully!\n");
			}
		else 
			printf("ERROR: Error in creating table!\n");

	treeindex.close();
	fclose(myfile);
	}
	else
		puts("FILE DOES NOT EXIST!");
	
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

void printRecord(int attr, int key, string value) {
	switch (attr) {
	    case 1:  // SELECT key
	      fprintf(stdout, "%d\n", key);
	      //fprintf(stderr, "%d\n", key);
	      break;
	    case 2:  // SELECT value
	      fprintf(stdout, "%s\n", value.c_str());
	      //fprintf(stderr, "%s\n", value.c_str());
	      break;
	    case 3:  // SELECT *
	      fprintf(stdout, "%d '%s'\n", key, value.c_str());
	      //fprintf(stderr, "%d '%s'\n", key, value.c_str());
	      break;
	    }	
}
