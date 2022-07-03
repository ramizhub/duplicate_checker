# __duplicate_checker__

duplicate_checker searches for duplicates in the selected directory and has the property of being executed by several processes.

Description
===========
duplicate_checker is a utility written in C that recursively searches for files with duplicate content. It uses a master-worker paradigm to effectively utilize all CPU cores. Communication between master and workers is performed over pipes.

[Design doc](http://shorturl.at/bsAGR)

Requirements
===========
The utility is written for GNU/Linux operating systems. The program has been tested on Ubuntu 20.04.3 LTS.

You will also need to install OpenSSL and Glib libraries. 

Libraries installation
-----------

[Glib documentation and installation info](https://docs.gtk.org/glib/)

[OpenSSL documentation](https://www.openssl.org/)

[OpenSSL installation info](https://help.dreamhost.com/hc/en-us/articles/360001435926-Installing-OpenSSL-locally-under-your-username)

Compiling
===========
There is a makefile in the repository. To compile, you need to open a terminal, go to the program folder and enter the following:
```
jon@snow:~/folder123$ make
```

Running
===========
```
Duplicate Checker supports the following command line arguments:
binary -f FOLDER_NAME -p PROCESSES_COUNT [-h] [-d]

FOLDER_NAME - string. 
PROCESS_COUNT - number.

-f	Set the directory for the search. 

-p	Set the number of 
	running processes (minimum 4).

-t	Set the number of tasks for each worker (minimum 1).

-h	DIsplay help and exit.

-d	Display debugging information.

To search for duplicates, you need to specify both keys.

The minimum number of cores on an average computer is assumed to be 4.

In output folders are separated by commas.
```

Example
-----------
There is a folder for test named __testfolder__ and we have to find duplicate files in it. Now we can choose arguments. The folder name is __testfolder__. The number of processes and the number of tasks for each process - let's assume that we will have __5__ running processes. And each running process will be loaded with __8__ tasks.

```
jon@snow:~folder123$ ./a.out -f testfolder -p 5 -t 8
                    DUPLICATES

testfolder/testfile1,testfolder/testfile2
jon@snow:~folder123$
```

In order to make sure that these files are completely identical, we can use GNU diff utility with -s key:

```
jon@snow:~folder123/testfolder$ diff -s testfile1 testfile2
Files testfile1 and testfile2 are identical
```

Support
===========
[Email](rearericu@gmail.com)


