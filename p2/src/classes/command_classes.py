import sys;

class command:
	# parameter: one line command string
	# returns: 0 if failed, command object otherwise (change this to be external)
	def __init__( self, cmd_type, cmd, points, timeout, maxIOs, description, solution):	# parse first word and return required command
		self.cmd_type = cmd_type	# LOAD or SELECT
		self.cmd = cmd
		self.points = int(points)
		self.timeout = int(timeout)
		self.maxIOs = int(maxIOs)
		self.description = description
		self.solution  = solution

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
		self.score = 0
		self.comment = ""
		# list of "query_results" objects
		self.results = []
		# list of "diff_penalty" objects
		self.diff_penalties = []
	
# stores details query result
class query_result:
	def __init__(self):
		self.query = ""
		self.part = ""		# which submitted part is being graded (A,B,C, or D)
		self.max_time = 0.0
		self.maxIOs = 0		# default value, should be reset according to config file
		self.student_ans = ""
		self.correct_ans = ""
		self.time = 0		# in seconds (-1 if command timed out or process failed)
		self.score = 0
		self.comment = ""

# stores data for penalty to be applied if diff between a pair of files exceeds threshold
class diff_penalty:
	def __init__(self, sid, penalty, comment):
		self.sid = sid
		self.penalty = penalty
		self.comment = comment
	
