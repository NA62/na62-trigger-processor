#!/bin/bash
#FARM_PARAMS='--L0DataSourceIDs=0x4:6,0xc:2,0x18:1,0x20:1,0x40:1,0x10:12,0x1c:5 --farmHostNames=10.194.20.45 --verbosity=3 --firstBurstID=629'
FARM_PARAMS='--L0DataSourceIDs=0x4:6,0xc:2,0x18:1,0x20:1,0x40:1,0x10:12,0x1c:5,0x44:1 --farmHostNames=10.194.20.45 --verbosity=3 --firstBurstID=629'
TRIGGER_PARAMS='--activeL0MaskIDs=1,2,3,4,6 --triggerXMLFile=/performance/user/marco/workspace/na62-farm-packets/config_START_RUN.cfg'

COMMON_PARAMS="$FARM_PARAMS $TRIGGER_PARAMS"
#echo $COMMON_PARAMS
