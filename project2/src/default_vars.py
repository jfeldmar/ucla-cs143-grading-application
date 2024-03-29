import os, re

# empty bruinbase compressed file
clean_bruinbase = "../bruinbase.zip"

# directory containing the student submissions (RELATIVE TO main.py)
# format: each student's directory has name = SID and contains that student's submitted files
# format: a submission.csv file must be present in order to extract student names
submission_dir = "submissions"
submission_dir_a = submission_dir + '/a'
submission_dir_b = submission_dir + '/b'
submission_dir_c = submission_dir + '/c'
submission_dir_d = submission_dir + '/d'
submissions_data_file = "submission.csv"

# files containing test commands for Part A
# IMPORTANT: this directory is hard-coded in the sample graderscriptfile
# 	If you are changing this directory, make sure to update the location of
#	LOAD files in the graderscriptfile(s) to work correctly with respect to the
#	Bruinbase code location
graders_file_directory = 'graderinput'
graderscriptfileA = graders_file_directory + '/testinputcommandsA.txt'

# files containing test commands for Part D
graderscriptfileD = graders_file_directory + '/testinputcommandsD.txt'


# list of files submitted (including optional) for each part of the project (A, B, C, D)
# [excludes README file]

part_A_files = ["SqlEngine.cc"]

 # SqlEngine.cc is optional
part_B_files = ["BTreeNode.h", "BTreeNode.cc", "SqlEngine.cc"]

 # BTreeNode.h, BTreeNode.cc, and SqlEngine.cc are optional
part_C_files = ["BTreeIndex.h", "BTreeIndex.cc", "BTreeNode.h", "BTreeNode.cc", "SqlEngine.cc"]

 # SqlEngine.cc, BTreeIndex.h, BTreeIndex.cc, BTreeNode.h, BTreeNode.cc, and Bruinbase.h are optional
part_D_files = ["SqlEngine.h", "SqlEngine.cc", "BTreeIndex.h", "BTreeIndex.cc", "BTreeNode.h", "BTreeNode.cc", "Bruinbase.h"] 

# when checking the amount of changes made to a resubmitted file, the diff command output
# must be below this threshhold to avoid having points deducted
diff_threshold = 0.5

# penalty (in points) of total final grade for each time the diff_threshhold is exceeded
diff_penalty_amt = 2

diff_D_vs_C = ["BTreeIndex.h", "BTreeIndex.cc", "BTreeNode.h", "BTreeNode.cc", "Bruinbase.h"] 
diff_C_vs_B = ["BTreeNode.h", "BTreeNode.cc", "SqlEngine.cc"]

# threshold after which a command automatically fails if it hasn't returned yet (in seconds)
global_command_timeout = 20

temp_file = "temp.cmd"

# location of test Bruinbase
bruinbase_loc = "bruinbase"

# make command to compile Bruinbase
make_bruinbase = "make -s"

# Bruinbase executable (command)
run_bruinbase = "./bruinbase"

# MyUCLA Gradebook compatible File Destination
UCLA_Gradebook_GRADES_File = "RESULTS/cs143-TSV-Grades.tab"

# Log of running diff on submissions
diff_log_file = "RESULTS/diff_results_log.out"

# Log of running commands on submissions
commands_log_file = "RESULTS/command_results_log.out"

# Name of pickle object (stores object data before it's written to log files)
pickle_file = "RESULTS/data.pkl"
