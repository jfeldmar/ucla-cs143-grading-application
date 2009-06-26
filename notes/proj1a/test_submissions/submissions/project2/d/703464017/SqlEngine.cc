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
  BTreeIndex bti;  // Index file

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;

  int    countVA, countEQ, countGT, countLT, countNE;
  int    EQ, GT, LT;
  string VA;
  vector<int> positionNE;
  int*   NE = NULL;

  IndexCursor ic;
  int    indexCase; // 1- with EQ,  2- with EQ, with >, 3- with EQ with <, 4- with EQ, with > and <
                    // 5- without EQ, 6- without EQ with >, 7- without EQ with <, 8- without EQ with > and <

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }


  // Check if index exists
  if( !(bti.open(table + ".idx", 'r') < 0) && (cond.size() >0) )
  {
//      bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//      goto index_exit;
      //fprintf(stdout, "%s\n", "index used");
      count = 0;
      countVA = countEQ = countGT = countLT = countNE = 0;
      EQ = GT = LT = 0;

      for( unsigned i=0; i<cond.size(); i++ )
      {
        //fprintf(stdout, "%d\n", i);
        switch (cond[i].attr) {
        case 1:
            // Key
            switch (cond[i].comp) {
            case SelCond::EQ:
                if( (++countEQ > 1) && (atoi(cond[i].value) != EQ) )
                {
                    goto exit_select;
                }
                else
                {
                    EQ = atoi(cond[i].value);
                }
            break;
            case SelCond::NE:
                countNE++;
                positionNE.push_back( i );
            break;
            case SelCond::GT:
                if( GT < atoi(cond[i].value) || countGT == 0 )
                {
                    GT = atoi(cond[i].value);
                    countGT++;
                }
            break;
            case SelCond::LT:
                if( LT > atoi(cond[i].value) || countLT == 0 )
                {
                    LT = atoi(cond[i].value);
                    countLT++;
                    //fprintf(stderr, "%d\n", LT);
                }
            break;
            case SelCond::GE:
                if( GT < atoi(cond[i].value)-1 || countGT == 0 )
                {
                    GT = atoi(cond[i].value)-1;
                    countGT++;
                }
            break;
            case SelCond::LE:
                if( LT > atoi(cond[i].value)+1 || countLT == 0 )
                {
                    LT = atoi(cond[i].value)+1;
                    countLT++;
                }
            break;
            }
        break;
        case 2:
            // Value
            ++countVA;
        break;
        }
      }

      // == Print Val
//            fprintf(stderr, "cEQ=%d, cGT=%d, cLT=%d, cNE=%d, cVA=%d, EQ=%d, GT=%d, LT=%d\n", countEQ, countGT, countLT, countNE, countVA, EQ, GT, LT);

      // Check if NE exist!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      if( countNE > 0 )
      {
          NE = new int[countNE];
          for( unsigned i=0; i<countNE; i++)
          {
              //fprintf(stderr, "HERE countNE\n");
              NE[i] = atoi(cond[positionNE.back()].value);
              positionNE.pop_back();
          }
          // Print thing in NE
////          fprintf(stderr, "NE %d: ", countNE);
//          for( unsigned i=0; i<countNE; i++ )
//              fprintf(stderr, "%d, ", NE[i]);
//          fprintf(stderr, "\n");
      }

      // Check if EQ is between GT and LT if apply
      if( countEQ>0 && (countGT>0 || countLT>0) )
      {
          if( countGT>0 && countLT>0 )
          {
              if( !(EQ>GT && EQ<LT) )
                  goto exit_select;
          }
          else if( countGT>0 )
          {
              if( !(EQ>GT) )
                  goto exit_select;
          }
          else
          {
              if( !(EQ<LT) )
                  goto exit_select;
          }
      }

      // Check if GT<LT
          if( countGT>0 && countLT>0 )
          {
              //fprintf(stderr, "%d %d\n", GT, LT);
              if( !(GT<LT) )
                  goto exit_select;
          }

      if( (countEQ+countGT+countLT)==0 && (countVA>0 || countNE>0) )
      {
          goto normal_search;
      }
      else if( countEQ>0 )
      {
          bti.locate( EQ, ic );
          bti.readForward( ic, key, rid );
          //read the tuple
          if ( (rc = rf.read( rid, key, value )) < 0 ) {
              fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
              goto exit_select;
          }
//          fprintf(stderr, "cEQ>0:%d, %d\n", EQ, key);
          // Check if the returned key  == EQ
          if( EQ != key ) goto index_exit;

          if( countVA>0 )
              {
                  //fprintf(stderr, "ER=0; HERE1\n");

                   //fprintf(stderr, "HERE3\n");
                 for( unsigned i = 0; i < cond.size(); i++ )
                 {
                     if( cond[i].attr==2 )
                     {
                         fprintf(stderr, "EQ==0: %s --- %s\n", value.c_str(),cond[i].value );
                        diff = strcmp(value.c_str(), cond[i].value);
                        //fprintf(stderr, "EQ==0:b\n");
                        switch (cond[i].comp) {
                          case SelCond::EQ:
                        if (diff != 0) goto index_next;
                        break;
                          case SelCond::NE:
                        if (diff == 0) goto index_next;
                        break;
                          case SelCond::GT:
                        if (diff <= 0) goto index_next;
                        break;
                          case SelCond::LT:
                        if (diff >= 0) goto index_next;
                        break;
                          case SelCond::GE:
                        if (diff < 0) goto index_next;
                        break;
                          case SelCond::LE:
                        if (diff > 0) goto index_next;
                        break;
                        }
                     }
                 }
              }
              if( countNE>0 )
                  for( unsigned i=0; i<countNE; i++ )
                  {
                      //fprintf(stderr, "Inside Check NE: %d\n", i);
                      if( key == NE[i] )
                          goto index_next;
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
      }
      else // countEQ == 0
      {
          if( !(countGT>0) )
            bti.getSmallestKey( GT );

          //bti.print_tree(bti.getRootPid(), bti.getTreeHeight());

          bti.locate( GT+1, ic );
          //fprintf(stdout, "%d %d\n", key, ((countGT>0)?GT+1:0));
          
//          fprintf(stdout, "%d %d\n", GT, LT);
//          fprintf(stderr, "HERE\n");
//          bti.readForward( ic, key, rid );
//          rf.read( rid, key, value );
//          fprintf(stdout, "%d\n", key);

          while( !(bti.readForward( ic, key, rid ) < 0) )
          {
//              if (key == 4589)
//                  fprintf(stdout, "HELLO: %d\n", key);

//              fprintf(stderr, "HERE\n");
              
              if( countLT>0 )
              {
                  //fprintf(stdout, "%s\n", "check LF success");
                  if( !(key<LT) )
                  {
                      //fprintf(stdout, "%s\n", "!(key<LT)");
                      goto index_exit;
                  }
              }


            //read the tuple
              //fprintf(stderr, "HERE\n");
              if ( (rc = rf.read( rid, key, value )) < 0 ) {
                  fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
                  goto exit_select;
              }
//fprintf(stderr, "ER=0; HERE2\n");
//fprintf(stderr, "cEQ=%d, cGT=%d, cLT=%d, cNE=%d, cVA=%d, EQ=%d, GT=%d, LT=%d\n", countEQ, countGT, countLT, countNE, countVA, EQ, GT, LT);

              if( countVA>0 )
              {
                  //fprintf(stderr, "ER=0; HERE1\n");
                  
                   //fprintf(stderr, "HERE3\n");
                 for( unsigned i = 0; i < cond.size(); i++ )
                 {
                     if( cond[i].attr==2 )
                     {
//                         fprintf(stderr, "EQ==0: %s --- %s\n", value.c_str(),cond[i].value );
                        diff = strcmp(value.c_str(), cond[i].value);
                        //fprintf(stderr, "EQ==0:b\n");
                        switch (cond[i].comp) {
                          case SelCond::EQ:
                        if (diff != 0) goto index_next;
                        break;
                          case SelCond::NE:
                        if (diff == 0) goto index_next;
                        break;
                          case SelCond::GT:
                        if (diff <= 0) goto index_next;
                        break;
                          case SelCond::LT:
                        if (diff >= 0) goto index_next;
                        break;
                          case SelCond::GE:
                        if (diff < 0) goto index_next;
                        break;
                          case SelCond::LE:
                        if (diff > 0) goto index_next;
                        break;
                        }
                     }
                 }
              }
              if( countNE>0 )
                  for( unsigned i=0; i<countNE; i++ )
                  {
                      //fprintf(stderr, "Inside Check NE: %d\n", i);
                      if( key == NE[i] )
                          goto index_next;
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
                index_next:
                    continue;
          };
      }

        
        index_exit:
        // print matching tuple count if "select count(*)"
          if (attr == 4) {
            fprintf(stdout, "%d\n", count);
          }
  }
  else
  {
      normal_search:
      //fprintf(stderr, "normal_search\n");
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

  // close the table file and return
  exit_select:
  rf.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */
//* New -----
    RC rc; // A reture code, indicate error, if any
    ifstream inFile;
    RecordFile outRec;
    RecordId rid;
    int row_aff = 0; // # of row(s) affected

    string record;
    int key;
    string value;

    BTreeIndex bti;
    if (index)
        bti.open(table + ".idx", 'w');

    // Open input file
    inFile.open(loadfile.c_str());
    if (!inFile.is_open()) {
        fprintf(stderr, "Error: Fail to open file \"%s\".\n", loadfile.c_str());
        return RC_FILE_OPEN_FAILED;
    }
    
    // Open output record
    if ((rc = outRec.open(table + ".tbl", 'w')) < 0) {
        fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
        return rc;
    }

    // Read from the file and write to the record
    getline(inFile, record);
    while (!inFile.eof()) {
        parseLoadLine(record, key, value); // Parse a single record
        if ((rc = outRec.append(key, value, rid)) < 0) {
            fprintf(stderr, "Error: while appending tuple to table %s\n", table.c_str());
            fprintf(stdout, "%d rows affected\n", row_aff);
            return rc;
        }

        // Insert indexes
        if (index) {

//            if (key == 40)
//                bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
            
            bti.insert(key, rid);}

        row_aff++;
        getline(inFile, record);
    }

//bti.print_tree(bti.getRootPid(), bti.getTreeHeight());

//    if (index)
//        bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
    
    fprintf(stdout, "Query OK, %d rows affected.\n", row_aff);

    inFile.close();
    outRec.close();
    if (index)
        bti.close();
    
// End ----- //*/

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

//RC SqlEngine::test() {
//
//    prt("\n  ----- Start testing -----\n");
//
//    RC rc;
//    BTreeIndex bti;
//    RecordId rid;
//
//    // Open a index file
//    prt("\n  Open *.idx file\n");
//    if ((rc = bti.open("data.idx", 'w')) < 0) {
//        prterr("Can't not open *.idx file\n", rc);
//        return rc;
//    }
////*/ ----- Insert -----
////
//    rid.pid = 1;
//    rid.sid = 0;
//
//    bti.insert(300, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(500, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(900, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(800, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(700, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(200, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(600, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(100, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(333, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(366, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(111, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//
//
//    bti.insert(112, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(334, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//
//    bti.insert(335, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//
//    bti.insert(201, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(202, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(301, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
//    bti.insert(302, rid);
//    cout << endl;
//    bti.print_tree(bti.getRootPid(), bti.getTreeHeight());
////*/
//
//    // Close the index file
//    prt("\n  Close *.idx file\n");
//    bti.close();
//
//    prt("\n  ----- End testing -----\n");
//
//    return 0;
//}
//
//RC SqlEngine::prt(const string& msg) {
//    fprintf(stdout, "%s\n", msg.c_str());
//}
//
//RC SqlEngine::prterr(const string& msg, RC rc) {
//    fprintf(stderr, "Error(%d): %s\n", rc, msg.c_str());
//}
