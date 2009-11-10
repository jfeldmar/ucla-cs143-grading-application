#!/usr/bin/python

import sys;

class command:
	# parameter: one line command string
	# returns: 0 if failed, command object otherwise (change this to be external)
	def __init__( self, cmd_type, cmd, points, description):	# parse first word and return required command
		self.cmd_type = cmd_type
		self.cmd = cmd
		self.points = points
		self.description = description
	cmd_type = ""			# OPEN, CLOSE, LOAD, SELECT
	cmd = ""
	points = 0
	description = ""

class ValidateException(Exception):		
	def __init__(self, cmd, comment):
		self.cmd = cmd
		self.comment = comment
	def __str__(self):
		return repr(self.cmd, self, comment)
