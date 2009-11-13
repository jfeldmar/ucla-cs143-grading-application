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
	@raise OSError: on missing command or if a timeout was reached
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

	    # if timeout reached, raise an exception
	    if fin_time < time.time():

        	# starting 2.6 subprocess has a kill() method which is preferable
        	# p.kill()
        	os.kill(p.pid, signal.SIGKILL)
		return (1, 0, 1)	# return timeout error
#		print >>sys.stderr, "MySQL command ", cmd, " timed out. Aborted"
#        	raise OSError("Process timeout has been reached")

	    ph_ret = p.returncode


	ph_out, ph_err = p.communicate()

	return (ph_out, ph_err, ph_ret)

