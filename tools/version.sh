#!/bin/bash

vpath="kernel/include/version.h"

os_name="Ulmer OS"
os_version=`git rev-list --count master`
os_author="Alexander Ulmer"
os_build_date=`date --utc --rfc-3339=date`

echo "#define OS_NAME \"$os_name\"" > $vpath
echo "#define OS_VERSION \"v$os_version\"" >> $vpath
echo "#define OS_AUTHOR \"$os_author\"" >> $vpath
echo "#define OS_BUILD_DATE \"$os_build_date\"" >> $vpath
