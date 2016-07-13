#include "PacketSeeker.h"
#include <pcap/pcap.h>
#include "socket/EthernetUtils.h"
#include <string.h>

#include <exceptions/UnknownCREAMSourceIDFound.h>
#include <exceptions/UnknownSourceIDFound.h>


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

	}
	void PacketSeeker::parse(std::function<void(l0::MEP*& mep)> my_function){

//		 std::function<void()> f_display_42 = []() { print_num(42); };
//		    f_display_42();


		 int npackets = 0;
		 int arp = 0;
		 int failcheckframe = 0;
		 int wrongdestip = 0;
		 int wrongdestport = 0;

		 //for 4090
		 uint_fast32_t MyIP = 638894602;

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
					//cout<< "Arp " << EthernetUtils::ipToString(hdr->ip.saddr) << endl;
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
					//cout<< "Received packet with wrong destination IP: " << EthernetUtils::ipToString(dstIP) << endl;
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

					//cout<<"---"<<endl;
					//cout<<"Packets number:"<<npackets<< " Is Fragment " << EthernetUtils::ipToString(hdr->ip.saddr) << endl;
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
			} catch (UnknownCREAMSourceIDFound const&e) {
				//container.free();
			} catch (NA62Error const& e) {
				//container.free();
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
