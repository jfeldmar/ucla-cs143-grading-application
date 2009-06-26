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
static bool flag = false; 
static BTreeIndex btree; 

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
 
  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }
  // If certain conditions with index load, use b+ tree.
    // Implement tree for all equality cases.
    BTLeafNode leafnode; 
    int searchKey; 
    IndexCursor cursor; 
    int key_read; 
    RecordId rid_read; 
    int rid_key; 
    string rid_value; 
    int error;
    int numkeys; 
    PageId next; 
    count = 0;
    IndexCursor first_cursor; 
    int key_value;
    int top;
    int bottom;  
    bool top_flag = false;
    bool bottom_flag = false;   
    IndexCursor top_cursor; 
    IndexCursor bottom_cursor;
    bool top_equal = false; 
    bool bottom_equal = false; 
    bool set = false;

    // Skip to full scan if searching by values. 
    for (unsigned i = 0; i < cond.size(); i++) {
      if (cond[i].attr == 2)
        goto full_scan; 
    }

    // If no conditions, use full scan.
    if (cond.size() == 0)
      goto full_scan; 

    // Skip to full scan if no .idx file exists (index was not turned on).
    // Try reading file, if error then doesn't exist. 

    // Open b+ tree since it exists. 
    if (btree.open(table + ".idx", 'r') != 0)
      goto full_scan;

      // Perform desired action depending on which comparator we use. 
      switch (cond[0].comp) {
      case SelCond::EQ:
        // Error if more than one equality statement. 
        for (int i = 1; i < cond.size(); i++){
          if (cond[i].comp != SelCond::NE)
            goto exit_select;     
        }

        searchKey = atoi(cond[0].value);

        // Traveling to and reading the correct value for the desired key. 
        error = btree.locate(searchKey, cursor);

        // Error if record does not exist.
        if (error != 0){
          fprintf(stderr, "Error: Record does not exist. \n");
          goto exit_select;         
        }            

        btree.readForward(cursor, key_read, rid_read); 

        // Does not print if key is greater than the searchKey and not equal.
        if(key_read != searchKey)
          goto print_count;  

        break;
      case SelCond::NE:
        // Full scan is faster for this one case. 
        if (cond.size() == 1)
          goto full_scan; 

        // Go through and see what others will be. If equal, do equal.
        for (int i=1; i < cond.size(); i++){
          if (cond[i].comp != SelCond::EQ)
            goto full_scan; 
          else{
            searchKey = atoi(cond[i].value);

            // Traveling to and reading the correct value for the desired key. 
            error = btree.locate(searchKey, cursor);
            btree.readForward(cursor, key_read, rid_read); 

            // Does not print if key is greater than the searchKey and not equal.
            if(key_read != searchKey)
              goto print_count;  
            
          }
        }

	break;
      case SelCond::GE:
      case SelCond::GT:
        searchKey = atoi(cond[0].value);

        // Error if more than one equality statement. 
        for (int i = 1; i < cond.size(); i++){
          // Equality condition. Exit. 
          if (cond[i].comp == SelCond::EQ)
            goto exit_select;

          // Not equal, do full scan.
          if (cond[i].comp == SelCond::NE)
            goto full_scan; 

          // Greater than, set old one to higher value.
          if (cond[i].comp == SelCond::GT){
            if (atoi(cond[i].value) > searchKey){
              searchKey = atoi(cond[i].value); 
              bottom_equal = false; 
            }
          }

          // Greater than or equal to, set to old one and print equal one.
          if (cond[i].comp == SelCond::GE){
            if (atoi(cond[i].value) > searchKey){
              searchKey = atoi(cond[i].value);
              bottom_equal = true;   
            }  
          }

          // Less than, check if range and execute.
          if (cond[i].comp == SelCond::LT){
            // If not a range, exit because not possible. 
            if (atoi(cond[i].value) <= (searchKey+1))
              goto print_count;  
            
            if (top_flag == true){
              if (atoi(cond[i].value) < top){
                top = atoi(cond[i].value);   
                top_equal = false;} 
            }else{    
              top = atoi(cond[i].value);
              top_equal = false;} 
  
            top_flag = true;
            set = true;  
          }
 
          // Less than or equal to, check if range and execute.
          if (cond[i].comp == SelCond::LE){
            // If not a range, exit because not possible. 
            if (atoi(cond[i].value) <= (searchKey+1))
              goto print_count;  
            
            if (top_flag == true){
              if (atoi(cond[i].value) < top){
                top = atoi(cond[i].value);   
                top_equal = true;} 
            }else{    
              top = atoi(cond[i].value);
              top_equal = true;}
  
            top_flag = true;
            set = false;  
          }
        }

        // Traveling to and reading the correct value for the desired key. 
        error = btree.locate(searchKey, cursor);
 
        // Error if record does not exist.
        if (error != 0){
          fprintf(stderr, "Error: Record does not exist. \n");
          goto exit_select;     
        }            

        // Read into a leaf node to read. 
        leafnode.read(cursor.pid, btree.pf); 

        // If equal, don't do this section. 
        if (cond[0].comp == SelCond::GT){
          leafnode.readEntry(cursor.eid, key_read, rid_read); 

          // Case where it is the only condition.
          if (searchKey == key_read){
            btree.readForward(cursor, key_read, rid_read);
            leafnode.read(cursor.pid, btree.pf); 
          }
        }

        numkeys = leafnode.getKeyCount(); 
        next = leafnode.leaf->getSiblingNode(); 

        // If range, stops at top value. 
        if (top_flag == true){
          error = btree.locate(top, top_cursor); 

          if (error == RC_NO_SUCH_RECORD)
            top_flag = false; 
        }

        // If no value between, will have same pid and eid values, output none.
        if ((cursor.eid == top_cursor.eid) && (cursor.pid == top_cursor.pid))
          goto print_count;  


        leafnode.read(cursor.pid, btree.pf); 
        numkeys = leafnode.getKeyCount(); 
        next = leafnode.leaf->getSiblingNode(); 

        // Loop for top flag. 
        if (top_flag == true){
          // While less than the last one, go through printing. 
          while ((cursor.pid <= top_cursor.pid) || (cursor.eid <= top_cursor.eid)){
            // If node is full, goes into next node. 
            if ((cursor.eid == (numkeys-1)) && (next != -1)){
                leafnode.read(next, btree.pf);
                cursor.pid = next; 
                cursor.eid = 0;
                next = leafnode.leaf->getSiblingNode(); 
            }
        
            error = btree.readForward(cursor, key_read, rid_read); 
       
            // Read into a RecordFile to read the values inside. 
            rf.read(rid_read, rid_key, rid_value); 
    
            // print the tuple 
            switch (attr) {
            case 1:  // SELECT key
              fprintf(stdout, "%d\n", rid_key);
              break;
            case 2:  // SELECT value
              fprintf(stdout, "%s\n", rid_value.c_str());
              break;
            case 3:  // SELECT *
              fprintf(stdout, "%d '%s'\n", rid_key, rid_value.c_str());
              break;
            }

            // If last value, prints and exits. 
            if ((cursor.eid == top_cursor.eid) && (cursor.pid == top_cursor.pid)){
              if (set == true) 
                goto print_count;
              else
                set = true;    
            }

            // Read into a leaf node to read. 
            leafnode.read(cursor.pid, btree.pf); 
            numkeys = leafnode.getKeyCount(); 

            count++; 
        }
          goto print_count; 
     }

        // While not pointing to NULL (-1), go through leafnodes printing. 
        while ((next != -1) || (cursor.eid < numkeys)){
          // If node is full, goes into next node. 
          if ((cursor.eid == (numkeys-1)) && (next != -1)){
              leafnode.read(next, btree.pf);
              cursor.pid = next; 
              cursor.eid = 0;
              next = leafnode.leaf->getSiblingNode(); 
          }
        
          error = btree.readForward(cursor, key_read, rid_read); 
       
          // Read into a RecordFile to read the values inside. 
          rf.read(rid_read, rid_key, rid_value); 
    
          // print the tuple 
         switch (attr) {
         case 1:  // SELECT key
           fprintf(stdout, "%d\n", rid_key);
           break;
         case 2:  // SELECT value
           fprintf(stdout, "%s\n", rid_value.c_str());
           break;
         case 3:  // SELECT *
           fprintf(stdout, "%d '%s'\n", rid_key, rid_value.c_str());
           break;
         }

          // Read into a leaf node to read. 
          leafnode.read(cursor.pid, btree.pf); 
          numkeys = leafnode.getKeyCount(); 

          count++; 

          // Test for error so does not repeat last entry.
          if (error == RC_NO_SUCH_RECORD)
            goto print_count; 
        } 

        goto print_count;                 
	break;
      case SelCond::LE:
      case SelCond::LT:
        searchKey = atoi(cond[0].value);

        if (cond[0].comp == SelCond::LE)
          set = false; 
        else
          set = true;  

        // Error if more than one equality statement. 
        for (int i = 1; i < cond.size(); i++){
          // Equality condition. Exit. 
          if (cond[i].comp == SelCond::EQ)
            goto exit_select;

          // Not equal, do full scan.
          if (cond[i].comp == SelCond::NE)
            goto full_scan; 

          // Less than, set old one to lower value.
          if (cond[i].comp == SelCond::LT){
            if (atoi(cond[i].value) < searchKey){
              searchKey = atoi(cond[i].value); 
              top_equal = false;
              set = true;   
            }
          }

          // Less than or equal to, set to old one and print equal one.
          if (cond[i].comp == SelCond::LE){
            if (atoi(cond[i].value) < searchKey){
              searchKey = atoi(cond[i].value);
              top_equal = true;
              set = false;     
            }  
          }

          // Greater than, check if range and execute.
          if (cond[i].comp == SelCond::GT){
            // If not a range, exit because not possible. 
            if (atoi(cond[i].value) >= (searchKey-1))
              goto print_count;  
            
            if (bottom_flag == true){
              if (atoi(cond[i].value) > bottom){
                bottom = atoi(cond[i].value);   
                bottom_equal = false;} 
            }else{    
              bottom = atoi(cond[i].value);
              bottom_equal = false;} 
  
            bottom_flag = true; 
          }
 
          // Greater than or equal to, check if range and execute.
          if (cond[i].comp == SelCond::GE){
            // If not a range, exit because not possible. 
            if (atoi(cond[i].value) >= (searchKey-1))
              goto print_count;  
            
            if (bottom_flag == true){
              if (atoi(cond[i].value) > bottom){
                bottom = atoi(cond[i].value);   
                bottom_equal = true;} 
            }else{    
              bottom = atoi(cond[i].value);
              bottom_equal = true;}
  
            bottom_flag = true; 
          }
        }

        // Traveling to and reading the correct value for the desired key. 
        error = btree.locate(searchKey, cursor);

        // Error if record does not exist.
        if (error == RC_NO_SUCH_RECORD){
          goto full_scan;     
        }            

        // Read into a leaf node to read. 
        leafnode.read(cursor.pid, btree.pf); 
        leafnode.readEntry(cursor.eid, key_read, rid_read); 

        // Case where it is the only condition.
        if (cursor.eid == 0){
          if (cond[0].comp == SelCond::LT)
            goto print_count;  
          else if (cond[0].comp == SelCond:: LE){
            error = btree.readForward(cursor, key_read, rid_read); 
       
            // Read into a RecordFile to read the values inside. 
            rf.read(rid_read, rid_key, rid_value); 
    
            // print the tuple 
            switch (attr) {
            case 1:  // SELECT key
              fprintf(stdout, "%d\n", rid_key);
              break;
            case 2:  // SELECT value
              fprintf(stdout, "%s\n", rid_value.c_str());
              break;
            case 3:  // SELECT *
              fprintf(stdout, "%d '%s'\n", rid_key, rid_value.c_str());
              break;
            }
            count++; 
            goto print_count;   
          }
        }    
        
        // Read from first place into leafnode. 
        btree.locate(-1000000, first_cursor);
        if (bottom_flag == true)
          btree.locate(bottom, first_cursor);
        leafnode.read(first_cursor.pid, btree.pf); 
        numkeys = leafnode.getKeyCount(); 
        next = leafnode.leaf->getSiblingNode(); 

        // While less than the last one, go through printing. 
        while ((first_cursor.pid <= cursor.pid) || (first_cursor.eid <= cursor.eid)){
          // If node is full, goes into next node. 
          if ((first_cursor.eid == (numkeys-1)) && (next != -1)){
              leafnode.read(next, btree.pf);
              first_cursor.pid = next; 
              first_cursor.eid = 0;
              next = leafnode.leaf->getSiblingNode(); 
          }
        
          error = btree.readForward(first_cursor, key_read, rid_read); 

          // Read into a RecordFile to read the values inside. 
          rf.read(rid_read, rid_key, rid_value); 
    
          // print the tuple 
         switch (attr) {
         case 1:  // SELECT key
           fprintf(stdout, "%d\n", rid_key);
           break;
         case 2:  // SELECT value
           fprintf(stdout, "%s\n", rid_value.c_str());
           break;
         case 3:  // SELECT *
           fprintf(stdout, "%d '%s'\n", rid_key, rid_value.c_str());
           break;
         }

          // If last value, prints and exits. 
          if ((first_cursor.eid == cursor.eid) && (first_cursor.pid == cursor.pid)){
              if (set == true) 
                goto print_count;
              else
                set = true;    
          }

          // Read into a leaf node to read. 
          leafnode.read(first_cursor.pid, btree.pf); 
          numkeys = leafnode.getKeyCount(); 

          count++; 

        } 

	break;
      }
    

    // the condition is met for the tuple. 
    // increase matching tuple counter
    count++;

    // Read into a RecordFile to read the values inside. 
    rf.read(rid_read, rid_key, rid_value); 
    
    // print the tuple 
    switch (attr) {
    case 1:  // SELECT key
      fprintf(stdout, "%d\n", rid_key);
      break;
    case 2:  // SELECT value
      fprintf(stdout, "%s\n", rid_value.c_str());
      break;
    case 3:  // SELECT *
      fprintf(stdout, "%d '%s'\n", rid_key, rid_value.c_str());
      break;
    }
 
//   }

  goto exit_select; 

  // Cases where we need a full scan and not use the tree.
  full_scan: 
  
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

  print_count: 
  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  exit_select:
  rf.close();
  btree.close(); 
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  RecordFile rf;  // Recordfile containing the table.
  RecordId rid;   // Record cursor for appending tuples into table.

  RC rc;
  int key;
  string value;
  string line;
  flag = false; 

  // If you can't open the table file, make a new one.
  rc = rf.open(table + ".tbl", 'w');

  // If index is set to true, create b+ tree index.
  if(index){
    flag = true; 
    btree.open(table + ".idx", 'w');   
  }

  // Open the given file.
  ifstream file;
  file.open(loadfile.c_str());

  while(getline(file, line))
  {
    //Parse the file and return the key and value.
    parseLoadLine(line, key, value);

    // Get Rid of last line in table.
    rid = rf.endRid();

    // Append to the end of table.
    rf.append(key, value, rid);

    // If index is set, insert rid, key pair into b+ tree.
    if(index)
      btree.insert(key, rid);  
  }
 
  file.close();
  if(index)
    btree.close(); 
  rf.close();
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
