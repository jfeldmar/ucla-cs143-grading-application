#!/usr/bin/python

import sys, subprocess
from subprocess import Popen, PIPE, STDOUT

from command_classes import *;	# data structures storing test command information

# import default variables
sys.path.append('../')
from default_vars import *;	# loads global variables from ../default_vars.py

# INPUT:	location and list of submission directories for parts B, C, and D
# PROCESS:	given "allowed submission files" for each of part B, C, and D
#			use diff to make sure the changes are not higher than a given threshold
# RETURN:	List of "diff_penalty" objects containing appropriate point/percentage deductions

def compare_submissions(dir_loc, grading_results, dirs_b, dirs_c, dirs_d):

	# for each student in grading_results
	#	for each file in part D that exists in Part C (if not in C, test in B; if not in B, skip)
	#		diff the files
	#		if diff amount > threshold
	#			 add entry to student's "student_result.diff_penalties" list
	#	for each file in part C that exists in Part B
	#		diff the files
	#		if diff amount > threshold
	#			 add entry to student's "student_result.diff_penalties" list
	# return number of penalties  found
	
	num_penalties = 0
	
	for student in grading_results:
		sid = student.sid
	#	print sid
		# Compare Part D to Part C (or Part B if given file not in Part C)
		if ( find(sid, dirs_d)):
			#print "found sid in dirs d"
			for allowed_file in diff_D_vs_C:
				file_loc_d = dir_loc + "/d/" + sid + "/" + allowed_file
				#print file_loc_d
				if ( os.path.exists(file_loc_d) ):
					#print "found file in d"
					file_loc_c = dir_loc + "/c/" + sid + "/" + allowed_file				
					file_loc_b = dir_loc + "/b/" + sid + "/" + allowed_file				
					if (os.path.exists(file_loc_c)):
						diff_value_d = compare_files(file_loc_c, file_loc_d, sid)
						if (diff_value_d > diff_threshold):
							student.diff_penalties.append( diff_penalty(sid, diff_penalty_amt, "Difference between Part D and Part C for file " + allowed_file + " is " + str(diff_value_d)))
							num_penalties += 1
							#print sid, " ", allowed_file ," - diff with Part C ", diff_value_d
					elif (os.path.exists(file_loc_b)):
						diff_value_d = compare_files(file_loc_b, file_loc_d, sid)
						if (diff_value_d > diff_threshold):
							student.diff_penalties.append( diff_penalty(sid, diff_penalty_amt, "Difference between Part D and Part B for file " + allowed_file + " is " + str(diff_value_d)))
							num_penalties += 1
							#print sid, " ", allowed_file ," - diff with Part B ", diff_value_d
		# Compare Part C to Part B
		if ( find(sid, dirs_c)):
			for allowed_file in diff_C_vs_B:
				file_loc_c = dir_loc + "/c/" + sid + "/" + allowed_file
				if ( os.path.exists(file_loc_c) ):
					#print "found file in c"
					file_loc_b = dir_loc + "/b/" + sid + "/" + allowed_file				
					if (os.path.exists(file_loc_b)):
						diff_value_c = compare_files(file_loc_b, file_loc_c, sid)
						if (diff_value_c > diff_threshold):
							student.diff_penalties.append( diff_penalty(sid, diff_penalty_amt, "Difference between Part C and Part B for file " + allowed_file + " is " + str(diff_value_c)))
							num_penalties += 1
							#print sid, " ", allowed_file ," - diff with Part B ", diff_value_c
	return num_penalties



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
