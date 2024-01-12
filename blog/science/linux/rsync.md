# rsync

## common usage

```bash

# eg1 rsync -av src_dir/ dst_dir/

```

## option -a

```bash

-a --> -rlptgoD

means:

-r recursive
-l copy links
-p permission same
-t update modification times on remote side
-g same group
-o same owner
-D copy devices files and special files

```

## option -n

```bash

perform a trial run with no changes made.

```

## option --delete

```bash

delete extraneous files from dest files.
that is, keep dst same as src.

```

## option -c

```bash

skip based on checksum, not mod-time & size

```

## option -bwlimit

```bash

socket speed, KB/s

```