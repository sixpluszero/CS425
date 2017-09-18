# UIUC CS425 MP1 
===========
This repository contains key source files that enable the distributed grep functionalities and the unit test for it.

Distributed Grep 
--------
The example vm1.log-vm10.log has been stored in each server's /usr/local/mp1 folder
The worker.cpp is the server side daemon source code, and the dgrep.cpp is the source code of the command line dgrep client.


Run the command
To deploy the source files and compile for binaries, type the following commands:
(1). ./push_remote.py
(2). ./run_remote.py
(3). Log in to one of the vm, enter /usr/local/mp1/
(4). Type the dgrep command, for example _./dgreep 'GET' vm.log_

  
     
     
Unit Test   
-------
The file “test_grep.py” is the unit test for distributed-grep. It should be placed at vm1:/usr/local/mp1. 
“chmod a+x test_grep.py” and use “./test_grep.py ” to run it. 
It would create a folder named “testgrep+time” to store detailed logs on every vm.
The test program firstly generates random log to each vm and secondly compares the result of grep and distributed-grep with words in different frequency, like “HTTP”(frequent), “.com”(sometimes), “uffba.msfyqmhq.xzvqj.elc.net”(rare) and etc. 
