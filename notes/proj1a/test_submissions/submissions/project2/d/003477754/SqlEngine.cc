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
#include "BTreeNode.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);

//===== Proxy functions =====

void SqlEngine::print_tuple(int attr, int key, string value)
{
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

bool check_value_cond(string cur_keyValue, string l_vBound, string u_vBound, string valueEqual, bool isLbEq, bool isUbEq)
{
	//if there is an equality condition on value
	if( strcmp( valueEqual.c_str(), "" ) != 0 )  
	{
		//if cur_keyValue is not equal to valueEqual
		if(strcmp( valueEqual.c_str(), cur_keyValue.c_str()) != 0)		
			return false;
	}
	//if we are looking for ranges
	else
	{
		int compResult;
		
		//if lowerBound exists,
		if( (strcmp( l_vBound.c_str(), "" ) != 0 ) )
		{  
			compResult = strcmp( cur_keyValue.c_str(), l_vBound.c_str() );
			//lowerBound condition is >=
			if( isLbEq && (compResult < 0) )
				return false;
			//lowerBound condition is > 
			else if ( !isLbEq && (compResult <= 0) )
				return false;
		}
		
		//if upperBound exists
		if( (strcmp( u_vBound.c_str(), "" ) != 0 ) ) 
		{
			compResult = strcmp( cur_keyValue.c_str(), u_vBound.c_str() );
			//upperBound condition is <=
			if( isUbEq && (compResult > 0 ) )
				return false;
			//upperBound condition is <
			else if ( !isUbEq && (compResult >= 0) )
				return false;
		}
	}
	
	return true;
}

//===== End Proxy functions =====

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
	BTreeIndex bti;  // BTreeIndex object
	RecordId   rid;  // record cursor for table scanning

	RC     	rc;
	int    	key, count, diff;     
	string 	value;
	bool	isIndex = false;
	
	//Two arrays to hold the <> on keys and values
	int 	notKey[20];
	string 	notValue[20];
	int     notKeyCounter = 0;
	int 	notValueCounter = 0;
	
	// open the table file
	if ((rc = rf.open(table + ".tbl", 'r')) < 0) 
	{	
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	}
    
	//check if btreeIndex exists 
	if( ((rc = bti.open(table, 'r')) == 0) )
	{
		//cout<<"index found "<<endl;	
		isIndex = true;
	}
	//===== Set up ends =====
  
	//===== Begin logic =====
	//set doIndex true if at least one of the condition check is on key 
	bool 	doIndex 	= false;
	bool 	isEmptySet 	= false;
	
	//variables for ranges with key
	int 	l_bound		= -1;
	int		u_bound		= -1;
	bool 	isLB		= false;
	bool 	isUB		= false;
	int		keyEqual 	= -1;
	int 	cond_value;
	
	//variables for ranges with value
	string  l_vBound  		= "";
	string  u_vBound    	= "";
	string	valueEqual	   	= "";
	string	cond_value2		= "";
	bool	isLbEq			= false;
	bool	isUbEq			= false;
	bool	haveAttrValue 	= false;
	bool 	printValid		= true;
	bool	printTuple		= true;
	SelCond	value_condition;
	
	//check conditions with BTreeIndex existing
	for( int i = 0 ; i < cond.size(); i++ )
	{
		//attribute to check is key
		if(cond[i].attr == 1)
		{
			cond_value = atoi(cond[i].value);
			
			//if condition is > or >=, replace with higher 
			//if condition is < or <=, replace with lower
			switch (cond[i].comp) 
			{
				case SelCond::EQ:
					//first encounter of keyEqual
					if(keyEqual == -1) 
					{
						keyEqual = cond_value; 
						doIndex  = true;
					}
					//if a second keyEqual condition is found with different key value then emptySet is true
					else if( cond_value != keyEqual) 
					{
						isEmptySet = true;  
					}
					break;
				case SelCond::GT:
					if ( !isLB || (l_bound < (cond_value + 1)) )
					{	
						l_bound = cond_value + 1;
						isLB 	= true;
						doIndex = true;
					}
					break;
				case SelCond::GE:
					if ( !isLB || (l_bound < cond_value ) )
					{
						l_bound = cond_value;
						isLB 	= true;
						doIndex = true;
					}
					break;
				case SelCond::LT:
					if ( !isUB || (u_bound > (cond_value - 1)) )
					{
						u_bound = cond_value - 1;
						isUB 	= true;
						doIndex = true;
					}
					break;
				case SelCond::LE:
					if ( !isUB || (u_bound > cond_value) )
					{
						u_bound = cond_value;
						isUB 	= true;
						doIndex = true;
					}
					break;
			}
		
			//doIndex = true;
		}
		//attribute to check is value
		else
		{
			haveAttrValue = true;
			cond_value2   = cond[i].value;
			
			//if condition is > or >=, replace with higher 
			//if condition is < or <=, replace with lower
			switch( cond[i].comp )
			{
				case SelCond::EQ:
					//first encounter of value comparator, set valueEqual
					if( strcmp( valueEqual.c_str(), "" ) == 0 )
					{ valueEqual = cond_value2; }
					//second encounter of value comparator, if value is not the same for equality, or value is the same but condition differs
					//this is an error and leads to empty set
					else if (  (strcmp( valueEqual.c_str(), cond_value2.c_str() ) !=0) )
					{ isEmptySet = true; }
					break;
				case SelCond::GT:
					if( ( strcmp( l_vBound.c_str(), "" ) == 0 ) || ( strcmp(l_vBound.c_str(), cond_value2.c_str() ) <= 0 ) )
					{
						l_vBound 	= cond_value2;
						isLbEq		= false;
					}
					break;
				case SelCond::GE:
					if( ( strcmp( l_vBound.c_str(), "" ) == 0 ) || ( strcmp(l_vBound.c_str(), cond_value2.c_str() ) < 0 ) )
					{
						l_vBound 	= cond_value2;
						isLbEq		= true;
					}
					break;
				case SelCond::LT:
					if( ( strcmp( u_vBound.c_str(), "" ) == 0 ) || ( strcmp(u_vBound.c_str(), cond_value2.c_str() ) >= 0 ) )
					{
						u_vBound 	= cond_value2;
						isUbEq 		= false;
					}
					break;
				case SelCond::LE:
					if( ( strcmp( u_vBound.c_str(), "" ) == 0 ) || ( strcmp(u_vBound.c_str(), cond_value2.c_str() ) > 0 ) )
					{
						u_vBound 	= cond_value2;
						isUbEq 		= true;
					}
					break;			
			}	
		
		}
		
		//if a NE condition
		if(cond[i].comp == SelCond::NE)
		{
			//doIndex = false;
			
			if(cond[i].attr == 1)
			{
				notKey[notKeyCounter] = atoi(cond[i].value);
				notKeyCounter++;
			}
			else if(cond[i].attr == 2)
			{
				notValue[notValueCounter] = cond[i].value;
				notValueCounter++;
			}
		}
	}
	
	//cout<<"l_bound: " << l_bound << "\t u_bound: " << u_bound << "\t keyEqual: " << keyEqual << endl;
	
	if(haveAttrValue)
	{
		//cout<<"l_vBound: " << l_vBound << "\t u_vBound: " << u_vBound << "\t valueEqual: " << valueEqual << endl;
		//cout<<"isLbEq: " << isLbEq << "\t isUbEq: " << isUbEq << endl;
	}

	//===== start check for emptySet =====
	
	//if bounds for key are not proper, then the query returns empty set
	if( (isUB) && (isLB) && (u_bound < l_bound) )
	{ isEmptySet = true; }
	//if equality exist for key
	else if( keyEqual != -1 )
	{
		//if lowerBound exists, and keyEqual is less than lowerBound
		if( (isLB) && (keyEqual < l_bound) )
		{ isEmptySet = true; }
		//if upperBound exists, and keyEqual is greater than upperBound
		else if ( (isUB) && (keyEqual > u_bound) ) 
		{ isEmptySet = true; }
		//if notEqual exist on the same key as equal
		for(int i = 0; i < notKeyCounter; i++)
		{
			if(notKey[i]==keyEqual)
			{
				isEmptySet = true;
				i = notKeyCounter;
			}
		}
		
	}
	//if bounds for key exist
	else if( (strcmp( l_vBound.c_str(), "" ) != 0 ) && (strcmp( u_vBound.c_str(), "" ) != 0 ) ) 
	{ 
		//if upperBound = lowerBound and condition has no = sign, bound is invalid, return empty set
		if( !isLbEq && !isUbEq && (strcmp( l_vBound.c_str(), u_vBound.c_str() ) == 0 ) )
			isEmptySet = true; 
	}
	//if equality exist for value
	else if( (strcmp( valueEqual.c_str(), "" ) != 0 )  )
	{
		//if lowerBound exists, and valueEqual is less than lowerBound
		if( (strcmp( l_vBound.c_str(), "" ) != 0 ) && (strcmp( valueEqual.c_str(), l_vBound.c_str() ) < 0 )  )
		{ isEmptySet = true; }
		//if upperBound exists, and valueEqual is greater than upperBound
		else if( (strcmp( u_vBound.c_str(), "" ) != 0 ) && (strcmp( valueEqual.c_str(), u_vBound.c_str() ) > 0 ) ) 
		{ isEmptySet = true; }
		//if notEqual exist on the same value as equal
		for(int i = 0; i < notValueCounter; i++)
		{
			if( strcmp( notValue[i].c_str(), valueEqual.c_str() ) == 0 )
			{
				isEmptySet = true;
				i = notValueCounter;
			}
		}
		
	}
	//check for bounds with value later
	//===== end check for emptySet =====
	
	if(isEmptySet)
	{
		//cout<<"empty set"<<endl;
		if (attr == 4) 
		{ fprintf(stdout, "%d\n", 0); }
		goto exit_select;
	}
	
	if(doIndex && isIndex)
	{
		IndexCursor	cursor;
		RecordId	rid;
		count = 0;
			
		//if keyEqual != -1 then print the tuple that meets the equal on key 
		if( keyEqual != -1 )
		{
			bti.locate(keyEqual, cursor);
			bti.readForward(cursor, key, rid);
			rf.read(rid, key, value);
			
			//IMPORTANT!! Check if the key returned is not equal to key searched, this should not be printed under equality
			if(keyEqual != key)
			{
				//cout<<"keyEqual != key" <<endl;
			}
			//If value attribute condition does not exist, or the value attribute condition matches 
			else
			{
				//TODO check notKey --> is it really necessary?
				
			
				//if a condition on value exists
				if(haveAttrValue)
				{
					printTuple 	= check_value_cond(value, l_vBound, u_vBound, valueEqual, isLbEq, isUbEq);	
								
					//Check for inequality 
					for( int i = 0 ; i < notValueCounter ; i++)
					{
						if( strcmp( notValue[i].c_str(), value.c_str() ) == 0)
						{ 
							printTuple = false; 
							i = notValueCounter;
						}
					}
				}

				if(printTuple && printValid)
				{
					if(attr != 4) { print_tuple(attr, key, value); }
					else          { count++; }
				}
				
				printValid 	= true;
				printTuple 	= true;
			}
				
			if (attr == 4) 
			{ fprintf(stdout, "%d\n", count); }	
		}
		
		//if there is a range
		else
		{
			//decide which key to locate
			//if lower bound does not exist or is less than 0
			if( (!isLB) || (l_bound < 0) ) 
			{ 
				cursor.pid = 1;
				cursor.eid = 0;
				//key = 0; 
			}
			else  	  
			{ 
				//key = l_bound;
				//locate the first key
				bti.locate(l_bound, cursor);
			}
						
			//go until the end or upperBound
			while( bti.readForward(cursor, key, rid) == 0 )
			{
				//if upperBound does not exist or key is still within upperBound
				if( (!isUB) || (key <= u_bound) )
				{
					rf.read(rid, key, value);

					//Check inequality on key 
					for( int i = 0 ; i < notKeyCounter ; i++ )
					{
						if( notKey[i] == key )
						{	
							printValid = false; 
							i = notKeyCounter;
						}
					}
					
					//if a condition on value exists
					if(haveAttrValue && printValid)
					{
					
						printTuple 	= check_value_cond(value, l_vBound, u_vBound, valueEqual, isLbEq, isUbEq);	
								
						//Check for inequality 
						for( int i = 0 ; i < notValueCounter ; i++)
						{
							if( strcmp( notValue[i].c_str(), value.c_str() ) == 0)
							{ 
								printTuple = false; 
								i = notValueCounter;
							}
						}

					}
					
					//if query does not have value condition or passes value condition
					if(printTuple && printValid)
					{
						if(attr != 4) { print_tuple(attr, key, value); }
						else          { count++; }
					} 			
					
					printValid	= true;
					printTuple 	= true;
				}
	
			}
			
			if (attr == 4) 
			{ fprintf(stdout, "%d\n", count); }
		}
	
	}
	//When not using BTreeIndex, scan the whole table
	else
	{
		// scan the table file from the beginning
		rid.pid = rid.sid = 0;
		count = 0;
		while( rid < rf.endRid() ) 
		{
			// read the tuple
			if ((rc = rf.read(rid, key, value)) < 0) 
			{
				fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
				goto exit_select;
			}

			// check the conditions on the tuple
			for (unsigned i = 0; i < cond.size(); i++) 
			{
				// compute the difference between the tuple value and the condition value
				switch (cond[i].attr) 
				{
					case 1:
						diff = key - atoi(cond[i].value);
						break;
					case 2:	
						diff = strcmp(value.c_str(), cond[i].value);
						break;
				}

				// skip the tuple if any condition is not met
				switch (cond[i].comp) 
				{
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

			// move to the next tuple
		    next_tuple:
		    ++rid;
		}

		// print matching tuple count if "select count(*)"
		if (attr == 4) 
		{
			fprintf(stdout, "%d\n", count);
		}
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
 
	BTreeIndex bti;
	RecordFile currentRecord;
	RecordId   rid;
	RC         rc; 

	//create a RecordFile name 'table'
	if ((rc = currentRecord.open(table + ".tbl", 'w')) < 0)
	{
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	}
	
	// if index is true, open an index file
	if(index) 
	{ 
		if(( rc = bti.open(table ,'w')) < 0 ) 
		{
			fprintf(stderr, "Error: problem opening index %s \n", table.c_str());
			return rc;
		}  
	}
  
	fstream file;

	int strSize = loadfile.length();
	int temp = 0;

	char theFile[100]; 

	while(temp < strSize)
	{
		theFile[temp] = loadfile[temp];    
		temp++;
	}  

	theFile[temp] = '\0';
	file.open( theFile );
  
	if( !file ) 
	{
		fprintf(stderr, "Error opening input stream");
		return RC_FILE_OPEN_FAILED;
	}
  
	int    key;
	int 	 counter = 0;
	string value;
	string line;

	//extract a line from file
	while( getline(file,line) )
	{
		//extract (key,value) pair from the line
		parseLoadLine(line, key, value);  
		currentRecord.append(key, value, rid);
	
		//insert index
		if(index)
		{
			bti.insert(key, rid);
			counter++;
			//cout<<"inserting key: " << key << " : " << bti.insert(key, rid) << " treeHeight: " << bti.getTreeHeight() << endl;
		}
	}
  
	file.close();
	currentRecord.close();
   
	if(index)  
	{	
		//bti.print_nodes();
		//bti.test_readForward(0);
		bti.close();
		//cout << "there were : " << counter << " inserts" << endl;
		//cout << "end SqlEngine::load treeHeight is: " << bti.getTreeHeight() << endl;
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
