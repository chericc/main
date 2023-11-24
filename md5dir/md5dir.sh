#!/bin/sh

Date=$(date "+%Y%m%d%H%M%S")
find ./ -type f | while read filename
do
    md5sum $filename >> mymd5.$Date
done

exit 0

# Usage: 
# 
# 1. Generate md5:
# sh md5dir.sh
# 
# 2. Check md5:
# md5sum -c mymd5.*
#