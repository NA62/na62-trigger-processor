//============================================================================
// Name        : simpleEclipseCPP11Project.cpp
// Author      : Jonas Kunze (kunze.jonas@gmail.com)
//============================================================================

#include <iostream>

#include <socket/EthernetUtils.h>
#include <l0/MEP.h>
#include <l0/MEPFragment.h>

#include <exceptions/UnknownCREAMSourceIDFound.h>
#include <exceptions/UnknownSourceIDFound.h>

#include "options/MyOptions.h"
//#include "socket/FragmentStore.h"

//#include "eventBuilding/SourceIDManager.h"
//#include <eventBuilding/Event.h>
#include "storage/EventSerializer.h"
#include "storage/SmartEventSerializer.h"
//#include "structs/Event.h"
#include "eventBuilding/Event.h"

#include "SharedMemory/SharedMemoryManager.h"
#include "exceptions/SerializeError.h"
#include "Utils/PacketSeeker.h"

#include <l1/L1TriggerProcessor.h>
#include <common/HLTriggerManager.h>

#include "structs/L0TPHeader.h"

using namespace std;
using namespace na62;

bool checkFrame(UDP_HDR* hdr, uint_fast16_t length) {
	/*
	 * Check IP-Header
	 */
	//				if (!EthernetUtils::CheckData((char*) &hdr->ip, sizeof(iphdr))) {
	//					LOG_ERROR << "Packet with broken IP-checksum received");
	//					container.free();
	//					continue;
	//				}
	if (hdr->isFragment()) {
		return true;
	}

	if (ntohs(hdr->ip.tot_len) + sizeof(ether_header) != length) {
		/*
		 * Does not need to be equal because of ethernet padding
		 */
		if (ntohs(hdr->ip.tot_len) + sizeof(ether_header) > length) {
			cout<<
			"Received IP-Packet with less bytes than ip.tot_len field! " <<
			(ntohs(hdr->ip.tot_len) + sizeof(ether_header) ) << ":"<<length << endl;
			return false;
		}
	}

	/*
	 * Does not need to be equal because of ethernet padding
	 */
	if (ntohs(hdr->udp.len) + sizeof(ether_header) + sizeof(iphdr) > length) {
		cout<<"Received UDP-Packet with less bytes than udp.len field! "<<(ntohs(hdr->udp.len) + sizeof(ether_header) + sizeof(iphdr)) <<":"<<length;
		return false;
	}

	//				/*
	//				 * Check UDP checksum
	//				 */
	//				if (!EthernetUtils::CheckUDP(hdr, (const char *) (&hdr->udp) + sizeof(udphdr), ntohs(hdr->udp.len) - sizeof(udphdr))) {
	//					LOG_ERROR << "Packet with broken UDP-checksum received" ) << ENDL;
	//					container.free();
	//					continue;
	//				}
	return true;
}

int main(int argc, char *argv[]) {

	TriggerOptions::Load(argc, argv);
	MyOptions::Load(argc, argv);

	HLTStruct HLTConfParams;
	HLTriggerManager::fillStructFromXMLFile(HLTConfParams);
	L1TriggerProcessor::initialize(HLTConfParams.l1);
	//L2TriggerProcessor::initialize(HLTConfParams.l2);


	SourceIDManager::Initialize(Options::GetInt(OPTION_TS_SOURCEID),
			Options::GetIntPairList(OPTION_DATA_SOURCE_IDS),
			Options::GetIntPairList(OPTION_L1_DATA_SOURCE_IDS));

	EventSerializer::initialize();
	SmartEventSerializer::initialize();

	SharedMemoryManager::initialize();


	int eventnumberMIN = 2000000;
	int eventnumberMAX = 0;
	//Detector counter
	uint c_lav = 0;
	uint c_cedar = 0;
	uint c_chanti = 0;
	uint c_rich = 0;
	uint c_chod = 0;
	uint c_irc = 0;

	na62::Event * test = new Event(1);

	PacketSeeker * packet_manager = new PacketSeeker(argv[1]);

	packet_manager->parse( [&](l0::MEP* & mep) -> void {
		int count_source_id = 0;
		for (uint i = 0; i != mep->getNumberOfFragments(); i++) {

			l0::MEPFragment* fragment = mep->getFragment(i);
			//cout<<fragment->getEventNumber()<<endl;
			int eventnumberTEMP = fragment->getEventNumber();
			if (eventnumberTEMP < eventnumberMIN){
				eventnumberMIN = eventnumberTEMP;
			}
			if (eventnumberTEMP > eventnumberMAX){
					eventnumberMAX = eventnumberTEMP;
			}
			///Get the number of the detector
			uint_fast8_t source = fragment->getSourceID();
			uint_fast8_t sub = 3;
			//LOG_INFO(fragment->getEventNumber());
			//Good Event number


			if ( fragment->getEventNumber() ==  139899){//recognise as control trigger
			//if ( fragment->getEventNumber() ==  215283){//recognise as control trigger
			//cout<<" "<<fragment->getEventNumber()<<endl;
			//if ( fragment->getEventNumber() == 183520){
				LOG_INFO("Match Event");

				if (source == 0x4) {
					LOG_INFO("Match Cedar");
					 c_cedar++;
				} else if (source == 0x10) {
					LOG_INFO("Match lav");
					 c_lav++;
				} else if (source == 0xc) {
					LOG_INFO("Match Chanti");
					 c_chanti++;
				} else if (source == 0x18) {
					LOG_INFO("Match Chod");
					 c_rich++;
				} else if (source == 0x1c) {
				    LOG_INFO("Match Rich");
					c_chod++;
				}else if (source  == 0x20) {
					LOG_INFO("Match IRC");
					c_irc++;

				}else if (source  == 0x28) {
					LOG_INFO("Match MUV1");

				}else if (source  == 0x30) {
					LOG_INFO("Match MUV3");

				}else if (source  == 0x40) {
					LOG_INFO("Match L0tp");

				}else if (source  == 0x44) {
					LOG_INFO("Match L1 PFake Packet!!");
//							fragment
//							int * pointer_fragment = (int *) fragment->getPayload();
//							for (int a = 0; a <= fragment->_/4; a++) {
//								std::cout<<*(pointer_fragment)<<std::endl;
//								pointer_fragment++;
//							}

				}else{
					LOG_INFO("Source ID: "<<fragment->getSourceID());
					count_source_id++;
						cout<<"Souce id: "<<((int)fragment->getSourceID())
								<<" SubId: "<<((int)fragment->getSourceSubID())
								<<" time: "<<count_source_id
								<<" Event number: "<< fragment->getEventNumber()
								<<endl;
				}

				if (test->addL0Fragment(fragment, 1)) {
					LOG_INFO("Event Complete!");

					test->readTriggerTypeWordAndFineTime();
					uint_fast16_t l0TrigFlags = test->getTriggerFlags();
					printf("l0 trigger flags %d \n", l0TrigFlags);



					bool result = SharedMemoryManager::storeL1Event(test);

					/*
					 * Process Level 1 trigger
					 */
					StrawAlgo strawalgo;
					test->readTriggerTypeWordAndFineTime();
					uint_fast8_t l1TriggerTypeWord = L1TriggerProcessor::compute(test, strawalgo);
					printf("l1 word %d \n",l1TriggerTypeWord);

//					LOG_INFO("Complete! Serializing");
//					EVENT_HDR* serializedevent = EventSerializer::SerializeEvent(test);
//					EVENT_HDR* smartserializedevent;
//					try {
//						smartserializedevent = SmartEventSerializer::SerializeEvent(test);
//					} catch(SerializeError) {
//						std::cout<<"Fragment exceed the memory"<<std::endl;
//
//					}
//
//					if (SmartEventSerializer::compareSerializedEvent(serializedevent, smartserializedevent)) {
//						std::cout<<" => Right serialization!"<<std::endl;
//					} else {
//						std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!Wrong serialization!"<<std::endl;
//					}

					/*std::cout << "Recreating event" << std::endl;
					na62::Event * event_from_serial = new Event(serializedevent, 1);

					std::cout << "Reserializing event" << std::endl;
					EVENT_HDR* smartreserializedevent = EventSerializer::SerializeEvent(event_from_serial);

					if (SmartEventSerializer::compareSerializedEvent(serializedevent, smartreserializedevent)) {
						std::cout<<" => Right serialization!"<<std::endl;
					} else {
						std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!Wrong serialization!"<<std::endl;
					}*/


				}else{
					//LOG_INFO("not Complete");
				}
			}
		}
	});






//	cout<<endl<<"###Filename: "<<filename<<endl;
//	cout<<"Global Statistics: "<<endl;
//	cout<<" Packets destination: "<<EthernetUtils::ipToString(MyIP)<<"IP: "<<MyIP<<endl;
//	cout<<" Number of packets: "<<npackets<<endl;
//	cout<<" Number of Arp: "<<arp<<endl;
//	cout<<" Number of fail check frame: "<< failcheckframe<<endl;
//	cout<<" Number of wrong destination ip: "<< wrongdestip<<endl;
//	cout<<" Number of wrong destination port: "<<  wrongdestport<<endl;
//	cout<<" Nfragment / Reassembled Frames / Unfinished Frames : "<<FragmentStore::getNumberOfReceivedFragments()<<"/"<<FragmentStore::getNumberOfReassembledFrames()<<"/"<<FragmentStore::getNumberOfUnfinishedFrames()<<endl;
//	cout<<" Range Event Number: ["<<eventnumberMIN<<" - "<<eventnumberMAX<<"]"<<endl;
//	cout<<" Range Mep Factor: ["<<mepfactorMIN<<" - "<<mepfactorMAX<<"]"<<endl;
//
//	LOG_INFO("Cedar fragments: " << c_cedar);
//	LOG_INFO("Chanti fragments: " << c_chanti);
//	LOG_INFO("Lav fragments: " << c_lav );
//	LOG_INFO("Rich fragments: " << c_rich);
//	LOG_INFO("Chod fragments: " << c_chod);
//	LOG_INFO("Irc fragments: " << c_irc);
//	LOG_INFO("Expected packets: " << SourceIDManager::NUMBER_OF_EXPECTED_L0_PACKETS_PER_EVENT);
}
