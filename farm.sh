#!/bin/bash
source common_params_farm.sh

killall na62-farm 
../na62-farm/Debug/na62-farm $PARAMS
