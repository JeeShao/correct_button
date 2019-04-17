#!/bin/bash 
### BEGIN INIT INFO
# Provides:          update.sh
# Required-start:    $local_fs $remote_fs $network $syslog
# Required-Stop:     $local_fs $remote_fs $network $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6      
# Short-Description: starts the test.sh
# Description:       starts
### END INIT INFO
cd /home/app/
\cp -rf /home/app/aa.txt /home/app/update/aa.txt
#rm -rf aa.txt

#touch AAA.txt
#echo 123456|sudo ./program_qt 
# & >/dev/null
