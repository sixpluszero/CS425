g++ -O3 -o worker worker.cpp
g++ -O3 -o dgrep dgrep.cpp -std=c++11 -pthread
pkill -f "./worker"
./worker &
