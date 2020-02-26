#include "Operation.h"

namespace kocca {
	namespace operations {
		Operation::Operation() {
			onDepthFrameOutput = NULL;
			onColorImageFrameOutput = NULL;
			onIRImageFrameOutput = NULL;
			onMarkersFrameOutput = NULL;
		}
	} // namespace operations
} // namespace kocca