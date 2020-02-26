#ifndef KOCCA_OPERATIONS_SEQUENCE_READING_H
#define KOCCA_OPERATIONS_SEQUENCE_READING_H

#include <mutex>
#include <thread>
#include <atomic>

#include "Operation.h"
#include "../datalib/Sequence.h"
#include "TimeCodedFrameBuffer.h"

namespace kocca {
	namespace operations {

		/**
		 * SequenceReading operation allows to read/play a Sequence.
		 */
		class SequenceReading: public Operation {
		public:

			/**
			 * Constructor.
			 * @param _sequence a pointer to the sequence object we want to read
			 * @param _refreshPeriod The time (in milliseconds) of pause between each data output during playing. Note that modifying this setting won't affect the sequence duration nor it's playing speed : it's only a balance between fluidity and resource consumption.
			 * @param maxBuffersSize The maximum size (in Mega octets) of reading buffer for each image stream. Setting this too low might cause lags if the hard drive brandwidth is insufficient, setting it too high will increase memory usage. 
			 * @throws EmptySequenceException if the sequence object contains no readable data
			 */
			SequenceReading(kocca::datalib::Sequence* _sequence, int _refreshPeriod = 30, int maxBuffersSize = 100);

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

			/**
			 * Gets the position of the playhead, in milliseconds within the sequence time.
			 */
			unsigned long long getPlayHeadPosition();

			/**
			 * Sets the position of the playhead.
			 * @param position the new position of the playhead, in milliseconds within the sequence time.
			 */
			bool setPlayHeadPosition(unsigned long long position);

			/**
			 * Moves the playhead to the time of the last frame (either an infrared, RGB, depth image frame or a MoCap markers frame) found before it's current position.
			 */
			bool goToPreviousFrame();

			/**
			 * Moves the playhead to the time of the first frame (either an infrared, RGB, depth image frame or a MoCap markers frame) found after it's current position.
			 */
			bool goToNextFrame();

			/**
			 * Moves the playhead to the begining of the sequence (0 ms).
			 */
			void goToBegining();

			/**
			 * Moves the playhead to the end of the sequence.
			 */
			void goToEnd();

			/**
			 * Indicates if the sequence is currently being played.
			 */
			bool isPlaying();

			/**
			 * Starts playing the sequence.
			 */
			void startPlaying();

			/**
			 * Stops playing the sequence.
			 */
			void stop();

			/**
			 * Gets the time (in milliseconds) of pause between each data output during playing.
			 */
			int getRefreshPeriod();

			/**
			 * Outputs the depth image frame corresponding to the current position of the playhead.
			 * @param force Forces the ouptut of the frame, even if it has already been output.
			 */
			void outputDepthFrame(bool force = false);

			/**
			 * Outputs the color image frame corresponding to the current position of the playhead.
			 * @param force Forces the ouptut of the frame, even if it has already been output.
			 */
			void outputImageFrame(bool force = false);

			/**
			 * Outputs the infrared image frame corresponding to the current position of the playhead.
			 * @param force Forces the ouptut of the frame, even if it has already been output.
			 */
			void outputIRFrame(bool force = false);

			/**
			 * Outputs the MoCap markers frame corresponding to the current position of the playhead.
			 */
			void outputMarkersFrame();

			/**
			 * Returns a pointer to the sequence object being read.
			 */
			kocca::datalib::Sequence* getSequence();

			/**
			 * Implementation for the imageFramesBufferingThread thread. It loads into memory the content of upcoming (=after current playhead position) color image frames.
			 */
			void imageBufferingLoop();

			/**
			 * Implementation for the irFramesBufferingThread thread. It loads into memory the content of upcoming (=after current playhead position) infrared image frames.
			 */
			void irBufferingLoop();

			/**
			 * Implementation for the depthFramesBufferingThread thread. It loads into memory the content of upcoming (=after current playhead position) depth image frames.
			 */
			void depthBufferingLoop();

			/**
			 * Updates the position of the playhead.
			 */
			void updatePlayHeadPosition();

			/**
			 * Implementation for the playingThread thread, that runs when the sequence is playing.
			 */
			void playingLoop();

			/**
			 * Callback function called every time the playhead's position changes, to notify it to other objects.
			 * @param position the new position of the playhead, in milliseconds.
			 */
			void (*onChangePlayheadPosition)(unsigned long long position);

			/**
			 * Callback function called to notify other objetcts that playing has stopped because the playhead has reached the end of the sequence
			 */
			void (*onStopAtTheEnd)();

			/**
			 * Callback function called to notify other objects that the reading buffers have a new ending point.
			 * 
			 */
			void (*onUpdateBufferEndingPoint)(unsigned long long endingPoint);

			/**
			 * Checks if the playhead has reached the end of the sequence.
			 */
			bool playHeadHasReachedEnd();

			/**
			 * Stops the upcoming images frames from being loaded into memory. 
			 */
			void stopBuffering();

			/**
			 * Destructor.
			 */
			~SequenceReading();

		protected:

			/**
			 * A pointer to the sequence object being read.
			 */
			kocca::datalib::Sequence* sequence;

			/**
			 * The time (in milliseconds) of pause between each data output during playing.
			 */
			int refreshPeriod;

			/**
			 * Whether or not the sequence is currently being played.
			 * @todo use std::atomic<bool> type to prevent conflicts
			 */
			bool playing;

			/**
			 * Whether or not the object should load into memory the upcoming images frames.
			 */
			std::atomic<bool> bufferingIsActive;

			/**
			 * The current position of the playhead, in milliseconds within the sequence's time.
			 * @todo use std::atomic<long long> type to prevent conflicts
			 */
			long long playHeadPosition;

			/**
			 * The local system time, in milliseconds, at which the playing began.
			 * @todo use std::atomic<long long> type to prevent conflicts
			 */
			long long startPlayingTime;

			/**
			 * The position, in milliseconds within the sequence's time, that the playhead was at, when the playing began.
			 * @todo use std::atomic<long long> type to prevent conflicts
			 */
			long long startPlayingPosition;

			/**
			 * The reading buffer for the color images frames.
			 */
			TimeCodedFrameBuffer imageFramesBuffer;

			/**
			 * The reading buffer for the infrared images frames.
			 */
			TimeCodedFrameBuffer irFramesBuffer;

			/**
			 * The reading buffer for the depth images frames.
			 */
			TimeCodedFrameBuffer depthFramesBuffer;

			/**
			 * The maximum size (in Mega octets) of reading buffer for each image stream.
			 */
			int framesBufferMaxSize;

			/**
			 * Mutex to lock the color image buffer and prevent threads access conflicts.
			 */
			std::mutex imageFramesBuffer_mutex;

			/**
			 * The thread that reads color images frames and loads their content into memory.
			 */
			std::thread* imageFramesBufferingThread;

			/**
			 * Mutex to lock the infrared image buffer and prevent threads access conflicts.
			 */
			std::mutex irFramesBuffer_mutex;

			/**
			 * The thread that reads infrared images frames and loads their content into memory.
			 */
			std::thread* irFramesBufferingThread;

			/**
			 * Mutex to lock the depth image buffer and prevent threads access conflicts.
			 */
			std::mutex depthFramesBuffer_mutex;

			/**
			 * The thread that reads depth images frames and loads their content into memory.
			 */
			std::thread* depthFramesBufferingThread;

			/**
			 * Calculates the end point (in milliseconds) of the buffers, then notifies it's new end position via a call to the  onUpdateBufferEndingPoint callback function.
			 */
			void updateBufferEndingPoint();

			/**
			 * The threads that periodically updates the playhead position and outputs data when the sequence is playing.
			 */
			std::thread* playingThread;

			/**
			 * Time of the color image frame that was the last to be output, or -1 if none has been output yet.
			 * @todo use std::atomic<unsigned long long> type to prevent conflicts
			 */
			unsigned long long lastOutputImageFrameTime;

			/**
			 * Time of the infrared image frame that was the last to be output, or -1 if none has been output yet.
			 * @todo use std::atomic<unsigned long long> type to prevent conflicts
			 */
			unsigned long long lastOutputIRFrameTime;

			/**
			 * Time of the depth image frame that was the last to be output, or -1 if none has been output yet.
			 * @todo use std::atomic<unsigned long long> type to prevent conflicts
			 */
			unsigned long long lastOutputDepthFrameTime;
		};
	} // namespace operations
} // namespace kocca

#endif; // KOCCA_OPERATIONS_SEQUENCE_READING_H
