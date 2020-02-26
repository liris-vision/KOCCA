#include "Sequence.h"
#include "../Exceptions.h"
#include <algorithm>

namespace kocca {
	namespace datalib {
		Sequence::Sequence() {
			intrinsicIRCalibrationParameters = NULL;
			intrinsicRGBCalibrationParameters = NULL;
			extrinsicIRCalibrationParameters = NULL;
			extrinsicRGBCalibrationParameters = NULL;
			calibrationFile = new KinectCalibrationFile();
		}

		IntrinsicCalibrationParametersSet* Sequence::getIntrinsicIRCalibrationParameters() {
			return intrinsicIRCalibrationParameters;
		}

		void Sequence::setIntrinsicIRCalibrationParameters(IntrinsicCalibrationParametersSet params) {
			if(intrinsicIRCalibrationParameters != NULL)
				delete intrinsicIRCalibrationParameters;

			intrinsicIRCalibrationParameters = new IntrinsicCalibrationParametersSet(params);
		}

		void Sequence::removeIntrinsicIRCalibrationParameters() {
			if(intrinsicIRCalibrationParameters != NULL) {
				delete intrinsicIRCalibrationParameters;
				intrinsicIRCalibrationParameters = NULL;
			}
		}

		IntrinsicCalibrationParametersSet* Sequence::getIntrinsicRGBCalibrationParameters() {
			return intrinsicRGBCalibrationParameters;
		}

		void Sequence::setIntrinsicRGBCalibrationParameters(IntrinsicCalibrationParametersSet params) {
			if(intrinsicRGBCalibrationParameters != NULL)
				delete intrinsicRGBCalibrationParameters;

			intrinsicRGBCalibrationParameters = new IntrinsicCalibrationParametersSet(params);
		}

		void Sequence::removeIntrinsicRGBCalibrationParameters() {
			if(intrinsicRGBCalibrationParameters != NULL) {
				delete intrinsicRGBCalibrationParameters;
				intrinsicRGBCalibrationParameters = NULL;
			}
		}

		ExtrinsicCalibrationParametersSet* Sequence::getExtrinsicIRCalibrationParameters() {
			return extrinsicIRCalibrationParameters;
		}

		void Sequence::setExtrinsicIRCalibrationParameters(ExtrinsicCalibrationParametersSet params) {
			if(extrinsicIRCalibrationParameters != NULL)
				delete extrinsicIRCalibrationParameters;

			extrinsicIRCalibrationParameters = new ExtrinsicCalibrationParametersSet(params);
		}

		void Sequence::removeExtrinsicIRCalibrationParameters() {
			if(extrinsicIRCalibrationParameters != NULL) {
				delete extrinsicIRCalibrationParameters;
				extrinsicIRCalibrationParameters = NULL;
			}
		}

		ExtrinsicCalibrationParametersSet* Sequence::getExtrinsicRGBCalibrationParameters() {
			return extrinsicRGBCalibrationParameters;
		}

		void Sequence::setExtrinsicRGBCalibrationParameters(ExtrinsicCalibrationParametersSet params) {
			if(extrinsicRGBCalibrationParameters != NULL)
				delete extrinsicRGBCalibrationParameters;

			extrinsicRGBCalibrationParameters = new ExtrinsicCalibrationParametersSet(params);
		}

		void Sequence::removeExtrinsicRGBCalibrationParameters() {
			if(extrinsicRGBCalibrationParameters != NULL) {
				delete extrinsicRGBCalibrationParameters;
				extrinsicRGBCalibrationParameters = NULL;
			}
		}

		void Sequence::readCalibrationData(TaskProgress* taskProgress) {
			boost::filesystem::path calibrationFilePath = rootDirectory / "kinect_calibration_parameters.kcf";

			if(boost::filesystem::exists(calibrationFilePath) && boost::filesystem::is_regular_file(calibrationFilePath)) {
				calibrationFile->loadFromFile(calibrationFilePath.string().c_str());

				if(calibrationFile->hasIntrinsicIRParameters())
					setIntrinsicIRCalibrationParameters(calibrationFile->getIntrinsicIRParameters());

				if(calibrationFile->hasIntrinsicRGBParameters())
					setIntrinsicRGBCalibrationParameters(calibrationFile->getIntrinsicRGBParameters());

				if(calibrationFile->hasExtrinsicIRParameters())
					setExtrinsicIRCalibrationParameters(calibrationFile->getExtrinsicIRParameters());

				if(calibrationFile->hasExtrinsicRGBParameters())
					setExtrinsicRGBCalibrationParameters(calibrationFile->getExtrinsicRGBParameters());
			}

			if(taskProgress != NULL)
				taskProgress->incrementProgress(5);
		}

		void Sequence::parseMarkersData(TaskProgress* taskProgress) {
			boost::filesystem::path markersDataFilePath = rootDirectory / "markersData.csv";

			if(boost::filesystem::exists(markersDataFilePath) && boost::filesystem::is_regular_file(markersDataFilePath))
				markersSequence.readFromFile(markersDataFilePath.string().c_str(), taskProgress, 40);
		}

		void Sequence::indexFrameFiles(boost::filesystem::path framesDirectory, std::vector<FramePath>* framesList, TaskProgress* taskProgress, float progressIncrement) {
			boost::filesystem::directory_iterator end;

			if (boost::filesystem::exists(framesDirectory) && boost::filesystem::is_directory(framesDirectory)) {
				int filesCount = std::count_if(boost::filesystem::directory_iterator(framesDirectory), boost::filesystem::directory_iterator(), static_cast<bool(*)(const boost::filesystem::path&)>(boost::filesystem::is_regular_file));

				for (boost::filesystem::directory_iterator i(framesDirectory); i != end; ++i) {
					if (boost::filesystem::is_regular_file(i->path())) {
						framesList->push_back(i->path());
						std::sort(framesList->begin(), framesList->end(), FramePath::compareFramePaths);
					}

					if (taskProgress != NULL)
						taskProgress->incrementProgress(progressIncrement / filesCount);
				}
			}
		}

		/**
		 * @throws InvalidKinectCalibrationFileException
		 * @throws TempFolderNotAvailableException
		 * @throws CalibrationDataNotAvailableException
		 * @throws FileReadingException
		 * @throws InvalidMocapDataFileException
		 * @throws DuplicateMarkerNameException
		 */
		void Sequence::readDataFromRootDirectory(TaskProgress* taskProgress) {
			if(boost::filesystem::exists(rootDirectory) && boost::filesystem::is_directory(rootDirectory)) {
				// parse markers data
				parseMarkersData(taskProgress);
				taskProgress->incrementProgress(10);
		
				// index images frames
				boost::filesystem::path imageFramesDirectory = rootDirectory / "image";
				indexFrameFiles(imageFramesDirectory, &imageFramesList, taskProgress, 26);

				boost::filesystem::path irFramesDirectory = rootDirectory / "infrared";
				indexFrameFiles(irFramesDirectory, &irFramesList, taskProgress, 26);

				// index depth frames
				boost::filesystem::path depthFramesDirectory = rootDirectory / "depth";
				indexFrameFiles(depthFramesDirectory, &depthFramesList, taskProgress, 26);

				// read calibration data
				readCalibrationData(taskProgress);
				taskProgress->incrementProgress(6);

				// update sequence duration
				updateDuration();
				taskProgress->incrementProgress(6);

				// finally, update progress to 100%
				if(taskProgress != NULL)
					taskProgress->setProgress(100);
			}
			else
				throw TempFolderNotAvailableException("Provided sequence root folder path does not exists");
		}

		void Sequence::setRootDirectory(boost::filesystem::path _directory) {
			rootDirectory = _directory;
		}

		void Sequence::setRootDirectory(const char* _directory) {
			rootDirectory = _directory;
		}

		boost::filesystem::path Sequence::getRootDirectory() {
			return(rootDirectory);
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		int Sequence::getImageFrameRank(unsigned long long time) {
			if(!imageFramesList.empty()) {
				int i = 0;

				while((i < imageFramesList.size()) && (imageFramesList.at(i).time < time))
					i++;

				return(i);
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		* @throws EmptySequenceStreamException
		*/
		int Sequence::getIRFrameRank(unsigned long long time) {
			if (!irFramesList.empty()) {
				int i = 0;

				while ((i < irFramesList.size()) && (irFramesList.at(i).time < time))
					i++;

				return(i);
			}
			else
				throw EmptySequenceStreamException("No infrared frame available");
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		int Sequence::getDepthFrameRank(unsigned long long time) {
			if(!depthFramesList.empty()) {
				int i = 0;

				while((i < depthFramesList.size()) && (depthFramesList.at(i).time < time))
					i++;

				return(i);
			}
			else
				throw EmptySequenceStreamException("No depth frame available");
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		FramePath Sequence::getImageFramePathByTime(unsigned long long time) {
			if(!imageFramesList.empty()) {
				int i = 1;

				while((i < imageFramesList.size()) && (imageFramesList.at(i).time < time))
					i++;

				return(imageFramesList.at(i-1));
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		* @throws EmptySequenceStreamException
		*/
		FramePath Sequence::getIRFramePathByTime(unsigned long long time) {
			if (!irFramesList.empty()) {
				int i = 1;

				while ((i < irFramesList.size()) && (irFramesList.at(i).time < time))
					i++;

				return(irFramesList.at(i - 1));
			}
			else
				throw EmptySequenceStreamException("No infrared frame available");
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		FramePath Sequence::getNextImageFramePathByTime(unsigned long long time) {
			if(!imageFramesList.empty()) {
				int i = 0;

				while((i < imageFramesList.size()) && (imageFramesList.at(i).time <= time))
					i++;

				return(imageFramesList.at(i));
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		* @throws EmptySequenceStreamException
		*/
		FramePath Sequence::getNextIRFramePathByTime(unsigned long long time) {
			if (!irFramesList.empty()) {
				int i = 0;

				while ((i < irFramesList.size()) && (irFramesList.at(i).time <= time))
					i++;

				return(irFramesList.at(i));
			}
			else
				throw EmptySequenceStreamException("No infrared frame available");
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		FramePath Sequence::getPreviousImageFramePathByTime(unsigned long long time) {
			if(!imageFramesList.empty()) {
				int i = 0;

				while((i < (imageFramesList.size() -1)) && (imageFramesList.at(i+1).time < time))
					i++;

				return(imageFramesList.at(i));
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		* @throws EmptySequenceStreamException
		*/
		FramePath Sequence::getPreviousIRFramePathByTime(unsigned long long time) {
			if (!irFramesList.empty()) {
				int i = 0;

				while ((i < (irFramesList.size() - 1)) && (irFramesList.at(i + 1).time < time))
					i++;

				return(irFramesList.at(i));
			}
			else
				throw EmptySequenceStreamException("No infrared frame available");
		}

		FramePath Sequence::getPreviousDepthFramePathByTime(unsigned long long time) {
			if(!depthFramesList.empty()) {
				int i = 0;

				while((i < (depthFramesList.size() -1)) && (depthFramesList.at(i+1).time < time))
					i++;

				return(depthFramesList.at(i));
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		FramePath Sequence::getDepthFramePathByTime(unsigned long long time) {
			if(!depthFramesList.empty()) {
				int i = 1;

				while((i < depthFramesList.size()) && (depthFramesList.at(i).time < time))
					i++;

				return(depthFramesList.at(i-1));
			}
			else
				throw EmptySequenceStreamException("No depth frame available");
		}

		FramePath Sequence::getNextDepthFramePathByTime(unsigned long long time) {
			if(!depthFramesList.empty()) {
				int i = 0;

				while((i < depthFramesList.size()) && (depthFramesList.at(i).time <= time))
					i++;

				return(depthFramesList.at(i));
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		 * @throws InvalidSequenceRankException
		 */
		FramePath Sequence::getImageFramePathByRank(int rank) {
			if(rank < imageFramesList.size())
				return(imageFramesList.at(rank));
			else
				throw InvalidSequenceRankException("No image frame available at this rank");
		}

		/**
		* @throws InvalidSequenceRankException
		*/
		FramePath Sequence::getIRFramePathByRank(int rank) {
			if (rank < irFramesList.size())
				return(irFramesList.at(rank));
			else
				throw InvalidSequenceRankException("No infrared frame available at this rank");
		}

		/**
		 * @throws InvalidSequenceRankException
		 */
		FramePath Sequence::getDepthFramePathByRank(int rank) {
			if(rank < depthFramesList.size())
				return(depthFramesList.at(rank));
			else
				throw InvalidSequenceRankException("No image depth available at this rank");
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		TimeCodedFrame Sequence::getImageFrameByTime(unsigned long long time) {
			if(!imageFramesList.empty()) {
				FramePath tcFramePath = getImageFramePathByTime(time);
				TimeCodedFrame tcFrame;
				tcFrame.time = tcFramePath.time;
				cv::Mat cvFrame = cv::imread(tcFramePath.path, cv::IMREAD_ANYCOLOR);
				cv::cvtColor(cvFrame, cvFrame, CV_BGRA2RGB);
				tcFrame.frame = cvFrame;
				return(tcFrame);
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		* @throws EmptySequenceStreamException
		*/
		TimeCodedFrame Sequence::getIRFrameByTime(unsigned long long time) {
			if (!irFramesList.empty()) {
				FramePath tcFramePath = getIRFramePathByTime(time);
				TimeCodedFrame tcFrame;
				tcFrame.time = tcFramePath.time;
				tcFrame.frame = cv::imread(tcFramePath.path, cv::IMREAD_GRAYSCALE);
				return(tcFrame);
			}
			else
				throw EmptySequenceStreamException("No infrared frame available");
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		TimeCodedFrame Sequence::getNextImageFrameByTime(unsigned long long time) {
			if(!imageFramesList.empty()) {
				FramePath tcFramePath = getNextImageFramePathByTime(time);
				TimeCodedFrame tcFrame;
				tcFrame.time = tcFramePath.time;
				cv::Mat cvFrame = cv::imread(tcFramePath.path, cv::IMREAD_ANYCOLOR);
				cv::cvtColor(cvFrame, cvFrame, CV_BGRA2RGB);
				tcFrame.frame = cvFrame;
				return(tcFrame);
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		* @throws EmptySequenceStreamException
		*/
		TimeCodedFrame Sequence::getNextIRFrameByTime(unsigned long long time) {
			if (!irFramesList.empty()) {
				FramePath tcFramePath = getNextIRFramePathByTime(time);
				TimeCodedFrame tcFrame;
				tcFrame.time = tcFramePath.time;
				tcFrame.frame = cv::imread(tcFramePath.path, cv::IMREAD_GRAYSCALE);
				return(tcFrame);
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		TimeCodedFrame Sequence::getNextDepthFrameByTime(unsigned long long time) {
			if(!imageFramesList.empty()) {
				FramePath tcFramePath = getNextDepthFramePathByTime(time);
				TimeCodedFrame tcFrame;
				tcFrame.time = tcFramePath.time;
				tcFrame.frame = cv::imread(tcFramePath.path, cv::IMREAD_ANYDEPTH);
				return(tcFrame);
			}
			else
				throw EmptySequenceStreamException("No depth frame available");
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		TimeCodedFrame Sequence::getPreviousImageFrameByTime(unsigned long long time) {
			if(!imageFramesList.empty()) {
				FramePath tcFramePath = getPreviousImageFramePathByTime(time);
				TimeCodedFrame tcFrame;
				tcFrame.time = tcFramePath.time;
				cv::Mat cvFrame = cv::imread(tcFramePath.path, cv::IMREAD_ANYCOLOR);
				cv::cvtColor(cvFrame, cvFrame, CV_BGRA2RGB);
				tcFrame.frame = cvFrame;
				return(tcFrame);
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		* @throws EmptySequenceStreamException
		*/
		TimeCodedFrame Sequence::getPreviousIRFrameByTime(unsigned long long time) {
			if (!irFramesList.empty()) {
				FramePath tcFramePath = getPreviousIRFramePathByTime(time);
				TimeCodedFrame tcFrame;
				tcFrame.time = tcFramePath.time;
				tcFrame.frame = cv::imread(tcFramePath.path, cv::IMREAD_GRAYSCALE);
				return(tcFrame);
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		TimeCodedFrame Sequence::getPreviousDepthFrameByTime(unsigned long long time) {
			if(!depthFramesList.empty()) {
				FramePath tcFramePath = getPreviousDepthFramePathByTime(time);
				TimeCodedFrame tcFrame;
				tcFrame.time = tcFramePath.time;
				tcFrame.frame = cv::imread(tcFramePath.path, cv::IMREAD_ANYDEPTH);
				return(tcFrame);
			}
			else
				throw EmptySequenceStreamException("No image frame available");
		}

		/**
		 * @throws EmptySequenceStreamException
		 */
		TimeCodedFrame Sequence::getDepthFrameByTime(unsigned long long time) {
			if(!depthFramesList.empty()) {
				FramePath tcFramePath = getDepthFramePathByTime(time);
				TimeCodedFrame tcFrame;
				tcFrame.time = tcFramePath.time;
				tcFrame.frame = cv::imread(tcFramePath.path, cv::IMREAD_ANYDEPTH);
				return(tcFrame);
			}
			else
				throw EmptySequenceStreamException("No depth frame available");
		}

		void Sequence::addImageFrame(boost::filesystem::path imageFramePath) {
			imageFramesList.push_back(imageFramePath);
		}

		void Sequence::addIRFrame(boost::filesystem::path irFramePath) {
			irFramesList.push_back(irFramePath);
		}

		void Sequence::addDepthFrame(boost::filesystem::path depthFramePath) {
			depthFramesList.push_back(depthFramePath);
		}

		void Sequence::addMarkerFrame(MocapMarkerFrame markerFrame) {
			markersSequence.addFrame(markerFrame);
		}

		bool Sequence::hasCalibrationData() {
			return (
					(intrinsicIRCalibrationParameters != NULL)
				||	(intrinsicRGBCalibrationParameters != NULL)
				||	(extrinsicIRCalibrationParameters != NULL)
				||	(extrinsicRGBCalibrationParameters != NULL));
		}

		bool Sequence::hasRecordedData() {
			return (
					(markersSequence.hasData())
				||	(!imageFramesList.empty())
				||	(!depthFramesList.empty()));
		}

		KinectCalibrationFile* Sequence::getCalibrationFile() {
			if(intrinsicIRCalibrationParameters != NULL)
				calibrationFile->setIntrinsicIRParameters(*intrinsicIRCalibrationParameters);

			if(intrinsicRGBCalibrationParameters != NULL)
				calibrationFile->setIntrinsicRGBParameters(*intrinsicRGBCalibrationParameters);

			if(extrinsicIRCalibrationParameters != NULL)
				calibrationFile->setExtrinsicIRParameters(*extrinsicIRCalibrationParameters);

			if(extrinsicRGBCalibrationParameters != NULL)
				calibrationFile->setExtrinsicRGBParameters(*extrinsicRGBCalibrationParameters);

			return(calibrationFile);
		}

		/**
		 * @throws KinectCalibrationFileExportException
		 */
		void Sequence::writeCalibrationData() {
			if(hasCalibrationData()) {
				boost::filesystem::path calibrationFilePath = rootDirectory / "kinect_calibration_parameters.kcf";
				calibrationFile->saveTo(calibrationFilePath.string().c_str());
			}
		}

		void Sequence::updateDuration() {
			// get the largest timestamp between the 3 vectors
			std::vector<unsigned long long> streamsDurations;

			if(!imageFramesList.empty())
				streamsDurations.push_back(imageFramesList.back().time);

			if (!irFramesList.empty())
				streamsDurations.push_back(irFramesList.back().time);

			if(!depthFramesList.empty())
				streamsDurations.push_back(depthFramesList.back().time);

			if(markersSequence.hasData())
				streamsDurations.push_back(markersSequence.markersData.back().time);

			if (streamsDurations.size() > 0) {
				std::sort(streamsDurations.begin(), streamsDurations.end());
				duration = streamsDurations.back();
			}
			else
				duration = 0;
		}

		unsigned long long Sequence::getDuration() {
			return duration;
		}

		int Sequence::getImageFramesCount() {
			return imageFramesList.size();
		}

		int Sequence::getIRFramesCount() {
			return irFramesList.size();
		}

		int Sequence::getDepthFramesCount() {
			return depthFramesList.size();
		}

		Sequence::~Sequence() {
			if(intrinsicIRCalibrationParameters != NULL)
				delete intrinsicIRCalibrationParameters;

			if(intrinsicRGBCalibrationParameters != NULL)
				delete intrinsicRGBCalibrationParameters;

			if(extrinsicIRCalibrationParameters != NULL)
				delete extrinsicIRCalibrationParameters;

			if(extrinsicRGBCalibrationParameters != NULL)
				delete extrinsicRGBCalibrationParameters;

			delete calibrationFile;
		}

		/**
		 * @throws NoNextEventException
		 */
		unsigned long long Sequence::getNextEventTime(unsigned long long time) {
			std::vector<unsigned long long> eventsTimes;

			if(!imageFramesList.empty()) {
				try {
					eventsTimes.push_back(getNextImageFrameByTime(time).time);
				}
				catch(EmptySequenceStreamException& esse) {} // nothing, we simply don't add an event to the list
			}

			if (!irFramesList.empty()) {
				try {
					eventsTimes.push_back(getNextIRFrameByTime(time).time);
				}
				catch (EmptySequenceStreamException& esse) {} // nothing, we simply don't add an event to the list
			}

			if(!depthFramesList.empty()) {
				try {
					eventsTimes.push_back(getNextDepthFrameByTime(time).time);
				}
				catch(EmptySequenceStreamException& esse) {} // nothing, we simply don't add an event to the list
			}

			if(markersSequence.hasData()) {
				try {
					eventsTimes.push_back(markersSequence.getNextFrameTime(time));
				}
				catch(EmptySequenceStreamException& esse) {} // nothing, we simply don't add an event to the list
			}

			if(!eventsTimes.empty()) {
				std::sort(eventsTimes.begin(), eventsTimes.end());
				return(eventsTimes.front());
			}
			else
				throw NoNextEventException("No next event found");
		}

		/**
		 * @throws NoPreviousEventException
		 */
		unsigned long long Sequence::getPreviousEventTime(unsigned long long time) {
			std::vector<unsigned long long> eventsTimes;

			if(!imageFramesList.empty()) {
				try {
					eventsTimes.push_back(getPreviousImageFrameByTime(time).time);
				}
				catch(EmptySequenceStreamException& esse) {} // nothing, we simply don't add an event to the list
			}

			if (!irFramesList.empty()) {
				try {
					eventsTimes.push_back(getPreviousIRFrameByTime(time).time);
				}
				catch (EmptySequenceStreamException& esse) {} // nothing, we simply don't add an event to the list
			}

			if(!depthFramesList.empty()) {
				try {
					eventsTimes.push_back(getPreviousDepthFrameByTime(time).time);
				}
				catch(EmptySequenceStreamException& esse) {} // nothing, we simply don't add an event to the list
			}

			if(markersSequence.hasData()) {
				try {
					eventsTimes.push_back(markersSequence.getPreviousFrameTime(time));
				}
				catch(EmptySequenceStreamException& esse) {} // nothing, we simply don't add an event to the list
			}

			if(!eventsTimes.empty()) {
				std::sort(eventsTimes.begin(), eventsTimes.end());
				return(eventsTimes.back());
			}
			else
				throw NoPreviousEventException("No previous event found");
		}

		void Sequence::writeMarkersData() {
			if(markersSequence.hasData()) {
				boost::filesystem::path markersDataFile = rootDirectory / "markersData.csv";
				markersSequence.writeToFile(markersDataFile.string().c_str());
			}
		}

		std::vector<FramePath> Sequence::getImageFramesList() {
			return(imageFramesList);
		}

		std::vector<FramePath> Sequence::getIRFramesList() {
			return(irFramesList);
		}

		std::vector<FramePath> Sequence::getDepthFramesList() {
			return(depthFramesList);
		}
	} // namespace datalib
} // namespace kocca
