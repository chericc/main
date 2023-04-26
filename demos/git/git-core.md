# Git Core Tutorial

## Description

This tutorial explains how to use the "core" Git commands to set up and work with a Git repository.

If  you just need to use Git as a revision control system you may prefer to start with "A Tutorial Introduction to Git".

However, an understanding of these low-level tools can be helpful if you want to understand Git's internals.

The core Git is often called "plumbing", with the prettier user interfaces on top of it called  "porcelain". You may not want to use the plumbing directly very often, but it can be good to know what the plumbing does for when the porcelain isn't flushing.



## Creating a git repository

```bash
mkdir git-tutorial
cd git-tutorial
git init
```

For your new empty project, it should show you three entries, among other things:

- a file called HEAD, that has ref: refs/heads/master in it. This is similar to a symbolic link and points at refs/heads/master relative to the HEAD file.
- a subdirectory called objects, which will contain all the objects of your project. You should never have any real reason to look at the objects directly, but you might want to know that these objects are what contains all the real data in your repository.
- a subdirectory called refs, which contains references to objects.

In particular, the refs subdirectory will contain two other subdirectories, named heads and tags respectively. They do exactly what their names imply: they contain references to any number of different heads of development, and to any tags that you have created to name specific versions in your repository.
