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

# files containing test commands for Part A and for Part D
graderscriptfileA = 'graderinput/testinputcommands_small.txt'
#graderscriptfileA = 'graderinput/testinputcommandsA.txt'
graderscriptfileD = 'graderinput/testinputcommandsD.txt'


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

# penalty of total final grade for each time the diff_threshhold is exceeded
diff_penalty_amt = 2

diff_D_vs_C = ["BTreeIndex.h", "BTreeIndex.cc", "BTreeNode.h", "BTreeNode.cc", "Bruinbase.h"] 
diff_C_vs_B = ["BTreeNode.h", "BTreeNode.cc", "SqlEngine.cc"]

# returns all directories in 'folder' whose name is 9 digits (i.e. SID number)
def listdirs(folder):
	dirs = []
	for d in (os.listdir(folder)):
		if (os.path.isdir(os.path.join(folder, d)) and re.match("^[0-9]{9}$",d)):
			dirs.append(d)
	return dirs
