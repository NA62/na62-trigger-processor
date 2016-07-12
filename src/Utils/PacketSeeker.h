#include  "structs/DataContainer.h"
#include "vector"

namespace na62 {
class PacketSeeker{
	public:
		PacketSeeker(char* filename);
		virtual ~PacketSeeker();
		inline std::vector<DataContainer> * getPackets() {
			return  &packets_;
		}
	private:
		std::vector<DataContainer> packets_;

};
}
