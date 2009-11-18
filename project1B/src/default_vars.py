########################################################
# THESE VARIABLE MAYBE CHANGED BY THE USER AS DESIRED
# IMPORTANT: Follow Python Syntax to avoid syntax error
#	(change at your own risk and backup original)
########################################################

test_DB = "CS143"
user = "cs144"

# timeout time for each mysql command (in seconds)
timeout = 10

# directory containing the student submissions
# format: each student's directory has name = SID and contains that student's submitted files
# format: a submission.csv file must be present in order to extract student names
submission_dir = "../submissions/b"
#submission_dir = "../submissions/test/"
submissions_data_file = submission_dir + "/submission.csv"

# name of create and load scripts submitted by students
# (this is submitted by the students and name should be given in the project specification)
create_script = "create.sql"
load_script = "load.sql"
#violate_script = "violate.sql"		#not tested/graded by this script

# name of file which includes the queries written by students
# (this is submitted by the students and name should be given in the project specification)
students_all_queries_file = "queries.sql"

# Number of queries submitted by student
# if changing value, adjust the way points are assigned accordingly
num_queries = 3

# this is the name of the UCLA MyGradebook compatible file
# the verbose result is printer to stdout and can be logged using redirection (>)
# note: the violate.sql script is not tested here, it must be checked manually
results_dir = "RESULTS"
result_file = results_dir + "/results.tab"

# This script tests the output for 2 queries provided by grader here
# if changing the number of queries, please edit code accordingly
queries_dir = "test_queries"
query1_file = queries_dir + "/test_query1.sql"
query2_file = queries_dir + "/test_query2.sql"
graders_query1_output = queries_dir + "/out_q1.txt"
graders_query2_output = queries_dir + "/out_q2.txt"


################# POINT VALUES ######################
#create ran successfully
create_pts = 10

#load ran successfully
load_pts = 10

#query1 - correct output/no errors (grader's test query)
query1_no_error_pts = 3
query1_pts = 7

#query2 - correct output/no errors (grader's test query)
query2_no_error_pts = 3
query2_pts = 7

#queries.sql - student's queries - correct number of queries and queries ran without error
has_all_queries_pts = 3		# has correct number of student queries
student_queries_pts = 7		# all student queries run without error (otherwise deduct points)

#NOTE: violate.sql is not tested in this program


######################################################
# THESE VARIABLE SHOULD NOT BE CHANGED BY THE USER
# (edit at your own risk)
########################################################

dir_file = results_dir + "/SIDs.txt"

drop_DB_script = "./scripts/dropDB.sql"
create_DB_script = "./scripts/createDB.sql"

sql_restart_command = '/etc/init.d/mysql restart'


