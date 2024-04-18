# git

## 分支

```bash

# 列出分支
git branch

# 列出远程分支
git branch -r

# 切换分支
git checkout `branchname`

# 删除本地分支
git -d `branchname`

# 拉取远程分支
git checkout -b `branchname` origin/`branchname`

# 查看分支信息
git reflog --date=local --all

```

## 撤销提交

```bash

# 回退上一次提交
git reset --mixed HEAD^

# 回退两次提交
git reset --mixed HEAD~2

```

## 查看远程地址

```bash

git remote -v

```

## 撤销临时修改

```bash

# 撤销工作区的修改
git checkout filename
git checkout .

# 撤销暂存区的修改
git rm --cached filename

```

## 提交

```bash

# 从指定文件读取commit内容
git commit -F filename

# 修改上一次的提交
git commit --amend

```

## 比较

```bash

# 比较某个提交的修改 ^表示其父提交
git diff <commit-id> <commit-id>^

# 比较某个提交与其之前某个提交的修改 ~表示提前几个commit
git diff <commit-id> <commit-id>~2

```

## patch

```bash

# will generate patch from HEAD-1 to HEAD
git format-patch HEAD -1
# or
git diff ver1 ver2

# apply patch
git apply <patch-file>

# adjust apply path 

## remove <n> leading components seperated by slashes('/')
## -p2
## makes
## a/dir/file 
## --> 
## file
## default is -p1
-p<n>

## prepending <root> to all filenames(-p<n> works first)
## --directory=modules/git-gui -p1
## makes
## a/git-gui.sh
## -->
## git-gui.sh
## --> 
## modules/git-gui/git-gui.sh
--directory=<root>

```

## old & new

```bash

echo 
git archive -o 

```
