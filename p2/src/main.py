#!/usr/bin/python

import sys
sys.path.append('./classes')
from load_tests import *;	# module to load grader specified test cases
from default_vars import *;	# loads global variables from default_vars.py

# steps:
#	check that grader's script is valid (store commands in array)
#	read submissions data (sid, name, files submitted)
#	for each submission
#		run grader's script
#		store score/result (short/long version)
##################################################################

load_grader_test_file()
exit(1)

# Load SID-Student Name tuples from submissions.csv file
# (use default_vars.py to change file name/location)
# store sid-name pairs in dictionary
submissions_reader = csv.DictReader(open(submissions_data_file), ['sid', 'name'], delimiter=',', quotechar='"')
name_dictionary = {}
for row in submissions_reader:
	name_dictionary[row['sid']] = row['name']

# get list of all files and directories
directories_a = os.listdir(submission_dir_a)
directories_b = os.listdir(submission_dir_b)
directories_c = os.listdir(submission_dir_c)
directories_d = os.listdir(submission_dir_d)

# extract only directory names from directories
onlydirs_a = []
onlydirs_b = []
onlydirs_c = []
onlydirs_d = []
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

############# COMPARE SUBMISSIONS USING DIFF ##############
