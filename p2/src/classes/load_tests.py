#!/usr/bin/python

import os, sys, re;
from command_classes import *;	# data structures storing test command information

# import default variables
sys.path.append('../')
from default_vars import *;	# loads global variables from ../default_vars.py

# upper/lower case important in commands

# read command file
# for each command (skip comments), create command object and add it to the commands array
# if error encountered quit with detail of where error found
# return error_code/error_msg, commands array

def load_grader_test_file():
	commands = []
	# save current working directory
	savedir = os.getcwd()
	f = open(graderscriptfile, 'r')
	# change to the directory of the grader's script file
	os.chdir(os.path.dirname(graderscriptfile))
	
	for line in f:
		# skip comment lines and empty lines
		if (re.match("^\s*#",line) or re.match("^\s*$", line)):
			continue
		cmd_type, cmd, points, description = validate_command(line)
		y = command(cmd_type, cmd, points, description)
		commands.append(y)
		print '\t', y.cmd
		
	f.close()
	# change to original working directory
	os.chdir(savedir)
	
	# return array of extracted commands
	return commands
		

def validate_command(command):
	cmd_type = ""
	cmd = ""
	points = 0
	description = ""
	
	# parse command
	# if open: ./bruinbase
	# if close: QUIT
	# if run: open file, parse LOAD/SELECT information
	
	# remove unnecessary whitespace
	command = command.strip()
	
	#process command
	try:
		# skip empty commands
		if ( len(command) != 0):
			tokens = command.split()	# split by whitespace characters
			
			if (tokens[0] == 'open'):
				cmd_type = 'open'
				cmd = './bruinbase'
			elif (tokens[0] == 'close'):
				cmd_type = 'close'
				cmd = 'QUIT'
			elif (tokens[0] == 'run'):
				if (len(tokens) < 2):
					raise ValidateException(command, "Insufficient Number of Parameters")
					
				cmd_type = 'run'
				# open filename (second parameter)
				fd = open(tokens[1], 'r')

				# read line which stores Points and Description
				line1_tokens = fd.readline().split()

				# store POINTS value (error if not integer)
				if (len(line1_tokens[0]) < 1):
					raise ValidateException(command, "Points Value not specified in line 1 of "+ tokens[1])
				if (not line1_tokens[0].isdigit()):
					raise ValidateException(command, "Points Value not a positive integer in line 1 of "+ tokens[1])
				else:
					points = line1_tokens[0]

				# store DESCRIPTION if not empty
				if (not line1_tokens[1] == ""):
					description = line1_tokens[1].strip()

				# store COMMAND if not empty (otherwise error)
				cmd = fd.readline().strip()
				if (cmd == ""):
					raise ValidateException(command, "Bruinbase command cannot be empty")
				
				# make sure the syntax of speficied LOAD/SELECT command is valid
				if (not valid_load_select_command(cmd)):
					error_str = "Invalid Syntanx in specified LOAD/SELECT command in line 2 of " + tokens[1] + " (for LOAD command, check if file exists)."
					raise ValidateException(command, error_str)
					
				# if SELECT command make sure second filename parameter exists
				if (None != re.match("^\s*SELECT", cmd, re.IGNORECASE)):
					if (len(tokens) < 3):
						raise ValidateException(cmd, "Invalid Solution File Provided")
					if (not os.path.exists(tokens[2])):
						raise ValidateException(cmd, "Invalid Solution File Provided")

				fd.close()
			else:
				raise ValidateException(command, "Invalid Syntax")

	except ValidateException, e:
		print >> sys.stderr, "Error in Grader's Script: ", graderscriptfile
		print >> sys.stderr, "Command '", e.cmd, "' is not valid: ", e.comment
		exit()
	except IOError, e:
		print >> sys.stderr, "Error in Grader's Script: ", graderscriptfile
		print >> sys.stderr, e
		exit()
			
	return cmd_type, cmd, points, description


def valid_load_select_command(cmd):
	# regular expressions for matching LOAD and SELECT statements
	# case-insensitive, filename optionally included in SINGLE quotes
	load_re = "^\s*LOAD\s+[^\s]+\s+FROM\s+'?([^'\s]+)'?(\s+WITH\s+INDEX)?\s*"
	select_re = "^\s*SELECT\s+[^\s]+\s+FROM\s+[^\s]+\s*"

	# See if load command matches
	if (None != re.match(load_re, cmd, re.IGNORECASE)):
		# make sure file to be loaded from exists
		filename = re.match(load_re, cmd, re.IGNORECASE).group(1)
		if (os.path.exists(filename)):
			return 1
		else:
			return 0
	elif (None != re.match(select_re, cmd, re.IGNORECASE)):
		return 1
	else:
		return 0
	
