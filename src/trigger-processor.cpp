#include "options/MyOptions.h"
#include "storage/SmartEventSerializer.h"
#include <common/HLTriggerManager.h>
#include "SharedMemory/SharedMemoryManager.h"
#include <l1/L1TriggerProcessor.h>

using namespace std;
using namespace na62;

int main(int argc, char *argv[]) {

	TriggerOptions::Load(argc, argv);
	MyOptions::Load(argc, argv);

	HLTStruct HLTConfParams;
	HLTriggerManager::fillStructFromXMLFile(HLTConfParams);
	L1TriggerProcessor::initialize(HLTConfParams.l1);

	SourceIDManager::Initialize(Options::GetInt(OPTION_TS_SOURCEID),
			Options::GetIntPairList(OPTION_DATA_SOURCE_IDS),
			Options::GetIntPairList(OPTION_L1_DATA_SOURCE_IDS));


	LOG_INFO("Initializing Shared Memory");
	SharedMemoryManager::initialize();
	LOG_INFO("Initializing done!");

	uint l1_num = 0;

	Event * fetched_event;
	TriggerMessager trigger_message;

	while (1) {
		if (SharedMemoryManager::getNextEvent(fetched_event, trigger_message)) {
			if(trigger_message.level == 1) {
				LOG_INFO("Received event: " << fetched_event->getEventNumber());

				/*
				 * Process Level 1 trigger
				 */
				fetched_event->readTriggerTypeWordAndFineTime();
				uint_fast8_t l1TriggerTypeWord = L1TriggerProcessor::compute(fetched_event);

				std::cout<<"Event Processed result: " <<  l1TriggerTypeWord <<" end;"<<std::endl;
				printf("l1 word %d \n",l1TriggerTypeWord);

				trigger_message.l1_trigger_type_word = l1TriggerTypeWord;
				//EVENT_HDR* serializedevent = SmartEventSerializer::SerializeEvent(fetched_event);

				SharedMemoryManager::pushTriggerResponseQueue(trigger_message);
				//Removing event from the shared memory
				SharedMemoryManager::removeL1Event(trigger_message.memory_id);
				l1_num++;
			}

			//LOG_INFO(getpid()<<" / l1 / "<<l1_num);
			//if( l1_num % 10 == 0 ) LOG_INFO(getpid()<<" / l1 / "<<l1_num);
			//if( l2_num % 10 == 0 ) LOG_INFO(getpid()<<" / l2 / "<<l2_num);

		} else {
			// sleep for a while
			//LOG_INFO("Nothing fetched trigger queue sleep for a while");
			usleep(1);
		}
	}
	return 0;
}
