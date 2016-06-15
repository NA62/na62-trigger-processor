#!/bin/bash

if [ $# == 0 ] ; then 
    echo "insert .pcap file ./run.sh /performance/networkDump/4090-126.pcap"
    exit 1
fi

./Debug/na62-farm-packets $1 --L0DataSourceIDs=0x4:6,0xc:2,0x18:1,0x20:1,0x40:1,0x10:12,0x1c:5 --farmHostNames=10.194.20.45 --verbosity=3 --firstBurstID=629
#./Debug/na62-farm-packets $1 --L0DataSourceIDs=0x4:6,0xc:2,0x18:1,0x20:1,0x40:1,0x10:12,0x30:1,0x1c:5,0x14:32,0x44:1 --farmHostNames=10.194.20.45 --verbosity=3 --firstBurstID=629
#--logtostdr=1 


#./na62-farm-packets /performance/networkDump/4102_125.pcap  --firstBurstID=629 --mergerHostNames=10.194.20.114,10.194.20.115,10.194.20.116 --farmHostNames=10.194.20.45,10.194.20.14,10.194.20.15,10.194.20.16,10.194.20.17,10.194.20.18,10.194.20.19,10.194.20.20,10.194.20.21,10.194.20.22,10.194.20.23,10.194.20.46,10.194.20.29,10.194.20.30,10.194.20.31,10.194.20.32,10.194.20.33,10.194.20.34,10.194.20.35,10.194.20.36,10.194.20.37,10.194.20.38,10.194.20.40,10.194.20.39,10.194.20.41,10.194.20.42,10.194.20.43,10.194.20.27,10.194.20.28 --numberOfFragmentsPerMEP=8 --incrementBurstAtEOB=0 --L1DataSourceIDs=0x8:1,0x24:432,0x28:6,0x2c:4 --L0DataSourceIDs=0xc:2,0x18:1,0x20:1,0x4:6,0x40:1,0x10:12,0x30:1,0x1c:5,0x14:32,0x44:1,0x48:1,0x4c:1 --creamMulticastPort=58914 

