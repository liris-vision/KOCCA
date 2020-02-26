#ifndef KOCCA_OPERATIONS_SEQUENCE_RECORDING_H
#define KOCCA_OPERATIONS_SEQUENCE_RECORDING_H

#include <mutex>
#include <thread>

#include "Operation.h"
#include "../datalib/Sequence.h"
#include "TimeCodedFrameBuffer.h"

namespace kocca {
	namespace operations {

		/**
		 * SequenceRecording operation allows to record incoming data from Kinect and Mocap streams, and to write it on the filesystem as a Kocca Sequence folder.
		 */
		class SequenceRecording: public Operation {
		protected:

			/**
			 * The sequence object being recorded.
			 */
			kocca::datalib::Sequence* sequence;

			/**
			 * A lock to prevent access conflicts to the sequence object.
			 */
			std::mutex sequence_mutex;

			/**
			 * The memory buffer where incoming color image frames are temporarily stored before they get written to the filesystem.
			 */
			kocca::TimeCodedFrameBuffer imageBuffer;

			/**
			 * A lock to prevent access conflicts to the imageBuffer object. 
			 */
			std::mutex imageBuffer_mutex;

			/**
			 * Threads (there can be several at the same time) that write color image frames of the imageBuffer to the filesystem, then remove them from imageBuffer.
			 */
			std::vector<std::thread*> imageBufferWritingThreads;

			/**
			 * The memory buffer where incoming infrared image frames are temporarily stored before they get written to the filesystem.
			 */
			kocca::TimeCodedFrameBuffer infraredBuffer;

			/**
			 * A lock to prevent access conflicts to the infraredBuffer object. 
			 */
			std::mutex infraredBuffer_mutex;

			/**
			 * Threads (there can be several at the same time) that write infrared image frames of the infraredBuffer to the filesystem, then remove them from infraredBuffer.
			 */
			std::vector<std::thread*> infraredBufferWritingThreads;

			/**
			 * The memory buffer where incoming depth image frames are temporarily stored before they get written to the filesystem.
			 */
			kocca::TimeCodedFrameBuffer depthBuffer;

			/**
			 * A lock to prevent access conflicts to the depthBuffer object. 
			 */
			std::mutex depthBuffer_mutex;

			/**
			 * Threads (there can be several at the same time) that write depth image frames of the depthBuffer to the filesystem, then remove them from depthBuffer.
			 */
			std::vector<std::thread*> depthBufferWritingThreads;

			/**
			 * Number of working threads that write the content of each image buffer to the filesystem.
			 */
			int writingThreadsNumberPerBuffer;

			/**
			 * Whether or not the sequence is currently being recorded.
			 */
			bool isRecording;

			/**
			 * The local system time at which the recording began.
			 */
			long long startRecordingTime;

			/**
			 * The time of the latest incoming color image frame that was processed.
			 */
			std::atomic<unsigned long long> latestImageFrameTime;

			/**
			 * The time of the latest incoming infrared image frame that was processed.
			 */
			std::atomic<unsigned long long> latestInfraredFrameTime;

			/**
			 * The time of the latest incoming depth image frame that was processed.
			 */
			std::atomic<unsigned long long> latestDepthFrameTime;

			/**
			 * The time of the latest incoming MoCap marker frame that was processed.
			 */
			std::atomic<unsigned long long> latestMarkersFrameTime;

			/**
			 * A lock to protect startRecordingTime from threads access conflicts.
			 */
			std::mutex startRecordingTime_mutex;

			/**
			 * Format/compression parameters for image stream
			 */
			std::vector<int> imageFormatParams;

			/**
			 * Format/compression parameters for infrared stream
			 */
			std::vector<int> infraredFormatParams;

			/**
			 * Format/compression parameters for depth stream
			 */
			std::vector<int> depthFormatParams;

			/**
			 * Gets the current total size (in bytes) of the three recording buffers.
			 */
			int getTotalBuffersSize();

			/**
			 * The total maximum size (in bytes) allowed for the three buffers added.
			 */
			uint64_t maxBuffersSize;

			/**
			 * To save resources, 3 out of 4 MoCap frames are not output (but all of them are recorded of course !). This cycle-counts skipped frames up to 3, then reset to 0 and so on ...
			 */
			int skippedMocapFramesCount;

			/**
			 * An error that happened in a thread (typically, a buffer writing thread) and that we memorize to re-throw it during a call to process_XXFrame().
			 * @todo check if this is still used, then remove it if it's not. 
			 */
			std::runtime_error* error;

		public:

			/**
			 * Constructor.
			 * @param _sequence 
			 * @param _maxBuffersSize 
			 * @param _writingThreadsNumberPerBuffer 
			 */
			SequenceRecording(kocca::datalib::Sequence* _sequence, uint64_t _maxBuffersSize = 1500000000, int _writingThreadsNumberPerBuffer = 4);

			/**
			 * Destructor.
			 */
			~SequenceRecording();

			/**
			 * Converts an absolute local system timestamp to a time relative to the sequence.
			 * @param time the local system timestamp, in milliseconds
			 * @return the time in the sequence, in milliseconds
			 */
			unsigned long long getRelativeTime(unsigned long long time);

			/**
			 * Processes an image frame of the depth stream, through the operation.
			 * @param tcFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 * @throws RecordBufferOverFlowException if the buffer exeeded the maximum allowed size
			 */
			bool processDepthFrame(kocca::datalib::TimeCodedFrame tcFrame);

			/**
			 * Processes an image frame of the RGB stream, through the operation.
			 * @param tcFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 * @throws RecordBufferOverFlowException if the buffer exeeded the maximum allowed size
			 */
			bool processColorImageFrame(kocca::datalib::TimeCodedFrame tcFrame);

			/**
			 * Processes an image frame of the infrared stream, through the operation.
			 * @param tcFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 * @throws RecordBufferOverFlowException if the buffer exeeded the maximum allowed size
			 */
			bool processIRImageFrame(kocca::datalib::TimeCodedFrame tcFrame);

			/**
			 * Processes a MoCap markers frame of the MoCap stream, through the operation.
			 * @param markerFrame the frame to process
			 * @return true if tcFrame was used during the processing, or false if it was ignored
			 */
			bool processMarkersFrame(kocca::datalib::MocapMarkerFrame markerFrame);

			/**
			 * Starts the recording.
			 */
			void startRecording();

			/**
			 * Stops the recording.
			 */
			void stop();

			/**
			 * Implementation for color image buffer writing threads, that write color image frames of the imageBuffer to the filesystem, then remove them from imageBuffer.
			 * @throws FileWritingException if an error happens during the call to cv::imwrite()
			 */
			void imageBufferWritingThreadLoop();

			/**
			 * Implementation for infrared image buffer writing threads, that write infrared image frames of the infraredBuffer to the filesystem, then remove them from infraredBuffer.
			 * @throws FileWritingException if an error happens during the call to cv::imwrite()
			 */
			void infraredBufferWritingThreadLoop();

			/**
			 * Implementation for depth image buffer writing threads, that write depth image frames of the depthBuffer to the filesystem, then remove them from depthBuffer.
			 * @throws FileWritingException if an error happens during the call to cv::imwrite()
			 */
			void depthBufferWritingThreadLoop();

			/**
			 * Prepares an EXISTING folder to receive the sequence data during recording. After calling this function, the temp folder should cointain 3 sub-folders ("image", "infrared", and "depth") and nothing else.
			 * @param tempDirectory the path where the recorded sequence data will be written.
			 * @throws TempFolderNotAvailableException if sequenceFolderPath doesn't exists or if it is not a directory.
			 */
			void cleanAndPrepareTempFolder(boost::filesystem::path tempDirectory);

			/**
			 * Stops the recording and memorizes a runtime error that happened in a separate thread, so we can re-throw it later.
			 * @param re the runtime error to memorize
			 */
			void setError(std::runtime_error re);
		};
	} // namespace operations
} // namespace kocca

#endif // KOCCA_OPERATIONS_SEQUENCE_RECORDING_H
