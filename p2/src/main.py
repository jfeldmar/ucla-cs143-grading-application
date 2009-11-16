#!/usr/bin/python


import sys, csv, os
from subprocess import Popen, PIPE, STDOUT

# IMPORTANT IF THIS SCRIPT IS NOT RUN FROM ITS OWN DIRECTORY
#	saves caller's directory, changes to script's directory
#	at the end of script, changes back to caller's directory
caller_dir = os.getcwd()
script_dir = sys.path[0]
os.chdir(script_dir)


sys.path.append('./classes')
from default_vars import *	# loads global variables and helper function from default_vars.py
from load_tests import *	# module to load grader-specified test cases
from diff_submissions import *	# module to check differences among parts of submissions
from helper_functions import *
# steps:
#	check that grader's script is valid (store commands in array)
#	read submissions data (sid, name, files submitted)
#	for each submission
#		run grader's script
#		store score/result (short/long version)
##################################################################

############# LOAD GRADER'S TEST FILE ##############
print "=== Loading Grader's Test Commands"

# load commands to be tested on Part A
commands_part_A = load_grader_test_file(graderscriptfileA)
print "=== Found ", len(commands_part_A), " commands."

## load commands to be tested on Part D
#commands_part_D = load_grader_test_file(graderscriptfileD)
#print "=== Found ", len(commands_part_D), " commands."

############# LOAD STUDENTS' SUBMISSION DIRECTORIES/NAMES ##############
	# Load SID-Student Name tuples from  all submissions.csv files
	#	 (for each of part A, B, C, and D) and combine
	
# submitted parts (directory names)
parts = ['a', 'b', 'c', 'd']

# list storing results of grading for each student
grading_results = []

# store sid-name pairs in dictionary
name_dictionary = {}

# collect all available name-sid pairs
for part in parts:
	submission_cvs_file = submission_dir + '/' + part + '/' + submissions_data_file
	submissions_reader = csv.DictReader(open(submission_cvs_file), ['sid', 'name'], delimiter=',', quotechar='"')
	for row in submissions_reader:
		# add entry to grading results list if this sid not already added
		if (not name_dictionary.has_key(row['sid'])):
			grading_results.append( student_result(row['sid'], row['name']))
		name_dictionary[row['sid']] = row['name']


print "=== Loading Student Submissions"
# print number of name-sid pairs found
print "\tName-SID Pairs: ", len(grading_results), " entries"

# get list of all submission directories for each part
# listdir(): returns all directories in 'folder' whose name is 9 digits (i.e. SID number)
directories_a = listdirs(submission_dir_a)
directories_b = listdirs(submission_dir_b)
directories_c = listdirs(submission_dir_c)
directories_d = listdirs(submission_dir_d)
# print summary
print "\tPART A - Submissions found: ", len(directories_a)
print "\tPART B - Submissions found: ", len(directories_b)
print "\tPART C - Submissions found: ", len(directories_c)
print "\tPART D - Submissions found: ", len(directories_d)


############# COMPARE SUBMISSIONS USING DIFF ##############

# Done in compare_submissions(...) function:
#	Compare specified files between Part D and C, and Part C and B
#	store 'diff_penalty' objects in student's 'student_result' object

#print "=== Checking for major changes between file submissions (using diff)"
#num_diff_penalties = compare_submissions(submission_dir, grading_results, directories_b, directories_c, directories_d)
#print "\tFound ", num_diff_penalties, " violations of threshold value ", diff_threshold

############# RUN TEST COMMANDS ON PART A ##############
#commands_part_A
print "Commands to run on Part A: ", len(commands_part_A)

# for each student in grading_results list
#	unzip clean version of bruinbase
#	copy allowed Part A files into bruinbase
#	"make" bruinbase => make sure no errors
#
# 	for each command in list for part A
#		[ ./bruinbase < input_query.txt > stdout.txt 2> stderr.txt ]
#		run bruinbase command (with timeout, check for error result, i.e. crash)
#		if LOAD command - add points if no error/timeout
#		if SELECT command -	check if no error
#					check if output correct (add points accordingly)
#					check if # pages as expected????

student = grading_results[0]
print "=== Grading SID - ", student.sid

# create a query_result instance to store information for Bruinbase query result
RD = query_result()
RD.part = "A"

install_clean_bruinbase(bruinbase_loc, clean_bruinbase)

# copy allowed files into test bruinbase
student_files = submission_dir_a + '/' + student.sid
copy_student_files(student_files, bruinbase_loc, part_A_files)

# make bruinbase
curdir = os.getcwd()
os.chdir(bruinbase_loc)
print "\t=== Executing make"
retcode = os.system(make_bruinbase)

# if compilation failed, score 0, go to next student
if retcode != 0:
	# score = 0
	RD.query = "All queries for Part A"
	RD.score = 0
	RD.comment += " Bruinbase did not successfully compile using Part A submission"
	
	print "\t\tScore: ", RD.score
	print "\t\tComments: ", RD.comment

	# SAVE COMMAND FOR STUDENT'S RESULT
	student.results.append(RD)
else:
	print "\t=== 'make' successful"

	# execute for each scheduled command in Part A
	for tcmd in commands_part_A:

		# print command being executed
		print "\t\t", tcmd.cmd
	
		# store query
		RD.query = tcmd.cmd
		# store timeout value
		RD.max_time = min(tcmd.timeout, global_command_timeout)
		# store max pages read value
		RD.maxIOs = tcmd.maxIOs
		
		# write command to a file so it can be passed to STDIN of bruinbase process
		fd = open(temp_file, 'w')
		fd.write(tcmd.cmd)
		fd.write('\n')
		fd.close()
		fd = open(temp_file, 'r')

		# run command, get output/error stream, parse
		if (os.path.exists(bruinbase_loc)):

			# start bruinbase process and pass command as STDIN
			# if err_code = 1, error encountered
			(mstdout, err, err_code) = runCmd( run_bruinbase, fd, min(tcmd.timeout, global_command_timeout))

#			print "stdout:\n", mstdout
#			print "stderr:\n", err

			# process failed if command timed out
			# no points awarded
			if ( err_code != 0 ):
				RD.score = 0
				RD.comment += " command (" + tcmd.cmd + ") failed - " + err
#				print "Command timed out or error encountered - ", mstdout, "/", err
			# otherwise, parse output
			else:
				# for LOAD command, check for error, assign points
				if (tcmd.cmd_type == "LOAD"):
				
					# if stderr not empty, then error encountered
					if (err != ""):
						RD.score = 0
						RD.comment += "Error: " + err
						#print "stderr:\n", err
					# otherwise, give points for LOAD command
					else:
						RD.score = tcmd.points

				# for SELECT command
				# check for error, if no error
				# parse output and timing info
				#
				elif (tcmd.cmd_type == "SELECT"):
					# check if error
					#	(error can come for invalid syntax, or error returned by Bruinbase)
					(time, pages, err_str) = parse_select_stats(err)
					
					# if error parsing time/pages or error in command
					if (time < 0):
						RD.score = 0
						RD.comment += "Error running select command (" + str(tcmd.cmd) + ") output: " + str(err_str)
#						print comments
						
					# otherwise, parse and grade result
					else:
						# check if Maximum Number of IOs exceeded
						if (pages > tcmd.maxIOs):
							RD.score = 0
							RD.comment += "Command exceeded maximum number of Pages Read (" + str(tcmd.maxIOs) + ")"
							#print comments
						else:
							# Expected result format
							re_select_stdout = "^\s*Bruinbase>\s*(.+)Bruinbase>\s*$"

							# match output between "Bruinbase>", '.' matches newline, case ignored
							q_result = re.match(re_select_stdout, mstdout, re.IGNORECASE|re.S)

							# if not match, output does not match format specifications
							if q_result == None:
								RD.score = 0
								RD.comment += "Invalid Bruinbase Output for Select Query"

							# store query result
							student_result = q_result.group(1)

							# check if output correct
							file_loc = script_dir + '/' + graders_file_directory
							score, comments = grade_output(student_result, tcmd.solution, file_loc)
							RD.score = tcmd.points * score
							RD.comment += comments
				
							#print "Score: ", score
				else:
					RD.score = 0
					RD.comment += tcmd.cmd + " => unrecognized command"
#					print comments
		else:
			exit("Error: Cannot find Bruinbase source code")

		print "\t\t\tScore: ", RD.score
		print "\t\t\tComments: ", RD.comment
		
		# SAVE COMMAND FOR STUDENT'S RESULT
		student.results.append(RD)

# delete temporary file
if os.path.exists(temp_file):
	os.remove(temp_file)

os.chdir(curdir)


############# RUN TEST COMMANDS ON PART D ##############
#commands_part_D
#print "Commands to run on Part D ", len(commands_part_D)

# switch back to the caller's directory
os.chdir(caller_dir)
exit(1)


