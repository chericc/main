#!/bin/bash

# cd into sdk root dir
# run cmd:
# repo forall -c sh ~/git_gen_diff_dir_workspace.sh
# cp files in ~/diff_worksapce/ into this project to log diff

targetfolder=~/diff_workspace

gitdirpath=`git rev-parse --show-toplevel`
gitdirname=`basename $gitdirpath`
targetgitfolder=$targetfolder/$gitdirname

echo "--- target folder is $targetfolder"
echo "--- dirname is $gitdirname"
echo "--- dirpath is $targetgitfolder"

rm -rf $targetgitfolder

# echo "get diff files..."
files=`git diff --name-only HEAD`

if [ -z "$files" ]
then
echo "no changes"
echo ""
exit 0
fi

mkdir -p $targetgitfolder

# echo "creating git diff..."
git diff HEAD > $targetgitfolder/git.diff

# echo "archive new..."
tar -cf $targetgitfolder/new.tar $files

# echo "archive old..."
git archive -o $targetgitfolder/old.tar HEAD $files

cd $targetgitfolder
rm new -rf
rm old -rf
mkdir -p new old
tar -xf new.tar -C new
tar -xf old.tar -C old
rm new.tar old.tar 

echo ""

sleep 1