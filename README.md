#Na62farm Packets
## How to run the Shared Memory farm
I compiled this project in my home: /performance/user/marco/workspace  
This path is shared among all nodes, so you don't have to copy executable every time you want to try them. 

### Compile everything first
####na62-farm-lib
```
git remote add mboretto https://github.com/MBoretto/na62-farm-lib
git fetch mboretto 
git checkout memorymaster
```

####na62-farm
```
git remote add mboretto https://github.com/MBoretto/na62-farm
git fetch mboretto 
git checkout memorymaster
```

add the flag USE_SHAREDMEMORY for the compiler

then clean all the projects and rebuild the na62-farm executable

####na62-farm-packets

```
git clone https://github.com/MBoretto/na62-farm-packets
```
The .cproject is commited and should compile fine on the dev1.
There are many configuration to create different executables:
##### Memory clean
Compile the configuration: Memoryclean.
Let you destroy the shared memory and the related queue.

##### Trigger processor
Compile the configuration: Trigger-processor
This is the process that performs trigger algorithms.

##### Test bench (not required)
Compile the configuration na62-farm-packets
The main executable let you reconstruct event from a .pcap file:

	./run.sh /performance/networkDump/4090-126.pcap 

Not useful for the shared memory farm

### Up and running
Log on 6 shell on farmdev30. I suggest you to avoid the usage of cssh because seems that caches the executable you run and if you try to run new recompiled version it still run the old one.
Move to the na62-farm direcotory, in my case:

	/performance/user/marco/workspace/na62-farm-packets

Then run:
	
	./CleanMemory/cleanmemory

To remove all the existing memory structure. Then run:

	./trigger_processor.sh

This wiil create the shared memory and will wait for event to process. 
Please notice that this script steal the param configuration from farm29 so you don't have to care about the paramenters! (This is useful when you test along with all the others nodes. It's a pain if you want to run just a farm standalone).Then run:

	./farm.sh

The main process will recognise the existence of the shared memory and write into. As said before also this script steal the configuration from farm29.

Now you can run as many trigger_processor processes as you want! 

### Other thing to know

- The shared memory version feats the SmartEventSerializer that is able to write on an existing location of memory. This serializer has been tested and works as the old one. It also  fix a very rare segfoult that can happen sometimes with the EventSerializer.
- The event constructor given a serialized event as input has been tested and works.
- The current implementation run smootly without segfault
- The implementation is able manage EOB packets.
- The trigger-processor don't sends statistics to the run control so if everything 'works' you should see packets disappear at l1 and reappear in the merger.
- I implement some control that avoid the mix between packets from differet burst (can happen if the trigger processor is not able to withstand the rate).
- At the end of burst there are some checks that let you 'regenerate the shared memory'
- If you clean the memory, you have to reinitialize all the processes because the pointer to the shared memory change. 
- the trigger-processor balance the load automatically, doesn'care how many of then are running
- I tried to init somo timing  test in trigger processor, but so far they don't work as expected.



