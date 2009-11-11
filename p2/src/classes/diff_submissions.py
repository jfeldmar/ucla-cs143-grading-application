#!/usr/bin/python

import sys, subprocess
from subprocess import Popen, PIPE, STDOUT

# import default variables
sys.path.append('../')
from default_vars import *;	# loads global variables from ../default_vars.py

# INPUT: location and array of submission directories
# PROCESSING: given "allowed submission files" for each of part B, C, and D
#		use diff to make sure the changes are not higher than a given threshhold
# RETURN: Array containing appropriate point/percentage deductions

def compare_submissions(dir_loc, name_dictionary, dirs_b, dirs_c, dirs_d):

	for mf in part_D_files:
		print "\t", mf
#	print dir_loc
#	exit()	
	# for each sid in name_dictionary
	#	for each file in part D that exists in Part C (if not in C, test in B; if not in B, skip)
	#		diff the files
	#	for each file in part C that exists in Part B
	#		diff the files

	diff_value_d = 0
	for sid in name_dictionary.keys():
#		print sid
		# Compare Part D to Part C (or Part B if that file not in Part C)
		if ( find(sid, dirs_d)):
#			print "found sid in dirs d"
			for allowed_file in diff_D_vs_C:
				file_loc_d = dir_loc + "/d/" + sid + "/" + allowed_file
#				print file_loc_d
				if ( os.path.exists(file_loc_d) ):
#					print "found file in d"
					file_loc_c = dir_loc + "/c/" + sid + "/" + allowed_file				
					file_loc_b = dir_loc + "/b/" + sid + "/" + allowed_file				
					if (os.path.exists(file_loc_c)):
						diff_value_d = compare_files(file_loc_c, file_loc_d, sid)
#						print sid, " ", allowed_file ," - diff with Part C ", diff_value_d
					elif (os.path.exists(file_loc_b)):
						diff_value_d = compare_files(file_loc_b, file_loc_d, sid)
	#					print sid, " ", allowed_file ," - diff with Part B ", diff_value_d
		if ( find(sid, dirs_c)):
			for allowed_file in diff_C_vs_B:
				file_loc_c = dir_loc + "/c/" + sid + "/" + allowed_file
				if ( os.path.exists(file_loc_c) ):
#					print "found file in c"
					file_loc_b = dir_loc + "/b/" + sid + "/" + allowed_file				
					if (os.path.exists(file_loc_b)):
						diff_value_c = compare_files(file_loc_b, file_loc_c, sid)
						print sid, " ", allowed_file ," - diff with Part B ", diff_value_c
				
		
	diff_value = compare_files("default_vars.py", "main.py", "sid")
	if (diff_value > diff_threshhold):
		print "Dock Points (diff-", diff_value, ")"
	return 0



def compare_files(file_old, file_new, sid):
	cmd1 = "diff -y  --ignore-blank-lines --ignore-all-space --ignore-space-change --ignore-case " + file_old + " " + file_new + "  | egrep '[<>]'|wc -l"
	cmd2 = "diff -y  --ignore-blank-lines --ignore-all-space --ignore-space-change --ignore-case " + file_old + " " + file_new + "  | wc -l"
	
	error1, output1 = call_system_command(cmd1)
	error2, output2 = call_system_command(cmd2)

	# if no errors encountered
	if (not error1 and not error2):
		return round( output1/output2, 2)
	else:
		error_str = "Not successful comparing " + file_old + " and " + file_new + " for SID=" + sid
		raise Exception(error_str)
	
	

	
# calls a command in shell
# returns (error, output)
#	where:	if error = False, command succeeded
#		if error = True, command failed
#		output = shell's return value in respose to command
def call_system_command(cmd):
	try:
		process = subprocess.Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE, bufsize=0)
		(output, stderr) = process.communicate()
		retcode = process.returncode
		
		# if returncode = 0, then no error
		if (retcode == 0):
			return False, float(output.strip())
		else:
			print >>sys.stderr, "ERROR: returncode: ", retcode
			print 'stderr: ', repr(stderr)
			return True, 0
	except OSError, e:
	    print >>sys.stderr, "ERROR: Execution failed:", e
	    return 0, 0

def find(value, mylist):
	for item in mylist:
		if (item == value):
			return True
	return False
