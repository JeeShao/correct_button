#!/bin/bash 
### BEGIN INIT INFO
# Provides:          start.sh
# Required-start:    $local_fs $remote_fs $network $syslog
# Required-Stop:     $local_fs $remote_fs $network $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6      
# Short-Description: starts the start.sh
# Description:       starts
### END INIT INFO
cd /home/app/
echo 123456|sudo -S ./program_qt_v1.1
# & >/dev/null
