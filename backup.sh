#!/bin/bash
cd /backup
filename="backup_`date +%Y`-`date +%m`-`date +%d`.tar.gz";
date 1>> backup.log
echo $filename 1>> backup.log
tar -cvf $filename $@
