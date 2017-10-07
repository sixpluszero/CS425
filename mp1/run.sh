g++ -O3 -o /usr/local/mp1/dgrep /usr/local/mp1/dgrep.cpp -std=c++11 -pthread
g++ -O3 -o /usr/local/mp1/dgrep_daemon /usr/local/mp1/worker.cpp
g++ -O3 -o /home/jialin2/dgrep_daemon /usr/local/mp1/worker.cpp
pkill -f "dgrep_daemon"
cd /home/jialin2
nohup ./dgrep_daemon &
