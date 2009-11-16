import os, re, subprocess, time, signal
from subprocess import Popen, PIPE, STDOUT


# returns all directories in 'folder' whose name is 9 digits (i.e. SID number)
def listdirs(folder):
	dirs = []
	for d in (os.listdir(folder)):
		if (os.path.isdir(os.path.join(folder, d)) and re.match("^[0-9]{9}$",d)):
			dirs.append(d)
	return dirs

# run command with timeout threshold
def runCmd(cmd, fd_stdin, timeout):
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

	p = subprocess.Popen(cmd, shell=True,
                	     stdout=subprocess.PIPE,
			     stdin = fd_stdin,
                	     stderr=subprocess.PIPE,
			     bufsize=0)
	# if timeout is not set wait for process to complete
	if not timeout:
	    ph_ret = p.wait()
	else:
	    fin_time = time.time() + timeout
	    while p.poll() == None and fin_time > time.time():
        	time.sleep(1)

	    # if timeout reached, return error code
	    if fin_time < time.time():

        	# starting 2.6 subprocess has a kill() method which is preferable
        	# p.kill()
        	os.kill(p.pid, signal.SIGKILL)
		return (0, "command timed out", 1)	# return timeout error

	    ph_ret = p.returncode


	ph_out, ph_err = p.communicate()

	return (ph_out, ph_err, ph_ret)

# parse
def parse_select_stats(result):
	# expected format: -- 0.000 seconds to run the select command. Read 1 pages
	stats_re = ".*\s+([0-9]*\.?[0-9]+)\s+seconds.*Read\s+([\d]+)\s+pages\s*$"
	err_re = ".*Error.*"

	if (re.match(err_re, result, re.IGNORECASE|re.S)):
		err_str =  result
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
def grade_output(student_result, grader_result, correct_dir):
	save_dir = os.getcwd()
	os.chdir(correct_dir)
	score = 0
	comment = ""

	if (not os.path.exists(grader_result)):
		err_str = "Unable to locate solution file: ", grader_result
		raise OSError(err_str)

	# create sets from grader's solution and student's result
	solution_tuples = open(grader_result, 'r').read().split('\n')

	# make sure grader's file is not empty
	if (len(solution_tuples) == 0):
		err_str = "ERROR: Unexpected Input - Grader's query output file has length 0: ", grader_result
		raise OSError( err_str )

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
			
#	print "SOLUTION:", solution_tuples
#	print "STUDENT OUT:",student_tuples
		
	# calculate score
	fraction_correct = float(len(set(solution_tuples) & set(student_tuples))) / float(len(solution_tuples))
	score = round (fraction_correct, 2)
	
	os.chdir(save_dir)
	return score, comment

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
def copy_student_files(student_files, bruinbase_loc, allowed_files):

	for allowed_file in allowed_files:
		src = student_files + '/' + allowed_file
		
		# if student submitted the file, copy it to test Bruinbase directory
		if (os.path.exists(src)):
			retcode = subprocess.call(["cp", src, bruinbase_loc])
			if retcode == 0:
				print "\t=== Copied file '", allowed_file, "' to bruinbase"
			else:
				err_str = "Error Copying Student File - ", allowed_file, " - to test bruinbase directory"
				exit(err_str)
