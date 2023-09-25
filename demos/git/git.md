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

```

## git review 前

（1）

```bash
git pull servers_0 master
```

（2）如果有冲突并本地还未commit

```bash
git stash save "save message"
git pull servers_0 master
git stash list 查看stash备份目录
git stash apply stash@{0} 将备份目录中的stash@{0}的代码释放，与远程最新代码合并，再手动修改冲突
```

（3）如果有冲突并本地已commit

```bash
git merge --abort
git rebase origin/master
```

手动修改冲突

```bash
git add 修改的文件名
git rebase --continue
```



## git review 后

（1）已通过评审但有冲突导致无法submit

```bash
git fetch origin
git rebase origin/master
```

手动修改冲突

```bash
git add 修改的文件名
git rebase --continue
git review
```

（2）评审有问题需要修改的情况

```
git commit --amend --no-edit
```
