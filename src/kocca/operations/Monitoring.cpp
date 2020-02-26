#include "Monitoring.h"

namespace kocca {
	namespace operations {
		Monitoring::Monitoring() {
			type = KOCCA_MONITORING_OPERATION;
			skippedMocapFramesCount = 0;
		}

		bool Monitoring::processDepthFrame(kocca::datalib::TimeCodedFrame tcFrame) {
			if(onDepthFrameOutput != NULL) {
				onDepthFrameOutput(tcFrame);
				return(true);
			}
			else
				return(false);
		}

		bool Monitoring::processColorImageFrame(kocca::datalib::TimeCodedFrame tcFrame) {
			if(onColorImageFrameOutput != NULL) {
				onColorImageFrameOutput(tcFrame);
				return(true);
			}
			else
				return(false);
		}

		bool Monitoring::processIRImageFrame(kocca::datalib::TimeCodedFrame tcFrame) {
			if (onIRImageFrameOutput != NULL) {
				onIRImageFrameOutput(tcFrame);
				return(true);
			}
			else
				return(false);
		}

		bool Monitoring::processMarkersFrame(kocca::datalib::MocapMarkerFrame markerFrame) {
			if(onMarkersFrameOutput != NULL) {
				if(skippedMocapFramesCount >= 3) {
					skippedMocapFramesCount = 0;
					onMarkersFrameOutput(markerFrame);
				}
				else
					skippedMocapFramesCount++;

				return(true);
			}
			else
				return false;
		}
	} // namespace operations
} // namespace kocca