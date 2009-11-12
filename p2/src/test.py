#!/usr/bin/python

def printhere(fd):
	print fd.readline()
	print fd.readline()

filename = "graderinput/testinputcommandsD.txt"

fd = open(filename, 'r')

n = 1
while 1:
	line = fd.readline()
	if not line:
		break
	pass
	print line
	if (n == 2):
		printhere(fd)

	n += 1
	
