#!/usr/bin/perl -w

###################################################
## load default test cases and allow user to edit them
## functionality: add/remove/save as default test cases
###################################################

use strict;

print "Content-type: text/html\n\n";
print <<ENDHTML;
<html>
<head>
<title>CS143 - Project 1A Grading Application</title>
<link rel="stylesheet" type="text/css" href="../html-css/styleSheet.css" />
</head>
<body>

<h1>CS143 - Project 1A Grading Application</h1>
<h2> Confirm/Add/Delete/Save Test Cases </h2>
ENDHTML
