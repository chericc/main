# git

## branch

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

# switch branch 
git checkout <branchname>

# delete branch 
git branch -d <branchname>

# create branch 
git branch <branchname>

# connect to remote branch 
git branch -u origin/<remote-branch> <local-branch>

# info
git branch -vv

# push local branch to remote
git push origin <local-branch-name>[:<remote-branch-name>]

# track branch
git branch --set-upstream-to=origin/<branchname> <branchname>

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

# apply with-reject
git apply --reject <patch-file>

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

## git log

```bash
# log recently changes
docdir=docs/docs
logfile=$docdir/log.md
echo "# log\n" > $logfile
echo "\`\`\`" > $logfile
git log -p --since "1 month ago" $docdir | sed -E "s/\`\`\`|^Author.*|^commit.*|^diff.*//g" | sed -E "s/(^Date.*)/\`\`\`\n## \1\n\n\`\`\`/g" >> $logfile
echo "\`\`\`" >> $logfile
```

```bash
# print more info
git log --oneline --graph --decorate --all
```

## git clone --recursive failed

```bash
git submodule update --init --recursive
# or 
git clone --recursive
```

## combine

```bash
git subtree add --prefix=sdk_diff/diff ../sdk_diff_log master --squash
```

## git merge commits between two branchs

```bash
# merge some commits from branch b_a to b_b.
git checkout b_a
git log
git checkout b_b
git cherry-pick [-x] <commit-id> <start_commit>^..<end_commit> ...
# resolving conflicts ...
# then
git add .
git cherry-pick --continue

```

## resolve conficts

```bash
git checkout --ours <binary-file>
git checkout --theirs <binary-file>
```

## 回退到指定版本

```bash
git checkout <commit|tag> --recurse-submodules
```

```bash
# 重新回到某个分支
git checkout branch-name
```

## git clone size

```bash
git clone -b branch --single-branch --depth 1
# sub modules
--shallow-submodules
```