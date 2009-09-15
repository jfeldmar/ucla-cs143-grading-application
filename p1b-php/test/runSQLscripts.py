#!/usr/bin/python

import os, sys, subprocess, getopt, re;
from subprocess import Popen, PIPE, STDOUT;

USAGE = """UCLA-CS143 - Project 1B Grading Script

runSQLscripts.py: Executes 'create' and 'load' scripts and records results.

USAGE: runSQLscripts.py -submissions <submission_dir> -db <mysql_DB> -user <mysql_user>
	-h, --help
		Displays input options
	-submissions <enter-location-here>
		Provide location of students' submission directories
	-d <database-name>
		(Optional) Specify database to be used in mysql
	-user <user-name>
		(Optional) Specify database user in mysql
	-default
		Show default global variable values
		
"""

################ Global Variables ####################
######################################################

# should come from ARGV ( submission directory, test Database, user, create/load script name )
submission_dir = "../submissions/b/"
results_dir = "results"
dir_file = results_dir +"/directories.txt"
result_file = results_dir +"/results.txt"

test_DB = "CS143"
user = "cs144"

drop_DB_script = "./scripts/dropDB-CS143.sql"
create_DB_script = "./scripts/createDB-CS143.sql"
create_script = "create.sql"
load_script = "load.sql"
#violate_script = "violate.sql"

DEFAULT = """
submission_dir = "../submissions/b/"
results_dir = "results"
dir_file = results_dir +"/directories.txt"
result_file = results_dir +"/results.txt"

test_DB = "CS143"
user = "cs144"

drop_DB_script = "./scripts/dropDB-CS143.sql"
create_DB_script = "./scripts/createDB-CS143.sql"
create_script = "create.sql"
load_script = "load.sql"

[to change default values, please edit file directly]
"""
######################################################
################ Global Variables ####################

#CHANGE GLOBAL VARIABLES BASED ON COMMAND-LINE INPUT
try:
	options, remainder = getopt.gnu_getopt(sys.argv[1:], 's:d:u:', ['submissions=', 'database=','user=','default'])
except getopt.GetoptError, err:
	print str(err)
	print USAGE
	sys.exit(2)

for opt, arg in options:
	if opt in ('-s', '--submissions'):
		print "Changing submission directory to %s" % (arg)
		if (re.match('/$', arg) == None):
			arg += '/'
		submission_dir = arg
	elif opt in ('-d', '--database'):
		print "Changing mysql database to %s" % (arg)
		test_DB = arg
	elif opt in ('-u', '--user'):
		print "Changing mysql user to %s" % (arg)
		user = arg
	elif opt == '--default':
		print DEFAULT
	else:
		print "option %s with value %s is not recognized."	% (opt, arg)
		assert False, "unhandled option"

#get all submission SID directories

# remove dir_file if it exists (list of SID directories)
if ( os.path.exists(dir_file)):
	os.remove(dir_file)

# create results directory if doesn't exist
if (not os.path.exists(results_dir)):
	os.mkdir(results_dir)

# remove previous result file
if ( os.path.exists(result_file)):
	os.remove(result_file)
	
# get list of all files and directories
directories = os.listdir(submission_dir)

# extract only directory names from directories
onlydirs = []
for d in directories:
	if ( os.path.isdir(submission_dir + d) == True ):
		onlydirs.append(d)

print "Number of directories (submissions) found: ", len(onlydirs)

#write directories to file (i.e. SIDs) (one per line)
outputDirFile = open(dir_file, 'w')
sep = "\n"
dirs = sep.join(onlydirs)
outputDirFile.write(dirs)
outputDirFile.close()

#create output file for script results
resultsFile = open(result_file, 'w')
resultsFile.write("SID, SCORE-create, SCORE-load, TOTAL-SCORE, NOTES")

# for each submission write to file output of running
# create.sql, load.sql, queries.sql, violate.sql

# 1. drop database test_DB
# 2. create database test_DB
# 3. for each submission
	# run create.sql
	# store success/failure score & output
	# run load.sql
	# store success/failure score & output
	# run queries.sql
	# store success/failure score & output
	# run violate.sql
	# store success/failure score & output

sids = list(onlydirs)
create_script_results = []
load_script_results = []
#violate_script_results = []

cmd_drop_DB = "mysql -u %s < %s"	% (user, drop_DB_script)
cmd_create_DB = "mysql -u %s < %s"	% (user, create_DB_script)

for sid in sids:

	cmd_create = "mysql -u %s %s < ../submissions/b/%s/%s"	% (user, test_DB, sid, create_script)
	cmd_load = "mysql -u %s %s < ../submissions/b/%s/%s"	% (user, test_DB, sid, load_script)
#	cmd_violate = "mysql -u %s %s < ../submissions/b/%s/%s"	% (user, test_DB, sid, violate_script)
	
	print "*****************************"
	print "****SID: ", sid, "*********"

	# 1. drop database test_DB
	if (subprocess.call(cmd_drop_DB, shell=True) != 0):
		create_script_results.append([sid, "0", "ERROR: unable to drop database"])
		print "Error: Unable to drop database ", test_DB," for SID: ", d	
		next
	print "Database ", test_DB," removed"

	# 2. create database test_DB
	if (subprocess.call(cmd_create_DB, shell=True) != 0):
		create_script_results.append([sid, "0", "ERROR: unable to create database"])
		print "Error: Unable to create database ", test_DB," for SID: ", d
		next
	print "Database ", test_DB, " created"

	print "---------"
	
	##################### RUN CREATE SCRIPT #####################
	try:
	    print "Running: ", create_script, "..."

	    process = subprocess.Popen(cmd_create, shell=True, stdout=PIPE, stderr=PIPE, bufsize=-1)
	    (stdout, stderr) = process.communicate()
	    retcode = process.returncode

	    #returncode 0 if no error, 1 if error, less than 0 if terminated by signal
    	    if retcode == 0:
	    	    create_failed = 0
		    create_script_results.append([sid, "1", "passed"])
		    print "Create Script: Successful."
	    else: 
	    	    create_failed = 1
		    if retcode < 0:
        		print >>sys.stderr, "ERROR: Child was terminated by signal", -retcode
			print >>sys.stderr, " when executing command: ", cmd_create
			print 'stderr: ', repr(stderr)
			create_script_results.append([sid, "0", "ERROR: child process terminted by signal"])
		    else:
	        	print >>sys.stderr, "ERROR: returncode: ", retcode
			print >>sys.stderr, " when executing command: ", cmd_create
			print 'stderr: ', repr(stderr)
			create_script_results.append([sid, "0", repr(stderr)])
	except OSError, e:
	    print >>sys.stderr, "ERROR: Execution failed:", e
	    
	print "---------"
	
	##################### RUN LOAD SCRIPT #####################
	# if create script failed => error in load script
	if (create_failed):
		print "Load Script not Executed because Create Script FAILED"
		load_script_results.append([sid, "0", "ERROR: " + create_script + " script failed"])
	else:
	# run load script and store results
		try:
		    print "Running: ", load_script, "..."
		    
		    process = subprocess.Popen(cmd_load, shell=True, stdout=PIPE, stderr=PIPE, bufsize=-1)
		    (stdout, stderr) = process.communicate()
		    retcode = process.returncode

		    #returncode 0 if no error, 1 if error, less than 0 if terminated by signal
    		    if retcode == 0:
	    		    load_failed = 0
			    load_script_results.append([sid, "1", "passed"])
			    print "Load Script: Successful."
		    else: 
	    		    load_failed = 1
			    if retcode < 0:
        			print >>sys.stderr, "ERROR: Child was terminated by signal", -retcode
				print >>sys.stderr, " when executing command: ", cmd_load
				print 'stderr: ', repr(stderr)
				load_script_results.append([sid, "0", "ERROR: child process terminted by signal"])
			    else:
	        		print >>sys.stderr, "ERROR: returncode: ", retcode
				print >>sys.stderr, " when executing command: ", cmd_load
				print 'stderr: ', repr(stderr)
				load_script_results.append([sid, "0", repr(stderr)])
		except OSError, e:
		    print >>sys.stderr, "ERROR: Execution failed:", e
		
	#	##################### RUN VIOLATE SCRIPT #####################
	#	# if load script failed => error in violate script
	#	if (load_failed):
	#		print "Violate Script not Executed because Load Script FAILED"
	#		violate_script_results.append([sid, "0", "ERROR: " + load_script + " script failed"])
	#	else:
	#	# run violate script and store results
	#		try:
	#		    print "Running: ", violate_script, "..."
	#		    
	#		    process = subprocess.Popen(cmd_violate, shell=True, stdout=PIPE, stderr=PIPE, bufsize=-1)
	#		    (stdout, stderr) = process.communicate()
	#		    retcode = process.returncode
	#
	#		    #returncode 0 if no error, 1 if error, less than 0 if terminated by signal
	#    		    if retcode == 0:
	#	    		    violate_failed = 0
	#			    violate_script_results.append([sid, "1", "passed"])
	#			    print "Violate Script: Successful."
	#		    else: 
	#	    		    violate_failed = 1
	#			    if retcode < 0:
	#        			print >>sys.stderr, "ERROR: Child was terminated by signal", -retcode
	#				print >>sys.stderr, " when executing command: ", cmd_violate
	#				print 'stderr: ', repr(stderr)
	#				violate_script_results.append([sid, "0", "ERROR: child process terminted by signal"])
	#			    else:
	#	        		print >>sys.stderr, "ERROR: returncode: ", retcode
	#				print >>sys.stderr, " when executing command: ", cmd_violate
	#				print 'stderr: ', repr(stderr)
	#				violate_script_results.append([sid, "0", repr(stderr)])
	#		except OSError, e:
	#		    print >>sys.stderr, "ERROR: Execution failed:", e


#write result to files
for i in range ( min(len(create_script_results),len(load_script_results))):
	if (create_script_results[i][0] == load_script_results[i][0]):
		sid = create_script_results[i][0]
		c_score = create_script_results[i][1]
		l_score = load_script_results[i][1]
		total_score = str( int(c_score) + int(l_score) )
		c_notes = create_script_results[i][2]
		l_notes = load_script_results[i][2]
		resultsFile.write(sid + "," + c_score + "," + l_score + "," + total_score + "," + c_notes + "; " + l_notes)

resultsFile.close()


