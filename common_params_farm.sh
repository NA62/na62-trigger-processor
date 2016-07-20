#!/bin/bash
#PARAMS=`ps ux | grep "na62-farm " | grep -v 'grep' | sed 's/.*na62-farm//'`
PARAMS=`ssh root@na62farm29 'ps ux | grep "na62-farm " | grep -v "grep" | sed "s/.*na62-farm//"'`
#echo $PARAMS

variable='--verbosity=2' 
PARAMS=`echo $PARAMS | sed "s@$variable@@g"`
variable='--logtostderr=0'
PARAMS=`echo $PARAMS | sed "s@$variable@@g"`

PARAMS=$PARAMS' --verbosity=3 --logtostderr=1 '

echo $PARAMS
