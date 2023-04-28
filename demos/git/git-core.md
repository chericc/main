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

- fill in the index file (aka cache) with the information about your working tree state.
- commit that index file as an object.

The first step is trivial: when you want to tell Git about any changes to your working tree, you use the git update-index program. That program normally just takes a list of filenames you want to update, but to avoid trivial mistakes, it refuses to add new entries to the index (or remove existing ones) unless you explicitly tell it that you're adding a new entry with the --add flag (or removing an entry with the --remove) flag.

So to populate the index with the two files you just created, you can do 

```bash
git update-index --add hello example
```

and you have now told Git to track those two files.

In fact, as you did that, if you now look into your object directory, you'll notice that Git will have added two new objects to the object database. if you did exactly the steps above, you should now be able to do

```bash
ls .git/objects/??/*
```

and see two files:

```bash
.git/objects/55/7db03de997c86a4a028e1ebd3a1ceb225be238
.git/objects/f2/4c74a2e500f5ee1332c86b94199f52b1d1d962
```

which correspond with the objects with names of 557db... and f24c7... respectively.

If you want to, you can use git cat-file to look at those objects, but you'll have to use the object name, not the filename of the object:

```bash
git cat-file -t 7db03de997c86a4a028e1ebd3a1ceb225be238
```

where the -t tells git cat-file to tell you what the "type" of the object is. Git will tell you that you have a "blob" object (i.e., just a regular file), and you can see the contents with

```bash
git cat-file blob 557db03
```

which will print out "Hello World". The object 557db03 is nothing more than the contents of your file hello.

Note: Don't confuse that object with the file hello itself. The object is literally just those specific contents of the file, and however much you later change the contents in file hello, the object we just looked at will never change. Objects are immutable.

Note: The second example demonstrates that you can abbreviate the object name to only the first several hexadecimal digits in most places.

Anyway, as we mentioned previously, you normally never actually take a look at the objects themselves, and typing long 40-character hex names is not something you'd normally want to do. The above digression was just to show that git update-index did something magical, and actually saved away the contents of your files into the Git object database.

Updating the index did something else too: it created a .git/index file. This is the index that describes your current working tree, and something you should be very aware of. Again, you normally never worry about the index file itself, but you should be aware of the fact that you have not actually really "checked in" your files into Git so far, you've only told Git about them.

However, since Git knows about them, you can now start using some of the most basic git commands to manipulate the files or look at their status.

In particular, let's not even check in the two files into Git yet, we'll start off by adding another line to hello first: 

```bash
echo "It's a new day for git" >> hello
```

and you can now, since you told Git about the previous state of hello, ask Git what has changed in the tree compared to your old index, using the git diff-files command:

```bash
git diff-files
```

Oops. That wasn't very readable. It just spit out its own internal version of a diff, but that internal version really just tells you that it has noticed that "hello" has been modified, and that the old object contents it had have been replaced with something else.

To make it readable, we call tell git diff-files to output the differences as a patch, using the -p flag:

```bash
git diff-files -p
diff --git a/hello b/hello
index 557db03..263414f 10064
--- a/hello
+++ b/hello
@@ -1 +1,2 @@
 Hello World
+It's a new day for git
```

