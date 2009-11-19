import sys, os, re, subprocess, time, signal
from subprocess import Popen, PIPE, STDOUT

sys.path.append('./classes')
from default_vars import *	# loads global variables and helper function from default_vars.py
from command_classes import *

# returns all directories in 'folder' whose name is 9 digits (i.e. SID number)
def listdirs(folder):
	dirs = []
	for d in (os.listdir(folder)):
		if (os.path.isdir(os.path.join(folder, d)) and re.match("^[0-9]{9}$",d)):
			dirs.append(d)
	return dirs


# Re-installs clean version of Bruinbase (base code given to students)
# Copies student's submitted files to Bruinbase (if none, score = 0; if directory not found, score = 0)
# "MAKE" Bruinbase solution (if compile error, score = 0)
# RETURNS: 1 if success, 0 if error encountered

def set_up(curr_student, part, allowed_files, curr_submission_dir):

	install_clean_bruinbase(bruinbase_loc, clean_bruinbase)

	# copy allowed files into test bruinbase (if student submitted any)
	student_files = curr_submission_dir + '/' + curr_student.sid
	
	if (not os.path.exists(student_files)):
		RD = query_result()
		RD.part = part	
		RD.query = "All queries for SID " + curr_student.sid
		RD.score = 0
		RD.comment += " Student Submission Directory not found."
		
		print "\t\tScore: ", RD.score
		print "\t\tComments: ", RD.comment
		
		curr_student.results.append(RD)
		return 0
		
	num_copied = copy_student_files(student_files, bruinbase_loc, allowed_files)

	if (num_copied == 0):
		RD = query_result()
		RD.part = part	
		RD.query = "All queries for Part " + RD.part
		RD.score = 0
		RD.comment += " No Files were submitted for Part " + RD.part
		
		print "\t\tScore: ", RD.score
		print "\t\tComments: ", RD.comment
		
		curr_student.results.append(RD)

		return 0

	# make bruinbase
	print "\t=== Executing make"
	(mstdout, err, err_code, time) = runCmd(make_bruinbase, None, 0, bruinbase_loc) 	# stdin=subprocess.PIPE, timeout 0 (i.e. not timed)

	# if compilation failed, score 0, go to next student
	if err_code != 0:
		# score = 0 for this submission (for current Part)
		RD = query_result()
		RD.part = part	
		RD.query = "All queries for Part " + RD.part
		RD.score = 0
		RD.comment += " Bruinbase did not successfully compile using Part " + RD.part + " submission"

		print "\t\tScore: ", RD.score
		print "\t\tComments: ", RD.comment

		curr_student.results.append(RD)

		return 0
		
	print "\t=== 'make' successful"

	return 1

def run_commands(curr_student, commands, part, script_dir, allowed_files, curr_submission_dir):
	# execute for each scheduled command in selected Part
	for tcmd in commands:
	
		# print command being executed
		print "\t\t", tcmd.cmd

		# if command is RESTART
		# 	run set up again:
		#		unzip clean bruinbase solution
		#		copy student's files into clean solution
		#		run make
		if (tcmd.cmd_type == 'RESTART'):
			set_up(curr_student, part, allowed_files, curr_submission_dir)
			continue

		RD = query_result()
		RD.part = part

		# store query
		RD.query = tcmd.cmd
		# store timeout value
		RD.max_time = min(tcmd.timeout, global_command_timeout)
		# store max pages read value
		RD.maxIOs = tcmd.maxIOs
		# store expected solution

		# write command to a file so it can be passed to STDIN of bruinbase process
		fd = open(temp_file, 'w')
		fd.write(tcmd.cmd)
		fd.write('\n')
		fd.close()
		fd = open(temp_file, 'r')
		
		# run command, get output/error stream, parse
		if (os.path.exists(bruinbase_loc)):

			# start bruinbase process and pass command as STDIN
			# if err_code = 1, error encountered
			(mstdout, err, err_code, time) = runCmd( run_bruinbase, fd, min(tcmd.timeout, global_command_timeout), bruinbase_loc)

			# store the actual amount of time the command took to run
			# not the time output by the command
			RD.time = time

#			print "stdout:\n", mstdout
#			print "stderr:\n", err

			# process failed if command timed out
			# no points awarded
			if ( err_code != 0 ):
				RD.score = 0
				RD.comment += " command (" + tcmd.cmd + ") failed - " + err
#				print "Command timed out or error encountered - ", mstdout, "/", err
			# otherwise, parse output
			else:
				# for LOAD command, check for error, assign points
				if (tcmd.cmd_type == "LOAD"):

					# if stderr not empty, then error encountered
					if (err != ""):
						RD.score = 0
						RD.comment += err
						#print "stderr:\n", err
						
					# otherwise, give points for LOAD command
					else:
						RD.score = tcmd.points

				# for SELECT command
				# check for error, if no error
				# parse output and timing info
				#
				elif (tcmd.cmd_type == "SELECT"):
					# check if error
					#	(error can come for invalid syntax, or error returned by Bruinbase)
					(time, pages, err_str) = parse_select_stats(err)
					
					# store the number of pages read
					RD.IOs = pages

					# if error parsing time/pages or error in command
					if (time < 0):
						RD.score = 0
						#err_str
						RD.comment += "Error running select command (" + str(tcmd.cmd) + ") output=> " + str(err_str)
#						print comments

					# otherwise, parse and grade result
					else:
						# check if Maximum Number of IOs exceeded
						if (pages > tcmd.maxIOs):
							RD.score = 0
							RD.comment += "Command exceeded maximum number of Pages Read (" + str(tcmd.maxIOs) + "maxIOs, command: " + tcmd.cmd + ")"
							#print comments
						else:
							# Expected result format
							re_select_stdout = "^\s*Bruinbase>\s*(.+)Bruinbase>\s*$"

							# match output between "Bruinbase>", '.' matches newline, case ignored
							q_result = re.match(re_select_stdout, mstdout, re.IGNORECASE|re.S)

							# if not match, output does not match format specifications
							if q_result == None:
								RD.score = 0
								RD.comment += "Invalid Bruinbase Output for Select Query"

							# store query result
							student_result = q_result.group(1)

							# check if output correct
							solution_file = script_dir + '/' + graders_file_directory + '/' + tcmd.solution

							if (not os.path.exists(solution_file)):
								err_str = "Unable to locate solution file: ", solution_file
								raise OSError(err_str)

							# read grader's solution file
							grader_result = open(solution_file, 'r').read()

							score, comments, solution_tuples, student_tuples = grade_output(student_result, grader_result, tcmd.cmd)

							RD.correct_ans = solution_tuples
							RD.student_ans = student_tuples
							RD.score = tcmd.points * score
							RD.comment += comments

							#print "Score: ", score
				else:
					RD.score = 0
					RD.comment += tcmd.cmd + " => unrecognized command"
		else:
			exit("Error: Cannot find Bruinbase source code")

		fd.close()
		
		print "\t\t\tScore: ", RD.score
		print "\t\t\tComments: ", RD.comment
		
		curr_student.results.append(RD)

	return






# run command with timeout threshold
def runCmd(cmd, fd_stdin, timeout, change_dir):
	'''
	Will execute a command, read the output and return it back.

	@param cmd: command to execute
	@param timeout: process timeout in seconds
	@return: a tuple of three: first stdout, then stderr, then exit code
			if exit code = 1, error encountered
	'''

	ph_out = None # process output
	ph_err = None # stderr
	ph_ret = None # return code
	ph_time = None # execution time
	

	p = subprocess.Popen(cmd, shell=True,
                	     stdout=subprocess.PIPE,
			     stdin = fd_stdin,
                	     stderr=subprocess.PIPE,
			     cwd = change_dir,
			     bufsize=0)
	# if timeout is not set wait for process to complete
	if not timeout:
		ph_ret = p.wait()
	else:
		start_time = time.time()
		fin_time = time.time() + timeout
		while p.poll() == None and fin_time > time.time():
			time.sleep(1)

		# if timeout reached, return error code
		now = time.time()
		if fin_time < now:

			# starting 2.6 subprocess has a kill() method which is preferable
			# p.kill()
			os.kill(p.pid, signal.SIGKILL)

			ph_out = 0
			ph_err = "command timed out"
			ph_ret = 1
			ph_time = -1

			return (ph_out, ph_err, ph_ret, ph_time)	# return timeout error

		ph_ret = p.returncode
		ph_time = int(now - start_time)


	ph_out, ph_err = p.communicate()

	return (ph_out, ph_err, ph_ret, ph_time)

# parse
def parse_select_stats(result):
	# expected format: -- 0.000 seconds to run the select command. Read 1 pages
	stats_re = ".*--\s+([0-9]*\.?[0-9]+)\s+seconds.*Read\s+([\d]+)\s+pages\s*$"
	err_re = ".*Error.*"

	if (re.match(err_re, result, re.IGNORECASE|re.S)):
		err_str =  result
		stats_re = "--\s+([0-9]*\.?[0-9]+)\s+seconds.*Read\s+([\d]+)\s+pages\s*$"
		err_str = re.sub(stats_re, '', err_str)
		return -1, 0, err_str
	elif (None == re.match(stats_re, result, re.IGNORECASE|re.S)):
		err_str = result, ": Invalid syntax for Timing/Page Data"
		return -1, 0, err_str
	else:
		time = re.match(stats_re, result, re.IGNORECASE|re.S).group(1)
		pages = re.match(stats_re, result, re.IGNORECASE|re.S).group(2)
		print "\t\t\tTime: ", time
		print "\t\t\tPages: ", pages
		return float(time), int(pages), ""

# compare student's output to expected output for current SELECT query
# student_result is a string
# grader_result is a file
# RETURNS:	score (0 <= score <= 1)
#		comments ( information about score, if necessary)
def grade_output(student_result, grader_result, cmd):
	score = 0
	comment = ""

	# create sets from grader's solution and student's result
	
	solution_tuples = grader_result.split('\n')
	
	# remove empty entries
	for line in solution_tuples:
		if not line.strip():
			solution_tuples.remove(line)

	# If last line of grader's results is timing/pages read data, remove it		
	# expected format: -- 0.000 seconds to run the select command. Read 1 pages
	stats_re = ".*\s+([0-9]*\.?[0-9]+)\s+seconds.*Read\s+([\d]+)\s+pages\s*$"
	if (re.match(stats_re, solution_tuples[-1], re.IGNORECASE|re.S)):
		solution_tuples.pop(-1)

	# parse student solution into a list
	student_tuples = student_result.splitlines()
	# remove empty entires
	for line in student_tuples:
		if not line.strip():
			student_tuples.remove(line)
					
	# calculate score
	fraction_correct = float(len(set(solution_tuples) & set(student_tuples))) / float(len(solution_tuples))
	score = round (fraction_correct, 2)
	
	# score is either 0% or 100% (not partial score allowed)
	if (score != 1):
		score = 0

	if (score != 1):
		comment = "SELECT command result is incorrect (command: " + cmd + ")"
#		print "SOLUTION:", solution_tuples
#		print "STUDENT OUT:",student_tuples
	
	return score, comment, solution_tuples, student_tuples

def install_clean_bruinbase(bruinbase_loc, clean_bruinbase):
	# remove current bruinbase version
	if os.path.exists(bruinbase_loc):
		retcode = subprocess.call(["rm","-rf", bruinbase_loc])
		if retcode == 0:
			print "\t=== Removed ", bruinbase_loc, " directory"
		else:
			err_str = "Error Deleting " + bruinbase_loc + " directory, check directory permissions"
			exit(err_str)

	# Extract clean version of bruinbase (i.e. zip given to students)
	retcode = subprocess.call(["unzip", "-q", "-d", bruinbase_loc, clean_bruinbase])
	if retcode == 0:
		print "\t=== Created Clean Bruinbase from archive file"
	else:
		exit("Error Extracting Bruinbase - check that the file exists and is valid")

# copy student submitted files into test Bruinbase
# if file doesn't exists in student's submission, do nothing
# only copies files which are allowed (specified by grader)
# RETURNS: number of files copied
def copy_student_files(student_files, bruinbase_loc, allowed_files):

	num_copied = 0
	
	for allowed_file in allowed_files:
		src = student_files + '/' + allowed_file
		
		# if student submitted the file, copy it to test Bruinbase directory
		if (os.path.exists(src)):
			retcode = subprocess.call(["cp", src, bruinbase_loc])
			if retcode == 0:
				print "\t=== Copied file '", allowed_file, "' to bruinbase"
				num_copied += 1
			else:
				err_str = "Error Copying Student File - ", allowed_file, " - to test bruinbase directory"
				exit(err_str)
	return num_copied

def GetInHMS(seconds):
    hours = seconds / 3600
    seconds -= 3600*hours
    minutes = seconds / 60
    seconds -= 60*minutes
    if hours == 0:
    	if minutes == 0:
	        return "%02d secs" % (seconds)
	else:
	        return "%02d mins %02d secs" % (minutes, seconds)
    else:
	return "%02d hrs %02d mins %02d secs" % (hours, minutes, seconds)
