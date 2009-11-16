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
	ph_time = None # execution time
	

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
		
		ph_out = 0
		ph_err = "command timed out"
		ph_ret = 1
		ph_time = -1
		
		return (ph_out, ph_err, ph_ret, ph_time)	# return timeout error
		
	    ph_ret = p.returncode


	ph_out, ph_err = p.communicate()
	ph_time = fin_time

	return (ph_out, ph_err, ph_ret, ph_time)

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
def grade_output(student_result, grader_result):
	score = 0
	comment = ""

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

	if (score != 1):
		print "SOLUTION:", solution_tuples
		print "STUDENT OUT:",student_tuples
	
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
