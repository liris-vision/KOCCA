#ifndef KOCCA_OPERATIONS_MONITORING_H
#define KOCCA_OPERATIONS_MONITORING_H

#include "Operation.h"

#include <atomic>

namespace kocca {
	namespace operations {

		/**
		 * Monitoring is an operation during wich we do ... nothing. We just output the incoming data unaltered, except that we skip (= don't output them) 3 MoCap markers frames out of 4 in order to save resouces : since the Optitrack has a high framerate (120 fps), it is not crucial to use each and every frame when it's just for visualisation.
		 */
		class Monitoring: public Operation {
		protected:

			/**
			 * Counter for skipped MoCap markers frames. Incremented at each skipped frame, every time the count reaches 3 we output the frame and reset skippedMocapFramesCount back to 0.
			 */
			int skippedMocapFramesCount;

		public:

			/**
			 * Constructor
			 */
			Monitoring();

			/**
			 * Processes an image frame of the depth stream, through the operation.
			 * @param tcFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			bool processDepthFrame(kocca::datalib::TimeCodedFrame tcFrame);

			/**
			 * Processes an image frame of the RGB stream, through the operation.
			 * @param tcFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			bool processColorImageFrame(kocca::datalib::TimeCodedFrame tcFrame);

			/**
			 * Processes an image frame of the infrared stream, through the operation.
			 * @param tcFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			bool processIRImageFrame(kocca::datalib::TimeCodedFrame tcFrame);

			/**
			 * Processes a MoCap markers frame of the MoCap stream, through the operation.
			 * @param markerFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			bool processMarkersFrame(kocca::datalib::MocapMarkerFrame markerFrame);
		};
	} // namespace operations
} // namespace kocca

#endif // KOCCA_OPERATIONS_MONITORING_H
