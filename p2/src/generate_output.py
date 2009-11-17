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

output = open('data.pkl', 'rb')
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
	

#print results to file
fd =open("results_file.out", 'w')

for student in grading_results:
	out_str = "##### Grades for: " + student.sid + " (" + student.name + ") ##### " 
	fd.write(out_str)

	fd.write("Diff Among Submissions Penalties:")
	for penalty in student.diff_penalties:
		out_str = "Penalty Amount: " + str(penalty.penalty) + '\n'
		out_str += "Comment: "+ penalty.comment + '\n'
		fd.write(out_str)

	fd.write("Bruinbase Commands Results:")
	for result in student.results:
		out_str = '\t'+ result.part+ " - "+ result.query+ '\n'
		out_str += "\tMax Time/IOs: "+ str(result.max_time) + '/'+ str(result.maxIOs) + '\n'
		out_str += "\tResult Time/Score:"+ str(result.time) + '/'+ str(result.score) + '\n'
		if (result.score == 0):
			out_str += "Student Ans: "+ result.student_ans + '\n'
			out_str += "Correct Ans: "+ result.correct_ans + '\n'
		out_str += "Comments: "+ result.comment+ '\n'

		fd.write(out_str)
	fd.flush()

fd.close()


# switch back to the caller's directory
os.chdir(caller_dir)

