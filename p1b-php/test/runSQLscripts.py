#!/usr/bin/python

import os, sys, subprocess, getopt, re;
from subprocess import Popen, PIPE, STDOUT;
from helper_UDfunctions import *;

USAGE = """UCLA-CS143 - Project 1B Grading Script

run-Create-Load-SQLscripts.py: Executes 'create' and 'load' scripts and records results.

USAGE: run-Create-Load-SQLscripts.py -submissions <submission_dir> -db <mysql_DB> -user <mysql_user>
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

graders_query1_output = "scripts/out_q1.txt"
graders_query2_output = "scripts/out_q2.txt"

query1_file = "query1.sql"
query2_file = "query2.sql"
students_query_file = "myquery.sql"
students_all_queries_file = "queries.sql"
num_queries = 3

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
outputDirFile.flush()
outputDirFile.close()

#create output file for script results
resultsFile = open(result_file, 'w')
resultsFile.write("SID, SCORE-create, SCORE-load, TOTAL-SCORE, NOTES")

# read GRADER's results for correct query1/query2 run
f1 = open(graders_query1_output, 'r')
f1.readline()			# skip first line which lists field names
right_records_set_q1 = set(f1.read().splitlines())

f2 = open(graders_query2_output, 'r')
f2.readline()			# skip first line which lists field names
right_records_set_q2 = list(f2.read().splitlines())

print "===Expected Query 1 Output==="
for i in right_records_set_q1:
	print i
print "===Expected Query 2 Output==="
for i in right_records_set_q2:
	print i

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
query1_script_results = []
query2_script_results = []
query3_script_results = []
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

	    process = subprocess.Popen(cmd_create, shell=True, stdout=PIPE, stderr=PIPE, bufsize=0)
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
			create_script_results.append([sid, "0", "ERROR: child process terminted by signal"])
		    else:
	        	print >>sys.stderr, "ERROR: returncode: ", retcode
			create_script_results.append([sid, "0", repr(stderr)])

		    print >>sys.stderr, " when executing command: ", cmd_create
		    print 'stderr: ', repr(stderr)

	except OSError, e:
	    create_failed = 1
	    print >>sys.stderr, "ERROR: Execution failed:", e
	    
	print "---------"
	
	##################### RUN LOAD SCRIPT #####################
	# if create script failed => error in load script
	if (create_failed):
		load_failed = 0
		print "Load  Script not Executed because Create Script FAILED"
		print "Query Script not Executed because Create Script FAILED"
		load_script_results.append([sid, "0", "ERROR: " + create_script + " script failed"])
		query1_script_results.append([sid, "0", "ERROR: " + create_script + " script failed"])
		query2_script_results.append([sid, "0", "ERROR: " + create_script + " script failed"])
		query3_script_results.append([sid, "0", "ERROR: " + create_script + " script failed"])
	else:
	# run load script and store results
		try:
		    print "Running: ", load_script, "..."
		    
		    process = subprocess.Popen(cmd_load, shell=True, stdout=PIPE, stderr=PIPE, bufsize=0)
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
				load_script_results.append([sid, "0", "ERROR: child process terminted by signal"])
			    else:
	        		print >>sys.stderr, "ERROR: returncode: ", retcode
				load_script_results.append([sid, "0", repr(stderr)])

			    print >>sys.stderr, " when executing command: ", cmd_load
			    print 'stderr: ', repr(stderr)
		except OSError, e:
		    load_failed = 1
		    print >>sys.stderr, "ERROR: Execution failed:", e

	print "---------"
		
	##################### RUN QUERY SCRIPT #####################
	# if load script failed => error in query script
	if (load_failed):
		print "Query Script not Executed because Load Script FAILED"
		query1_script_results.append([sid, "0", "ERROR: " + load_script + " script failed"])
		query2_script_results.append([sid, "0", "ERROR: " + load_script + " script failed"])
		query3_script_results.append([sid, "0", "ERROR: " + load_script + " script failed"])
	elif (not create_failed and not load_failed):
		# run query script and store results

			#query1_file = "query1.sql"
			#query2_file = "query2.sql"
			#students_query_file = "myquery.sql"
			#students_all_queries_file = "queries.sql"
			#right_records_set_q1


		##############uncomment when queries are separated into different files (one query per file)
		# get student's query statements
		#student_q1 = open(query1_file.readlines())
		#student_q2 = open(query2_file.readlines())
		#student_q3 = open(query3_file.readlines())
		
		try:
			print "Running: ", students_all_queries_file, "..."


			# parse STUDENT's queries file (separate into individual queries)
			student_queries = extract_queries("../submissions/b/%s/%s" % (sid, students_all_queries_file), num_queries)

			print "Found ", len(student_queries), " queries."
			if (len(student_queries) == 0):
				print "Insufficient number of queries, at least 1 query required"
				sys.exit(2)
			if (len(student_queries) > num_queries):
				print "Running first ", num_queries, " queries only."

			print "***"
		except OSError, e:
		    print >>sys.stderr, "ERROR: Execution failed:", e

		try:
			# run QUERY 1
			if len(student_queries) > 0:
				cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, student_queries[0])

				fail, score, toprint = run_query(cmd, right_records_set_q1, 1)
				if (not fail):
					print "Query 1 Score: ", round(100*score), "%"
				else:
					print "Query 1 FAILED. Score: 0"
					toprint
		except OSError, e:
		    print >>sys.stderr, "ERROR: Execution failed:", e

		try:
			# run QUERY 2
			if len(student_queries) > 1:
				cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, student_queries[1])

				fail, score, toprint = run_query(cmd, right_records_set_q2, 2)
				if (not fail):
					print "Query 2 Score: ", round(100*score), "%"
				else:
					print "Query 2 FAILED. Score: 0"
					print toprint
		except OSError, e:
		    print >>sys.stderr, "ERROR: Execution failed:", e

		try:					
			# run QUERY 3
			if len(student_queries) > 2:
				cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, student_queries[2])
				process = subprocess.Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE, bufsize=0)
				(stdout, stderr) = process.communicate()
				retcode = process.returncode

				#returncode 0 if no error, 1 if error, less than 0 if terminated by signal
				try:
					if retcode == 0:
					    score = 100
					    print "Query 3 Score:  100.0 %"
					else: 
					    if retcode < 0:
        					toprint = "Query 3 - ERROR: Child was terminated by signal", -retcode
					    else:
						toprint = "Query 3 - ERROR: returncode: ", retcode

					    print "Query 3 Score:  0 %"

					    toprint += " when executing command: ", cmd
					    toprint += "stderr: " , repr(stderr)
				except OSError, e:
					print >>sys.stderr, "ERROR: Execution failed:", e
		except OSError, e:
		    print >>sys.stderr, "ERROR: Execution failed:", e
			
#write result to files
for i in range ( min(len(create_script_results),len(load_script_results))):
	if (create_script_results[i][0] == load_script_results[i][0]):
		sid = create_script_results[i][0]
		c_score = create_script_results[i][1]
		l_score = load_script_results[i][1]
		total_score = str( int(c_score) + int(l_score) )
		c_notes = create_script_results[i][2]
		l_notes = load_script_results[i][2]
		resultsFile.write(sid + "," + c_score + "," + l_score + "," + total_score + "," + c_notes + "; " + l_notes + "\n")

resultsFile.flush()
resultsFile.close()


