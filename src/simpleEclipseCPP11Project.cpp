//============================================================================
// Name        : simpleEclipseCPP11Project.cpp
// Author      : Jonas Kunze (kunze.jonas@gmail.com)
//============================================================================

#include <iostream>
#include <pcap/pcap.h>
#include <socket/EthernetUtils.h>
#include <l0/MEP.h>
#include <l0/MEPFragment.h>
#include <LKr/LkrFragment.h>
#include <eventBuilding/SourceIDManager.h>

#include <exceptions/UnknownCREAMSourceIDFound.h>
#include <exceptions/UnknownSourceIDFound.h>

#include "options/MyOptions.h"
#include "socket/FragmentStore.h"

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

//get file
char *filename = argv[1];

int length = sizeof(argv[1])/sizeof(char);
//cout<<"Lenght:"<<length<<endl;
char * Cfilename = new char[length];
Cfilename = argv[1];



//error buffer
char errbuff[PCAP_ERRBUF_SIZE];

//open file and create pcap handler
pcap_t * handler = pcap_open_offline(filename, errbuff);

//The header that pcap gives us
struct pcap_pkthdr *header;

//The actual packet
 const u_char *packet;


 MyOptions::Load(argc, argv);
 SourceIDManager::Initialize(Options::GetInt(OPTION_TS_SOURCEID),
 			Options::GetIntPairList(OPTION_DATA_SOURCE_IDS),
 			Options::GetIntPairList(OPTION_CREAM_CRATES),
 			Options::GetIntPairList(OPTION_INACTIVE_CREAM_CRATES),
 			Options::GetInt(OPTION_MUV_CREAM_CRATE_ID));



 int eventnumberMIN = 2000000;
 int eventnumberMAX = 0;

 uint_fast16_t mepfactorMIN = 200;
 uint_fast16_t mepfactorMAX = 0;



 int npackets = 0;

 int arp = 0;
 int failcheckframe = 0;
 int wrongdestip = 0;

 int wrongdestport = 0;

 uint_fast16_t destPort = 0;
 //uint_fast32_t MyIP = 622117386;//10.194.20.37

 //for 4090*
 uint_fast32_t MyIP = 638894602;
 //622117386

 cout<<"Packets destination: "<<EthernetUtils::ipToString(MyIP)<<endl;

int count_source_id = 0;
 while (pcap_next_ex(handler, &header, &packet) >= 0){
	++npackets;

	//if(npackets >= 335 || npackets <= 330) continue;
	try {
		DataContainer container;

		container.data = (char *) packet;
		container.length = header->len;
		container.ownerMayFreeData = true;

	//////////////////
	//Copied from HandleFrameTask
	/////////////
		UDP_HDR* hdr = (UDP_HDR*) container.data;
		const uint_fast16_t etherType = /*ntohs*/(hdr->eth.ether_type);
		const uint_fast8_t ipProto = hdr->ip.protocol;
		destPort = ntohs(hdr->udp.dest);
		const uint_fast32_t dstIP = hdr->ip.daddr;

		if (etherType != 0x0008/*ETHERTYPE_IP*/|| ipProto != IPPROTO_UDP) {
			arp++;
			//cout<< "Arp " << EthernetUtils::ipToString(hdr->ip.saddr) << endl;
			continue;
		}

		/*
		 * Check checksum errors
		 */
		if (!checkFrame(hdr, container.length)) {
			cout<<"Packets number:"<<npackets<<" Fail check frame"<<endl;
			failcheckframe++;
			//cout<< "Received broken packet from " << EthernetUtils::ipToString(hdr->ip.saddr) << endl;
			continue;
		}

		/*
		 * Check if we are really the destination of the IP datagram
		 */
		 //for 4102*

		 if (MyIP != dstIP){
			cout<<"Packets number:"<<npackets<<" Wrong ip"<<endl;
			wrongdestip++;
			//cout<< "Received packet with wrong destination IP: " << EthernetUtils::ipToString(dstIP) << endl;
			cout<< "Received packet with wrong destination IP: " <<dstIP << " ie "<< EthernetUtils::ipToString(dstIP)<< endl;

			continue;
		}

		if (hdr->isFragment()){

			//continue;
			//cout<<"---"<<endl;
			//cout<<"Packets number:"<<npackets<< " Is Fragment " << EthernetUtils::ipToString(hdr->ip.saddr) << endl;
			container = FragmentStore::addFragment(std::move(container));
			if (container.data == nullptr) {

				//cout<<"Packets number: "<<npackets<< "Skipping packets null ptr from pointer " << endl;
				continue;
			}

			//cout<< "Packets reconstructed! " << endl;
			hdr = reinterpret_cast<UDP_HDR*>(container.data);
			destPort = ntohs(hdr->udp.dest);
		}

		//if (destPort != L0_Port) {
		//cout<<"Destinazione: "<<destPort<<endl;
		if (destPort != 58913) {
			cout<<"Packets number:"<<npackets<<" Wrong destination port"<<endl;
			++wrongdestport;
			continue;
		}

		hdr = reinterpret_cast<UDP_HDR*>(container.data);
		const char * UDPPayload = container.data + sizeof(UDP_HDR);

		const uint_fast16_t & UdpDataLength = ntohs(hdr->udp.len) - sizeof(udphdr);
		l0::MEP* mep = new l0::MEP(UDPPayload, UdpDataLength, container);
		uint_fast16_t mepfactorTEMP = mep->getNumberOfFragments();
		if (mepfactorMIN > mepfactorTEMP) {
			mepfactorMIN = mepfactorTEMP;
		}
		if (mepfactorMAX < mepfactorTEMP) {
			mepfactorMAX = mepfactorTEMP;
		}
		if (mepfactorTEMP != 8) {
			cout<<"Packets number:"<<npackets<<" Mep n "<<mepfactorTEMP<<endl;
		}
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
            uint_fast8_t source = 28;
            uint_fast8_t sub = 3;
            if ( fragment->getEventNumber() ==  139899){

				if ( fragment->getSourceID() == source) {
					//if (fragment->getSourceSubID() == sub) {
					count_source_id++;
					cout<<"Souce id: "<<((int)fragment->getSourceID())
							<<" SubId: "<<((int)fragment->getSourceSubID())
							<<" time: "<<count_source_id
							<<" Event number: "<< fragment->getEventNumber()
							<<ENDL;
					//}
				}
            }
		}
	} catch (UnknownSourceIDFound const& e) {
		//container.free();
	} catch (UnknownCREAMSourceIDFound const&e) {
		//container.free();
	} catch (NA62Error const& e) {
		//container.free();
	}
 }

cout<<endl<<"###Filename: "<<Cfilename<<endl;
cout<<"Global Statistics: "<<endl;
cout<<" Packets destination: "<<EthernetUtils::ipToString(MyIP)<<"IP: "<<MyIP<<endl;
cout<<" Number of packets: "<<npackets<<endl;
cout<<" Number of Arp: "<<arp<<endl;
cout<<" Number of fail check frame: "<< failcheckframe<<endl;
cout<<" Number of wrong destination ip: "<< wrongdestip<<endl;
cout<<" Number of wrong destination port: "<<  wrongdestport<<endl;
cout<<" Nfragment / Reassembled Frames / Unfinished Frames : "<<FragmentStore::getNumberOfReceivedFragments()<<"/"<<FragmentStore::getNumberOfReassembledFrames()<<"/"<<FragmentStore::getNumberOfUnfinishedFrames()<<endl;
cout<<" Range Event Number: ["<<eventnumberMIN<<" - "<<eventnumberMAX<<"]"<<endl;
cout<<" Range Mep Factor: ["<<mepfactorMIN<<" - "<<mepfactorMAX<<"]"<<endl;

	return 0;
}
