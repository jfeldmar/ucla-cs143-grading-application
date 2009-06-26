#!/bin/sh

sudo /etc/init.d/apache2 restart

cd /var/www/p1a/src
sudo xterm  -e 'ls && bash' &

cd ~/comprehensive/proj1a/test_submissions/submissions/project1/a/
sudo xterm -e 'ls && bash'  &

cd /var/www/p1a/src/cgi-bin
sudo nedit /var/log/apache2/error.log n3_error_process_uploads.cgi n2_process-submissions.cgi n1_initialize.cgi &

cd ~/comprehensive/proj1a
sudo nedit notes.txt notes2.txt programming_notes.txt &

firefox http://localhost/p1a/src/html-css/start.html &

exit
