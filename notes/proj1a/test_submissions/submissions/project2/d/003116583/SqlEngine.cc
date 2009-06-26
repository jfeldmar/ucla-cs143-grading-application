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
#include <string>

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
  IndexCursor ic;
RecordId recid;
BTreeIndex b;

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;
int size = 1;
int i = 0;
int check = 0;
int loop = 0;
int equal;
int checkend = 0;
int checker;
int NE;
int valcheck;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }
  checker = b.open(table + ".idx", 'r');

// NOT EQUAL TESTER
i = 0;
equal = 0;
valcheck = 0;
while(i < cond.size())
{
	switch (cond[i].comp) 
        {
           case SelCond::NE:
	equal++;
	break;
        }
        if(cond[i].attr == 2)
	{	
		valcheck++;		
	}   
     i++;
}
if((equal == cond.size()) || (checker < 0) || (valcheck == cond.size()))
{
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

}
else
{
i = 0;
key = 0x80000000;
while(i < cond.size())
{
	switch (cond[i].comp) 
     {
           case SelCond::GT:
		if(atoi(cond[i].value) > key)
		{
			key = atoi(cond[i].value)+1;
		}
		break;
	case SelCond::GE:
		if(atoi(cond[i].value) <= key)
		{
			key = atoi(cond[i].value);
		}
		break;
     }
     i++;
}

//To compensate for EQUAL
i = 0;
equal = 0;
while(i < cond.size())
{
	switch (cond[i].comp) 
        {
           case SelCond::EQ:
	   if(key < atoi(cond[i].value))
	   {
		key = atoi(cond[i].value);
	   }
	   equal++;
	   break;
        }
        i++;
}
b.locate(key, ic);
int first = key;
b.readForward(ic,  key, recid );

count = 0;
size = cond.size()-equal;
int reads = 0;
while(size == (cond.size()-equal))
{
	reads++;
	rf.read(recid, key, value); 
	i = 0;
	size = 0;
	NE = 0;
	while(i < cond.size())
	{
		if(cond[i].attr == 1)
		{
			switch (cond[i].comp) 
			{
      				case SelCond::EQ:
					if(atoi(cond[i].value) == key)
					{
						size++;
					}
					break;
      				case SelCond::NE:
					if(atoi(cond[i].value) != key)
					{
						size++;
					}
					else {NE = 1;}
					break;
      				case SelCond::GT:
					if(atoi(cond[i].value) < key)
					{
						size++;
					}
					break;
 				case SelCond::LT:
					if(atoi(cond[i].value) > key)
					{
						size++;
					}
					break;
     				 case SelCond::GE:
					if(atoi(cond[i].value) <= key)
					{
						size++;
					}
					break;
     				case SelCond::LE:
					if(atoi(cond[i].value) >= key)
					{
						size++;
					}
					break;
			}
		}
		else 
		{
				diff = strcmp(value.c_str(), cond[i].value);
     				 switch (cond[i].comp) 
				{	
				      case SelCond::EQ:
    				  	if (diff == 0) size++;
 					break;
				      case SelCond::NE:
					if (diff != 0) {size++;}
					else {NE = 1;}
					break;
				      case SelCond::GT:
					if (diff > 0) size++;
					break;
				      case SelCond::LT:
					if (diff < 0) size++;
					break;
				      case SelCond::GE:								if (diff >= 0) size++;
					break;
				      case SelCond::LE:
					if (diff <= 0) size++;
					break;
      				}
		}
		i++;
	}
	if(size == cond.size())
	{
		count ++;
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
		
	}
	// compensate for NOT EQUAL
	if(NE == 1)
	{
		size++;
	}
		if(checkend == -1013)
	{
		break;
	}
	checkend = b.readForward(ic, key, recid );
}
	if (attr == 4) 
	{
		fprintf(stdout, "%d\n", count);
	}
}

 // close the table file and return
  exit_select:
rf.close();
b.close();
return rc;


}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */
RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

BTreeIndex btree;

RC     rc;
//int* keyptr;
//string* valueptr;
  int   key = 0;     
  string value = " ";

 // open the table file
  if ((rc = rf.open(table + ".tbl", 'w')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

int cnt = 1;

if(index)
{
	btree.open(table + ".idx", 'w');
}

string line;
  ifstream myfile (loadfile.c_str());
  if (myfile.is_open())
  {
    while (! myfile.eof())
    {
     getline (myfile,line);
	if(line !="")
	{
		parseLoadLine(line, key, value);
		//parseLoadLine(line, *keyptr, *valueptr);
		rid = rf.endRid();

    		//if ((rc =  rf.append(*keyptr, *valueptr, rid)) != 0) 
		if ((rc =  rf.append(key, value, rid)) != 0) 
		{
      			fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str()); 
		}
		else
		{	
			if(index)
			{
				btree.insert(key, rid);
			}
		}
	}	
     }
}

  else cout << "Unable to open file" << endl; 

if(index)
{
	btree.close();
}

int closerf = rf.close();
myfile.close();

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
