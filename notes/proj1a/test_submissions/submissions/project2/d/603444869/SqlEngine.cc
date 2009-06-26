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

RC SqlEngine::range_select(const string& table, int attr, int range_start, int range_end, bool gt_inclusive, bool lt_inclusive) {
    //cout << "range start: " << range_start << " range end " << range_end<<endl;
    if(range_start > range_end)
        return 1;
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning
  RecordId   stop_rid;  // record cursor for table scanning

  RC     rc;
  int    key;     
  int    stop_key;     
  string value;
  int    count = 0;
  int    diff;
  int oldkey;

  BTreeIndex* my_index = new BTreeIndex(table+".idx");
  my_index->open(table+".idx",'r');
  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }
            IndexCursor index;
            IndexCursor stop_index;
            my_index->locate(range_end,stop_index);
            my_index->readForward(stop_index,stop_key,stop_rid,false);
            stop_rid.pid = (stop_rid.pid-stop_rid.sid)/10;
            if ((rc = rf.read(stop_rid, stop_key, value)) < 0) {
                fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
                return 1;
            }
            //cout << "doing locate with "  << atoi(cond[i].value) << endl;
            my_index->locate(range_start,index); //locate the key and store it in index
            //cout << "got locate " << index.pid << " " << index.eid << endl;
            my_index->readForward(index,key,rid);
            rid.pid = (rid.pid-rid.sid)/10;
            bool stop = false;
            while(!stop) {
                if ((rc = rf.read(rid, key, value)) < 0) {
                    fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
                    return 1;
                } else {
                    //cout << "got key: " << key << endl;
                    if (key >= stop_key || key == range_end) 
                    {
                        //cout << "stop key: " << stop_key << " range_ned: " << range_end<< endl;
                        if(!lt_inclusive)
                            break;
                        stop = true;
                    }
                    if(gt_inclusive || key > range_start) {
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
                    case 4: // COUNT(*)
                    count++;
                    break;
                    }
                    } else
                        gt_inclusive = true;
                    my_index->readForward(index,key,rid);
                    rid.pid = (rid.pid-rid.sid)/10;
                }
            }
            if(attr == 4) {
                    fprintf(stdout, "%d\n", count);
            }
    return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

  RC     rc;
  int    key;     
  string value;
  int    count=0;
  int    diff;
  int oldkey;

  vector<int> ineq;

  BTreeIndex* my_index = new BTreeIndex(table+".idx");
  my_index->open(table+".idx",'r');
  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  bool gotequal = false;
  int range_start = -LONG_MAX;
  int range_end = LONG_MAX;
  bool gt_inclusive = true;
  bool lt_inclusive = true;
  //cout << "got select\n";
    for (unsigned i = 0; i < cond.size(); i++) {
    if(cond[i].attr == 2)
        goto nonbtree;
    switch (cond[i].comp) {
      case SelCond::EQ:
          gotequal=true;
          //cout << "got equal\n";
        switch (cond[i].attr) {
        case 1://key
            IndexCursor index;
            //cout << "doing locate with "  << atoi(cond[i].value) << endl;
            oldkey = atoi(cond[i].value);
            my_index->locate(oldkey,index); //locate the key and store it in index
            //cout << "got locate " << index.pid << " " << index.eid << endl;
            my_index->readForward(index,key,rid);
            rid.pid = (rid.pid-rid.sid)/10;
            //cout << "my rid.pid is " << rid.pid << " sid: " << rid.sid << endl;
            if ((rc = rf.read(rid, key, value)) < 0) {
                fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
                goto exit_select;
            } else {
                // print the tuple 
                if(oldkey == key) {
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
                case 4: // COUNT(*)
                count++;
                break;
                }
                if(attr == 4)
                    fprintf(stdout, "%d\n", count);
                } 
            }
        break;
        case 2://value
        //shouldn't have to select with value
        //if we are selecting with value
        goto nonbtree;
        break;
        }
	break;
      case SelCond::NE:
       ineq.push_back(atoi(cond[i].value)); 
	break;
      case SelCond::GT:
        if(atoi(cond[i].value) > range_start) 
        range_start = atoi(cond[i].value);
        gt_inclusive = false;
	break;
      case SelCond::LT:
        if(atoi(cond[i].value) < range_end) 
        range_end = atoi(cond[i].value);
        lt_inclusive = false;
	break;
      case SelCond::GE:
        if(atoi(cond[i].value) > range_start) 
        range_start = atoi(cond[i].value);
        gt_inclusive = true;
	break;
      case SelCond::LE:
        if(atoi(cond[i].value) < range_end) 
        range_end = atoi(cond[i].value);
        lt_inclusive = true;
	break;
      }
    }
    if (!gotequal) {
        for(int q = 0; q < ineq.size(); q++) {
            if(ineq[q] >= range_start && ineq[q] <= range_end)
                goto nonbtree;
        }
        range_select(table,attr,range_start, range_end,gt_inclusive,lt_inclusive);
    }

  // close the table file and return
  exit_select:
  //cout << "almost finished select\n";
    //my_index->close();
    delete my_index;
  rf.close();
  //cout << "finished select\n";
  return rc;

  /* DO NON B+ TREE SEARCH */
  // scan the table file from the beginning
nonbtree:
  rid.pid = rid.sid = 0;
  count = 0;
  int firstkey=0;
  while (rid < rf.endRid()) {
    // read the tuple
    if ((rc = rf.read(rid, key, value)) < 0) {
      fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
      goto exit_select;
    }
    if(!firstkey) {
        firstkey = key;
    } else {
        if(firstkey == key)
            break; //starting to repeat, break!
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

  if(attr==4) {
          fprintf(stdout, "%d\n", count);
  }
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */
    fstream filestr;
    const char* p = loadfile.c_str();
    filestr.open(p,fstream::in|fstream::out|fstream::app);

    BTreeIndex* my_index = new BTreeIndex(table+".idx");
    string line;
    string value;
    int key;

    RecordFile rf;   // RecordFile containing the table
    RecordId   rid;  // record cursor for table scanning

    RC     rc;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'w')) < 0) {
    fprintf(stderr, "Error: table %s could not be created\n", table.c_str());
    return rc;
  }

    while (getline(filestr,line,',')) { 
        if (line == "\n" || line == "\0")
            break;
        getline(filestr,value);
        key = atoi(line.c_str());
        //cout << key << " " << value.substr(1,value.length()-2) << endl;
        // write the tuple
        value = value.substr(1,value.length()-2);
        value += "\0";
        if ((rc = rf.append(key, value, rid)) < 0) {
        fprintf(stderr, "Error: while writing a tuple into table %s\n", table.c_str());
        break;
        } else {
            //add the rid and key pair to b+ tree
            //cout << "inserting " << key << " " << rid.pid*10 + rid.sid << endl;
            rid.pid = rid.pid*10 + rid.sid; //to store the "actual rid", can't use the same pid
            my_index->insert(key,rid);
        }
    }

    my_index->close();
    rf.close();
    filestr.close();
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
