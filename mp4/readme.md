# UIUC CS425 MP4
This README file describes the basic usage of our membership system. This folder contains the source code of membership system written by C++. Make sure you have installed C++ complier with C++11. To compile the source code, type `make clean; make` in this folder and the Makefile will build up the source code, generate the binary named server.

## Deploy(From local)
On your local computer, please type `init.sh jialin2` or `init.sh zijunc2` to init the code. This will write some configuration to make sure which account/port combination you will be using.

To deploy this system in the VM group, we use the deploy.py to upload and build in each VM instance. Plase make sure you have password-less access to your VM.

Type `python deploy.py X Y` will send the copy of source code from local to VMs ranging from X to Y. For example, to deploy on all the VMs, simply type `python deploy.py 1 10`.

## Start(From local)
To start all the nodes in the group on your local computer, type `python start.py X Y`. Daemons on VM range [X, Y] will start and join the cluster.

## Crash(From local)
To kill nodes in the cluster, type `python crash.py X Y`. Daemons on VM range [X, Y] will be killed.

## Command line tool(On VM)
Suppose we login to VM X, we can perform the following commands.
1. Crash a daemon
    * Simply type `pkill -u jialin2/zijunc2 server`
2. List membership of the daemon in this vm
    * `python mp4/cmd.py member`
3. List id of the daemon in this vm
    * `python mp4/cmd.py id`
4. Let the daemon in this vm to leave
    * `python mp4/cmd.py leave`
5. Let the daemon in this vm rejoin
    * `python mp4/cmd.py join`
6. List all the files in this VM.
    * `python mp4/cmd.py store`
7. List all the VM IPs that store this file.
    * `python mp4/sdfs.py ls sdfsfilename`
8. Put local file to SDFS.
    * `python mp4/sdfs.py put path/to/local/file sdfsfilename`
9. Get file from SDFS to local.
    * `python mp4/sdfs.py get sdfsfilename path/to/local/file`
10. Delete file in SDFS.
    * `python mp4/sdfs.py delete sdfsfilename`
10. Submit a task to SAVA.
    * `python mp4/sava.py SSSP mp4/apps/SSSP.cpp rinput TOP-25 MIN`
    * `python mp4/sava.py PageRank mp4/apps/PageRank.cpp rinput TOP-25 SUM`
    
    
    
