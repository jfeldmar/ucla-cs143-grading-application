#!/usr/bin/python

import os, sys, subprocess, getopt, re, csv;
from subprocess import Popen, PIPE, STDOUT;
from helper_UDfunctions import *;	# import helper functions from helper_UDfunctions.py
from default_vars import *;	# loads global variables from default_vars.py

# Load SID-Student Name tuples from submissions.csv file
# (use default_vars.py to change file name/location)
# store sid-name pairs in dictionary
submissions_reader = csv.DictReader(open(submissions_data_file), ['sid', 'name'], delimiter=',', quotechar='"')
name_dictionary = {}
for row in submissions_reader:
	name_dictionary[row['sid']] = row['name']

################ Global Variables ####################

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
	
	[ To view/change default values, please see the configuration file (default_vars.py). ]

"""

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
		print "To see default values please open the file: default_vars.py"
	else:
		print "option %s with value %s is not recognized."	% (opt, arg)
		assert False, "unhandled option"

################ END - Global Variables ####################

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

#create output file for script results (in format compatible with MyUCLA Gradebook
# tab-delimited fields: SID <tab> Name <tab> Score <tab> [Comment]
resultsFile = open(result_file, 'w')

# read GRADER's test query1 and query2
q1 = open(query1_file, 'r')
right_query1 = q1.read()
q1.close()

q2 = open(query2_file, 'r')
right_query2 = q2.read()
q2.close()

# read GRADER's results for correct query1/query2 run
f1 = open(graders_query1_output, 'r')
f1.readline()			# skip first line which lists field names
right_records_set_q1 = set(f1.read().splitlines())
f1.close()

f2 = open(graders_query2_output, 'r')
f2.readline()			# skip first line which lists field names
right_records_set_q2 = list(f2.read().splitlines())
f2.close()

print "\n===Query 1==="
print right_query1
print "===Expected Query 1 Output==="
for i in right_records_set_q1:
	print i
print "\n===Query 2==="
print right_query2
print "===Expected Query 2 Output==="
for i in right_records_set_q2:
	print i

# for each submission write to file output of running
# create.sql, load.sql, queries.sql, violate.sql

# 1. for each submission
	# drop database test_DB
	# create database test_DB
	# run create.sql
	# print/store success/failure score & comments
	# run load.sql
	# print/store success/failure score & comments
	# run grader's queries
	# print/store success/failure score & comments
	# run student's queries.sql
	# store success/failure score

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
	points = 0

	cmd_create = "mysql -u %s %s < ../submissions/b/%s/%s"	% (user, test_DB, sid, create_script)
	cmd_load = "mysql -u %s %s < ../submissions/b/%s/%s"	% (user, test_DB, sid, load_script)
#	cmd_violate = "mysql -u %s %s < ../submissions/b/%s/%s"	% (user, test_DB, sid, violate_script)
	
	print "\n*****************************"
	print "****SID: ", sid, "*********"

	# 1. drop database test_DB if it exists
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

	    #returncode is 0 if no error, 1 if error, less than 0 if terminated by signal
    	    if retcode == 0:
	    	    create_failed = 0
		    create_script_results.append([sid, "1", "passed"])
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
	    
	if (create_failed):
	    print "Create Script: Failed - 0 points."
	else:
	    print "Create Script: Successful - ", create_pts ," points."
	    points += create_pts	

	    
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

#		if (load_failed):
#		    print "Load Script: Failed - 0 points."
#		else:
#		    print "Load Script: Successful - ", load_pts ," points."	
#		    points += load_pts	

	print "---------"
		
	##################### TEST QUERIES #####################
	# if load script failed => error in query script
	if (load_failed):
		print "Query Script not Executed because Load Script FAILED"
		query1_script_results.append([sid, "0", "ERROR: " + load_script + " script failed"])
		query2_script_results.append([sid, "0", "ERROR: " + load_script + " script failed"])
		query3_script_results.append([sid, "0", "ERROR: " + load_script + " script failed"])
	elif (not create_failed and not load_failed):		

	##################### RUN GRADER's QUERIES ON STUDENT'S DATABASE #####################
	# execute grader's queries on database and compare output to expected (grader's) output

	# run QUERY 1			
		try:
			cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, right_query1)
			fail1, score, toprint = run_query(cmd, right_records_set_q1, 1)
			if (fail1):
				print toprint	# print error message if error encountered
		except OSError, e:
			fail1 = 1
			print >>sys.stderr, "ERROR: Execution failed:", e
		
#		# assign/print score for query 1
#		if (fail1):
#			print "Grader Query 1 Failed - 0 points."
#		else:
#			# give credit for query running without errors
#			# give credit for correctness of results ( 0 <= score <= 1)
#			q1pts = float(query1_pts) * float(score) + float(query1_no_error_pts)
#			points += q1pts
#			print "Grader Query 1 Score: ", q1pts, " points."

	# run QUERY 2
		try:
			cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, right_query2)
			fail2, score, toprint = run_query(cmd, right_records_set_q2, 2)
			if (fail2):
				print toprint	# print error message if error encountered
		except OSError, e:
		    fail2 = 1
		    print >>sys.stderr, "ERROR: Execution failed:", e

#		# assign/print score for query 2
#		if (fail2):
#			print "Grader Query 2 Failed - 0 points."
#		else:
#			# give credit for query running without errors
#			# give credit for correctness of results ( 0 <= score <= 1)
#			q2pts = float(query2_pts) * float(score) + float(query2_no_error_pts)
#			points += q2pts
#			print "Grader Query 2 Score: ", q2pts, " points."

	##################### TEST STUDENT's QUERIES #####################
	# make sure student submitted correct number (3) of queries and they execute without errors

		try:
			print "\nRunning: ", students_all_queries_file, "..."

			# parse STUDENT's queries file (separate into individual queries)
			student_queries = extract_queries("../submissions/b/%s/%s" % (sid, students_all_queries_file), num_queries)

			if (len(student_queries) == 0):
				print "Insufficient number of queries, at least 1 query required"
				print "Student Queries: No Queries Fount - 0 points"
			if (len(student_queries) > num_queries):
				print "Running first ", num_queries, " queries only."

			print "***"
			fail_extract = 0;
		except OSError, e:
		    fail_extract = 1;
		    print >>sys.stderr, "ERROR: Execution failed:", e

		if (not fail_extract):
			# assign/print point for having correct number of queries

			# find how many queries found as fraction (can't be greater than 1)
			queries_found = min (len(student_queries)/num_queries, 1)

			print "Found ", len(student_queries), " queries: ", queries_found * has_all_queries_pts ," points."
			points += queries_found * has_all_queries_pts

			# run QUERY 1
			try:
				if len(student_queries) > 0:
					cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, student_queries[0])

					fail, toprint = test_query(cmd)
					if (fail):
						print toprint
				else:
					print "Student Query 1: FAILED (no query found) - 0 points."
			except OSError, e:
			    fail = 1
			    print >>sys.stderr, "ERROR: Execution failed:", e

	#		# assign/print score for student's query 1
	#		if (fail):
	#			print "Student Query 1: FAILED - 0 points."
	#		else:
	#			print "Student Query 1: Executed Successfully - ", student_queries_pts/num_queries ," points."

			# run QUERY 2
			try:
				if len(student_queries) > 1:
					cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, student_queries[1])

					fail, toprint = test_query(cmd)
					if (fail):
						print toprint
				else:
					print "Student Query 2: FAILED (no query found) - 0 points."
			except OSError, e:
			    fail = 1
			    print >>sys.stderr, "ERROR: Execution failed:", e

	#		# assign/print score for student's query 2
	#		if (fail):
	#			print "Student Query 2 FAILED - 0 points."
	#		else:
	#			print "Student Query 2: Executed Successfully - ", student_queries_pts/num_queries ," points."

			# run QUERY 3
			try:
				if len(student_queries) > 2:
					cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, student_queries[2])
					fail, toprint = test_query(cmd)
					if (fail):
						print toprint
				else:
					print "Student Query 3: FAILED (no query found) - 0 points."
			except OSError, e:
			    fail = 1
			    print >>sys.stderr, "ERROR: Execution failed:", e

	#		# assign/print score for student's query 3
	#		if (fail):
	#			print "Student Query 3 FAILED - 0 points."
	#		else:
	#			print "Student Query 3: Executed Successfully - ", student_queries_pts/num_queries ," points."
			
##write result to files
#for i in range ( min(len(create_script_results),len(load_script_results))):
#	if (create_script_results[i][0] == load_script_results[i][0]):
#		sid = create_script_results[i][0]
#		c_score = create_script_results[i][1]
#		l_score = load_script_results[i][1]
#		total_score = str( int(c_score) + int(l_score) )
#		c_notes = create_script_results[i][2]
#		l_notes = load_script_results[i][2]
#		resultsFile.write(sid + "," + c_score + "," + l_score + "," + total_score + "," + c_notes + "; " + l_notes + "\n")

resultsFile.flush()
resultsFile.close()


