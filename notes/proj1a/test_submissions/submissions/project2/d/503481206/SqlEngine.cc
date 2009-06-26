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
/*bool bruinbase_silent;*/


RC SqlEngine::run(FILE* commandline/*, bool silent*/)
{
  /*if (!(bruinbase_silent = silent)) // bruinbase_silent used by sqlparse()*/
    fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  gene2printf("in SqlEngine::select(attr==%d, table==%s, cond (sizeis %d))\n",attr,table.c_str(),cond.size());
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

  RC     rc;
  int    key;     
  string value;
  int    count = 0;
  //int    diff;
  long long diff;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  // Does it make sense to use an index?
  // WHERE clause has condition using a key(s)? Assume cond.size() <= INT_MAX+1
  bool consult_index = false;
  for (int z = cond.size()-1; z >= 0 && !consult_index; --z) {
    if (cond[z].attr == 1 // condition uses key
        && cond[z].comp != SelCond::NE) // and is not inequality comparison
      consult_index = true;
  }
  gene2printf("consult_index is %s\n",consult_index?"true":"false");

/******************************************************************************/
  BTreeIndex index;
  // If we want to use index, and there exists one. Else flag consult_index
#if debug==1
  if (!FORCE_NO_INDEX &&
      consult_index && index.open(table, 'r') >= 0 || (consult_index = false)) {
#else
  if (consult_index && index.open(table, 'r') >= 0 || (consult_index = false)) {
#endif
    gene2printf("SqlEngine::select()\tindex opened\n");
    /**
     * Start by pruning conditions (see below for algorithm). Pruning should 
     * result in 1 equal-to condition, or in any number of not-equal-to and at
     * most 1 each of greater|lesser[or equal]-than conditions. 
     *
     * Pruning assumes: 
     *  1) all conditions are ANDed together
     *  2) there are no duplicate keys in index.
     * -EXAMPLE CASES:----------------------------------------------------------
     * if key >  a and key >  b and a >  b, then ignore key >  b condition
     * if key <= a and key <= b and a <= b, then ignore key <= b condition
     * if key =  a and key =  b and a <> b, then ignore ALL      conditions
     * -DIAGRAM:----------------------------------------------------------------
     * for '>', '>=': greater value wins. if equal, the noninclusive one wins
     *       b   a                              a wins
     *  -----[---(--------------------------    (
     *           a  b                           b wins
     *  ---------(--[-----------------------    [
     *           ab together                    a ; noninclusive wins
     *  ---------([-------------------------    (
     * for '<', '<=': lesser value wins. if equal, noninclusive one wins
     *       b   a                              b wins
     *  -----]---)--------------------------    ]
     *           a  b                           a wins
     *  ---------)--]-----------------------    )
     *         ab together                      a ; noninclusive wins
     *  -------)]---------------------------    )
     *
     * It is more practical to identify which condition to use in 'cond' than
     * to actually remove it from the vector. It is const anyways.
     */
    // Go through 'cond' once to find the "best" condition(s) for index lookup
    //  First find greatest value for '>' and '>=', least value for '<' and '<='
    //  Also detect when result set will be empty. (AND => INTERSECT operator)
    int eq = -1, ge = -1, gt = -1, le = -1, lt = -1; // indices to cond
    for (int z = cond.size()-1; z >= 0; --z) {
      if (cond[z].attr != 1)
        continue; // Skip non key conditions
      key = atoi(cond[z].value); // current key value

      switch (cond[z].comp) {
        case SelCond::EQ:
          if (eq >= 0 && key != atoi(cond[eq].value)
              || ge >= 0 && key <  atoi(cond[ge].value)
              || gt >= 0 && key <= atoi(cond[gt].value)
              || le >= 0 && key >  atoi(cond[le].value)
              || lt >= 0 && key >= atoi(cond[lt].value)) goto soft_exit_select;
          else eq = z;
          break;
        case SelCond::NE:
          if (eq >= 0 && key == atoi(cond[eq].value)) goto soft_exit_select;
          break;
        case SelCond::GE:
          if (eq >= 0 && key >  atoi(cond[eq].value)) goto soft_exit_select;
          if (ge <  0 || key >  atoi(cond[ge].value)) // greater value wins
            ge = z;
          break;
        case SelCond::GT:
          if (eq >= 0 && key >= atoi(cond[eq].value)) goto soft_exit_select;
          if (gt <  0 || key >  atoi(cond[gt].value)) // greater value wins
            gt = z;
          break;
        case SelCond::LE:
          if (eq >= 0 && key <  atoi(cond[eq].value)) goto soft_exit_select;
          if (le <  0 || key <  atoi(cond[le].value)) // lesser value wins
            le = z;
          break;
        case SelCond::LT:
          if (eq >= 0 && key <= atoi(cond[eq].value)) goto soft_exit_select;
          if (lt <  0 || key <  atoi(cond[lt].value)) // lesser value wins
            lt = z;
          break;
      }
    } // end scan to 'cond'

    int lo = -1, hi = -1; // indices to 'cond' for ranged queries' limits
    // Find lower bound
    if (ge >= 0 && gt >= 0)
      lo = atoi(cond[ge].value) > atoi(cond[gt].value) ? ge : gt;
    else if ((lo = ge) < 0)
      lo = gt; // might still be negative
    // Find upper bound
    if (le >= 0 && lt >= 0)
      hi = atoi(cond[le].value) < atoi(cond[lt].value) ? le : lt;
    else if ((hi = le) < 0)
      hi = lt; // might still be negative

    /**
     * Now apply precedence: '=' first then '>[=]' then '<[=]'
     * Algorithm as follows:
     * e.g. '>='
     * For each tuple that matches '>=', (read using readForward)
     *     check for the conditions that do not utilise the index e.g. '<>'
     *     if exist '<[=]' condition
     *         if tuple does not satisfy a '<' or '<=' condition
     *             break
     * e.g. '<='
     * For each tuple starting with the first, (read using readForward)
     *     check for the conditions that do not utilise the index e.g. '<>'
     *         if tuple does not satisfy a '<' (or '<=') condition
     *             break
     */
    // Set key to locate()
    if (eq >= 0) 
      key = atoi(cond[eq].value);
    else if (lo >= 0)
      key = atoi(cond[lo].value);
    else
      key = INT_MIN; // tested, works

    // Find tuple specified by search key, obtain cursor to readForward() with
    IndexCursor cursor;
#if debug==1
    cursor.pid = cursor.eid = INT_MIN;
#endif
    if (index.locate(key, cursor) < 0) { // modifies cursor if success
      gene2printf("index.locate(key==%d, cursor) failed!\n\n",key);
      goto exit_select;
    }
#if debug==1
    assert(cursor.pid != INT_MIN && cursor.eid != INT_MIN);
#endif
    // Retreive at least the first tuple
    do {
      bool to_break = false; // avoiding extra goto's
#if debug==1
      int _key = key = INT_MAX;
      int _ridp = rid.pid = INT_MAX;
      int _rids = rid.sid = INT_MAX;
#endif
      if ((rc = index.readForward(cursor, key, rid)) < 0) { // modifies all param
        gene2printf("(rc = index.readForward(cursor, key, rid)) < 0\n");
        break;
      }
#if debug==1
      assert(key != _key);
      assert(rid.pid != _ridp);
      assert(rid.sid != _rids);
#endif

      // Check whether upper bound (if any) has been reached
      if (hi >= 0) { 
        to_break = false;
        switch (cond[hi].comp) {
          case SelCond::LE:
            to_break = key >  atoi(cond[hi].value);
            break;
          case SelCond::LT:
            to_break = key >= atoi(cond[hi].value);
            break;
        }
        gene2printf("An upperbound has been reached? %d\n",to_break?"true":"false");
        if (to_break) break;
      }

      // Read the tuple
      if ((rc = rf.read(rid, key, value)) < 0) {
        fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
        goto exit_select;
      }

      // Check for all conditions
      to_break = false;
      for (int z = cond.size()-1; z >= 0; --z) {
        switch (cond[z].attr) {
          case 1:
            diff = (long long) key - strtoll(cond[z].value, NULL, 10);
            break;
          case 2:
            diff = strcmp(value.c_str(), cond[z].value);
            break;
        }
        // Skip the tuple if any condition is not met
        to_break = false;
        switch (cond[z].comp) {
          case SelCond::EQ: to_break = diff != 0; break;
          case SelCond::NE: to_break = diff == 0; break;
          case SelCond::GT: to_break = diff <= 0; break;
          case SelCond::LT: to_break = diff >= 0; break;
          case SelCond::GE: to_break = diff <  0; break;
          case SelCond::LE: to_break = diff >  0; break;
        }
        if (to_break) break;
      } // end conditions check
      // Skip the tuple if any condition is not met
      if (to_break) continue;

      // Tuple matches. Increase matching tuple counter.
      ++count;

      // Print the tuple 
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

    } while (eq < 0); // for ranged queries

  } else {
/***************************************
#if debug==1
    assert(1==0);
#endif
/***************************************/

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
          diff = (long long) key - strtoll(cond[i].value, NULL, 10);
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
  } // else don't use index

  soft_exit_select:
  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  exit_select:
  if (consult_index) // if index was opened
    index.close();
  rf.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  RC r = 0; // value to return
  BTreeIndex btree;

  // Open file
  fstream filestrm;
  filestrm.open(loadfile.c_str(), ios_base::in);
  if (!filestrm.is_open())
    return RC_FILE_OPEN_FAILED;

  if (index)
    if ((r = btree.open(table, 'w')) < 0) {
      fprintf(stderr, "Error: cannot create index for %s\n", table.c_str());
      filestrm.close();
      return r;
    }

  // Open the table file, which is created if does not exist
  RecordFile rf; // table file
  if ((r = rf.open(table + ".tbl", 'w')) < 0) {
    fprintf(stderr, "Error: cannot open table %s\n", table.c_str());
    filestrm.close();
    return r;
  }

  // For each line in loadfile
  string line; // std::string for call-by-name to std::getline()
  while (getline(filestrm, line).good()) {
    // Parse for key value pair
    int	key; // call-by-reference to parseLoadLine()
    string value; // call-by-reference to parseLoadLine()
    if ((r = parseLoadLine(line, key, value)) < 0) {
      fprintf(stderr, "Error: cannot parse line \"%s\"\n", line.c_str());
      break;
    }

    // Append key-value pair to table
    RecordId rid; // record cursor for call-by-ref to RecordFile::append()
    if ((r = rf.append(key, value, rid)) < 0) {
      fprintf(stderr, "Error: while appending a tuple to table %s\n", table.c_str());
      break;
    }

    if (index)
      btree.insert(key, rid);
  } // End while-loop

  // Clean up and return
  if (index)
    btree.close();
  rf.close();
  filestrm.close();
  return r;
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
