#include "MocapMarker.h"

namespace kocca {
	namespace datalib {
		MocapMarker::MocapMarker(double x_, double y_, double z_, std::string name_) {
			name = name_;
			coords.x = x_;
			coords.y = y_;
			coords.z = z_;
		}
	} // namespace datalib
} // namespace kocca