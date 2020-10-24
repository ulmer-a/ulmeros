rm -f tools/version.h

maj_version="0"
min_version=`git rev-list --count master`

version_str="$v(maj_version).$(min_version)"

echo $(version_str)

