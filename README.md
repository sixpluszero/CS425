# CS425
We implemented the following projects from scratch. A project is always based on its previous project. The detailed description of each project is in the subfolder, the requirements' pdf and the report.

### MP1: Distributed Log Querier 
Implemented a "grep" function to query distributed log files on multiple machines.
### MP2: Distributed Group Membership  
A machine could join or leave a membership based on heartbeat detection. At most 4 machines can fail simultaneously. Message Cost: O(1).
### MP3: Distributed File System  
4 replica (quorum detection). Time-bounded write-write conflicts. Tolerant up to 3 machine failures at a time.
### MP4: Distributed Graph Processing System  
Based on Pregel Algorithm. Implemented PageRank and Single Source Shortest Path functions(SSSP). ~170% faster than GraphX in PageRank. ~400% faster than GraphX in SSSP.
