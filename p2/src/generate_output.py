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

output = open(pickle_file, 'rb')
grading_results = pickle.load(output)
output.close()


############# CREATE MYUCLA GRADEBOOK FILE (TSV) ##############
# for each student
#	for results of diff, part A, and part D
#		add scores, comments
#	print sid, name, score, comments

fd = open(UCLA_Gradebook_GRADES_File, 'w')
for student in grading_results:
	total_score = 0
	total_comments = ""
	for diff_result in student.diff_penalties:
		total_score -= diff_result.penalty
		if (diff_result.comment != ""):
			if (total_comments != ""):
				total_comments += '; '
			total_comments +=  diff_result.comment
		
	for result in student.results:
		total_score += result.score
		if (result.comment != ""):
			if (total_comments != ""):
				total_comments += '; '
			total_comments +=  result.comment
		
	if (total_score < 0):
		total_score = 0
	
	# output student result (convert newlines in comments to commas)
	output = str(student.sid) + '\t' + student.name + '\t' + str(total_score) + '\t' + total_comments.replace('\n', ',') + '\n'
	fd.write(output)
fd.close()
	
############# CREATE LOG FILE ##############
# NOTE: | represents tab ('\t')
# SID | Query | student ans| time(seconds) | pages read | correct ans | time threshold | pages read threshold | score | comment | #
#####################################################################################
# SID | penalty | comment
#####################################################################################
#print results to file
fd =open(commands_log_file, 'w')
fd_diff =open(diff_log_file, 'w')

for student in grading_results:
	out_str = ""

	for diff_penalty in student.diff_penalties:
		out_str = student.sid + "\t"
		out_str += str(diff_penalty.penalty) + "\t"
		out_str += diff_penalty.comment + "\n"
		fd_diff.write(out_str)

	for result in student.results:
		out_str = student.sid + "\t"
		out_str += result.query + "\t"
		out_str += '\\n'.join(result.student_ans) + "\t"
		out_str += str(result.time) + "\t"		
		out_str += str(result.IOs) + "\t"
		out_str += '\\n'.join(result.correct_ans) + "\t"
		out_str += str(result.max_time) + "\t"
		out_str += str(result.maxIOs) + "\t"
		out_str += str(result.score) + "\t"
		out_str += result.comment + "\n"
		fd.write(out_str)
		
	fd.flush()
	fd_diff.flush()

fd.close()
fd_diff.close()


# switch back to the caller's directory
os.chdir(caller_dir)

