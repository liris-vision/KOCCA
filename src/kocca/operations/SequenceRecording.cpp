#include "SequenceRecording.h"
#include "../utils.h"
#include "boost/filesystem.hpp"
#include "../Exceptions.h"

namespace kocca {
	namespace operations {
		/**
		 * @throws TempFolderNotAvailableException
		 */
		SequenceRecording::SequenceRecording(kocca::datalib::Sequence* _sequence, uint64_t _maxBuffersSize, int _writingThreadsNumberPerBuffer) {
			type = KOCCA_RECORDING_OPERATION;

			maxBuffersSize = _maxBuffersSize;
			writingThreadsNumberPerBuffer = _writingThreadsNumberPerBuffer;

			imageFormatParams.push_back(CV_IMWRITE_JPEG_QUALITY);
			imageFormatParams.push_back(100);

			infraredFormatParams.push_back(CV_IMWRITE_PNG_COMPRESSION);
			infraredFormatParams.push_back(0);

			depthFormatParams.push_back(CV_IMWRITE_PNG_COMPRESSION);
			depthFormatParams.push_back(0);	

			bool isRecording = false;
	
			sequence = _sequence;

			skippedMocapFramesCount = 0;

			latestImageFrameTime = 0;
			latestInfraredFrameTime = 0;
			latestDepthFrameTime = 0;
			latestMarkersFrameTime = 0;

			cleanAndPrepareTempFolder(sequence->getRootDirectory());

			error = NULL;
		}

		SequenceRecording::~SequenceRecording() {
			if(isRecording) {
				try {
					// stop recording, in case destructor is called while the object is still recording
					stop();
				}
				catch(std::exception& e) {
					// we do nothing
				}
			}

			// wait for the end of the threads that dumps files from buffers
			for (int i = 0; i < imageBufferWritingThreads.size(); i++)
				imageBufferWritingThreads.at(i)->join();

			for (int i = 0; i < infraredBufferWritingThreads.size(); i++)
				infraredBufferWritingThreads.at(i)->join();

			for (int i = 0; i < depthBufferWritingThreads.size(); i++)
				depthBufferWritingThreads.at(i)->join();

			// finally, sort sequence frames and recalculate its total length
			sequence->updateDuration();
		}

		int SequenceRecording::getTotalBuffersSize() {
			return (
					(1920 * 1080 * 3 * imageBuffer.size())
				+	(512 * 424 * 2 * infraredBuffer.size())
				+	(512 * 424 * 2 * depthBuffer.size()));
		}

		/**
		 * @throws TempFolderNotAvailableException
		 */
		void SequenceRecording::cleanAndPrepareTempFolder(boost::filesystem::path tempDirectory) {
			if(boost::filesystem::exists(tempDirectory)) {
				if(boost::filesystem::is_directory(tempDirectory)) {
					boost::filesystem::directory_iterator end;
					std::vector<boost::filesystem::path> tempChilds;

					for(boost::filesystem::directory_iterator i(tempDirectory); i != end; ++i)
						tempChilds.push_back(i->path());

					for(int i = 0; i < tempChilds.size(); i++)
						if(boost::filesystem::exists(tempChilds.at(i))) {
							try {
								if(boost::filesystem::is_directory(tempChilds.at(i))) {
									if((tempChilds.at(i) == (tempDirectory / "image")) || (tempChilds.at(i) == (tempDirectory / "infrared")) || (tempChilds.at(i) == (tempDirectory / "depth"))) {
										for(boost::filesystem::directory_iterator j(tempChilds.at(i)); j != end; ++j)
											boost::filesystem::remove_all(j->path());
									}
									else
										boost::filesystem::remove_all(tempChilds.at(i));
								}
								else
									boost::filesystem::remove(tempChilds.at(i));
							}
							catch(boost::filesystem::filesystem_error fse) {
								// file has already been deleted, we do nothing
							}
						}

					if(!boost::filesystem::exists(tempDirectory / "image"))
						boost::filesystem::create_directory(tempDirectory / "image");

					if (!boost::filesystem::exists(tempDirectory / "infrared"))
						boost::filesystem::create_directory(tempDirectory / "infrared");

					if(!boost::filesystem::exists(tempDirectory / "depth"))
						boost::filesystem::create_directory(tempDirectory / "depth");
				}
				else
					throw TempFolderNotAvailableException("Temporary recording path is not a valid directory");
			}
			else
				throw TempFolderNotAvailableException("Temporary recording path does not exists");
		}

		/**
		 * @throws RecordBufferOverFlowException
		 * @throws std::runtime_error
		 */
		bool SequenceRecording::processDepthFrame(kocca::datalib::TimeCodedFrame tcFrame) {
			if(onDepthFrameOutput != NULL) {
				kocca::datalib::TimeCodedFrame displayTCFrame;
				displayTCFrame.frame = tcFrame.frame.clone();
				displayTCFrame.time = tcFrame.time;

				if (displayTCFrame.time > latestDepthFrameTime) {
					latestDepthFrameTime = displayTCFrame.time;
					onDepthFrameOutput(displayTCFrame);
				}
			}

			if(isRecording) {
				if(getTotalBuffersSize() < maxBuffersSize) {
					tcFrame.time = getRelativeTime(tcFrame.time);
					depthBuffer_mutex.lock();
					depthBuffer.push_back(tcFrame);
					depthBuffer_mutex.unlock();
					return true;
				}
				else {
					try {
						stop();
					}
					catch(std::exception& e) {
						// we do nothing, we keep throwing a RecordBufferOverFlowException (below)
					}

					throw RecordBufferOverFlowException("Recording buffers have reached maximum allowed size");
				}
			}
			else if(error != NULL) {
				throw error;
			}
			else
				return false;
		}

		/**
		 * @throws RecordBufferOverFlowException
		 * @throws std::runtime_error
		 */
		bool SequenceRecording::processColorImageFrame(kocca::datalib::TimeCodedFrame tcFrame) {
			unsigned long long recordedTime;

			if(isRecording)
				tcFrame.time = getRelativeTime(tcFrame.time);

			if(onColorImageFrameOutput != NULL) {
				kocca::datalib::TimeCodedFrame displayTCFrame;
				displayTCFrame.frame = tcFrame.frame.clone();
				displayTCFrame.time = tcFrame.time;

				if (displayTCFrame.time > latestImageFrameTime) {
					latestImageFrameTime = displayTCFrame.time;
					onColorImageFrameOutput(displayTCFrame);
				}
			}

			if(isRecording) {
				if(getTotalBuffersSize() < maxBuffersSize) {
					imageBuffer_mutex.lock();
					imageBuffer.push_back(tcFrame);
					imageBuffer_mutex.unlock();
					return true;
				}
				else {
					try {
						stop();
					}
					catch(std::exception& e) {
						// we do nothing, we keep throwing a RecordBufferOverFlowException (below)
					}

					throw RecordBufferOverFlowException("Recording buffers have reached maximum allowed size");
				}
			}
			else if(error != NULL) {
				throw error;
			}
			else
				return false;
		}

		bool  SequenceRecording::processIRImageFrame(kocca::datalib::TimeCodedFrame tcFrame) {
			unsigned long long recordedTime;

			if (isRecording)
				tcFrame.time = getRelativeTime(tcFrame.time);

			if (onIRImageFrameOutput != NULL) {
				kocca::datalib::TimeCodedFrame displayTCFrame;
				displayTCFrame.frame = tcFrame.frame.clone();
				displayTCFrame.time = tcFrame.time;

				if (displayTCFrame.time > latestInfraredFrameTime) {
					latestInfraredFrameTime = displayTCFrame.time;
					onIRImageFrameOutput(displayTCFrame);
				}
			}

			if (isRecording) {
				if (getTotalBuffersSize() < maxBuffersSize) {
					infraredBuffer_mutex.lock();
					infraredBuffer.push_back(tcFrame);
					infraredBuffer_mutex.unlock();
					return true;
				}
				else {
					try {
						stop();
					}
					catch (std::exception& e) {
						// we do nothing, we keep throwing a RecordBufferOverFlowException (below)
					}

					throw RecordBufferOverFlowException("Recording buffers have reached maximum allowed size");
				}
			}
			else if (error != NULL) {
				throw error;
			}
			else
				return false;
		}

		/**
		 * @throws std::runtime_error
		 */
		bool SequenceRecording::processMarkersFrame(kocca::datalib::MocapMarkerFrame markerFrame) {
			// @TODO : utiliser plutôt le temps de la frame : data->fTimestamp
			// -> multiplier par 1000 pour convertir en ms
			// -> puis appliquer correction de latence mesurée lorsque cette mesure sera implémentée
			markerFrame.time = getRelativeTime(markerFrame.time);

			if(onMarkersFrameOutput != NULL) {
				// we skip 3 MoCap markers frames out of 4 from being ouptut (but ALL of them are recorded !)
				if(skippedMocapFramesCount >= 3) {
					skippedMocapFramesCount = 0;

					if (markerFrame.time > latestMarkersFrameTime) {
						latestMarkersFrameTime = markerFrame.time;
						onMarkersFrameOutput(markerFrame);
					}
				}
				else
					skippedMocapFramesCount++;
			}

			if(isRecording) {
				sequence->addMarkerFrame(markerFrame);
				return true;
			}
			else if(error != NULL) {
				throw error;
			}
			else
				return false;
		}

		void SequenceRecording::startRecording() {
			startRecordingTime = -1;
			isRecording = true;

			for(int i = 0; i < writingThreadsNumberPerBuffer; i++) {
				std::thread* oneImageBufferWritingThread = new std::thread(&SequenceRecording::imageBufferWritingThreadLoop, this);
				imageBufferWritingThreads.push_back(oneImageBufferWritingThread);

				std::thread* oneInfraredBufferWritingThread = new std::thread(&SequenceRecording::infraredBufferWritingThreadLoop, this);
				infraredBufferWritingThreads.push_back(oneInfraredBufferWritingThread);

				std::thread* oneDepthBufferWritingThread = new std::thread(&SequenceRecording::depthBufferWritingThreadLoop, this);
				depthBufferWritingThreads.push_back(oneDepthBufferWritingThread);
			}
		}

		/**
		 * @throws KinectCalibrationFileExportException
		 */
		void SequenceRecording::stop() {
			isRecording = false;
			sequence->writeCalibrationData();
			sequence->writeMarkersData(); //@TODO : write markers data in a separate thread in order to avoid slowing call to stop()
		}

		/**
		 * @throws FileWritingException
		 */
		void SequenceRecording::imageBufferWritingThreadLoop() {
			imageBuffer_mutex.lock();

			while(isRecording || !imageBuffer.empty()) {
				imageBuffer_mutex.unlock();

				if(imageBuffer_mutex.try_lock()) {
					if (!imageBuffer.empty()) {
						kocca::datalib::TimeCodedFrame tcFrame = imageBuffer.front();
						imageBuffer.pop_front();
						imageBuffer_mutex.unlock();

						std::ostringstream imageFileName;
						imageFileName << tcFrame.time << ".jpeg";

						boost::filesystem::path imageFramePath = sequence->getRootDirectory() / "image" / imageFileName.str();

						cv::cvtColor(tcFrame.frame, tcFrame.frame, CV_BGRA2RGB);

						if (cv::imwrite(imageFramePath.string(), tcFrame.frame, imageFormatParams)) {
							sequence_mutex.lock();
							sequence->addImageFrame(imageFramePath);
							sequence_mutex.unlock();
						}
						else
							throw FileWritingException("Failed to write an image frame");
					}
					else
						imageBuffer_mutex.unlock();
				}
		
				if(isRecording)
					usleep(10);

				imageBuffer_mutex.lock();
			}

			imageBuffer_mutex.unlock();
		}

		/**
		* @throws FileWritingException
		*/
		void SequenceRecording::infraredBufferWritingThreadLoop() {
			infraredBuffer_mutex.lock();

			while (isRecording || !infraredBuffer.empty()) {
				infraredBuffer_mutex.unlock();

				if (infraredBuffer_mutex.try_lock()) {
					if (!infraredBuffer.empty()) {
						kocca::datalib::TimeCodedFrame tcFrame = infraredBuffer.front();
						infraredBuffer.pop_front();
						infraredBuffer_mutex.unlock();

						std::ostringstream infraredFileName;
						infraredFileName << tcFrame.time << ".png";

						boost::filesystem::path infraredFramePath = sequence->getRootDirectory() / "infrared" / infraredFileName.str();

						if (cv::imwrite(infraredFramePath.string(), tcFrame.frame, infraredFormatParams)) {
							sequence_mutex.lock();
							sequence->addIRFrame(infraredFramePath);
							sequence_mutex.unlock();
						}
						else
							throw FileWritingException("Failed to write an infrared frame");
					}
					else
						infraredBuffer_mutex.unlock();
				}

				if (isRecording)
					usleep(10);

				infraredBuffer_mutex.lock();
			}

			infraredBuffer_mutex.unlock();
		}

		void SequenceRecording::depthBufferWritingThreadLoop() {
			depthBuffer_mutex.lock();

			while(isRecording || !depthBuffer.empty()) {
				depthBuffer_mutex.unlock();

				if(depthBuffer_mutex.try_lock()) {
					if (!depthBuffer.empty()) {
						cv::Mat frame = depthBuffer.front().frame;
						unsigned long long frameTime = depthBuffer.front().time;
						depthBuffer.pop_front();
						depthBuffer_mutex.unlock();
						std::ostringstream depthFileName;
						depthFileName << frameTime << ".png";

						boost::filesystem::path depthFramePath = sequence->getRootDirectory() / "depth" / depthFileName.str();

						if (cv::imwrite(depthFramePath.string(), frame, depthFormatParams)) {
							sequence_mutex.lock();
							sequence->addDepthFrame(depthFramePath);
							sequence_mutex.unlock();
						}
						else
							throw FileWritingException("Failed to write a depth frame");
					}
					else
						depthBuffer_mutex.unlock();
				}
		
				if(isRecording)
					usleep(10);

				depthBuffer_mutex.lock();
			}

			depthBuffer_mutex.unlock();
		}

		unsigned long long SequenceRecording::getRelativeTime(unsigned long long time) {
			startRecordingTime_mutex.lock();

			if(startRecordingTime == -1)
				startRecordingTime = time;

			startRecordingTime_mutex.unlock();

			return(time - startRecordingTime);
		}

		void SequenceRecording::setError(std::runtime_error re) {
			try {
				stop();
			}
			catch(std::exception& e) {
				// we do nothing, we keep re as the exception that will be considered the source of problem
			}

			error = new std::runtime_error(re);
		}
	} // namespace operations
} // namespace kocca
