#get all submission SID directories

import os, sys, subprocess, time;
from subprocess import Popen, PIPE, STDOUT;
# should come from ARGV
submission_dir = "../submissions/b/"
dir_file = "output/directories.txt"
result_file = "results/results.txt"
results_dir = "results"

# remove dir_file if it exists (list of SID directories)
if ( os.path.exists(dir_file)):
	os.remove(dir_file)

# create results directory if doesn't exist
if (not os.path.exists(results_dir)):
	os.mkdir(results_dir)

# remove previous result file
if ( os.path.exists(result_file)):
	os.remove(result_file)
	
# get location of submissions if passed as argument
if len(sys.argv) != 1:
	submission_dir = sys.argv[1:]

# get list of all files and directories
directories = os.listdir(submission_dir)

# extract only directory names from directories
onlydirs = []
for d in directories:
	if ( os.path.isdir(submission_dir + d) == True ):
		onlydirs.append(d)

print "Number of directories (submissions) found: ", len(onlydirs)

#write directories to file (i.e. SIDs) (one per line)
outputDirFile = open(dir_file, 'w')
sep = "\n";
dirs = sep.join(onlydirs);
outputDirFile.write(dirs)
outputDirFile.close()

#create output file for script results
resultsFile = open(result_file, 'w')
resultsFile.write("SID, SCORE, NOTES\n")

testoutfile = open('./output/testout.txt', 'w')

# for each submission write to file output of running
# create.sql, load.sql, queries.sql, violate.sql

# 1. drop database CS143
# 2. create database CS143
# 3. for each submission
	# run create.sql
	# store success/failure score & output
	# run load.sql
	# store success/failure score & output
	# run queries.sql
	# store success/failure score & output
	# run violate.sql
	# store success/failure score & output

if (subprocess.call('mysql -u cs144 CS143 < ./scripts/dropDB-CS143.sql', shell=True) != 0):
	print "Error: Unable to drop database CS143, program terminated"	
	sys.exit()
print "Database CS143 removed"

if (subprocess.call('mysql -u cs144 < ./scripts/createDB-CS143.sql', shell=True) != 0):
	print "Error: Unable to create database CS143, program terminated"	
	sys.exit()
print "Database CS143 created"

cmd = ' mysql -u cs144 CS143 < ../submissions/b/903384091/create.sql'

try:
    process = subprocess.Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE)
    process.wait()
    retcode = process.returncode
    (stdout, stderr) = process.communicate();
    print "======================="
    print "retcode: "
    print retcode
    print "stdout: "
    print stdout
    print "stderr: "
    print stderr
    print "======================="
    
    #returncode 0 if no error, 1 if error, less than 0 if terminated by signal
    if retcode < 0:
        print >>sys.stderr, "ERROR: Child was terminated by signal", -retcode
    else:
    	if retcode == 0:
		print "Command Executed Successfully"
	else:
	        print >>sys.stderr, "ERROR: Child returned", retcode
except OSError, e:
    print >>sys.stderr, "ERROR: Execution failed:", e


testoutfile.close()
resultsFile.close()


