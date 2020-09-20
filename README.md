# ChunkBD (chunked block device)

Represents set of files in storage directory (ex. blk0, blk1, ...) as single seamless virtual block device. 

## Example
> WARNING!
>
> Use of program requires root privileges! All listed commands guessing you're already logged as root.

ChunkBD uses NBD linux kernel module for creating virtual block device, wich need loaded before using ChunkBD. Usualy it is pre-installed in distributives and need just be loaded:
```
modprobe nbd
```
Typical usage of ChunkBD:
```
# create real storage directry
mkdir /tmp/storageDir

# create device of 500 MiB space, wich will be represented with set of 5 MiB chunk files on real filesystem
./chunkbd -c 5242880 -s 524288000 /dev/nbd0 /tmp/storageDir

# on other terminal...
mke2fs /dev/nbd0
mount /dev/nbd0 /mnt
echo Hello, world > /mnt/test
```

After this you can see how it stores at real file system. Output should be like this:
```
$ ll /tmp/storageDir/
drwxr-xr-x  2 root root     180 сен 20 20:59 ./
drwxrwxrwt 44 root root    2520 сен 20 20:58 ../
-rw-r--r--  1 root root 4263936 сен 20 20:59 blk0
-rw-r--r--  1 root root 5242880 сен 20 20:59 blk25
-rw-r--r--  1 root root 2142208 сен 20 20:59 blk26
-rw-r--r--  1 root root 5152768 сен 20 20:59 blk51
-rw-r--r--  1 root root 5242880 сен 20 20:59 blk76
-rw-r--r--  1 root root 3190784 сен 20 20:59 blk77
-rw-r--r--  1 root root 5242880 сен 20 20:59 blk99
```

## Build

Developed on g++10, but should work at any c++20 compatible compiler.

To build just run make in project directory:
```
make
```

## Powered by BUSE
For details and license see [BUSE readme](buse/README.md)