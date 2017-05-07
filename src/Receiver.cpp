#include <time.h>
#include <cstdlib>
#include <stdint.h>

#include "structs/TriggerMessager.h"
#include "SharedMemory/QueueReceiver.h"
#include "SharedMemory/SharedMemoryManager.h"

using namespace na62;
//Main
int main(int argc, char *argv[]){


	SharedMemoryManager::initialize();

	//Starting Receiver
	//==================
	QueueReceiver* receiver = new QueueReceiver();
	receiver->startThread("QueueReceiver");
	uint event_id_to_process = 0;

	while (1) {
		sleep(1);
	}
	/*
	 * Join other threads
	 */
	AExecutable::JoinAll();
	return 0;
}
