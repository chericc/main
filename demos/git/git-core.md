# Git Core Tutorial

## Description

This tutorial explains how to use the "core" Git commands to set up and work with a Git repository.

If  you just need to use Git as a revision control system you may prefer to start with "A Tutorial Introduction to Git".

However, an understanding of these low-level tools can be helpful if you want to understand Git's internals.

The core Git is often called "plumbing", with the prettier user interfaces on top of it called  "porcelain". You may not want to use the plumbing directly very often, but it can be good to know what the plumbing does for when the porcelain isn't flushing.

Back when this document was originally written, many porcelain commands were shell scripts. For simplicity, it still uses them as examples to illustrate how plumbing is fit together to form the porcelain commands. The source tree includes some of these scripts in contrib / examples/ for reference. Although these are not implemented as shell scripts anymore, the description of what the plumbing layer commands do is still valid.

Note: Deeper technical details are often marked as Notes, which you can skip on your first reading.

## Creating a git repository

Creating a new Git repository couldn't be easier: all Git repositories start out empty, and the only thing you need to do is find yourself a subdirectory that you want to use as a working tree - either an empty one for a totally new project, or an existing working tree that you want to import into Git.

For out first example, we're going to start a totally new repository from scratch, with no pre-existing files, and we'll call it git-tutorial. To start up, create a subdirectory for it, change into that subdirectory, and initialize the Git infrastructure with git init:

```bash
mkdir git-tutorial
cd git-tutorial
git init
```

to which Git will reply

```bash
Initialized empty Git repository in .git/
```

which is just Git's way of saying that you haven't been doing anything strange, and that it will have created a local .git directory setup for your new project. You will now have a .git directory, and you can inspect that with ls. For your new empty project, it should show you three entries, among other things:

- a file called HEAD, that has ref: refs/heads/master in it. This is similar to a symbolic link and points at refs/heads/master relative to the HEAD file.
- a subdirectory called objects, which will contain all the objects of your project. You should never have any real reason to look at the objects directly, but you might want to know that these objects are what contains all the real data in your repository.
- a subdirectory called refs, which contains references to objects.

In particular, the refs subdirectory will contain two other subdirectories, named heads and tags respectively. They do exactly what their names imply: they contain references to any number of different heads of development, and to any tags that you have created to name specific versions in your repository.

One note: the special master head is the default branch, which is why the .git/HEAD file created points to it even if it doesn't yet exist. Basically, the HEAD link is supposed to always point to the branch you are working on right now, and you always start out expecting to work on the master branch.

However, this is only a convention, and you can name you branches anything you want, and don't have to ever even have a master branch, A number of the Git tools will assume that  .git/HEAD is valid, though.

Note: An object is identified by its 160-bit SHA-1 hash, aka object name, and a reference to an object is always the 40-byte hex representation of that SHA-1 name. The files in the refs subdirectory are expected to contain these hex references (usually with a final \n at the end), and you should thus expect to see a number of 41-byte files containing these references in these refs subdirectories when you actually start populating your tree.

Note: An advanced user may want to take a look at gitrepository-layout after finishing this tutorial.

You have now created your first Git repository. Of course, since it's empty, that's not very useful, so let's start populating it with data.

### Populating a git repository

We'll keep this simple and stupid, so we'll start off with populating a few trivial files just to get a feel for it.

Start off with just creating any random files that you want to maintain in your Git repository. We'll start off with a few bad examples, just to get a feel for how this works:

```bash
echo "Hello World" > hello
echo "Silly example" > example
```

You have now created two files in your working tree (aka working directory), but to actually check in your hard work, you will have to go through two steps:



