/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @date 3/24/2008
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 */

#ifndef BRUINBASE_H
#define BRUINBASE_H

// Richard's #defines
#define richard_debug 0

// Gene's #defines
#define debug 0
#define verbose 0
#define geneprintf(format, ...) if(debug && verbose >1) fprintf(stderr, format, ##__VA_ARGS__)
#define gene2printf(format, ...) if(debug && verbose >0) fprintf(stderr, format, ##__VA_ARGS__)
#if debug==1
  #include <assert.h>
  #define FORCE_NO_INDEX 0
#endif
extern bool bruinbase_silent;

typedef int RC;

const int RC_FILE_OPEN_FAILED    = -1001;
const int RC_FILE_CLOSE_FAILED   = -1002;
const int RC_FILE_SEEK_FAILED    = -1003;
const int RC_FILE_READ_FAILED    = -1004;
const int RC_FILE_WRITE_FAILED   = -1005;
const int RC_INVALID_FILE_MODE   = -1006;
const int RC_INVALID_PID         = -1007;
const int RC_INVALID_RID         = -1008;
const int RC_INVALID_FILE_FORMAT = -1009;
const int RC_NODE_FULL           = -1010;
const int RC_INVALID_CURSOR      = -1011;
const int RC_NO_SUCH_RECORD      = -1012;
const int RC_END_OF_TREE         = -1013;
const int RC_INVALID_ATTRIBUTE   = -1014;

#endif // BRUINBASE_H
