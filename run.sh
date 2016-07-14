#!/bin/bash
if [ $# == 0 ] ; then 
    echo "insert .pcap file ./run.sh /performance/networkDump/4090-126.pcap"
    exit 1
fi

source common_params.sh
./Debug/na62-farm-packets $1 $COMMON_PARAMS

