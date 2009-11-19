import os, sys, subprocess, getopt, re, time, signal;
from subprocess import Popen, PIPE, STDOUT;

# construct string for MyUCLA Gradebook TSV file entry and write to file
# format: sid <tab> name <tab> score <tab> [comment]
def write_grade(fd, sid, name, score, comment):
	try:
		total_score = str(round(score,1))
		out_str = str(sid) + "\t" + str(name) + "\t"+ str(total_score) + "\t" + str(comment) + "\n"
		fd.write(out_str)
		fd.flush()
	except OSError, e:
		print >>sys.stderr, "Unable to store gradebook entry for SID: " + sid
		exit()
	return

# given a set of expected results and a set of student's results
# assign score (0 to 1.0) based on how many results are matching
def grade(grader_results, student_results, q_num):	
	if (len(grader_results) == 0):
		print "ERROR: Unexpected Input - Grader's query output file has length 0"
		return -1

	# if results all match, score is 100
	fraction_correct = float(len(grader_results & student_results)) / float(len(grader_results))
	score = round (fraction_correct, 2)
	if (score != 1):
		print "Query ", q_num, ": expecting " , '\\n'.join(grader_results), " got ", '\\n'.join(student_results)		
	return score

# SPLIT UP THE QUERIES IN THE STUDENT'S SUBMITTED FILE
# (use semi-colon as separator)
def extract_queries(students_all_queries_file, num):
	all_student_queries = open(students_all_queries_file, 'r').read()
	all_queries = all_student_queries.split(';')
	queries = []
	processed_queries = 0
	for q in all_queries:
		# process the first num queries only
		if (processed_queries >= num):
			break
		# process non-empty queries only
		if ( len(q.strip()) != 0):
			queries.append(q)
			processed_queries += 1
	return queries

# RUN GIVEN GRADER'S QUERY ON STUDENT'S DATABASE AND COMPARE RESULTS TO GRADER'S RESULTS
def run_query(query, grader_result, num, timeout):
	fail_query, score, toprint = (0,0,0)
	(failed, stdout, stderr, retcode) = runCmd(query, timeout)

	if (failed):
		error_str = 'Command timed out (timeout = ', timeout, ' seconds).'
		return failed, score, error_str

	#returncode 0 if no error, 1 if error, less than 0 if terminated by signal
	try:
		if retcode == 0:
		    fail_query = 0
		    student_result = stdout.splitlines()
		    
		    # remove empty entries
		    for line in student_result:
			    if not line.strip():
				    student_result.remove(line)

		    # remove field names line
		    if (len(student_result) != 0):
	    		student_result.pop(0)

		    score = grade(grader_result, set(student_result), num)
		    toprint = "Query ", num , " Score: ", score
		    if score != 1:
		    	toprint += "\n" , "Student's Result:", "\n"
			for r in student_result:
				toprint += r, "\n"
		else: 
		    fail_query = 1
		    if retcode < 0:
        		toprint = "ERROR: Child was terminated by signal", -retcode
		    else:
			toprint = "ERROR: returncode: " + str(retcode)

		    toprint += " when executing command: " + query
		    toprint +='stderr: ' + repr(stderr)
	except OSError, e:
		print >>sys.stderr, "ERROR: Execution failed:", e
	return fail_query, score, toprint
	

# RUN GIVEN STUDENT'S QUERY AND MAKE SURE IT EXECUTES WITHOUT ERROR
def test_query(student_query, timeout):
	(fail_query, toprint) = (1, "")	# initialize variable

	(failed, stdout, stderr, retcode) = runCmd(student_query, timeout)
	
	if (failed):
		error_str = 'Command timed out (timeout = ' + str(timeout) + ' seconds).'
		return failed, error_str

	#returncode 0 if no error, 1 if error, less than 0 if terminated by signal
	try:
		if retcode == 0:
		    fail_query = 0
		    toprint = ""
		else: 
		    fail_query = 1
		    if retcode < 0:
        		toprint = "ERROR: Child was terminated by signal: " + stderr
		    else:
			toprint = "ERROR: " + stderr

		    toprint += " when executing command: " + student_query
		    toprint += "stderr: " + repr(stderr)
	except OSError, e:
		print >>sys.stderr, "ERROR: Execution failed:", e
	return fail_query, toprint

# Run Command with a Timeout value
def runCmd(cmd, timeout):
	'''
	Will execute a command, read the output and return it back.

	@param cmd: command to execute
	@param timeout: process timeout in seconds
	@return: a tuple of three: first stdout, then stderr, then exit code
	@raise OSError: on missing command or if a timeout was reached
	'''

	ph_out = None # process output
	ph_err = None # stderr
	ph_ret = None # return code

	p = subprocess.Popen(cmd, shell=True,
                	     stdout=subprocess.PIPE,
                	     stderr=subprocess.PIPE,
			     bufsize=0)
	# if timeout is not set wait for process to complete
	if not timeout:
	    ph_ret = p.wait()
	else:
	    fin_time = time.time() + timeout
	    while p.poll() == None and fin_time > time.time():
        	time.sleep(1)

	    # if timeout reached, raise an exception
	    if fin_time < time.time():

        	# starting 2.6 subprocess has a kill() method which is preferable
        	# p.kill()
        	os.kill(p.pid, signal.SIGKILL)
		return (1, 0, 0, 1)	# return timeout error

	    ph_ret = p.returncode

	ph_out, ph_err = p.communicate()

	return (0, ph_out, ph_err, ph_ret)

