# UIUC CS425 MP2
This README file describes the basic usage of our membership system.
This folder contains the source code of membership system written by C++. To compile the source code, type "make" in this folder and the Makefile will build up the source code, generate the binary named server.
## Deploy
To deploy this system in the VM group, we use the _deploy.py_ to upload and build in each VM instance.
Usage:
'''
python deploy.py X Y
'''
It will send the copy of source code from local to VMs ranging from X to Y. For example, to deploy on all the VMs, simply type `python deploy.py 1 10`

## Coldstart
To start all the nodes in the group, type `python start.py X Y`, and daemons on VM range [X, Y] will start, joining the virtual circle

## Command line tool
Suppose we login to VM X, we can perform the following commands.
1. Crash a daemon
...Simply type `pkill server`
2. List membership of the daemon in this vm
...python mp2/cmd.py member
3. List id of the daemon in this vm
...python mp2/cmd.py id
4. Let the daemon in this vm to leave
...python mp2/cmd.py leave
5. Let the daemon in this vm rejoin
...python mp2/cmd.py join

