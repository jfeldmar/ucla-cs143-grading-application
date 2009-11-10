#!/usr/bin/python

from re import *

your_re = compile(raw_input("Expression for testing? "));

while 1:
        info = raw_input("Enter string to test: ");
        if info == "": break
        print info
        if search(your_re,info):
                print "Matches!"
        else:
                print "Does NOT match"
