#!/bin/bash

# cd into sdk root dir
# run cmd:
# rm -rf ~/diff_workspace/; repo forall -c sh ~/git_gen_diff_dir_workspace.sh
# cp changed_files in ~/diff_worksapce/ into this project to log diff

targetfolder=~/diff_workspace

gitdirpath=`git rev-parse --show-toplevel`
gitdirname=`basename $gitdirpath`
targetgitfolder=$targetfolder/$gitdirname

# echo "--- target folder is $targetfolder"
# echo "--- dirname is $gitdirname"
# echo "--- dirpath is $targetgitfolder"

rm -rf $targetgitfolder

# echo "get diff changed_files..."
changed_files=`git diff --name-only HEAD`
new_files=`git ls-files --others --exclude-standard`

echo "`pwd`"

if [ -z "$changed_files" ] && [ -z "$new_files" ]
then
echo "no changes or new"
echo ""
exit 0
fi

echo "--> $targetgitfolder"

mkdir -p $targetgitfolder

# echo "creating git diff..."
git diff HEAD > $targetgitfolder/git.diff

# echo "archive new..."
# changed_files could be removed
tar --ignore-failed-read --warning=no-failed-read -cf $targetgitfolder/new.tar $changed_files $new_files

# echo "archive old..."
git archive -o $targetgitfolder/old.tar HEAD $changed_files

cd $targetgitfolder
rm new -rf
rm old -rf
mkdir -p new old
tar -xf new.tar -C new
tar -xf old.tar -C old
rm new.tar old.tar 

echo ""

sleep 1