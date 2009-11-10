#!/usr/bin/python

import sys

# import default variables
sys.path.append('../')
from default_vars import *;	# loads global variables from ../default_vars.py

# INPUT: location and array of submission directories
# PROCESSING: given "allowed submission files" for each of part A, B, C, and D
#		use diff to make sure the changes are not higher than a given threshhold
# RETURN: Array containing appropriate point/percentage deductions

