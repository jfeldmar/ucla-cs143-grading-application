#!/usr/bin/python


import sys, csv, os, time, pickle
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

start_time = time.time()

############# LOAD GRADER'S TEST FILE ##############
print "=== Loading Grader's Test Commands"

# load commands to be tested on Part A
commands_part_A = load_grader_test_file(graderscriptfileA)
print "=== Found ", len(commands_part_A), " commands."

## load commands to be tested on Part D
commands_part_D = load_grader_test_file(graderscriptfileD)
print "=== Found ", len(commands_part_D), " commands."

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

print "=== Checking for major changes between file submissions (using diff)"
num_diff_penalties = compare_submissions(submission_dir, grading_results, directories_b, directories_c, directories_d)
print "\tFound ", num_diff_penalties, " violations of threshold value ", diff_threshold

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


############# RUN TEST COMMANDS ON PART A and PART D ##############
#commands_part_A
print "Commands to run on Part A: ", len(commands_part_A)
print "Commands to run on Part D ", len(commands_part_D)

for student in grading_results:
	sys.stderr.flush()

	print "=== Grading SID - ", student.sid, " - Part A"
	print >> sys.stderr, "=== Grading SID - ", student.sid, " - Part A"

	if (set_up(student, "A", part_A_files, submission_dir_a)):
		run_commands(student, commands_part_A, "A", script_dir, part_A_files, submission_dir_a)

	print "=== Grading SID - ", student.sid, " - Part D"
	print >> sys.stderr, "=== Grading SID - ", student.sid, " - Part D"

	if (set_up(student, "D", part_D_files, submission_dir_d)):
		run_commands(student, commands_part_D, "D", script_dir, part_D_files, submission_dir_d)

	#print >> sys.stderr, "Results Size: ", len(student.results)
	
#	if (grading_results.index(student) == 4):
#		break	
		
	
# delete temporary file
if os.path.exists(temp_file):
	os.remove(temp_file)

total_time = int(time.time() - start_time)
print >> sys.stderr, "Auto-Grader Execution Time: ", GetInHMS(total_time)

# calculate max total points
# max Part A points
pts_a = 0
for cmd in commands_part_A:
	pts_a += cmd.points
# max Part D points
pts_d = 0
for cmd in commands_part_D:
	pts_d += cmd.points

print >> sys.stderr,  "Max Part A Points: ", str(pts_a)
print >> sys.stderr,  "Max Part D Points: ", str(pts_d)
print >> sys.stderr,  "Max TOTAL Points: ", str(pts_a + pts_d)


output = open('data.pkl', 'wb')
pickle.dump(grading_results, output)
output.close()

os.system('python generate_output.py')

# switch back to the caller's directory
os.chdir(caller_dir)

exit(1)

