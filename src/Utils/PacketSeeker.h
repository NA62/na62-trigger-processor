#include  "structs/DataContainer.h"
#include "vector"
#include <functional>
#include <l0/MEP.h>

namespace na62 {
class PacketSeeker{
	public:
		PacketSeeker(char* filename);
		virtual ~PacketSeeker();
		void parse(std::function<void(l0::MEP*& mep)> my_function);
		inline std::vector<DataContainer> * getPackets() {
			return  &packets_;
		}
	private:
		std::vector<DataContainer> packets_;

};
}
