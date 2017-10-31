#!/bin/bash

if [ "$1" == "jialin2" ]
then
    echo 'BASEPORT=6000' > ./mp_config.py
    echo 'USER="jialin2"' >> ./mp_config.py
    echo "#define BASEPORT 6000" > ./include/config.hpp
else
    echo 'BASEPORT=7000' > ./mp_config.py
    echo 'USER="zijunc2"' >> ./mp_config.py
    echo "#define BASEPORT 7000" > ./include/config.hpp
fi