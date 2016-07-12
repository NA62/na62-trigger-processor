#include "PacketSeeker.h"
#include <pcap/pcap.h>
#include <string.h>


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
	PacketSeeker::~PacketSeeker() {
		int counter = 0;
		for ( auto &packet : packets_ ){
			delete[] packet.data;
			counter++;
		}
		packets_.clear();
	}
}
