#include "PacketSeeker.h"
#include <pcap/pcap.h>
#include "socket/EthernetUtils.h"
#include <string.h>

#include <exceptions/UnknownCREAMSourceIDFound.h>
#include <exceptions/UnknownSourceIDFound.h>

#include "l1/L1TriggerProcessor.h"

#include <l0/MEPFragment.h>



namespace na62 {
	PacketSeeker::PacketSeeker(char * filename) {
		//error buffer
		char errbuff[PCAP_ERRBUF_SIZE];

		//open file and create pcap handler
		pcap_t * handler = pcap_open_offline(filename, errbuff);

		//The header that pcap gives us
		struct pcap_pkthdr *header;

		//The actual packet
		const u_char *packet;


		int counter = 0;
		while (pcap_next_ex(handler, &header, &packet) >= 0){
			//Storing reconstructed packets in the heap

			DataContainer temp_container;
			temp_container.data = new char[header->len];
			temp_container.length = header->len;
			temp_container.ownerMayFreeData = true;

			::memcpy(temp_container.data, packet, header->len);

			packets_.push_back(temp_container);
			counter++;
		}
		std::cout<<"Packets loaded: "<<counter<<std::endl;

		int l1counter = 0;
		std::vector<DataContainer> temp_packets;

		//Create L1 Fake packets for each L0TP Packets
		this->parse( [&](l0::MEP* & mep) -> void {
			//std::cout<<"Parsing one packets "<<std::endl;
			/*
			 * Setup L1 block if L1 is active copying informations from L0TP MEps
			 */
			if (mep->getSourceID() == SOURCE_ID_L0TP) {
				std::cout<<"Packets is a L0 packets"<<std::endl;

				//if (SourceIDManager::isL1Active()) {
					//LOG_INFO("Invent L1 MEP for event " << mep->getFirstEventNum());
					uint16_t mep_factor = mep->getNumberOfFragments();
					uint16_t fragmentLength = L1TriggerProcessor::GetL1DataPacketSize() + 8; //event length in bytes

					const uint32_t L1BlockLength = mep_factor * fragmentLength
							+ 8; //L1 block length in bytes
					char * L1Data = new char[L1BlockLength + sizeof(UDP_HDR)]; //include UDP header

					// set MEP header
					l0::MEP_HDR * L1Hdr = (l0::MEP_HDR *) (L1Data + sizeof(UDP_HDR));
					L1Hdr->firstEventNum = mep->getFirstEventNum();
					L1Hdr->sourceID = SOURCE_ID_L1;
					L1Hdr->mepLength = L1BlockLength;
					L1Hdr->eventCount = mep_factor;
					L1Hdr->sourceSubID = 0;


					char * virtualFragment = + sizeof(UDP_HDR) + L1Data
							+ 8 /* mep header */;
					l0::MEPFragment * L1Fragment;
					for (uint i = 0; i != mep_factor; i++) {
						L1Fragment = mep->getFragment(i);
						// copy the fragment header

						memcpy(virtualFragment,
								L1Fragment->getDataWithMepHeader(), 8);
						uint16_t temp;
						temp = *(uint16_t *) (virtualFragment) & 0xffff0000;
						temp |= fragmentLength;
						*(uint16_t *) (virtualFragment) = temp;
						virtualFragment += fragmentLength;
					}

//					std::cout<<"No udp header"<<std::endl;
//					int * Pointer1 = (int *) L1Data;
//					int * Pointer2 = (int *) mep->getRawData().data;
//					for ( uint a = 0; a <= ( L1BlockLength + sizeof(UDP_HDR))/4; a++  ){
//						std::cout<<*(Pointer1)<<" "<<*(Pointer2)<<std::endl;
//						Pointer1++;
//						Pointer2++;
//					}

					//set udp header
					//Works with this!!!
					memcpy(L1Data, mep->getRawData().data, sizeof(UDP_HDR));
					UDP_HDR*  updated_header = (UDP_HDR*) L1Data;
					updated_header->setPayloadSize(L1BlockLength);

					DataContainer temp_container;
					temp_container.data = L1Data;
					temp_container.length = L1BlockLength;
					temp_container.ownerMayFreeData = true;

					temp_packets.push_back(temp_container);
					std::cout<<"L1 fake packets created"<<std::endl;
					l1counter++;


		//					l0::MEP* mep_L1 = new l0::MEP(L1Data + sizeof(UDP_HDR),	L1BlockLength, { L1Data, L1BlockLength, true });
		//					uint sourceNum = SourceIDManager::sourceIDToNum(mep_L1->getSourceID());
		//					MEPsReceivedBySourceNum_[sourceNum].fetch_add(1,std::memory_order_relaxed);
		//					BytesReceivedBySourceNum_[sourceNum].fetch_add(	L1BlockLength + sizeof(UDP_HDR), std::memory_order_relaxed);
		//					for (uint i = 0; i != mep_factor; i++) {
		//						// Add every fragment
		//						L1Builder::buildEvent(mep_L1->getFragment(i), burstID_);
		//					}
				//}
			}
		});

		packets_.insert( packets_.end(), temp_packets.begin(), temp_packets.end() );
		//packets_.swap(temp_packets);

		std::cout<<"L1 Packets loaded: "<<l1counter<<" Array size: "<<packets_.size()<<std::endl;
	}


	void PacketSeeker::parse(std::function< void (l0::MEP*& mep)> my_function){

		 int npackets = 0;
		 int arp = 0;
		 int failcheckframe = 0;
		 int wrongdestip = 0;
		 int wrongdestport = 0;

		 //for 4090
		 uint_fast32_t MyIP = 638894602;
		 //for 5623
		 //uint_fast32_t MyIP = 236241418;

		 for (auto packet : packets_) {
			++npackets;
			try {
				//////////////////
				//Copied from HandleFrameTask
				/////////////
				UDP_HDR* hdr = (UDP_HDR*) packet.data;
				const uint_fast16_t etherType = (hdr->eth.ether_type);//ntohs
				const uint_fast8_t ipProto = hdr->ip.protocol;
				uint_fast16_t destPort = ntohs(hdr->udp.dest);
				const uint_fast32_t dstIP = hdr->ip.daddr;

				if (etherType != 0x0008 || ipProto != IPPROTO_UDP) {//ETHERTYPE_IP
					arp++;
					//std::cout<< "Arp " << EthernetUtils::ipToString(hdr->ip.saddr) << std::endl;
					continue;
				}

				// Check checksum errors
//TODO include checke frame
//				if (!checkFrame(hdr, packet.length)) {
//					//cout<<"Packets number:"<<npackets<<" Fail check frame"<<endl;
//					failcheckframe++;
//					//cout<< "Received broken packet from " << EthernetUtils::ipToString(hdr->ip.saddr) << endl;
//					continue;
//				}


				// Check if we are really the destination of the IP datagram
				//for 4102
				 if (MyIP != dstIP){
					std::cout<<"Packets number:"<<npackets<<" Wrong ip"<<std::endl;
					wrongdestip++;
					std::cout<< "Received packet with wrong destination IP: " <<dstIP << " ie "<< EthernetUtils::ipToString(dstIP)<< std::endl;
					continue;
				}

				if (hdr->isFragment()) {

//					packet = FragmentStore::addFragment(std::move(packet));
//					if (packet.data == nullptr) {
//
//						//cout<<"Packets number: "<<npackets<< "Skipping packets null ptr from pointer " << endl;
//						continue;
//					}
//
//					//cout<< "Packets reconstructed! " << endl;
//					hdr = reinterpret_cast<UDP_HDR*>(packet.data);
//					destPort = ntohs(hdr->udp.dest);
//
//					//TODO eluding fragmement store stuff
//					continue;

					std::cout<<"---"<<std::endl;
					std::cout<<"Packets number:"<<npackets<< " Is Fragment " << EthernetUtils::ipToString(hdr->ip.saddr) << std::endl;
				}

				//if (destPort != L0_Port) {
				//cout<<"Destinazione: "<<destPort<<endl;
				if (destPort != 58913) {
					std::cout<<"Packets number: "<<npackets<<" Wrong destination port"<<std::endl;
					++wrongdestport;
					continue;
				}

				hdr = reinterpret_cast<UDP_HDR*>(packet.data);
				const char * UDPPayload = packet.data + sizeof(UDP_HDR);

				const uint_fast16_t & UdpDataLength = ntohs(hdr->udp.len) - sizeof(udphdr);
				l0::MEP* mep = new l0::MEP(UDPPayload, UdpDataLength, packet);
				uint_fast16_t mepfactorTEMP = mep->getNumberOfFragments();


				my_function(mep);



//				for (uint i = 0; i != mep->getNumberOfFragments(); i++) {
//					l0::MEPFragment* fragment = mep->getFragment(i);
//					//cout<<fragment->getEventNumber()<<endl;
//					int eventnumberTEMP = fragment->getEventNumber();
//					if (eventnumberTEMP < eventnumberMIN){
//						eventnumberMIN = eventnumberTEMP;
//					}
//					if (eventnumberTEMP > eventnumberMAX){
//							eventnumberMAX = eventnumberTEMP;
//					}
//
//				}
			} catch (UnknownSourceIDFound const& e) {
				//container.free();
				//std::cout<< "UnknownSourceIDFound: " << std::endl;
			} catch (UnknownCREAMSourceIDFound const&e) {
				//container.free();
				std::cout<< "UnknownCREAMSourceIDFound: " << std::endl;
			} catch (NA62Error const& e) {
				//container.free();
				std::cout<< "NA62Error: " << std::endl;
			}
		 }
	}

	PacketSeeker::~PacketSeeker() {
		int counter = 0;
		for ( auto &packet : packets_ ){
			delete[] packet.data;
			counter++;
		}
		packets_.clear();
	}
}
