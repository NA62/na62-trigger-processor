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
	uint l1_num_last_burst = 0;

	boost::timer::cpu_timer timer;
	boost::timer::cpu_timer up_time;

	bool iseob = true;

	Event * fetched_event;
	TriggerMessager trigger_message;
	uint highest_burst_id_received = 0;

	while (1) {
		if (SharedMemoryManager::getNextEvent(fetched_event, trigger_message)) {
			if (highest_burst_id_received < trigger_message.burst_id) {
				LOG_INFO("Previous Burst id: "<<trigger_message.burst_id);
				highest_burst_id_received = trigger_message.burst_id;
			}

			if(trigger_message.level == 1) {
				//LOG_INFO("Received event: " << fetched_event->getEventNumber());

				timer.start();
				if (iseob == true) {
					//first event arrive start again the up_time timer
					up_time.start();
				}
				iseob = false;
				/*
				 * Process Level 1 trigger
				 */
				fetched_event->readTriggerTypeWordAndFineTime();
				uint_fast8_t l1TriggerTypeWord = L1TriggerProcessor::compute(fetched_event);

				//printf("Event Processed l1 word %d \n",l1TriggerTypeWord);

				trigger_message.l1_trigger_type_word = l1TriggerTypeWord;
				//EVENT_HDR* serializedevent = SmartEventSerializer::SerializeEvent(fetched_event);

				SharedMemoryManager::pushTriggerResponseQueue(trigger_message);
				//Removing event from the shared memory
				SharedMemoryManager::removeL1Event(trigger_message.memory_id);
				l1_num++;
				l1_num_last_burst++;
			}

//			if( l1_num % 10 == 0 ) {
//				LOG_INFO(getpid()<<" / l1 / "<<l1_num);
//				LOG_INFO(getpid()<<" / l1_num_last_burst / "<<l1_num_last_burst);
//			}

		} else {
			//if () > 1) {
				//std::cout<<"EOB "<< timer.format() <<std::endl;
			//}

			boost::timer::cpu_times eob_trigger = timer.elapsed();

//			std::cout<<"wall: "<<eob_trigger.wall<<std::endl;
//			std::cout<<"user "<<eob_trigger.user<<std::endl;
//			std::cout<<"system: "<<eob_trigger.system<<std::endl;

			//The timer triggs the endof bust
			boost::chrono::duration<double> seconds = boost::chrono::nanoseconds(eob_trigger.wall);
			if (seconds.count() > 3 && !iseob) {

				LOG_INFO("  ");
				std::cout<<"EOB !!! "<<(seconds.count())<<std::endl;
				iseob = true;
				boost::timer::cpu_times up_time_results = up_time.elapsed();
				LOG_INFO("pid: " << getpid()<<" / l1 / "<<l1_num);
				LOG_INFO("pid: " << getpid()<<" / l1_num_last_burst / "<<l1_num_last_burst);
				LOG_INFO("pid: " << getpid()<<" Uptime: / "<< (up_time_results.user )<< "ns");

				l1_num_last_burst = 0;
			}
			//LOG_INFO("Nothing fetched trigger queue sleep for a while");
			usleep(1);
		}



	}
	return 0;
}
