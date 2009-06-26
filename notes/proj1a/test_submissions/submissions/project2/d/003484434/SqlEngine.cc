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
#include <string>
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

void SqlEngine::print(int attr, int key, const string& value)
{
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
  BTreeIndex bti;
  bool   has_index = false;
  IndexCursor ic;
  int key2 = 0;

  rc = bti.open(table, 'r');

  if(rc == 0)
	has_index = true;

  rid.pid = rid.sid = 0;
  count = 0;

// open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

if(has_index)
{
	int lower = -1, upper = -1, equal = -1;
	bool is_empty = false, has_equal = false, str_has_equal = false, only_values = true;
	bool has_lower = false, has_upper = false;
	bool str_has_upper = false, str_has_lower = false, is_LE = true, is_GE = true, only_NE = true;
	string lower_str = "", upper_str = "", equal_str = "", not_equal_str = "";
	string* value_NE = new string[10];
	int* key_NE = new int[10];
	int key_NE_size = 0, value_NE_size = 0;

	if(cond.size() == 0)
	{
		goto no_index;
	}

	for(int i = 0; i < cond.size(); i++)	// want to set bounds
        {
	  if(cond[i].attr == 1)	// the condition is on keys
	  {
	  	only_values = false;
		if(cond[i].comp == SelCond::NE)
		{
			key_NE[key_NE_size] = atoi(cond[i].value);
			key_NE_size++;
		}
		
		else if(cond[i].comp == SelCond::GT || cond[i].comp == SelCond::GE)
		{
			only_NE = false;
			if(!has_lower)
			{
				lower = atoi(cond[i].value);
				has_lower = true;		// we have a lower bound
			}
			if(cond[i].comp == SelCond::GT)
				lower++;
		}
		else if(cond[i].comp == SelCond::LT || cond[i].comp == SelCond::LE)
		{
			only_NE = false;
			if(!has_upper)
			{
				upper = atoi(cond[i].value);
				has_upper = true;
			}
			else
			{
				if(atoi(cond[i].value) < upper)
					upper = atoi(cond[i].value);
			}
			if(cond[i].comp == SelCond::LT)
				upper--;
		}
		else if(cond[i].comp == SelCond::EQ)
		{
			only_NE = false;
			if(!has_equal)
			{
				equal = atoi(cond[i].value);
				has_equal = true;
			}
			else
			{
				if(atoi(cond[i].value) != equal)
					is_empty = true;
			}
		}
		else
		{
			// just counting the tuples
		}
	  }
	  else		// condition on values
	  {
		if(cond[i].comp == SelCond::NE)	// NE goes to old search
		{
			value_NE[value_NE_size] = cond[i].value;
			value_NE_size++;
		}

		else if(cond[i].comp == SelCond::EQ)
		{
			only_NE = false;
			str_has_equal = true;	// need to check string equality
			if(strcmp(equal_str.c_str(), "") == 0)
				equal_str = cond[i].value;
			else	// the lower bound already exists
			{
				if(strcmp(cond[i].value, equal_str.c_str()) != 0)
					is_empty = true;	// can't have two different equals on value
			}
		}
		else if(cond[i].comp == SelCond::GT || cond[i].comp == SelCond::GE)	// >=
		{
			only_NE = false;
			str_has_lower = true;
			if(strcmp(lower_str.c_str(), "") == 0)	// if no lower bound established
				lower_str = cond[i].value;

			if(strcmp(cond[i].value, lower_str.c_str()) > 0)
				lower_str = cond[i].value;
			if(cond[i].comp == SelCond::GT)	// add one to the end char for >
				is_GE = false;
		}
		else if(cond[i].comp == SelCond::LT || cond[i].comp == SelCond::LE)
		{
			only_NE = false;
			str_has_upper = true;
			if(strcmp(upper_str.c_str(), "") == 0)	// no upper bound
				upper_str = cond[i].value;
			if(strcmp(cond[i].value, upper_str.c_str()) < 0)
				upper_str = cond[i].value;
			if(cond[i].comp == SelCond::LT)
				is_LE = false;
		}
	  }
        }

	if(only_NE)	// only condition is on not equals <>
	{
		goto no_index;
	}

	// check the validity of the upper and lower bounds
	if(has_lower && has_upper && lower > upper)
		is_empty = true;
	if(has_equal && has_upper && has_lower && (equal < lower || equal > upper))
		is_empty = true;
	if(has_equal && has_upper)
	{
		if(equal > upper)
			is_empty = true;
	}
	if(has_equal && has_lower)
	{
		if(equal < lower)
			is_empty = true;
	}

	if(only_values)
		goto no_index;
	
    // check the conditions on the tuple
    //for (unsigned i = 0; i < cond.size(); i++) {
      // compute the difference between the tuple value and the condition value
      // skip the tuple if any condition is not met
      if(!is_empty && cond.size() > 0)	// if the set we're looking for is not empty
      {
	      if(cond[0].comp == SelCond::EQ || has_equal)
	      {
		rc = bti.locate(equal, ic);
		rc = bti.readForward(ic, key, rid);
		rc = rf.read(rid, key, value);

		bool can_print = true;

		for(int i = 0; i < key_NE_size; i++)
		{
			//cout << "key_NE[" << i << "]: " << key_NE[i] << endl;
			if(key == key_NE[i])
				can_print = false;
		}

		for(int i = 0; i < value_NE_size; i++)
		{
			//cout << "value_NE[" << i << "]: " << value_NE[i] << endl;
			if(strcmp(value.c_str(), (value_NE[i]).c_str()) == 0)
				can_print = false;
		}

		// SelCond::EQ condition
		if(key == equal && can_print)
		{
			if(str_has_equal)
			{
				if(strcmp(value.c_str(), equal_str.c_str()) == 0)
				{
					print(attr, key, value);
					count++;
				}
			}
			else if(str_has_lower && str_has_upper)
			{
				bool lower_check;
				if(is_GE)
					lower_check = strcmp(value.c_str(), lower_str.c_str()) >= 0;
				else
					lower_check = strcmp(value.c_str(), lower_str.c_str()) > 0;

				bool upper_check;
				if(is_LE)
					upper_check = strcmp(value.c_str(), upper_str.c_str()) <= 0;
				else
					upper_check = strcmp(value.c_str(), upper_str.c_str()) < 0;


				if(lower_check && upper_check)
				{
					print(attr, key, value);
					count++;
				}
			}
			else if(str_has_lower)	// lower bound on the tuple
			{
				bool lower_check;
				if(is_GE)
					lower_check = strcmp(value.c_str(), lower_str.c_str()) >= 0;
				else
					lower_check = strcmp(value.c_str(), lower_str.c_str()) > 0;

				// check that output is > lower bound
				if(lower_check)
				{
					print(attr, key, value);
					count++;
				}
			}
			else if(str_has_upper)
			{
				bool upper_check;
				if(is_LE)
					upper_check = strcmp(value.c_str(), upper_str.c_str()) <= 0;
				else
					upper_check = strcmp(value.c_str(), upper_str.c_str()) < 0;

				// check that output is > lower bound
				if(upper_check)
				{
					print(attr, key, value);
					count++;
				}
			}
			else
			{
				print(attr, key, value);
				count++;
			}
		}
	      }

	      if(!has_equal)
	      {
		      if(cond[0].comp == SelCond::GT || cond[0].comp == SelCond::GE 
		      	|| cond[0].comp == SelCond::NE)
		      {
		        if(!has_lower)		// there's no lower bound, force read from pid 1, eid 0
			{
				ic.pid = 1;	// first record page, first entry
				ic.eid = 0;
			}

			else
				rc = bti.locate(lower, ic);

			bool read_last = false;
			while(rc = bti.readForward(ic, key, rid) == 0)
			{
				read_last = true;
				rc = rf.read(rid, key, value);

				if(has_upper)
				{
					if(key > upper)
						goto finished;
				}

				bool can_print = true;

				for(int i = 0; i < key_NE_size; i++)
				{
					//cout << "key_NE[" << i << "]: " << key_NE[i] << endl;
					if(key == key_NE[i])
						can_print = false;
				}

				for(int i = 0; i < value_NE_size; i++)
				{
					//cout << "value_NE[" << i << "]: " << value_NE[i] << endl;
					if(strcmp(value.c_str(), (value_NE[i]).c_str()) == 0)
						can_print = false;
				}

				if(can_print)
				{
					if(upper == -1)	// no limit!
					{
						if(str_has_equal)
						{
							if(strcmp(value.c_str(), equal_str.c_str()) == 0)
							{
								print(attr, key, value);
								count++;
							}
						}
						else if(str_has_lower && str_has_upper)
						{
							bool lower_check;
							if(is_GE)
								lower_check = strcmp(value.c_str(), lower_str.c_str()) >= 0;
							else
								lower_check = strcmp(value.c_str(), lower_str.c_str()) > 0;

							bool upper_check;
							if(is_LE)
								upper_check = strcmp(value.c_str(), upper_str.c_str()) <= 0;
							else
								upper_check = strcmp(value.c_str(), upper_str.c_str()) < 0;


							if(lower_check && upper_check)
							{
								print(attr, key, value);
								count++;
							}
						}
						else if(str_has_lower)	// lower bound on the tuple
						{
							bool lower_check;
							if(is_GE)
								lower_check = strcmp(value.c_str(), lower_str.c_str()) >= 0;
							else
								lower_check = strcmp(value.c_str(), lower_str.c_str()) > 0;

							// check that output is > lower bound
							if(lower_check)
							{
								print(attr, key, value);
								count++;
							}
						}
						else if(str_has_upper)
						{
							bool upper_check;
							if(is_LE)
								upper_check = strcmp(value.c_str(), upper_str.c_str()) <= 0;
							else
								upper_check = strcmp(value.c_str(), upper_str.c_str()) < 0;

							// check that output is > lower bound
							if(upper_check)
							{
								print(attr, key, value);
								count++;
							}
						}
						else
						{
							print(attr, key, value);
							count++;
						}
					}
					else
					{
						if(key <= upper)
						{	
							if(str_has_equal)
							{
								if(strcmp(value.c_str(), equal_str.c_str()) == 0)
								{
									print(attr, key, value);
									count++;
								}
							}
							else if(str_has_lower && str_has_upper)
							{
								bool lower_check;
								if(is_GE)
									lower_check = strcmp(value.c_str(), lower_str.c_str()) >= 0;
								else
									lower_check = strcmp(value.c_str(), lower_str.c_str()) > 0;

								bool upper_check;
								if(is_LE)
									upper_check = strcmp(value.c_str(), upper_str.c_str()) <= 0;
								else
									upper_check = strcmp(value.c_str(), upper_str.c_str()) < 0;


								if(lower_check && upper_check)
								{
									print(attr, key, value);
									count++;
								}
							}
							else if(str_has_lower)	// lower bound on the tuple
							{
								bool lower_check;
								if(is_GE)
									lower_check = strcmp(value.c_str(), lower_str.c_str()) >= 0;
								else
									lower_check = strcmp(value.c_str(), lower_str.c_str()) > 0;

								// check that output is > lower bound
								if(lower_check)
								{
									print(attr, key, value);
									count++;
								}
							}
							else if(str_has_upper)
							{
								bool upper_check;
								if(is_LE)
									upper_check = strcmp(value.c_str(), upper_str.c_str()) <= 0;
								else
									upper_check = strcmp(value.c_str(), upper_str.c_str()) < 0;

								// check that output is > lower bound
								if(upper_check)
								{
									print(attr, key, value);
									count++;
								}
							}
							else
							{
								print(attr, key, value);
								count++;
							}
						}
					}
				}
			}

			finished:
			// a symptom of this b+tree is such that the last one isn't 
			// printed, even though it exists. so we forcefully call 
			// readForward and print it if it meets all of the conditions
			if(upper == -1 && read_last)
			{
				bti.readForward(ic, key, rid);
				rc = rf.read(rid, key, value);

				bool can_print = true;

				for(int i = 0; i < key_NE_size; i++)
				{
					//cout << "key_NE[" << i << "]: " << key_NE[i] << endl;
					if(key == key_NE[i])
						can_print = false;
				}

				for(int i = 0; i < value_NE_size; i++)
				{
					//cout << "value_NE[" << i << "]: " << value_NE[i] << endl;
					if(strcmp(value.c_str(), (value_NE[i]).c_str()) == 0)
						can_print = false;
				}

				if(can_print)
				{
					if(str_has_equal)
					{
						if(strcmp(value.c_str(), equal_str.c_str()) == 0)
						{
							print(attr, key, value);
							count++;
						}
					}
					else if(str_has_lower && str_has_upper)
					{
						bool lower_check;
						if(is_GE)
							lower_check = strcmp(value.c_str(), lower_str.c_str()) >= 0;
						else
							lower_check = strcmp(value.c_str(), lower_str.c_str()) > 0;

						bool upper_check;
						if(is_LE)
							upper_check = strcmp(value.c_str(), upper_str.c_str()) <= 0;
						else
							upper_check = strcmp(value.c_str(), upper_str.c_str()) < 0;


						if(lower_check && upper_check)
						{
							print(attr, key, value);
							count++;
						}
					}
					else if(str_has_lower)	// lower bound on the tuple
					{
						bool lower_check;
						if(is_GE)
							lower_check = strcmp(value.c_str(), lower_str.c_str()) >= 0;
						else
							lower_check = strcmp(value.c_str(), lower_str.c_str()) > 0;

						// check that output is > lower bound
						if(lower_check)
						{
							print(attr, key, value);
							count++;
						}
					}
					else if(str_has_upper)
					{
						bool upper_check;
						if(is_LE)
							upper_check = strcmp(value.c_str(), upper_str.c_str()) <= 0;
						else
							upper_check = strcmp(value.c_str(), upper_str.c_str()) < 0;

						// check that output is > lower bound
						if(upper_check)
						{
							print(attr, key, value);
							count++;
						}
					}
					else
					{
						print(attr, key, value);
						count++;
					}
				}
			}
		      }
		      if(cond[0].comp == SelCond::LT || cond[0].comp == SelCond::LE)
		      {
			if(has_lower)
				rc = bti.locate(lower, ic);
			else	// no lower bound, so read from first record
			{
				ic.pid = 1;
				ic.eid = 0;
			}

			while(rc = bti.readForward(ic, key, rid) >= 0 && key <= upper)
			{
				rc = rf.read(rid, key, value);

				bool can_print = true;

				for(int i = 0; i < key_NE_size; i++)
				{
					//cout << "key_NE[" << i << "]: " << key_NE[i] << endl;
					if(key == key_NE[i])
						can_print = false;
				}

				for(int i = 0; i < value_NE_size; i++)
				{
					//cout << "value_NE[" << i << "]: " << value_NE[i] << endl;
					if(strcmp(value.c_str(), (value_NE[i]).c_str()) == 0)
						can_print = false;
				}
				
				if(can_print)
				{
					if(str_has_equal)
					{
						if(strcmp(value.c_str(), equal_str.c_str()) == 0)
						{
							print(attr, key, value);
							count++;
						}
					}
					else if(str_has_lower && str_has_upper)
					{
						bool lower_check;
						if(is_GE)
							lower_check = strcmp(value.c_str(), lower_str.c_str()) >= 0;
						else
							lower_check = strcmp(value.c_str(), lower_str.c_str()) > 0;

						bool upper_check;
						if(is_LE)
							upper_check = strcmp(value.c_str(), upper_str.c_str()) <= 0;
						else
							upper_check = strcmp(value.c_str(), upper_str.c_str()) < 0;


						if(lower_check && upper_check)
						{
							print(attr, key, value);
							count++;
						}
					}
					else if(str_has_lower)	// lower bound on the tuple
					{
						bool lower_check;
						if(is_GE)
							lower_check = strcmp(value.c_str(), lower_str.c_str()) >= 0;
						else
							lower_check = strcmp(value.c_str(), lower_str.c_str()) > 0;

						// check that output is > lower bound
						if(lower_check)
						{
							print(attr, key, value);
							count++;
						}
					}
					else if(str_has_upper)
					{
						bool upper_check;
						if(is_LE)
							upper_check = strcmp(value.c_str(), upper_str.c_str()) <= 0;
						else
							upper_check = strcmp(value.c_str(), upper_str.c_str()) < 0;

						// check that output is > lower bound
						if(upper_check)
						{
							print(attr, key, value);
							count++;
						}
					}
					else
					{
						print(attr, key, value);
						count++;
					}
				}
			}
		      }
	      }

      }
}

else
{
    // scan the table file from the beginning
no_index:
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
  bti.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */
  RecordId out_id;
  out_id.pid = out_id.sid = 0;

  RecordFile out_file(table+".tbl", 'w');
  string line;
  char loc[100];
  int ind;
  BTreeIndex bti;
  
  // copy the input string loadfile into a char array for file input
  for(ind = 0; loadfile[ind] != 0; ind++)
  	loc[ind] = loadfile[ind];
  loc[ind] = '\0'; // close the string


  if(index)
  {
	bti.open(table, 'w');
  }

  // open the in file
  ifstream myfile(loc);

  if(!myfile.eof())
	  getline(myfile, line);
  
  while(!myfile.eof())
  {
	int key = 0;
	string str;
	int rc = parseLoadLine(line, key, str);
	if(rc == 0)
	{
		out_file.append(key, str, out_id);
		if(index)
			bti.insert(key, out_id);
	}
	else
		return RC_INVALID_FILE_FORMAT;
  	getline(myfile, line);
  }

	int key = 0;
	string str;
	int rc = parseLoadLine(line, key, str);
	if(rc == 0)
	{
		out_file.append(key, str, out_id);
		if(index)
			bti.insert(key, out_id);
	}
	else
	{
		if(index)
			bti.close();
	        //fprintf(stderr, "Invalid file!");
		return RC_INVALID_FILE_FORMAT;
	}


  if(index)
	  bti.close();
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
