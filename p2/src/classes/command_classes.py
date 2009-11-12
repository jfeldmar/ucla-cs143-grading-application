#!/usr/bin/python

import sys;

class command:
	# parameter: one line command string
	# returns: 0 if failed, command object otherwise (change this to be external)
	def __init__( self, cmd_type, cmd, points, description, solution):	# parse first word and return required command
		self.cmd_type = cmd_type
		self.cmd = cmd
		self.points = points
		self.description = description
		self.solution  = solution
	cmd_type = ""			# LOAD or SELECT
	cmd = ""
	points = 0
	description = ""
	solution_file = ""

class ValidateException(Exception):		
	def __init__(self, cmd, comment):
		self.cmd = cmd
		self.comment = comment
	def __str__(self):
		return repr(self.cmd, self, comment)

# stores data per student for MyUCLA Gradebook format: sid <tab> name <tab> score <tab> [comment]
class student_result:
	def __init__(self, sid, name):
		self.sid = sid
		self.name = name
	score = 0
	comment = ""
	# list of "query_results" objects
	results = []
	# list of "diff_penalty" objects
	diff_penalties = []
	
# stores details query result
class query_result:
	query = ""
	student_ans = ""
	time = ""		# in seconds
	correct_ans = ""
	threshhold = 1		# default value, should be reset according to config file
	score = 0
	comment = ""

# stores data for penalty to be applied if diff between a pair of files exceeds threshold
class diff_penalty:
	def __init__(self, sid, amount, comment):
		self.sid = sid
		self.amount = amount
		self.comment = comment
	
