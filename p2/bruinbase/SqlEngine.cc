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
  IndexCursor ic;	// is the index to our file!
  ic.pid = 0;
  ic.eid = 0;
  rid.pid = 0;
  rid.sid = 0;
  int locate_key = 0;
  int l_bound = -1;
  int u_bound = -1;
  int equal = -1;
  string l_bound_val = "";
  string u_bound_val = "";
  string equal_val = "";
  string value_val = "";
  int key_notequal_list[100];
  int key_notequal_count = 0;
  string value_notequal_list[100];
  int value_notequal_count = 0;
  
  //char* value_val = NULL;
  
  bool val_in_query = false;
  bool less_than_equal = false;
  bool greatter_than_equal = false;	//to be checked when upper or lower bounds for values are set.
  bool last_value = true;
  bool only_notequal = true;
  bool key_notequal = true;
  bool value_notequal = true;
  bool u_bound_exists = false;
  bool l_bound_exists = false;
  bool equal_exists = false;
  
  bool isemptyset = false;
  int key_value = -1;
  int temp_bound = -1;
  
  
  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;
  rc = 0;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }
  // open the index file if it exists
  BTreeIndex bti;
  bool index_exists = false;
  bool all_values = true;
  
  if((rc = bti.open(table, 'r')) >= 0)
  {
	index_exists = true;
  }
  
  // scan the table file from the beginning
  rid.pid = rid.sid = 0;
  count = 0;
  
  /////////////// my code goes here, i think////////////////////////////
  
  
  for (unsigned i = 0; i < cond.size() && all_values == true; i++) 
  {
	  if(cond[i].attr == 2 && all_values == true || cond[i].comp == SelCond::NE ) /////////.................problem may occur here.
	  {
		all_values = true;
	  }
	  else
	  {
		all_values = false;
	  }
	  if(cond[i].comp == SelCond::NE && only_notequal == true)
	  {
		only_notequal = true;
	  }
	  else
	  {
		only_notequal = false;
	  }
  }
   if(cond.size() == 0)
   {
	  all_values = true;		//if not condition given,
   }
   
  if(all_values)
  {
	// cout << " true" <<endl;
  }
/////////////////////////////////////////////////////////////////////////
 //////////////////////////////////////////////////////////////////////////////////

  if(index_exists == true && all_values == false && only_notequal == false)		//here is where i check using the index.
  {
	/////////////
	  
	  for (unsigned i = 0; i < cond.size(); i++) 
		{
			if( cond[i].attr == 1)
			{
				key_value = atoi(cond[i].value);
				switch ( cond[i].comp)
				{
					case SelCond::EQ:
					if(equal_exists == false)	//if(equal == -1)
					{
						equal = key_value;
						equal_exists = true;
					}
					else if(key_value != equal)
					{
						isemptyset = true;
					}
					break;
					case SelCond::GT: 			//will start as greatter than equal with one higher value.
					if(key_value + 1 > l_bound || l_bound_exists == false )///l_bound == -1)
					{
						l_bound = key_value + 1;
						l_bound_exists = true;
					}
					break;
					case SelCond::LT:
					if(u_bound > key_value - 1 || u_bound_exists == false )//u_bound == -1)
					{
						u_bound = key_value - 1;
						u_bound_exists = true;
					}
					break;
					case SelCond::GE:
					if(key_value  > l_bound || l_bound_exists == false )//l_bound == -1)
					{
						l_bound = key_value;
						l_bound_exists = true;
					}
					break;
					case SelCond::LE:
					if(u_bound > key_value || u_bound_exists == false )//u_bound == -1 )
					{
						u_bound = key_value;
						u_bound_exists = true;
					}
					break;
					case SelCond::NE:
						//index_exists = false;
						key_notequal_list[key_notequal_count] = key_value;
						key_notequal_count++;
					break;
				}
			}		///now what if the attribute is a value instead of key?
			else if( cond[i].attr == 2 ) //attribute provided is type value
			{
				val_in_query = true;
				//diff = strcmp(value.c_str(), cond[i].value);
				value_val = cond[i].value;
				switch ( cond[i].comp)
				{
					case SelCond::EQ:
						if (strcmp(equal_val.c_str(), "") == 0 )
						{
							equal_val = value_val;
						}
						else if(strcmp(equal_val.c_str(), cond[i].value ) != 0 )
							{
								isemptyset = true;
							}
						break;
					case SelCond::GT:	//remember to increament the value by one
						if(strcmp( l_bound_val.c_str(), "") == 0 || strcmp(l_bound_val.c_str(), cond[i].value) <= 0)
						{
							l_bound_val = value_val;
							greatter_than_equal = false;
						}
						break;
					case SelCond::LT:   //remember to decreament the value by one
						if(strcmp( u_bound_val.c_str(), "") == 0 || strcmp(u_bound_val.c_str(), cond[i].value) >= 0)
						{
							u_bound_val = value_val;
							less_than_equal = false;
						}
						break;
					case SelCond::GE:
						if(strcmp( l_bound_val.c_str(), "") == 0 || strcmp(l_bound_val.c_str(), cond[i].value) < 0)
						{
							l_bound_val = value_val;
							greatter_than_equal = true;
						}
						break;
					case SelCond::LE:
						if(strcmp( u_bound_val.c_str(), "") == 0 || strcmp(u_bound_val.c_str(), cond[i].value) > 0)
						{
							u_bound_val = value_val;
							less_than_equal = true;
						}
						break;
					case SelCond::NE:
						//index_exists = false;
						value_notequal_list[value_notequal_count] = value_val;
						value_notequal_count++;
						break;
				}
				
			}	////check for value
		}	//end of for loop for cond bound check.
			
		//////now check and set the bounds for the values.
		if(val_in_query == true)
		{
			if( strcmp( u_bound_val.c_str(), "") != 0 && strcmp(l_bound_val.c_str(), "") != 0 && strcmp(u_bound_val.c_str(), l_bound_val.c_str()) < 0 )
			{
				isemptyset = true;
			}
			else if(strcmp(equal_val.c_str(), "") != 0)
			{
				if( strcmp(l_bound_val.c_str(), "") != 0 && strcmp(equal_val.c_str(), l_bound_val.c_str()) < 0)
				{
					isemptyset = true;
				}
				else if( strcmp(u_bound_val.c_str(), "") != 0 && strcmp(equal_val.c_str(), u_bound_val.c_str()) > 0)
				{
					isemptyset = true;
				}
			}
		}

		//now check equal l_bound and u_bound are valid. need to generate the lower and upper bound values
		///for key
		if(u_bound == l_bound && u_bound_exists && l_bound_exists )//u_bound != -1 && equal == -1)
		{
			equal = u_bound;
			equal_exists = true;
		}
		else if (l_bound_exists && u_bound_exists && u_bound < l_bound) //if(l_bound != -1 && u_bound != -1 && u_bound < l_bound)
			{
				isemptyset = true;
			}
			else if(equal_exists == true) //if( equal != -1)
			{
				if( l_bound_exists == true && equal < l_bound) //if( l_bound != -1 && equal < l_bound)
				{
					isemptyset = true;
				}		
				else if( u_bound_exists && equal > u_bound) //if( u_bound != -1 && equal > u_bound)
				{
					isemptyset = true;
				}
			}
	  
	  if(isemptyset)
	  {
		// cout << "empty set" << endl;
		goto exit_select;
	  }
	  else
	  {
		// cout << " not an empty set " << endl;
		// cout<< "l_bound = " << l_bound << " u_bound = " << u_bound << " equal = " << equal << endl;
		// cout<< "l_bound_val = " << l_bound_val << " u_bound_val = " << u_bound_val << " equal_val = " << equal_val << endl;
		if(greatter_than_equal)
		{
			// cout<<" greatter_than_equal "<<endl;
		}
		else
		{
			// cout << "greatter_than " << endl;
		}
		if(less_than_equal)
		{
			// cout<<" less_than_equal "<<endl;
		}
		else
		{
			// cout << "less_than " << endl;
		}
	  } 
	  
////////////////////// now u have the bounds for all key value, call the readforward accordingly!////////////////////
	if(equal_exists == true ) //if(equal != -1)
	{// if equal set just call it.
		rc = bti.locate(equal, ic);
		if(rc < 0)
		{
			// cout << " value not found in equall " << endl;
			goto exit_select;
		}
		rc = bti.readForward(ic, key, rid);
		// if(rc < 0)
		// {
			// cout << " value not found in equalr " << endl;
		//	goto exit_select;
		//}
		if ((rc = rf.read(rid, key, value)) < 0) 
		{
		  fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
		  goto exit_select;
		}
		if( key == equal )
		{
			if(val_in_query)		//if any value needs to be compared.
			{
				for(int looper = 0; looper < value_notequal_count; looper++)
				{
					if(strcmp(value.c_str(), value_notequal_list[looper].c_str()) == 0)
					{
						value_notequal = false;
					}
				}
				
				////////////////
				if( compare_value(l_bound_val, u_bound_val, equal_val, value, less_than_equal, greatter_than_equal) && value_notequal == true )
				{
					print_this(key, value, attr);
					count++;
					goto exit_select;
				}
				else
				{
					// cout << " value not found in equalr.. " << endl;
					value_notequal = true;
					goto exit_select;
				}
			}
			else
			{
				key_notequal = true;
				for(int looper = 0; looper < key_notequal_count; looper++)
				{
					if(key == key_notequal_list[looper])
						key_notequal = false;
				}
				if(key_notequal)
				{
					print_this(key, value, attr);
					count++;
					goto exit_select;
				}
				
			}
		}
	}
	else		//equal not set, go on to do other useful stuff.
	{
		if(l_bound_exists) //if(l_bound != -1)
		{
			temp_bound = l_bound;
			if(u_bound_exists) //if(u_bound != -1)		//lower bound exists and upper bound exists.
			{
				last_value = true;	//to force it to read one more page.
				rc = bti.locate(temp_bound, ic);
				if(rc < 0)
				{
					// cout << " value not found in equalllbub.. " << " RC = " << rc << endl;
					// cout << "pid, eid " << ic.pid << "  " << ic.eid << endl;
					// cout << " going to next value.. " << endl;
					//
					ic.eid = ic.eid - 1;	//to trigger the even of increasing the pid to next node pointer
					rc = bti.readForward(ic, key, rid);
					if( rc < 0)
					{
						// cout << " value not found in equalrlb.... " << " RC = " << rc << endl;
						goto exit_select;
					}
					// cout << " pid, eid " << ic.pid <<"  " << ic.eid << " rc " << rc << endl;
					goto look_out_here2;
				}
				else
				{
					////////
					look_out_here2:
					value_notequal = true;
					rc = bti.readForward(ic, key, rid);
					if(rc == -1013)
					{
						rc = 0;
						last_value = false;
					}
					if(rc < 0)
					{
						// cout << " value not found in equalrlb " << " RC = " << rc << endl;
						goto exit_select;
					}
					if(key > u_bound)	
					{
						goto exit_select;
					}
					while(rc >= 0)
					{
						if ((rc = rf.read(rid, key, value)) < 0) 
						{
						  fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
						  goto exit_select;
						}
						if( key <= u_bound)
						{
							if(val_in_query)		//if any value needs to be compared.
							{
								for(int looper = 0; looper < value_notequal_count; looper++)
								{
									if(strcmp(value.c_str(), value_notequal_list[looper].c_str()) == 0)
									{
										value_notequal = false;
									}
								}
								if( compare_value(l_bound_val, u_bound_val, equal_val, value, less_than_equal, greatter_than_equal) && value_notequal == true )
								{
									print_this(key, value, attr);
									count++;
								}
								value_notequal = true;	//reset the value
							}
							else
							{
								key_notequal = true;
								for(int looper = 0; looper < key_notequal_count; looper++)
								{
									if(key == key_notequal_list[looper])
										key_notequal = false;
								}
								if(key_notequal)
								{
									print_this(key, value, attr);
									count++;
								}
								key_notequal = true;
							}
							//goto exit_select;
						} //will it go berserk is u_bound happens to be low on the first try when key read?
						else if(key > u_bound)	
							{
								goto exit_select;
							}
						rc = bti.readForward(ic, key, rid);
						if(last_value == true && rc < 0)
						{
							rc = 0;
							last_value = false;
						}
					}
				}
				goto exit_select; //done. go to the end.
			}
			else if(u_bound_exists == false) //if(u_bound == -1)	// lower bound exists and upper bound does not esist.
			{
				last_value = true;
				rc = bti.locate(temp_bound, ic);
				if(rc < 0)
				{
					// cout << " value not found in equalllb. " << " RC = " << rc << endl;
					// cout << "pid, eid " << ic.pid << "  " << ic.eid << endl;
					// cout << " going to next value " << endl;
					// 
					ic.eid = ic.eid - 1;	//to trigger the even of increasing the pid to next node pointer
					rc = bti.readForward(ic, key, rid);
					if( rc < 0)
					{
						// cout << " value not found in equalrlb.. " << " RC = " << rc << endl;
						goto exit_select;
					}
					// cout << " pid, eid " << ic.pid <<"  " << ic.eid << " rc " << rc << endl;
					goto look_out_here;
				}
				else
				{
					look_out_here:
					rc = bti.readForward(ic, key, rid);
					if(rc == -1013)
					{
						rc = 0;
						last_value = false;
					}
					if(rc < 0)
					{
						// cout << " value not found in equalrlb. " << " RC = " << rc << endl;
						goto exit_select;
					}
					
					while(rc >= 0)
					{
						if ((rc = rf.read(rid, key, value)) < 0) 
						{
						  fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
						  goto exit_select;
						}
						if(val_in_query)		//if any value needs to be compared.
						{
							for(int looper = 0; looper < value_notequal_count; looper++)
							{
								if(strcmp(value.c_str(), value_notequal_list[looper].c_str()) == 0)
								{
									value_notequal = false;
								}
								
							}
							if( compare_value(l_bound_val, u_bound_val, equal_val, value, less_than_equal, greatter_than_equal) && value_notequal == true )
							{
								print_this(key, value, attr);
								count++;
							}
							value_notequal == true;
						}
						else
						{
							key_notequal = true;
							for(int looper = 0; looper < key_notequal_count; looper++)
							{
								if(key == key_notequal_list[looper])
									key_notequal = false;
							}
							if(key_notequal == true)
							{
								print_this(key, value, attr);
								count++;
							}
							key_notequal = true;
						}
						//goto exit_select;
						rc = bti.readForward(ic, key, rid);
						if(last_value == true && rc < 0 )
						{
							rc = 0;
							last_value = false;
						}
						value_notequal = true;
					}
				}
				goto exit_select; //done. go to the end.
			}
		}
		else if(l_bound_exists == false )//if(l_bound == -1)
		{
			last_value = true;	//to force it to read one more page.
			temp_bound = 0;
			if( u_bound_exists )//if(u_bound != -1)		//lower bound does not exist and upper bound exists.
			{
				//rc = bti.locate(temp_bound, ic);
				//if( rc < 0 )
				//{
					//// cout << "locate exited with error = " << rc;
				//	goto exit_select;
				//}
				//read from the first page!
				ic.pid = 1;
				ic.eid = 0;
				rc = bti.readForward(ic, key, rid);
				if(rc < 0)
				{
					// cout << " value not found in equalrlb2 " << " RC = " << rc << endl;
					goto exit_select;
				}
				if(key > u_bound)	
				{
					goto exit_select;
				}
				while(rc >= 0)
				{
					if ((rc = rf.read(rid, key, value)) < 0) 
					{
					  fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
					  goto exit_select;
					}
					if( key <= u_bound)
					{
						if(val_in_query)		//if any value needs to be compared.
						{
							for(int looper = 0; looper < value_notequal_count; looper++)
							{
								if(strcmp(value.c_str(), value_notequal_list[looper].c_str()) == 0)
								{
									value_notequal = false;
								}
							}
							if( compare_value(l_bound_val, u_bound_val, equal_val, value, less_than_equal, greatter_than_equal) )
							{
								print_this(key, value, attr);
								count++;
							}
							value_notequal = true;
						}
						else
						{
							key_notequal = true;
							for(int looper = 0; looper < key_notequal_count; looper++)
							{
								if(key == key_notequal_list[looper])
									key_notequal = false;
							}
							if(key_notequal)
							{
								print_this(key, value, attr);
								count++;
							}
							key_notequal = true;
						}
						//goto exit_select;
					} //will it go berserk is u_bound happens to be low on the first try when key read?
					else if(key > u_bound)	
						{
							goto exit_select;
						}
					rc = bti.readForward(ic, key, rid);
					if(last_value == true && rc < 0 )
					{
						rc = 0;
						last_value = false;
					}
				}
				goto exit_select;
			}
		}
	}
  }
  

  ///////////////////////////////////// his code ///////////////////////////////////////////////
  if((all_values == true || index_exists == false || only_notequal == true ))
  {
	  while ((rid < rf.endRid() )) 
	  {
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
  
///////////  
  
   exit_select:
  
  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }

  /////////////////////////	
	
  // close the table file and return
 
  rf.close();
  bti.close();
  return rc;
}

//////////////////////////////////////// helper function////////////////////////////////////

void  SqlEngine::print_this(const int key, const std::string value, const int attr)
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

//////helper function 
bool SqlEngine::compare_value(const string l_bound_val, const string u_bound_val, const string equal_val, const string value, const bool less_than_equal, const bool greatter_than_equal)
{
	bool toprint = true;
	
	if( strcmp( equal_val.c_str(), "" ) != 0)		// if equality exists.
	{
		if( strcmp( equal_val.c_str(), value.c_str() ) == 0)
			toprint = true;
		else
			toprint = false;
	}
	else
	{
		if( strcmp( l_bound_val.c_str() , "" ) != 0)	//if lower bound set. check greatter than equal flag.
		{
			if(greatter_than_equal)
			{
				if(strcmp(value.c_str() , l_bound_val.c_str()) >= 0)
				{
					toprint = true;
				}
				else
				{
					toprint = false;
				}
			}
			else 	//check only greatter then
			{
				if(strcmp(value.c_str() , l_bound_val.c_str()) > 0)
				{
					toprint = true;
				}
				else
				{
					toprint = false;
				}
			}
		}
		if( strcmp( u_bound_val.c_str() , "" ) != 0 && toprint == true )	//if lower bound set.
		{
			if(less_than_equal)
			{
				if(strcmp(value.c_str() , u_bound_val.c_str()) <= 0)
				{
					toprint = true;
				}
				else
				{
					toprint = false;
				}				
			}
			else
			{
				if(strcmp(value.c_str() , u_bound_val.c_str()) < 0)
				{
					toprint = true;
				}
				else
				{
					toprint = false;
				}
			}
		}
	}
	return toprint;
}

//////////////////////////////////////// helper function ends ////////////////////////////////////

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */

  RC     rc;
  int    key;     
  string value;
  string line_read;
  int    size;
  int    counter = 0;
  char   file_name[200];
  
  //const static int OFFSET = 16;
  BTreeIndex bti;

  RecordFile rf;   // RecordFile containing / to contain the table
  RecordId   rid;  // record cursor for table scanning

  // open the table file in write mode
  if ((rc = rf.open(table + ".tbl", 'w')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  fstream file;
  size = loadfile.length(); // gets the size of string loadfile to convert to character
  counter = 0;
  if(index == true)
  {
	rc = bti.open(table, 'w');
	if(rc < 0)
	{
		fprintf(stderr, "Error while indexing.error returned with value :  %d  \n", rc);
		return rc;
	}
  }
  
  while(counter < size){
    file_name[counter] = loadfile[counter];
    counter++;
  }
  file_name[counter] = '\0';
  
  file.open(file_name); //needed the name to be character type pointer
  if(!file){
    fprintf(stderr, "Error openeing input stream");
    return RC_FILE_OPEN_FAILED;
  }
  //calling parse loadline to gives us the key value pair.
  getline(file,line_read);
  while(!file.eof())
  {
    
	rc = parseLoadLine(line_read, key, value);
    if(rc >= 0)
	{
		rf.append(key,value,rid);
	}
	else
	{
		fprintf(stderr, "Error:  Invalide file format. Error no %d  \n", rc);
		return rc;
	}
	
	if(key == 0)
	{
		// cout<<"0 detected"<<endl;
	}

	rc = bti.insert(key, rid);
	if(rc < 0)
	{
		fprintf(stderr, "Error. Insert in index failed with error value :  %d  \n", rc);
		return rc;
	}
	getline(file,line_read);
  }
  
  file.close();
  rf.close();
  //close index file
  if(index == true)
  {
	bti.close();  
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
