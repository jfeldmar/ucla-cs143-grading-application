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

# remove current bruinbase version
if os.path.exists('./bruinbase'):
	retcode = subprocess.call(["rm","-rf", "./bruinbase"])
	if retcode == 0:
		print "\t=== Removed ./bruinbase directory"
	else:
		print >>sys.stderr, "Error Deleting ./bruinbase directory, check directory permissions"
		exit()

# Extract clean version of bruinbase (i.e. zip given to students)
retcode = subprocess.call(["unzip","-d", "./bruinbase", clean_bruinbase])
if retcode == 0:
	print "\t=== Created Clean Bruinbase from archive file"
else:
	print >>sys.stderr, "Error Extracting Bruinbase - check that the file exists and is valid"
	exit()
	
for allowed_file in part_A_files:
	src = submission_dir_a + '/' + student.sid + '/' + allowed_file
	if (os.path.exists(src)):
		retcode = subprocess.call(["cp", src, "./bruinbase"])
		if retcode == 0:
			print "\t=== Copied file '", allowed_file, "' to bruinbase"
		else:
			print >>sys.stderr, "Error Extracting Bruinbase - check that the file exists and is valid"
			exit()
	else:
		print >>sys.stderr, "Required File Not Submitted: ", allowed_file

# make bruinbase
curdir = os.getcwd()
os.chdir('bruinbase')
print "\t=== Executing make"
retcode = os.system("make")
if retcode == 0:
	print "\t=== 'make' successful"
else:
	print >>sys.stderr, "Error - Code does not compile with submitted files"
	exit()

for tcmd in commands_part_A:
	# print command being executed
	print tcmd.cmd
	
	# write command to a file so it can be passed to STDIN of bruinbase process
	fd = open("temp.cmd", 'w')
	fd.write(tcmd.cmd)
	fd.write('\n')
	fd.close()
	fd = open("temp.cmd", 'r')
	
	# run command, get output/error stream, parse
	if (os.path.exists('bruinbase')):

		# start bruinbase process and pass command as STDIN
		(mstdout, mstderr, err_code) = runCmd('./bruinbase', fd, command_timeout)
		print "error??? ", err_code
		
		# for SELECT command, parse output and timing info
		if (tcmd.cmd_type == "SELECT"):
			# match content between "Bruinbase>", '.' matches newline, case ignored
			re_stdout = "^\s*Bruinbase>\s*(.+)Bruinbase>\s*$"
			
			q_result = re.match(re_stdout, mstdout, re.IGNORECASE|re.S)
			if q_result == None:
				sys.exit("no match found")
			print "stdout:\n", q_result.group(1)			
			print "stderr:\n", mstderr
			
			result = q_result.group(1)
			err = mstderr
			
			# check if no error
			# check if output correct
			# assign points
			
		# for LOAD command, parse error
		elif (tcmd.cmd_type == "LOAD"):
			#check if error
			#assign points
			print "stderr:\n", mstderr
		else:
			err_str = tcmd.cmd + " => unrecognized command"
			exit(err_str)
	else:
		exit("Cannot find Bruinbase source code")

os.chdir(curdir)


############# RUN TEST COMMANDS ON PART D ##############
#commands_part_D
#print "Commands to run on Part D ", len(commands_part_D)

# switch back to the caller's directory
os.chdir(caller_dir)
exit(1)


