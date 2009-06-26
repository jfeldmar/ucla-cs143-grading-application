/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>, revised J. Su, R. Moy
 * @date 3/24/2008, revised 11-7-08
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"		//Added by J. Su on 11-7-08
#include <vector>
#include <algorithm>
#include <climits>
using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);
void printTuple(int attr, int key, string value);
const bool DEBUG = false;

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
	int    count = 0;
	int    diff;
	bool   index;

	// open the table file
	if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
		fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
		return rc;
	}

	//Loop through each condition
	//Separate into conditions that can use index or can't use index
	//Loop through each condition that can use index
	//	If first one
	//		Add its results to a vector of indexable results
	//	Else
	//		Intersect its results with existing vector of indexable results
	//Loop through each of the indexable results
	//	Loop through each condition that can't use index
	//		If it fails any condition
				//Discard this result
	//Print out remaining results
	BTreeIndex btIndex;
	rc = btIndex.open(table+".idx", 'r');
	
	index = !rc;		//if return code 0, index exists

	//Check whether index exists or not.
	if (index)
	{
		bool existsIndexCondition = false;
		bool existsEqualityCondition = false;
		bool existsInequalityCondition = false;
		bool inequalityChecked = false;
		int largest = INT_MIN;
		int smallest = INT_MAX;

		//Separate into conditions that can use index or can't use index
		vector<SelCond> indexConditions;
		vector<SelCond> nonIndexConditions;
		for (int i = 0; i < cond.size(); i++)
			if (cond.at(i).attr == 1)
				if (cond.at(i).comp != SelCond::NE)
				{
					indexConditions.push_back(cond.at(i));
					existsIndexCondition = true;
					if (cond.at(i).comp == SelCond::EQ)
					{
						existsEqualityCondition = true;
					}
					else
					{
						existsInequalityCondition = true;
						int searchKey = atoi(cond.at(i).value);

						if (cond.at(i).comp == SelCond::LT)
						{	if (searchKey - 1< smallest)
								smallest = searchKey-1;
						}
						else if(cond.at(i).comp == SelCond::LE)
						{
							if (searchKey < smallest)
								smallest = searchKey;
						}
						else if (cond.at(i).comp == SelCond::GT)
						{
							if (searchKey +1 > largest)
								largest = searchKey+1;
						}
						else if (cond.at(i).comp == SelCond::GE)
						{
							if (searchKey > largest)
								largest = searchKey;
						}
					}
				}
				else
					nonIndexConditions.push_back(cond.at(i));
			else
				nonIndexConditions.push_back(cond.at(i));

		//Loop through each condition that can use index
		//Note: RecordId may have some unnecessary reads,
		//but I think necessary for GT, GE, LT, LE
		unsigned int numConditionsCounted = 0;
		vector<RecordId> indexableResults;
		for (int i = 0; i < indexConditions.size(); i++)
		{
			vector<RecordId> possibleResults;
			SelCond sc = indexConditions.at(i);
			IndexCursor ic;
			int searchKey = atoi(sc.value);

			//Add the results to the vector possibleResults
			if (sc.comp == SelCond::EQ)
			{
				++numConditionsCounted;
				if (searchKey >= largest && searchKey <= smallest)
				{
					//Locate the entry
					rc = btIndex.locate(searchKey, ic);
					if (rc != RC_NO_SUCH_RECORD)
					{
						
						if (rc != 0) {
							//fprintf(stderr, "Error %d in SqlEngine::select.  BTreeIndex::locate failed.", rc);
							goto exit_select;
						}

						//Read the entry
						if (ic.pid != RC_END_OF_TREE)
							rc = btIndex.readForward(ic, key, rid);
						else
							rc = RC_NO_SUCH_RECORD;
						
						//if (rc != 0) {
						//	fprintf(stderr, "Error %d in SqlEngine::select.  BTreeIndex::readForward failed.", rc);
						//	goto exit_select;
						//}

						if (rc == 0 && key==searchKey)
							possibleResults.push_back(rid);
					}
				}
			}
			else if (!existsEqualityCondition && !inequalityChecked)
			{
				++numConditionsCounted;
				inequalityChecked = true;
				//locate first entry in the range
				rc = btIndex.locate(largest, ic);
				
				if (rc != RC_NO_SUCH_RECORD)
				{

					if (rc != 0)
					{
						//fprintf(stderr, "Error %d in SqlEngine::select.  BTreeIndex::locate failed.", rc);
						goto exit_select;
					}

					// Read from the lower entry to the upper entry
					if (ic.pid != RC_END_OF_TREE)
						rc = btIndex.readForward(ic, key, rid);
					else
						rc = RC_NO_SUCH_RECORD;

					while (rc == 0 && key <= smallest)
					{
						possibleResults.push_back(rid);
						if (ic.pid != RC_END_OF_TREE)
							rc = btIndex.readForward(ic, key, rid);
						else
							break;
					}
				}
			}
			else
				continue;


			//Intersect possibleResults with indexableResults
			//if (i == 0)
			if (numConditionsCounted <= 1)
			{
				indexableResults = possibleResults;
			}
			else
			{
				//Size wouldn't exceed our current size
				vector<RecordId> newResults(indexableResults.size());
				vector<RecordId>::iterator endIt;

				//indexableResults may already be sorted... but... just to make sure.
				sort(indexableResults.begin(), indexableResults.end());
				sort(possibleResults.begin(), possibleResults.end());
				endIt = set_intersection(indexableResults.begin(), indexableResults.end(), possibleResults.begin(), possibleResults.end(), newResults.begin());
				//endIt points to the last element in newResults
				indexableResults.assign(newResults.begin(), endIt);
				
			}
		}

		//If no indexConditions
			//Load indexableResults with all RecordIds

		//if (indexConditions.size() == 0)
		if (!existsIndexCondition)
		{
			// scan the table file from the beginning
			rid.pid = rid.sid = 0;
			//	count = 0;
			while (rid < rf.endRid()) {
				indexableResults.push_back(rid);
				rid++;
			}
		}

		count = 0;
		//Loop through each of the possibleRecords
			//Loop through each condition that can't use index
				//If it fails any condition
					//Discard this result
		for (int i = 0; i < indexableResults.size(); i++)
		{
			rc = rf.read(indexableResults.at(i), key, value);
			if (rc < 0) {
				fprintf(stderr, "Error %d: while reading a tuple from table %s\n", rc, table.c_str());
				fprintf(stderr, "\trid.pid: %d, rid.sid: %d\n", indexableResults.at(i).pid, indexableResults.at(i).sid);
				goto exit_select;
			}
			
			bool passedConditions = true;
			for (int j = 0; passedConditions && j < nonIndexConditions.size(); j++)
			{
				switch (nonIndexConditions[j].attr) {
					case 1:
						diff = key - atoi(nonIndexConditions[j].value);
						break;
					case 2:
						diff = strcmp(value.c_str(), nonIndexConditions[j].value);
						break;
				}

				// skip the tuple if any condition is not met
				switch (nonIndexConditions[j].comp) {
					case SelCond::EQ:
						if (diff != 0)
							passedConditions = false;
						break;
					case SelCond::NE:
						if (diff == 0)
							passedConditions = false;
						break;
					case SelCond::GT:
						if (diff <= 0)
							passedConditions = false;
						break;
					case SelCond::LT:
						if (diff >= 0)
							passedConditions = false;
						break;
					case SelCond::GE:
						if (diff < 0)
							passedConditions = false;
						break;
					case SelCond::LE:
						if (diff > 0)
							passedConditions = false;
						break;
				}
			}

			// if the condition is met for the tuple. 
			// increase matching tuple counter
			if (passedConditions)
			{
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
			}
		}
	}
	else
	{
		//Original implementation
		// scan the table file from the beginning
		rid.pid = rid.sid = 0;
	//	count = 0;
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
	if (index)
		btIndex.close();
	return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
	BTreeIndex btIndex;	//This is supposed to initialize a static member variable
	fstream fidIn(loadfile.c_str(), fstream::in);
	if (!fidIn) {
		cerr << "Error: RC_FILE_OPEN_FAILED in SqlEngine::load" << endl;
		return RC_FILE_OPEN_FAILED;
	}
	
	RecordFile rf;   // RecordFile containing the table
	string line;
	RC rc;		//Return code
	
	// create the table file
	if ((rc = rf.open(table + ".tbl", 'w')) != 0) {
		fprintf(stderr, "Error %d: Cannot create %s.tbl\n", rc, table.c_str());
		return rc;
	}

	// create the index file
	if (index)
	{
		rc = btIndex.open(table + ".idx", 'w');
		if (rc != 0) {
			fprintf(stderr, "Error %d: Cannot create %s.idx\n", rc, table.c_str());
			return rc;
		}
	}

	while (getline(fidIn, line)) {
		if (DEBUG)
			cout << line << endl;

		int key;
		string value;
		RecordId ri = rf.endRid();				//set recordId to the end of the RecordFile
		rc = parseLoadLine(line, key, value);
		if (rc) {
			cerr << "Error " << rc <<": parseLoadLine failed in SqlEngine::load" << endl;
			return rc;
		}

		//key, value has the data from line
		if (DEBUG)
			cerr << "key: " << key << "\tValue: " << value << endl;

		if(rc = rf.append(key, value, ri) != 0)			// Insert key, value into the sequential file
		{
			cerr << "Error: Cannot append a value (" << key << ", " << value << ")\n";	
			return rc; 
		}
		
		//Check index
		if (index)	{
			//Project 2D
			btIndex.insert(key, ri);
		}
	}
	fidIn.close();

	if (fidIn)
	{
		cerr << "Error " << RC_FILE_CLOSE_FAILED << ": in SqlEngine::Load" << endl;
		return RC_FILE_CLOSE_FAILED;
	}
	
	//close the table
	if (rc = rf.close() != 0) {
		fprintf(stderr, "Error: Cannot close %s.tbl\n", table.c_str());
		return rc;
	}
	if (index) {
		//btIndex.printNode(3, false);
		if (rc = btIndex.close() != 0) {
			fprintf(stderr, "Error: Cannot close %s.idx\n", table.c_str());
			return rc;
		}
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

void printTuple(int attr, int key, string value)
{
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
}
