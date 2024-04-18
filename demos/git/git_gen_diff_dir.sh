targetfolder=~/diff

echo "target folder is $targetfolder"

if [ $# != 2 ] 
then
    echo "usage: $0 <old_id> <new_id>"
    exit 1
fi

mkdir -p $targetfolder

git diff $1 $2 > $targetfolder/git.diff

files=`git diff --name-only $1 $2`
git archive -o $targetfolder/new.tar $2 $files
git archive -o $targetfolder/old.tar $1 $files
cd $targetfolder
rm new -rf
rm old -rf
mkdir -p new old
tar -xf new.tar -C new
tar -xf old.tar -C old
rm new.tar old.tar -f