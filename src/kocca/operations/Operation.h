#ifndef KOCCA_OPERATIONS_OPERATION_H
#define KOCCA_OPERATIONS_OPERATION_H

#include "../datalib/TimeCodedFrame.h"
#include "../datalib/MocapMarkerFrame.h"

namespace kocca {
	namespace operations {

		/**
		 * Operation types
		 */
		enum OperationType {
			KOCCA_RECORDING_OPERATION,
			KOCCA_READING_OPERATION,
			KOCCA_MONITORING_OPERATION,
			KOCCA_CALIBRATION_OPERATION
		};

		/**
		 * Base interface for KOCCA operations. This is a purely virtual class, with no implementation. Descendant classes MUST implement purely virtual methods processDepthFrame, processColorImageFrame, processIRImageFrame and processMarkersFrame.
		 */
		class Operation {
		public:

			/**
			 * Constructor
			 */
			Operation();

			/**
			 * The type of operation. Useful to know to wich class we must cast an instance of Operation.
			 */
			OperationType type;

			/**
			 * Processes an image frame of the depth stream, through the operation.
			 * @param tcFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			virtual bool processDepthFrame(kocca::datalib::TimeCodedFrame tcFrame) = 0;

			/**
			 * Processes an image frame of the RGB stream, through the operation.
			 * @param tcFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			virtual bool processColorImageFrame(kocca::datalib::TimeCodedFrame tcFrame) = 0;

			/**
			 * Processes an image frame of the infrared stream, through the operation.
			 * @param tcFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			virtual bool processIRImageFrame(kocca::datalib::TimeCodedFrame tcFrame) = 0;

			/**
			 * Processes a MoCap markers frame of the MoCap stream, through the operation.
			 * @param markerFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			virtual bool processMarkersFrame(kocca::datalib::MocapMarkerFrame markerFrame) = 0;

			/**
			 * Callback function that will - if defined - be called to ouput the depth image frames after processing.
			 */
			void (*onDepthFrameOutput)(kocca::datalib::TimeCodedFrame);

			/**
			 * Callback function that will - if defined - be called to ouput the RGB image frames after processing.
			 */
			void (*onColorImageFrameOutput)(kocca::datalib::TimeCodedFrame);

			/**
			 * Callback function that will - if defined - be called to ouput the infrared image frames after processing.
			 */
			void (*onIRImageFrameOutput)(kocca::datalib::TimeCodedFrame);

			/**
			 * Callback function that will - if defined - be called to ouput the MoCap markers frames after processing.
			 */
			void (*onMarkersFrameOutput)(kocca::datalib::MocapMarkerFrame);
		};
	} // namespace operations
} // namespace kocca

#endif // KOCCA_OPERATIONS_OPERATION_H
