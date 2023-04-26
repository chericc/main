# git

## 版本回退

```bash
1、git reset --mixed 版本号:此为默认方式，不带任何参数的at reset，即时这种方式，它回退到某个版本，只保留源码，回跟commitoindex信息
本地工作目录内容以及暂存区内容保留
2、git reset --soft 版本号: 回退到某个版本，只回退了commt 的信息，不会恢复到index fle-级，如果还要提交，直接COmm调期可本地工作目录内容以及暂存区内容全部回退至某一个版本
3、git reset --hard 版本号: 彻底回退到某人版本，本地的源码也会变为某个版本的内容
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
