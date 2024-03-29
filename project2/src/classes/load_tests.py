#!/usr/bin/python

import os, sys, re;
from command_classes import *;	# data structures storing test command information

# import default variables
sys.path.append('../')
from default_vars import *;	# loads global variables from ../default_vars.py

# NOTE: upper/lower case important in commands

# read command file
# for each command (skip comments), create command object and add it to the commands array
# if error encountered quit with detail of where error found
# return error_code/error_msg, commands array

def load_grader_test_file(graderscriptfile):

	# commands list which will be returned by function
	commands = []
	
	# save current working directory
	savedir = os.getcwd()
	
	# open grader's test command file
	fd = open(graderscriptfile, 'r')
	
	# switch to the directory of the grader's script file
	os.chdir(os.path.dirname(graderscriptfile))

	# read file
	while 1:
		# skip lines until not empty line and not comment line
		line = skip_comment_empty_lines(fd)

		# if reached end of file, exit loop
		if not line:
			break
		pass
		
		# extract RESTART, LOAD, or SELECT command and its appropriate information
		cmd_type, cmd, points, timeout, maxIOs, description, solution = validate_command(fd, line, graderscriptfile)
		
		y = command(cmd_type, cmd, points, timeout, maxIOs, description, solution)
		commands.append(y)
		
		print '\t', y.cmd
		
	fd.close()

	# switch to original working directory
	os.chdir(savedir)
	
	# return array of extracted commands
	return commands
		
# PARAMETERS:
#	fd			=> file descriptor to grader's test script (at correct location)
#	command1		=> current line from file containing command
#	graderscriptfile	=> name of grader's test script file
# RETURNS:
# 	cmd_type	=> command type ('LOAD' or 'SELECT')
#	cmd		=> command string
#	points		=> maximum points for a successful execution of the command
#	timeout		=> command timeout thresheld
#	maxIOs		=> maximum allowed IOs (i.e. Pages Read)
#	description	=> description of test command
#	solution_file	=> name/location of solution file (for SELECT command)

def validate_command(fd, command1, graderscriptfile):
	cmd_type = ""
	cmd = ""
	points = 0
	description = ""
	solution_file = ""	# applies only to SELECT commands
	timeout = 0
	maxIOs = 0

	# parse command
	# if run: parse RESTART/LOAD/SELECT command information
	
	#process command
	try:

		# read RESTART/LOAD/SELECT command data
		if ( is_restart_command(command1)):
			cmd_type = "RESTART"
			cmd = "RESTART"
		elif ( is_load_command(command1)):
			cmd_type = "LOAD"
			cmd = command1
		elif ( is_select_command(command1)):
			cmd_type = "SELECT"
			cmd = command1

			# skip empty lines and comments
			command2 = skip_comment_empty_lines(fd)

			# for SELECT command get solution filename
			if( os.path.exists( command2 ) ):
				solution_file = command2
			else:
				raise ValidateException(command2, "Expecting SELECT command solution file - Invalid Solution File Provided")
		else:
			raise ValidateException(command1, "Invalid Syntax - Expecting LOAD or SELECT command")

		if (cmd_type != "RESTART"):
			# skip empty lines and comments
			command3 = skip_comment_empty_lines(fd)

			# split line by whitespace characters
			tokens = command3.split()	

			# read Score/Timeout/MaxIOs/Description line
			points, timeout, maxIOs, description = get_command_info(command3, tokens)


	except ValidateException, e:
		print >> sys.stderr, "Error in Grader's Script: ", graderscriptfile
		print >> sys.stderr, "Command '", e.cmd, "' is not valid: ", e.comment
		exit()
	except IOError, e:
		print >> sys.stderr, "Error in Grader's Script: ", graderscriptfile
		print >> sys.stderr, e
		exit()
			
	return cmd_type, cmd, points, timeout, maxIOs, description, solution_file

# read max points, timeout threshold, maxIOs values from given string tokens
# PARAMETERS:
#	cmd	=>	command string
#	tokens	=>	tokens of line from grader's test script (for current command)
# RETURNS:
#	points		=>	max points allowed for current command
#	timeout		=>	timeout threshold for current command
#	maxIOs		=>	max pages read for current command
#	description	=>	description of current command
def get_command_info(cmd, tokens):
	# store POINTS/TIMEOUT/MAXIOS values (error if not number)
	try:
		points = int(tokens[0])
		timeout = int(tokens[1])	# value in seconds
		if (timeout <= 0):
			err_str = "Invalid Timeout Value, expecting value greater than 0 seconds (" + cmd + ")"
			exit(err_str)
		maxIOs = int(tokens[2])
	except ValueError:
		raise ValidateException(cmd, "Expecting line containing Points/Timeout/MaxIOs/Description Line (values must be separated by whitespace)")

	# store DESCRIPTION if not empty
	description = ""
	if (len(tokens) > 3):
		description = tokens[3]
	
	return points, timeout, maxIOs, description

# detect if given string is the RESTART command
# returns 1 if matched, 0 otherwise
def is_restart_command(cmd):
	restart_re = "\s*RESTART\s*"
	
	if (None != re.match(restart_re, cmd, re.IGNORECASE)):
		return 1
	else:
		return 0

# detect if given string is the LOAD command
# returns 1 if matched, 0 otherwise
def is_load_command(cmd):
	# regular expressions for matching LOAD statements
	# case-insensitive, filename must be included in SINGLE quotes
	load_re = "^\s*LOAD\s+[^\s]+\s+FROM\s+'([^'\s]+)'(\s+WITH\s+INDEX)?\s*"

	if (None != re.match(load_re, cmd, re.IGNORECASE)):
		# make sure file to be loaded from exists
		filename = re.match(load_re, cmd, re.IGNORECASE).group(1)
		if (os.path.exists(filename)):
			return 1
		else:
			err_str = "LOAD command target file '" + filename + "' does not exists"
			raise ValidateException(cmd, err_str)
	else:
		return 0

# detect if given string is the SELECT command
# returns 1 if matched, 0 otherwise
def is_select_command(cmd):
	# regular expressions for matching SELECT statements
	# case-insensitive
	select_re = "^\s*SELECT\s+[^\s]+\s+FROM\s+[^\s]+\s*"

	if (None != re.match(select_re, cmd, re.IGNORECASE)):
		return 1
	else:
		return 0


# input:	file descriptor, currently read line
# process:	read next line until line is not empty and doesn't have comment (i.e. starts with #)
# return:	non-trivial line
def skip_comment_empty_lines(fd):
	line = fd.readline()
	if not line:
		return line.strip()

	while (re.match("^\s*#",line) or re.match("^\s*$", line)):
		line = fd.readline()
		if not line:
			break
					
	return line.strip()
