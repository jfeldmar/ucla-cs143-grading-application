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
#include "BTreeNode.h"
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
  BTreeIndex bt;   // the index
  
  bool   btExist;

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    return rc;
  }

  // open the btreeindex
  if ((rc = bt.open(table + ".idx", 'r')) != 0)
  {
	btExist = false;
  }
  else 
	btExist = true;

  #pragma region//Implementation is based on the partition of the conditions

	  /* Order to put comps in with key values
	     =  >=  >  <  <=        |      <>
	  */

  vector<SelCond> partitionedKeyCond;
  vector<SelCond> valueCond;

  // First push the equality conditions into the data structure
  int i = 0;
  while (i < cond.size())
  {
	  if (cond[i].comp == SelCond::EQ && cond[i].attr == 1)
		partitionedKeyCond.push_back(cond[i]);

	  i++;
  }

  // Now push the >= conditions into the data structure
  i = 0;
  while (i < cond.size())
  {
	  if (cond[i].comp == SelCond::GE && cond[i].attr == 1)
		partitionedKeyCond.push_back(cond[i]);

	  i++;
  }

  // Now push the > conditions into the data structure
  i = 0;
  while (i < cond.size())
  {
	  if (cond[i].comp == SelCond::GT && cond[i].attr == 1)
		partitionedKeyCond.push_back(cond[i]);

	  i++;
  }

  // Now push the < conditions into the data structure
  i = 0;
  while (i < cond.size())
  {
	  if (cond[i].comp == SelCond::LT && cond[i].attr == 1)
		partitionedKeyCond.push_back(cond[i]);

	  i++;
  }

  // Now push the <= conditions into the data structure
  i = 0;
  while (i < cond.size())
  {
	  if (cond[i].comp == SelCond::LE && cond[i].attr == 1)
		partitionedKeyCond.push_back(cond[i]);

	  i++;
  }

  // Push the <> conditions into the data structure
  i = 0;
  while (i < cond.size())
  {
	  if (cond[i].comp == SelCond::NE && cond[i].attr == 1)
		partitionedKeyCond.push_back(cond[i]);

	  i++;
  }

  // Push all value attrs into the another data structure
  i = 0; 
  while (i < cond.size())
  {
	  if (cond[i].attr == 2)
		valueCond.push_back(cond[i]);

	  i++;
  }

  #pragma endregion

  #pragma region // Uncomment this if you want to see the conditions coming in

  /////*for (int k = 0; k < partitionedKeyCond.size(); k++)
  ////{
	 //// cout << "  VALUE[" << k << "] = " << partitionedKeyCond[k].value << "    ATTR: ";

	 //// if (partitionedKeyCond[k].attr == 1)
		////  cout << "key";
	 //// else if (partitionedKeyCond[k].attr == 2)
		////  cout << "value";

	 //// cout << "      COMP: ";

	 //// 
	 //// if (partitionedKeyCond[k].comp == 0)
		////  cout << "EQ";
	 //// else if (partitionedKeyCond[k].comp == 1)
		////  cout << "NE";
	 //// else if (partitionedKeyCond[k].comp == 2)
		////  cout << "LT";
	 //// else if (partitionedKeyCond[k].comp == 3)
		////  cout << "GT";
	 //// else if (partitionedKeyCond[k].comp == 4)
		////  cout << "LE";
	 //// else if (partitionedKeyCond[k].comp == 5)
		////  cout << "GE";

	 //// cout << endl;
  ////}*/

  #pragma endregion

// Figure out if we should use the BT Index or not, we will have to use the original thing eventually
 
  bool useBTIndex = true;

  // If BTIndex doesn't exist, use the original
  if (!btExist)
	  useBTIndex = false;
  
  // If no conditions, don't use index
  if (partitionedKeyCond.size() == 0)
	  useBTIndex = false;

  // If <> is the only condtion on the key attribute
  i = 0;
  while (i < partitionedKeyCond.size())
  {
	  if (partitionedKeyCond[0].attr == 1 && partitionedKeyCond[0].comp == SelCond::NE)	
		  useBTIndex = false;

	  if (true)
		  break;

	  i++;
  }

  if (useBTIndex)
  {

	switch(partitionedKeyCond[0].comp)
	{
	  case SelCond::EQ:
	  #pragma region //There is an EQ case that determines everything	

	  // If equal case we only check that the rest of the conditions don't violate
		  // Extract the key	  
		  key = atoi(partitionedKeyCond[0].value);
	
		  // check the rest of the conditions one by one
		  for (int j = 1; j <partitionedKeyCond.size(); j++)
		  {		
			  // compute the diff
			  diff = key - atoi(partitionedKeyCond[j].value);

			  switch (partitionedKeyCond[j].comp) 
			  {
				  case SelCond::EQ:
					  if (diff != 0)
						  break;
					  goto next_tuple;;
				  case SelCond::NE:
					  if (diff == 0) 
						  break;
					  goto next_tuple;;
				  case SelCond::GT:
					  if (diff <= 0) 
						  break;
					  goto next_tuple;;
				  case SelCond::LT:
					  if (diff >= 0) 
						  break;
					  goto next_tuple;;
				  case SelCond::GE:
					  if (diff < 0) 
						  break;
					  goto next_tuple;;
				  case SelCond::LE:
					  if (diff > 0) 
						  break;
					  goto next_tuple;;
			  };

			  // If we hit here then we know we messed up
			  goto exit_select;
				
			  // Other wise we keep checking
			  next_tuple:
			  ;
		  }

		  // Here we know that no other conditions will violate and we simply find this key
		  IndexCursor c;
		  key = atoi(partitionedKeyCond[0].value);
		  bt.locate(key, c);
		  
		  // If we have an invalid cursor, that means then we are done
		  if (c.eid == -1 && c.pid == -1)
			return RC_END_OF_TREE;
		  else
			rid = bt.ridFinder(c);

		  int keyread;
		  rf.read(rid, keyread, value);

		  // print the tuple if we ACTUALLY found something
		  if (key == keyread)
		  {
			  switch (attr) {
			  case 1:  // SELECT key
				fprintf(stdout, "%d\n", keyread);  
				break;
			  case 2:  // SELECT value
				fprintf(stdout, "%s\n", value.c_str()); 
				break;
			  case 3:  // SELECT *
				fprintf(stdout, "%d '%s'\n", keyread, value.c_str());
				break;
			  }
		  }

		  count = 1;

	  #pragma endregion
		  break;

	  case SelCond::GE:
	  case SelCond::GT:
	  case SelCond::LT:
	  case SelCond::LE:

	  #pragma region//For all comparators >, >= , <, <=  find all tuples with in range and push it into a new vector

		  int maxGT = (INT_MAX-5)*-1;
		  int maxGE = (INT_MAX-5)*-1;
		  int minLE = INT_MAX;
		  int minLT = INT_MAX;

		  // For >, find the max key value
		  int i = 0;
		  while (i < partitionedKeyCond.size())
		  {
			  if (partitionedKeyCond[i].comp == SelCond::GT && atoi(partitionedKeyCond[i].value) > maxGT)
				  maxGT = atoi(partitionedKeyCond[i].value);

			  i++;
		  }

		  // For >=, find the max key value
		  i = 0;
		  while (i < partitionedKeyCond.size())
		  {
			  if (partitionedKeyCond[i].comp == SelCond::GE && atoi(partitionedKeyCond[i].value) > maxGE)
				  maxGE = atoi(partitionedKeyCond[i].value);

			  i++;
		  }

		  // For <=, find the min key value
		  i = 0;
		  while (i < partitionedKeyCond.size())
		  {
			  if (partitionedKeyCond[i].comp == SelCond::LE && atoi(partitionedKeyCond[i].value) < minLE)
				  minLE = atoi(partitionedKeyCond[i].value);

			  i++;
		  }

		  // For <, find the min key value
		  i = 0;
		  while (i < partitionedKeyCond.size())
		  {
			  if (partitionedKeyCond[i].comp == SelCond::LT && atoi(partitionedKeyCond[i].value) < minLT)
				  minLT = atoi(partitionedKeyCond[i].value);

			  i++;
		  }

		  // Compute the range of our query
		  int rangeBegin = max(maxGT+1,maxGE);
		  int rangeEnd = min(minLE, minLT-1);

		  // Get the keys within the range and push it to be further checked if there is also no <> on it
		  bt.locate(rangeBegin, c);

		  // If we have an invalid cursor, that means then we are done
		  if (c.eid == -1 && c.pid == -1)
			return RC_END_OF_TREE;
		  else
			rid = bt.ridFinder(c);

		  // Store rids to be later checked
		  vector<RecordId> ridsToCheck;

		  //////// Extract key, value pairs
		  //////rf.read(rid, keyread, value);
		  //////if (keyread > rangeEnd)
			 ////// return 0;
		  //////else
		  //////{
			 ////// // push onto data structure if not part of the <>
			 ////// i = 0; 
			 ////// bool pushit = true;
			 ////// while (i < partitionedKeyCond.size())
			 ////// {
				//////  if (partitionedKeyCond[i].comp == SelCond::NE && atoi(partitionedKeyCond[i].value) == key)
				//////	  pushit = false;

				//////  i++;
			 ////// }

			 ////// if (pushit == true)
				//////ridsToCheck.push_back(rid);
		  //////}

		  // Continue moving along bottom of tree extracting values
		  while(bt.readForward(c, keyread, rid) == 0)
		  { 
			  //cout << "RANGE is     " << rangeBegin << "  -  " << rangeEnd << "|||| Last Key read, rid: " << keyread << " - " << rid.pid << ", " << rid.sid << endl;

			  if (keyread > rangeEnd)
				break;

			  else  
			  {
				  // push onto data structure if not part of the <>
				  i = 0; 
				  bool pushit = true;
				  while (i < partitionedKeyCond.size())
				  {
					  if (partitionedKeyCond[i].comp == SelCond::NE && atoi(partitionedKeyCond[i].value) == keyread)
						  pushit = false;

					  i++;
				  }

				  if (pushit == true)
					ridsToCheck.push_back(rid);
			  }
		  }

		  // Use old implementation to scan all rids
		  int curRid = 0;
		  count = 0;
		  while (curRid < ridsToCheck.size())
		  {
			  rid.pid = ridsToCheck[curRid].pid;
			  rid.sid = ridsToCheck[curRid].sid;
			  rf.read(rid, keyread, value);


			  // check the conditions on the tuple
			  for (unsigned i = 0; i < valueCond.size(); i++) {

				  // compute the difference between the tuple value and the condition value
				  switch (valueCond[i].attr) {
					case 1:
						diff = keyread - atoi(valueCond[i].value);
						break;
					case 2:
						diff = strcmp(value.c_str(), valueCond[i].value);
						break;
				  }

				  // skip the tuple if any valueCondition is not met
				  switch (valueCond[i].comp) {
					  case SelCond::EQ:
						  if (diff != 0) goto next_tuple_value_inside_Index;
						  break;
					  case SelCond::NE:
						  if (diff == 0) goto next_tuple_value_inside_Index;
						  break;
					  case SelCond::GT:
						  if (diff <= 0) goto next_tuple_value_inside_Index;
						  break;
					  case SelCond::LT:
						  if (diff >= 0) goto next_tuple_value_inside_Index;
						  break;
					  case SelCond::GE:
						  if (diff < 0) goto next_tuple_value_inside_Index;
						  break;
					  case SelCond::LE:
						  if (diff > 0) goto next_tuple_value_inside_Index;
						  break;
				  }
			  }

			  // the condition is met for the tuple. 
			  // increase matching tuple counter
			  count++;

			  // print the tuple 
			  switch (attr) {
				case 1:  // SELECT key
					fprintf(stdout, "%d\n", keyread);  
					break;
				case 2:  // SELECT value
					fprintf(stdout, "%s\n", value.c_str());
					break;
				case 3:  // SELECT *
					fprintf(stdout, "%d '%s'\n", keyread, value.c_str());
					break;
			  }

			  // move to the next tuple
			  next_tuple_value_inside_Index:
			  ++curRid;
		  }
			  
      #pragma endregion

		  break;
	};
	  
  }
  else
  {
	  #pragma region // Do the old implementation

	  // scan the table file from the beginning
	  rid.pid = rid.sid = 0;
	  count = 0;
	  while (rid < rf.endRid()) 
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
		if (diff != 0) goto next_tuple_value;
		break;
		  case SelCond::NE:
		if (diff == 0) goto next_tuple_value;
		break;
		  case SelCond::GT:
		if (diff <= 0) goto next_tuple_value;
		break;
		  case SelCond::LT:
		if (diff >= 0) goto next_tuple_value;
		break;
		  case SelCond::GE:
		if (diff < 0) goto next_tuple_value;
		break;
		  case SelCond::LE:
		if (diff > 0) goto next_tuple_value;
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
		next_tuple_value:
		++rid;
	  }// end of while (rid < rf.endRid()) 

	  #pragma endregion
  }


  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;


  // We are done!
	// close the table file,index, and return
  exit_select:

  rf.close();

  if(btExist)
	bt.close();

  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	/* your code here */

	RecordFile rf;   // RecordFile containing the table
    RecordId   rid;  // record cursor for table scanning
	RC rc;
	BTreeIndex bt;	// btree index used to store index
	RC key;
	string value;

	// open create record file
	if ((rc = rf.open(table + ".tbl",'w')) != 0)
	{
		fprintf(stderr, "Error: Problem opening/creating file\n");
		return RC_FILE_OPEN_FAILED;
	}

	// open create load file
	const char * inputfile = loadfile.c_str();
	ifstream infile;
	infile.open(inputfile, fstream::in);

	if (!infile)
	{
		fprintf(stderr,"Error: Could not open infile\n");
		return RC_FILE_OPEN_FAILED;
	}

	// open create index 
	if (index == true)
	{
		//if (bt.open(table + ".idx", 'r') != 0)
		//{
			if (bt.open(table + ".idx", 'w') != 0)
			{
				fprintf(stderr,"Error: Could not open index\n");
				return RC_FILE_OPEN_FAILED;
			}
		//}
	}

	string s2;
	RC parseval;
	while(getline(infile,s2))
	{	
		//fprintf(stdout,"Read line: %s\n",s2.c_str());
		if(parseLoadLine(s2, key, value)==0) // 0 means no error
		{
			//fprintf(stdout, "key: %d, value: %s\n",key,value.c_str());
			rf.append(key,value,rid);
			if (index == true)
				bt.insert(key,rid);
		}
		else
		{
			fprintf(stderr, "Error: Could not parse line\n");
			exit(1);
		}
	}
	
	infile.close();

	if (rf.close() != 0)
	{
		fprintf(stderr,"Error: could not close recordfile");
		return RC_FILE_CLOSE_FAILED;
	}

	if (index == true)
	{
		//bt.printEntireTree(bt.getRoot(),1);
		if (bt.close() != 0)
		{
			fprintf(stderr,"Error: could not close indexfile");
			return RC_FILE_CLOSE_FAILED;
		}
	}

	/* end of my code */
  return 0;
}



////////RC SqlEngine::load(const string& table, const string& loadfile, bool index)
////////{
////////  	RecordFile rf;   			// RecordFile containing the table
////////    RecordId   rid;			    // record cursor for table scanning
////////	BTreeIndex bt;
////////
////////	ifstream   lf;				// Load File 
////////
////////	int        rc;
////////	int        key;     
////////	string     value;
////////	int        count;
////////	int        diff;
////////	char	   buffer[256];
////////
////////	// The load should roughly take the following steps.
////////  	// 1. open the load file
////////	lf.open(loadfile.data(),ios::in);	
////////	if (!lf.is_open())
////////		return RC_FILE_OPEN_FAILED;
////////
////////  	// 2. open/create the RecordFile for the table
////////	if ((rc = rf.open(table +  ".tbl", 'r')) != 0) 
////////	{
////////		rf.open(table +  ".tbl", 'w');
////////	}
////////	else 
////////		return RC_FILE_OPEN_FAILED;
////////
////////	// Create index if index parameter is true
////////	if (index == true)
////////	{
////////		if (bt.open(table +  ".idx", 'w') != 0);
////////	}
////////
////////	// 3. For each line read from the load file
////////	//    a. Parse the line to get the key and value pair
////////	//       (parseLoadLine() can be used for this parsing step)
////////	while (lf.getline(buffer,256))
////////	{
////////		parseLoadLine(buffer, key, value);
////////		
////////  //    b. Insert the pair to the RecordFil
////////  //    Find the end first since rf.end() returns a const
////////
////////		// scan the table file from the beginning
////////		rid=rf.endRid();
////////
////////		// append
////////		rf.append(key,value,rid);
////////
////////		if (index == true)
////////		{
////////			bt.insert(key,rid);
////////		//cout << "                                                    ROOT PID = " << bt.getRoot() << endl;
////////		//	bt.printEntireTree(bt.getRoot(), 1);
////////		}
////////	}
////////
////////
////////	// 4. Close the load file and RecordFile
////////	   // close the table file and return
////////
////////  	if (rf.close() != 0)
////////		return RC_FILE_CLOSE_FAILED;
////////		
////////	lf.close();
////////
////////	bt.printEntireTree(bt.getRoot(), 1);
////////
////////	if (index == true)
////////	{
////////		if(bt.close() != 0)
////////		{
////////			return RC_FILE_CLOSE_FAILED;
////////		}
////////	}
////////
////////
////////  	return 0;
////////}


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
