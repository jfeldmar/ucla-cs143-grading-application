#!/usr/bin/python

import os, sys, subprocess, getopt, re, csv;
from subprocess import Popen, PIPE, STDOUT;

# IMPORTANT IF THIS SCRIPT IS NOT RUN FROM ITS OWN DIRECTORY
#	saves caller's directory, changes to script's directory
#	at the end of script, changes back to caller's directory
caller_dir = os.getcwd()
script_dir = sys.path[0]
os.chdir(script_dir)


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
		# drop forward slash at the end of string if present
		submission_dir = re.sub('/$', '', submission_dir)	
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
	if ( os.path.isdir(submission_dir + '/' + d) == True ):
		onlydirs.append(d)

print "Number of directories (submissions) found: ", len(onlydirs)

#write directories to file (i.e. SIDs) (one per line)
outputDirFile = open(dir_file, 'w') or sys.exit("Unable to Open " + dir_file + " for storing directory names.")
sep = "\n"
dirs = sep.join(onlydirs)
outputDirFile.write(dirs)
outputDirFile.flush()
outputDirFile.close()

#create output file for script results (in format compatible with MyUCLA Gradebook
# tab-delimited fields: SID <tab> Name <tab> Score <tab> [Comment]
resultsFile = open(result_file, 'w') or sys.exit("Unable to open output Gradebook file: " + result_file)

# read GRADER's test query1 and query2
q1 = open(query1_file, 'r') or sys.exit("Unable to open grader's query 1: " + query1_file)
right_query1 = q1.read()
q1.close()

q2 = open(query2_file, 'r') or sys.exit("Unable to open grader's query 2: " + query2_file)
right_query2 = q2.read()
q2.close()

# read GRADER's results for correct query1/query2 run
f1 = open(graders_query1_output, 'r') or sys.exit("Unable to open grader's query 1 solution file: " + graders_query1_output)
f1.readline()			# skip first line which lists field names
right_records_set_q1 = set(f1.read().splitlines())
f1.close()

f2 = open(graders_query2_output, 'r') or sys.exit("Unable to open grader's query 2 solution file: " + graders_query2_output)
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

sys.stdout.flush()

loop_start = time.time()

for sid in sids:
	sys.stdout.flush()
	
	current_start = time.time()

	# placeholder variables for gradebook file output
	points = 0
	comments = ""
	
	# restart MySQL to kill possible hanging processes from previous MySQL queries and commands
	os.system(sql_restart_command)
	
	# in case when program output is written to file
	# this command still displays grading progress to the standard error output
	# (this allows the user to more easily track progress)
	print >>sys.stderr, "Grading SID: ", sid, "( ", name_dictionary[sid], ")"

	cmd_create = "mysql -u %s %s < %s/%s/%s"	% (user, test_DB, submission_dir, sid, create_script)
	cmd_load = "mysql -u %s %s < %s/%s/%s"	% (user, test_DB, submission_dir, sid, load_script)
#	cmd_violate = "mysql -u %s %s < %s/%s/%s"	% (user, test_DB, submission_dir, sid, violate_script)
	
	print "\n*****************************"
	print "****SID: ", sid, "*********"

	# 1. drop database test_DB if it exists
	if (subprocess.call(cmd_drop_DB, shell=True) != 0):
		create_script_results.append([sid, "0", "ERROR: unable to drop database"])
		error_str = "Error: Unable to drop database ", test_DB," for SID: ", d
		print error_str

		# write results to gradebook and continue to next SID
		write_grade(resultsFile, sid, name_dictionary[sid], 0, error_str)	
		continue
		
	print "Database ", test_DB," removed"

	sys.stdout.flush()

	# 2. create database test_DB
	if (subprocess.call(cmd_create_DB, shell=True) != 0):
		create_script_results.append([sid, "0", "ERROR: unable to create database"])
		error_str =  "Error: Unable to create database ", test_DB," for SID: ", d
		print error_str
		
		# write results to gradebook and continue to next SID
		write_grade(resultsFile, sid, name_dictionary[sid], 0, error_str)	
		continue
		
	print "Database ", test_DB, " created"

	sys.stdout.flush()
	
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

		    #print >>sys.stderr, " when executing command: ", cmd_create
		    print 'stderr: ', repr(stderr)

	except OSError, e:
	    create_failed = 1
	    print >>sys.stderr, "ERROR: Execution failed:", e
	    
	if (create_failed):
	    print "Create Script: Failed - 0 points."
	    comments += " create script failed;"
	else:
	    print "Create Script: Successful - ", create_pts ," points."
	    points += create_pts	

	sys.stdout.flush()
	
	print "---------"
	
	##################### RUN LOAD SCRIPT #####################
	# if create script failed => error in load script
	if (create_failed):
		load_failed = 0
		print "Load  Script not Executed because Create Script FAILED"
		print "Query Script not Executed because Create Script FAILED"

		comments += " Load  Script not Executed because Create Script FAILED; Query Script not Executed because Create Script FAILED"

		load_script_results.append([sid, "0", "ERROR: " + create_script + " script failed"])
		query1_script_results.append([sid, "0", "ERROR: " + create_script + " script failed"])
		query2_script_results.append([sid, "0", "ERROR: " + create_script + " script failed"])
		query3_script_results.append([sid, "0", "ERROR: " + create_script + " script failed"])
		
		# write results to gradebook and continue to next SID
		write_grade(resultsFile, sid, name_dictionary[sid], points, comments)			
		continue
		
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

			    #print >>sys.stderr, " when executing command: ", cmd_load
			    print 'stderr: ', repr(stderr)
		except OSError, e:
		    load_failed = 1
		    print >>sys.stderr, "ERROR: Execution failed:", e

		if (load_failed):
		    print "Load Script: Failed - 0 points."
		    comments += " load script failed;"
		else:
		    print "Load Script: Successful - ", load_pts ," points."	
		    points += load_pts	

	sys.stdout.flush()
	
	print "---------"
		
	##################### TEST QUERIES #####################
	# if load script failed => error in query script
	if (load_failed):
		print "Query Script not Executed because Load Script FAILED"
		
		query1_script_results.append([sid, "0", "ERROR: " + load_script + " script failed"])
		query2_script_results.append([sid, "0", "ERROR: " + load_script + " script failed"])
		query3_script_results.append([sid, "0", "ERROR: " + load_script + " script failed"])

		comments += " Query Script not Executed because Create Script FAILED"
		
		# write results to gradebook and continue to next SID
		write_grade(resultsFile, sid, name_dictionary[sid], points, comments)			
		continue
		
	elif (not create_failed and not load_failed):		

	##################### RUN GRADER's QUERIES ON STUDENT'S DATABASE #####################
	# execute grader's queries on database and compare output to expected (grader's) output

	# run QUERY 1			
		fail1 = 1	# initialize variable
		try:
			cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, right_query1)
			fail1, score, toprint = run_query(cmd, right_records_set_q1, 1, timeout)
			if (fail1):
				print toprint	# print error message if error encountered
		except OSError, e:
			fail1 = 1
			print >>sys.stderr, "ERROR: Execution failed:", e
		
		# assign/print score for query 1
		if (fail1):
			print "Grader Query 1 Failed - 0 points."
			comments += " Grader Query 1 Failed;"
		else:
			# give credit for query running without errors
			# give credit for correctness of results ( 0 <= score <= 1)
			q1pts = float(query1_pts) * float(score) + float(query1_no_error_pts)
			points += q1pts
			print "Grader Query 1 Score: ", q1pts, " points."

		sys.stdout.flush()

	# run QUERY 2
		fail2 = 1	# initialize variable
		try:
			cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, right_query2)
			fail2, score, toprint = run_query(cmd, right_records_set_q2, 2, timeout)
			if (fail2):
				print toprint	# print error message if error encountered
		except OSError, e:
		    fail2 = 1
		    print >>sys.stderr, "ERROR: Execution failed:", e

		sys.stdout.flush()

		# assign/print score for query 2
		if (fail2):
			print "Grader Query 2 Failed - 0 points."
			comments += " Grader Query 2 Failed;"
		else:
			# give credit for query running without errors
			# give credit for correctness of results ( 0 <= score <= 1)
			q2pts = float(query2_pts) * float(score) + float(query2_no_error_pts)
			points += q2pts
			print "Grader Query 2 Score: ", q2pts, " points."

		sys.stdout.flush()
	##################### TEST STUDENT's QUERIES #####################
	# make sure student submitted correct number (3) of queries and they execute without errors

		fail_extract = 1 # initialize variable
		try:
			print "\nRunning: ", students_all_queries_file, "..."

			# parse STUDENT's queries file (separate into individual queries)
			student_queries = extract_queries("%s/%s/%s" % ( submission_dir, sid, students_all_queries_file), num_queries)

			if (len(student_queries) == 0):
				print "Insufficient number of queries, at least 1 query required"
				print "Student Queries: No Queries Fount - 0 points"
			if (len(student_queries) > num_queries):
				print "Running first ", num_queries, " queries only."

			print "***"
			fail_extract = 0
		except OSError, e:
		    fail_extract = 1
		    print >>sys.stderr, "ERROR: Execution failed:", e

		sys.stdout.flush()

		if (fail_extract):
			comments += " Unable to parse student's queries into separate queries"
		else:
			# assign/print point for having correct number of queries

			# find how many queries found as fraction (can't be greater than 1)
			queries_found = min (len(student_queries), num_queries)

			# give points for the presence of correct number of queries
			has_all_queries = float(queries_found) / float(num_queries) * has_all_queries_pts
			
			print "Found ", len(student_queries), " queries: ", has_all_queries , "/", has_all_queries_pts," points."
			points += has_all_queries
			
			per_query_pts = round(float(student_queries_pts) / float(num_queries), 2)

			# run QUERY 1
			fail = 1	# reset variable
			try:
				if len(student_queries) > 0:
					cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, student_queries[0])

					fail, toprint = test_query(cmd, timeout)
					if (fail):
						print toprint
				else:
					print "Student Query 1: FAILED (no query found) - 0 points."
			except OSError, e:
			    fail = 1
			    print >>sys.stderr, "ERROR: Execution failed:", e

			# assign/print score for student's query 1
			if (fail):
				print "Student Query 1: FAILED - 0 points."
				comments += " Student Query 1 FAILED;"
			else:
				print "Student Query 1: Executed Successfully - ", per_query_pts ," points."
				points += per_query_pts

			sys.stdout.flush()

			# run QUERY 2
			fail = 1	# reset variable			
			try:
				if len(student_queries) > 1:
					cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, student_queries[1])

					fail, toprint = test_query(cmd, timeout)
					if (fail):
						print toprint
				else:
					print "Student Query 2: FAILED (no query found) - 0 points."
					comments += " Student Query 2 - no query found,"
			except OSError, e:
			    fail = 1
			    print >>sys.stderr, "ERROR: Execution failed:", e

			# assign/print score for student's query 2
			if (fail):
				print "Student Query 2 FAILED - 0 points."
				comments += " Student Query 2 FAILED;"
			else:
				print "Student Query 2: Executed Successfully - ", per_query_pts ," points."
				points += per_query_pts

			sys.stdout.flush()

			# run QUERY 3
			fail = 1	# reset variable
			try:
				if len(student_queries) > 2:
					cmd = "mysql -u %s %s -e \"%s\""	% (user, test_DB, student_queries[2])
					fail, toprint = test_query(cmd, timeout)
					if (fail):
						print toprint
				else:
					print "Student Query 3: FAILED (no query found) - 0 points."
					comments += " Student Query 3 - no query found,"
			except OSError, e:
			    fail = 1
			    print >>sys.stderr, "ERROR: Execution failed:", e

			# assign/print score for student's query 3
			if (fail):
				print "Student Query 3 FAILED - 0 points."
				comments += " Student Query 3 FAILED;"
			else:
				print "Student Query 3: Executed Successfully - ", per_query_pts ," points."
				points += per_query_pts

	else:	
		comments += " Error Processing Submission"
				
	# write results to gradebook and continue to next SID
	write_grade(resultsFile, sid, name_dictionary[sid], points, comments)			

	print >>sys.stderr, "Done. [Time: ", round(time.time() - current_start,2) , "secs]"
	sys.stdout.flush()
			
resultsFile.close()

total_time = time.time() - loop_start
print >> sys.stderr, "Total Elapsed Time: ", int(total_time/60), "min ", round(total_time%60.0,2) , "secs."

# switch back to the caller's directory
os.chdir(caller_dir)
