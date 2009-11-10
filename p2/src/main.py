#!/usr/bin/python

import sys, csv
sys.path.append('./classes')
from default_vars import *;	# loads global variables and helper function from default_vars.py
from load_tests import *;	# module to load grader-specified test cases
from diff_submissions import *;	# module to check differences among parts of submissions

# steps:
#	check that grader's script is valid (store commands in array)
#	read submissions data (sid, name, files submitted)
#	for each submission
#		run grader's script
#		store score/result (short/long version)
##################################################################

############# LOAD GRADER'S TEST FILE ##############
print "=== Loading Grader's Test Commands"
commands = load_grader_test_file()
print "=== Found ", len(commands), " commands."


############# LOAD STUDENTS' SUBMISSION DIRECTORIES/NAMES ##############
	# Load SID-Student Name tuples from  all submissions.csv files
	#	 (for each of part A, B, C, and D) and combine
	
# submitted parts (directory names)
parts = ['a', 'b', 'c', 'd']

# store sid-name pairs in dictionary
name_dictionary = {}

# collect all available name-sid pairs
for part in parts:
	submission_cvs_file = submission_dir + '/' + part + '/' + submissions_data_file
	submissions_reader = csv.DictReader(open(submission_cvs_file), ['sid', 'name'], delimiter=',', quotechar='"')
	for row in submissions_reader:
		name_dictionary[row['sid']] = row['name']


# print number of name-sid pairs found
print "=== Name-SID Pairs: ", len(name_dictionary), " entries"

# get list of all submission directories for each part
directories_a = listdirs(submission_dir_a)
directories_b = listdirs(submission_dir_b)
directories_c = listdirs(submission_dir_c)
directories_d = listdirs(submission_dir_d)
# print summary
print "PART A: Submissions found: ", len(directories_a)
print "PART B: Submissions found: ", len(directories_b)
print "PART C: Submissions found: ", len(directories_c)
print "PART D: Submissions found: ", len(directories_d)


##write directories to file (i.e. SIDs) (one per line)
#outputDirFile = open(dir_file, 'w') or sys.exit("Unable to Open " + dir_file + " for storing directory names.")
#sep = "\n"
#dirs = sep.join(onlydirs)
#outputDirFile.write(dirs)
#outputDirFile.flush()
#outputDirFile.close()

############# COMPARE SUBMISSIONS USING DIFF ##############


exit(1)
