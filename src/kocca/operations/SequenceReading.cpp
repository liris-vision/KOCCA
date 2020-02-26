#include "SequenceReading.h"
#include "../utils.h"
#include "../datalib/FramePath.h"
#include "../Exceptions.h"

namespace kocca {
	namespace operations {
		/**
		 * @throws EmptySequenceException
		 */
		SequenceReading::SequenceReading(kocca::datalib::Sequence* _sequence, int _refreshPeriod, int maxBuffersSize): Operation() {
			type = KOCCA_READING_OPERATION;
			refreshPeriod = _refreshPeriod;
			playing = false;
			bufferingIsActive = true;
			playHeadPosition = 0;
			onChangePlayheadPosition = NULL;
			onUpdateBufferEndingPoint = NULL;
			framesBufferMaxSize = maxBuffersSize;
			lastOutputImageFrameTime = -1;
			lastOutputIRFrameTime = -1;
			lastOutputDepthFrameTime = -1;
			imageFramesBufferingThread = NULL;
			irFramesBufferingThread = NULL;
			depthFramesBufferingThread = NULL;
			playingThread = NULL;

			if(_sequence->hasRecordedData()) {
				sequence = _sequence;
				imageFramesBufferingThread = new std::thread(&SequenceReading::imageBufferingLoop, this);
				irFramesBufferingThread = new std::thread(&SequenceReading::irBufferingLoop, this);
				depthFramesBufferingThread = new std::thread(&SequenceReading::depthBufferingLoop, this);
			}
			else
				throw EmptySequenceException("No recorded data found in sequence");
		}

		bool SequenceReading::processDepthFrame(kocca::datalib::TimeCodedFrame tcFrame) {
			return false;
		}

		bool SequenceReading::processColorImageFrame(kocca::datalib::TimeCodedFrame tcFrame) {
			return false;
		}

		bool SequenceReading::processIRImageFrame(kocca::datalib::TimeCodedFrame tcFrame) {
			return false;
		}

		bool SequenceReading::processMarkersFrame(kocca::datalib::MocapMarkerFrame markerFrame) {
			return false;
		}

		unsigned long long SequenceReading::getPlayHeadPosition() {
			return playHeadPosition;
		}

		bool SequenceReading::setPlayHeadPosition(unsigned long long position) {
			if(position > sequence->getDuration()) {
				position = sequence->getDuration();
				stop();
		
				if(onStopAtTheEnd != NULL)
					onStopAtTheEnd();
			}
			else {
				playHeadPosition = position;

				imageFramesBuffer_mutex.lock();
	
				if((position < imageFramesBuffer.getStartingPoint()) || (position > imageFramesBuffer.getEndingPoint()))
					imageFramesBuffer.clear();

				imageFramesBuffer_mutex.unlock();

				irFramesBuffer_mutex.lock();

				if ((position < irFramesBuffer.getStartingPoint()) || (position > irFramesBuffer.getEndingPoint()))
					irFramesBuffer.clear();

				irFramesBuffer_mutex.unlock();

				depthFramesBuffer_mutex.lock();

				if((position < depthFramesBuffer.getStartingPoint()) || (position > depthFramesBuffer.getEndingPoint()))
					depthFramesBuffer.clear();

				depthFramesBuffer_mutex.unlock();
			}

			outputImageFrame();
			outputIRFrame();
			outputDepthFrame();
			outputMarkersFrame();

			if(onChangePlayheadPosition != NULL)
				onChangePlayheadPosition(playHeadPosition);

			return true;
		}

		/**
		 * @throws NoPreviousEventException
		 */
		bool SequenceReading::goToPreviousFrame() {
			long long previousEventTime = sequence->getPreviousEventTime(playHeadPosition);
			setPlayHeadPosition(previousEventTime);
			return(true);
		}

		/**
		 * @throws NoNextEventException
		 */
		bool SequenceReading::goToNextFrame() {
			long long nextEventTime = sequence->getNextEventTime(playHeadPosition);
			setPlayHeadPosition(nextEventTime);
			return(true);
		}

		void SequenceReading::goToBegining() {
			setPlayHeadPosition(0);
		}

		void SequenceReading::goToEnd() {
			setPlayHeadPosition(sequence->getDuration());
		}

		bool SequenceReading::isPlaying() {
			return playing;
		}

		void SequenceReading::playingLoop() {
			while(playing) {
				updatePlayHeadPosition();
				outputImageFrame();
				outputIRFrame();
				outputDepthFrame();
				outputMarkersFrame();
				std::this_thread::sleep_for(std::chrono::milliseconds(refreshPeriod));
			}
		}

		void SequenceReading::startPlaying() {
			if(!playing) {
				startPlayingTime = getMSTime();
				playing = true;
				startPlayingPosition = playHeadPosition;
				playingThread = new std::thread(&SequenceReading::playingLoop, this);
			}
		}

		void SequenceReading::stop() {
			playing = false;
			
			if((playingThread != NULL) && (playingThread->joinable()))
				playingThread->join();
		}

		int SequenceReading::getRefreshPeriod() {
			return refreshPeriod;
		}

		kocca::datalib::Sequence* SequenceReading::getSequence() {
			return sequence;
		}

		void SequenceReading::outputImageFrame(bool force) {
			if((onColorImageFrameOutput != NULL) && !sequence->getImageFramesList().empty()) {
				try {
					imageFramesBuffer_mutex.lock();
					kocca::datalib::TimeCodedFrame outputFrame = imageFramesBuffer.getFrameAtTime(playHeadPosition);
					imageFramesBuffer_mutex.unlock();

					if(force || (outputFrame.time != lastOutputImageFrameTime)) {
						onColorImageFrameOutput(outputFrame);
						lastOutputImageFrameTime = outputFrame.time;
					}
				}
				catch(FrameNotInBufferException& e) {
					imageFramesBuffer_mutex.unlock();
					kocca::datalib::TimeCodedFrame outputFrame = sequence->getImageFrameByTime(playHeadPosition);
					onColorImageFrameOutput(outputFrame);
				}
			}
		}

		void SequenceReading::outputIRFrame(bool force) {
			if ((onIRImageFrameOutput != NULL) && !sequence->getIRFramesList().empty()) {
				try {
					irFramesBuffer_mutex.lock();
					kocca::datalib::TimeCodedFrame outputFrame = irFramesBuffer.getFrameAtTime(playHeadPosition);
					irFramesBuffer_mutex.unlock();

					if (force || (outputFrame.time != lastOutputIRFrameTime)) {
						onIRImageFrameOutput(outputFrame);
						lastOutputIRFrameTime = outputFrame.time;
					}
				}
				catch (FrameNotInBufferException& e) {
					irFramesBuffer_mutex.unlock();
					kocca::datalib::TimeCodedFrame outputFrame = sequence->getIRFrameByTime(playHeadPosition);
					onIRImageFrameOutput(outputFrame);
				}
			}
		}

		void SequenceReading::outputDepthFrame(bool force) {
			if((onDepthFrameOutput != NULL) && !sequence->getDepthFramesList().empty()) {
				try {
					depthFramesBuffer_mutex.lock();
					kocca::datalib::TimeCodedFrame outputFrame = depthFramesBuffer.getFrameAtTime(playHeadPosition);
					depthFramesBuffer_mutex.unlock();

					if(force || (outputFrame.time != lastOutputDepthFrameTime)) {
						onDepthFrameOutput(outputFrame);
						lastOutputDepthFrameTime = outputFrame.time;
					}
				}
				catch(FrameNotInBufferException& e) {
					depthFramesBuffer_mutex.unlock();
					kocca::datalib::TimeCodedFrame outputFrame = sequence->getDepthFrameByTime(playHeadPosition);
					onDepthFrameOutput(outputFrame);
				}
			}
		}

		void SequenceReading::outputMarkersFrame() {
			if((onMarkersFrameOutput != NULL) && sequence->markersSequence.hasData()) {
				try {
					kocca::datalib::MocapMarkerFrame frame = sequence->markersSequence.getFrameAtTime(playHeadPosition);
					onMarkersFrameOutput(frame);
				}
				catch(OutOfSequenceException& e) {
					// we do nothing
				}
			}
		}

		void SequenceReading::updatePlayHeadPosition() {
			imageFramesBuffer_mutex.lock();
			irFramesBuffer_mutex.lock();
			depthFramesBuffer_mutex.lock();

			bool hasReachedEnd = false;

			playHeadPosition = startPlayingPosition + (getMSTime() - startPlayingTime);

			if(playHeadPosition > sequence->getDuration()) {
				playHeadPosition = sequence->getDuration();
				stop();
				hasReachedEnd = true;
			}
	
			imageFramesBuffer.purgeBefore(playHeadPosition);
			imageFramesBuffer_mutex.unlock();

			irFramesBuffer.purgeBefore(playHeadPosition);
			irFramesBuffer_mutex.unlock();

			depthFramesBuffer.purgeBefore(playHeadPosition);
			depthFramesBuffer_mutex.unlock();

			if(hasReachedEnd && (onStopAtTheEnd != NULL))
				onStopAtTheEnd();

			if(onChangePlayheadPosition != NULL)
				onChangePlayheadPosition(playHeadPosition);
		}

		bool SequenceReading::playHeadHasReachedEnd() {
			return(playHeadPosition >= sequence->getDuration());
		}

		void SequenceReading::stopBuffering() {
			bufferingIsActive = false;

			if((imageFramesBufferingThread != NULL) && (imageFramesBufferingThread->joinable()))
				imageFramesBufferingThread->join();

			if ((irFramesBufferingThread != NULL) && (irFramesBufferingThread->joinable()))
				irFramesBufferingThread->join();

			if((depthFramesBufferingThread != NULL) && (depthFramesBufferingThread->joinable()))
				depthFramesBufferingThread->join();
		}

		SequenceReading::~SequenceReading() {
			stop();
			stopBuffering();
		}

		void SequenceReading::updateBufferEndingPoint() {
			if(onUpdateBufferEndingPoint != NULL) {
				unsigned long long int bufferEndingPoint = 0;

				imageFramesBuffer_mutex.lock();
				irFramesBuffer_mutex.lock();
				depthFramesBuffer_mutex.lock();

				if((imageFramesBuffer.size() > 0) && (irFramesBuffer.size() > 0) && (depthFramesBuffer.size() > 0)) {
					std::vector<unsigned long long> buffersEndingPoints;
					buffersEndingPoints.push_back(imageFramesBuffer.back().time);
					buffersEndingPoints.push_back(irFramesBuffer.back().time);
					buffersEndingPoints.push_back(depthFramesBuffer.back().time);
					std::sort(buffersEndingPoints.begin(), buffersEndingPoints.end());
					bufferEndingPoint = buffersEndingPoints.back();
				}

				imageFramesBuffer_mutex.unlock();
				irFramesBuffer_mutex.unlock();
				depthFramesBuffer_mutex.unlock();

				onUpdateBufferEndingPoint(bufferEndingPoint);
			}
		}

		/**
		 * @throws InvalidSequenceRankException
		 * @throws EmptySequenceStreamException
		 */
		void SequenceReading::imageBufferingLoop() {
			while(bufferingIsActive) {
				if(imageFramesBuffer_mutex.try_lock()) {
					if((sequence->getImageFramesCount() > 0) && (imageFramesBuffer.size() < framesBufferMaxSize)) {
						kocca::datalib::FramePath nextFramePath;

						if(imageFramesBuffer.size() > 0) {
							long long lastBufferedFrameTime = imageFramesBuffer.at(imageFramesBuffer.size() - 1).time;

							if(playHeadPosition > lastBufferedFrameTime)
								lastBufferedFrameTime = playHeadPosition;

							int nextFrameRank = sequence->getImageFrameRank(lastBufferedFrameTime) + 1;
				
							if(nextFrameRank < sequence->getImageFramesCount())
								nextFramePath = sequence->getImageFramePathByRank(nextFrameRank);
						}
						else
							nextFramePath = sequence->getImageFramePathByTime(playHeadPosition);

						if(!nextFramePath.path.empty()) {
							kocca::datalib::TimeCodedFrame nextTimeCodedFrame;
							nextTimeCodedFrame.time = nextFramePath.time;
							cv::Mat cvFrame = cv::imread(nextFramePath.path, cv::IMREAD_ANYCOLOR);
							cv::cvtColor(cvFrame, cvFrame, CV_BGRA2RGB);
							nextTimeCodedFrame.frame = cvFrame;
							imageFramesBuffer.push_back(nextTimeCodedFrame);
						}
					}

					imageFramesBuffer_mutex.unlock();

					if (onUpdateBufferEndingPoint != NULL)
						updateBufferEndingPoint();
				}
				
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}

		/**
		* @throws InvalidSequenceRankException
		* @throws EmptySequenceStreamException
		*/
		void SequenceReading::irBufferingLoop() {
			while (bufferingIsActive) {
				if (irFramesBuffer_mutex.try_lock()) {
					if ((sequence->getIRFramesCount() > 0) && (irFramesBuffer.size() < framesBufferMaxSize)) {
						kocca::datalib::FramePath nextFramePath;

						if (irFramesBuffer.size() > 0) {
							long long lastBufferedFrameTime = irFramesBuffer.at(irFramesBuffer.size() - 1).time;

							if (playHeadPosition > lastBufferedFrameTime)
								lastBufferedFrameTime = playHeadPosition;

							int nextFrameRank = sequence->getIRFrameRank(lastBufferedFrameTime) + 1;

							if (nextFrameRank < sequence->getIRFramesCount())
								nextFramePath = sequence->getIRFramePathByRank(nextFrameRank);
						}
						else
							nextFramePath = sequence->getIRFramePathByTime(playHeadPosition);

						if (!nextFramePath.path.empty()) {
							kocca::datalib::TimeCodedFrame nextTimeCodedFrame;
							nextTimeCodedFrame.time = nextFramePath.time;
							nextTimeCodedFrame.frame = cv::imread(nextFramePath.path, cv::IMREAD_GRAYSCALE);
							irFramesBuffer.push_back(nextTimeCodedFrame);
						}
					}

					irFramesBuffer_mutex.unlock();

					if (onUpdateBufferEndingPoint != NULL)
						updateBufferEndingPoint();
				}

				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}

		void SequenceReading::depthBufferingLoop() {
			while(bufferingIsActive) {
				if(depthFramesBuffer_mutex.try_lock()) {
					if((sequence->getDepthFramesCount() > 0) && (depthFramesBuffer.size() < framesBufferMaxSize)) {
						kocca::datalib::FramePath nextFramePath;
			
						if(depthFramesBuffer.size() > 0) {
							long long lastBufferedFrameTime = depthFramesBuffer.at(depthFramesBuffer.size() - 1).time;
							int nextFrameRank = sequence->getDepthFrameRank(lastBufferedFrameTime) + 1;
				
							if(nextFrameRank < sequence->getDepthFramesCount())
								nextFramePath = sequence->getDepthFramePathByRank(nextFrameRank);
						}
						else
							nextFramePath = sequence->getDepthFramePathByTime(playHeadPosition);

						if(!nextFramePath.path.empty()) {
							kocca::datalib::TimeCodedFrame nextTimeCodedFrame;
							nextTimeCodedFrame.time = nextFramePath.time;
							nextTimeCodedFrame.frame = cv::imread(nextFramePath.path, CV_LOAD_IMAGE_ANYDEPTH);
							depthFramesBuffer.push_back(nextTimeCodedFrame);
						}
					}
		
					depthFramesBuffer_mutex.unlock();

					if(onUpdateBufferEndingPoint != NULL)
						updateBufferEndingPoint();
				}
				
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
		}
	} // namespace operations
} // namespace kocca
