#ifndef KOCCA_DATALIB_SEQUENCE_H
#define KOCCA_DATALIB_SEQUENCE_H

#include <vector>

#include "boost/filesystem.hpp"

#include "TimeCodedFrame.h"
#include "FramePath.h"
#include "MocapMarkerFrame.h"
#include "MocapMarkersSequence.h"
#include "ExtrinsicCalibrationParametersSet.h"
#include "IntrinsicCalibrationParametersSet.h"
#include "KinectCalibrationFile.h"
#include "TaskProgress.h"

namespace kocca {
	namespace datalib {

		/**
		 * Modelises a Kocca sequence and allows to access it's data
		 * A Kocca sequence can have three image streams (image, infrared and depth), a NatNet markers stream and a kinect calibration file
		 * Each of the three image streams consists of a series of timecoded frames (= individual images associated to a timestamp)
		 * All this data is stored in a folder with a fixed predefined structure
		 */
		class Sequence {
		public:

			/**
			 * Contains all of the sequence's makers data
			 */
			MocapMarkersSequence markersSequence;

			/**
			 * Constructor.
			 */
			Sequence();

			/**
			 * Sets the sequence's root directory, where sequence data can be read or written.
			 * @param _directory the path to the directory as a C string
			 */
			void setRootDirectory(const char* _directory);

			/**
			 * Sets the sequence's root directory, where sequence data can be read or written.
			 * @param _directory the path to the directory as a boost::filesystem::path
			 */
			void setRootDirectory(boost::filesystem::path _directory);

			/**
			 * Gets the root directory.
			 */
			boost::filesystem::path getRootDirectory();

			/**
			 * Reads the data found in the sequence's root directory, and loads it into the current sequence object.
			 * @param taskProgress a TaskProgress pointer, allowing to track the progress of this operation from other objects.
			 * @throws TempFolderNotAvailableException
			 * @throws InvalidKinectCalibrationFileException
		 	 * @throws CalibrationDataNotAvailableException
		 	 * @throws FileReadingException
		 	 * @throws InvalidMocapDataFileException
		 	 * @throws DuplicateMarkerNameException
			 */
			void readDataFromRootDirectory(TaskProgress* taskProgress = NULL);

			/**
			 * Reads and parses the Kinect calibration file from the sequence's root directory, if there's any, and loads the data into the current sequence.
			 * @param taskProgress a TaskProgress pointer, allowing to track the progress of this operation from other objects.
			 */
			void readCalibrationData(TaskProgress* taskProgress = NULL);

			/**
			 * Reads and parses the MoCap markers data file from the sequence's root directory, if there's any, and loads the data into the current sequence.
			 * @param taskProgress a TaskProgress pointer, allowing to track the progress of this operation from other objects.
			 */
			void parseMarkersData(TaskProgress* taskProgress = NULL);

			/**
			 * Lists and sorts the image files that are in the framesDirectory folder, then puts the sorted list in the framesList vector.
			 * @param framesDirectory the directory in which to look for image files.
			 * @param framesList the vector in which to store the sorted list of files.
			 * @param taskProgress a TaskProgress pointer, allowing to track the progress of this operation from other objects.
			 * @param progressIncrement the amount of progress (in percentage) to increment taskProgress of for the operation
			 */
			void indexFrameFiles(boost::filesystem::path framesDirectory, std::vector<FramePath>* framesList, TaskProgress* taskProgress = NULL, float progressIncrement = 40.0);

			/**
			 * Gets the color image frame that should be displayed at a certain time of the sequence (= gets the last frame of the color image stream whose timecode is <= to the one in argument)
			 * @param time the time at wich we want the frame that should be displayed, in milliseconds
			 * @throws EmptySequenceStreamException if the sequence has no color image frame.
			 */
			TimeCodedFrame getImageFrameByTime(unsigned long long time);

			/**
			 * Gets the first frame of the color image stream whose timecode is > to the one in argument (= gets the next frame that should be displayed after a certain time of the sequence)
			 * @param time the time (in milliseconds) after wich we want the next frame that should be displayed
			 * @throws EmptySequenceStreamException if the sequence has no color image frame.
			 */
			TimeCodedFrame getNextImageFrameByTime(unsigned long long time);

			/**
			 * Gets the frame BEFORE the last of the color image stream whose timecode is <= to the one in argument
			 * @param time the time at wich we want the frame before the one that should be displayed, in milliseconds
			 * @throws EmptySequenceStreamException if the sequence has no color image frame.
			 */
			TimeCodedFrame getPreviousImageFrameByTime(unsigned long long time);

			/**
			 * Gets the infrared image frame that should be displayed at a certain time of the sequence (= gets the last frame of the infrared stream whose timecode is <= to the one in argument)
			 * @param time the time at wich we want the frame that should be displayed, in milliseconds
			 * @throws EmptySequenceStreamException if the sequence has no infrared image frame.
			 */
			TimeCodedFrame getIRFrameByTime(unsigned long long time);

			/**
			 * Gets the first frame of the infrared image stream whose timecode is > to the one in argument (= gets the next frame that should be displayed after a certain time of the sequence)
			 * @param time the time (in milliseconds) after wich we want the next frame that should be displayed
			 * @throws EmptySequenceStreamException if the sequence has no infrared image frame.
			 */
			TimeCodedFrame getNextIRFrameByTime(unsigned long long time);

			/**
			 * Gets the frame BEFORE the last of the infrared stream whose timecode is <= to the one in argument
			 * @param time the time at wich we want the frame before the one that should be displayed, in milliseconds
			 * @throws EmptySequenceStreamException if the sequence has no infrared image frame.
			 */
			TimeCodedFrame getPreviousIRFrameByTime(unsigned long long time);

			/**
			 * Gets the depth image frame that should be displayed at a certain time of the sequence (= gets the last frame of the depth stream whose timecode is <= to the one in argument)
			 * @param time the time at wich we want the frame that should be displayed, in milliseconds
			 * @throws EmptySequenceStreamException if the sequence has no depth image frame.
			 */
			TimeCodedFrame getDepthFrameByTime(unsigned long long time);

			/**
			 * Gets the first frame of the depth image stream whose timecode is > to the one in argument (= gets the next frame that should be displayed after a certain time of the sequence)
			 * @param time the time (in milliseconds) after wich we want the next frame that should be displayed
			 * @throws EmptySequenceStreamException if the sequence has no depth image frame.
			 */
			TimeCodedFrame getNextDepthFrameByTime(unsigned long long time);

			/**
			 * Gets the frame BEFORE the last of the depth stream whose timecode is <= to the one in argument
			 * @param time the time at wich we want the frame before the one that should be displayed, in milliseconds
			 * @throws EmptySequenceStreamException if the sequence has no depth image frame.
			 */
			TimeCodedFrame getPreviousDepthFrameByTime(unsigned long long time);

			/**
			 * Adds a timecoded frame to the sequence's image stream.
			 * @param imageFramePath the path of the added frame.
			 */
			void addImageFrame(boost::filesystem::path imageFramePath);

			/**
			 * Adds a timecoded frame to the sequence's infrared stream.
			 * @param irFramePath the path of the added frame.
			 */
			void addIRFrame(boost::filesystem::path irFramePath);

			/**
			 * Adds a timecoded frame to the sequence's depth stream.
			 * @param depthFramePath the path of the added frame.
			 */
			void addDepthFrame(boost::filesystem::path depthFramePath);

			/**
			 * Adds a MoCap marker frame to the sequence.
			 * @param markerFrame the added marker frame
			 */
			void addMarkerFrame(MocapMarkerFrame markerFrame);

			/**
			 * Gets the rank (in the playing order) of a color image frame within the sequence from it's timestamp.
			 * @param time the time of the frame for which we want the rank.
			 * @throws EmptySequenceStreamException if the sequence doesn't have any color image frame
			 */
			int getImageFrameRank(unsigned long long time);

			/**
			 * Gets the rank (in the playing order) of an infrared frame within the sequence from it's timestamp.
			 * @param time the time of the frame for which we want the rank.
			 * @throws EmptySequenceStreamException if the sequence doesn't have any infrared frame
			 */
			int getIRFrameRank(unsigned long long time);

			/**
			 * Get the rank (in the playing order) of a depth frame within the sequence from it's timestamp
			 * @param time the time of the frame for which we want the rank.
			 * @throws EmptySequenceStreamException if the sequence doesn't have any depth frame
			 */
			int getDepthFrameRank(unsigned long long time);

			/**
			 * Gets the total number of color image frames in the sequence.
			 */
			int getImageFramesCount();

			/**
			 * Gets the total number of infrared image frames in the sequence.
			 */
			int getIRFramesCount();

			/**
			 * Gets the total number of depth image frames in the sequence.
			 */
			int getDepthFramesCount();

			/**
			 * Gets the file path of the last frame of the image stream who's timecode is <= to the one specified (= the frame that should currently be displayed at a certain time of the sequence)
			 * @param time the time at wich we want the path of the frame that should be displayed, in milliseconds
			 * @throws EmptySequenceStreamException if the sequence has no color image frame.
			 */
			FramePath getImageFramePathByTime(unsigned long long time);

			/**
			 * Gets the file path of the first frame of the color image stream whose timecode is > to the one in argument (= gets the next frame that should be displayed after a certain time of the sequence)
			 * @param time the time (in milliseconds) after wich we want the path of the next frame that should be displayed
			 * @throws EmptySequenceStreamException if the sequence has no color image frame.
			 */
			FramePath getNextImageFramePathByTime(unsigned long long time);

			/**
			 * Gets the file path of the frame BEFORE the last of the color image stream whose timecode is <= to the one in argument
			 * @param time the time at wich we want the path of the frame before the one that should be displayed, in milliseconds
			 * @throws EmptySequenceStreamException if the sequence has no color image frame.
			 */
			FramePath getPreviousImageFramePathByTime(unsigned long long time);

			/**
			 * Gets the file path of the last frame of the infrared stream who's timecode is <= to the one specified (= the frame that should currently be displayed at a certain time of the sequence)
			 * @param time the time at wich we want the path of the frame that should be displayed, in milliseconds
			 * @throws EmptySequenceStreamException if the sequence has no infrared frame.
			 */
			FramePath getIRFramePathByTime(unsigned long long time);

			/**
			 * Gets the file path of the first frame of the infrared stream whose timecode is > to the one in argument (= gets the next frame that should be displayed after a certain time of the sequence)
			 * @param time the time (in milliseconds) after wich we want the path of the next frame that should be displayed
			 * @throws EmptySequenceStreamException if the sequence has no infrared frame.
			 */
			FramePath getNextIRFramePathByTime(unsigned long long time);

			/**
			 * Gets the file path of the frame BEFORE the last of the infrared stream whose timecode is <= to the one in argument
			 * @param time the time at wich we want the path of the frame before the one that should be displayed, in milliseconds
			 * @throws EmptySequenceStreamException if the sequence has no infrared frame.
			 */
			FramePath getPreviousIRFramePathByTime(unsigned long long time);

			/**
			 * Gets the file path of the last frame of the depth stream who's timecode is <= to the one specified (= the frame that should currently be displayed at a certain time of the sequence)
			 * @param time the time at wich we want the path of the frame that should be displayed, in milliseconds
			 * @throws EmptySequenceStreamException if the sequence has no depth frame.
			 */
			FramePath getDepthFramePathByTime(unsigned long long time);

			/**
			 * Gets the file path of the first frame of the depth stream whose timecode is > to the one in argument (= gets the next frame that should be displayed after a certain time of the sequence)
			 * @param time the time (in milliseconds) after wich we want the path of the next frame that should be displayed
			 * @throws EmptySequenceStreamException if the sequence has no depth frame.
			 */
			FramePath getNextDepthFramePathByTime(unsigned long long time);

			/**
			 * Gets the file path of the frame BEFORE the last of the depth stream whose timecode is <= to the one in argument
			 * @param time the time at wich we want the path of the frame before the one that should be displayed, in milliseconds
			 * @throws EmptySequenceStreamException if the sequence has no depth frame.
			 */
			FramePath getPreviousDepthFramePathByTime(unsigned long long time);

			/**
			 * Gets the file path of the frame of the color image stream at the specified rank
			 * @param rank the rank for which we want the frame's file path
			 * @throws InvalidSequenceRankException if the sequence has no color image frame at this rank.
			 */
			FramePath getImageFramePathByRank(int rank);

			/**
			 * Gets the file path of the frame of the infrared stream at the specified rank
			 * @param rank the rank for which we want the frame's file path
			 * @throws InvalidSequenceRankException if the sequence has no infrared frame at this rank.
			 */
			FramePath getIRFramePathByRank(int rank);

			/**
			 * Gets the file path of the frame of the depth stream at the specified rank
			 * @param rank the rank for which we want the frame's file path
			 * @throws InvalidSequenceRankException if the sequence has no depth frame at this rank.
			 */
			FramePath getDepthFramePathByRank(int rank);

			/**
			 * Get the total sequence duration, in milliseconds
			 */
			unsigned long long getDuration();

			/**
			 * Gets the time of the first frame (wether it's color image, infrared, depth or Mocap markers frame) whose time is after the time in argument.
			 * @param time the time after which we want the first frame time, in milliseconds.
			 * @throws NoNextEventException if the sequence has no frame after time
			 * @return the time of the first frame after time, in milliseconds.
			 */
			unsigned long long getNextEventTime(unsigned long long time);

			/**
			 * Gets the time of the last frame (wether it's color image, infrared, depth or Mocap markers frame) whose time is before the time in argument.
			 * @param time the time before which we want the first frame time, in milliseconds.
			 * @throws NoNextEventException if the sequence has no frame before time
			 * @return the time of the last frame before time, in milliseconds.
			 */
			unsigned long long getPreviousEventTime(unsigned long long time);

			/**
			 * Update the duration information of the sequence, after it's data has been modified
			 */
			void updateDuration();

			/**
			 * Writes all the sequence's Mocap markers frames data in a file named "markersData.csv" in the sequence's root folder.
			 */
			void writeMarkersData();

			/**
			 * Checks if the sequence has any calibration data.
			 * @return true if the sequence has any calibration data, false otherwise.
			 */
			bool hasCalibrationData();

			/**
			 * Checks if the sequence has any recorded data, wether it's color image frames, infrared frames, depth frames or Mocap markers frames.
			 * @return true if the sequence has any recorded data, false otherwise.
			 */
			bool hasRecordedData();

			/**
			 * Gets a pointer to the KinectCalibrationFile object of the sequence.
			 */
			KinectCalibrationFile* getCalibrationFile();

			/**
			 * Writes the sequence's calibration data in a file named "kinect_calibration_parameters.kcf"in the sequence's root folder.
			 */
			void writeCalibrationData();

			/**
			 * Gets the sequence's intrinsic calibration parameters for the infrared sensor.
			 */
			IntrinsicCalibrationParametersSet* getIntrinsicIRCalibrationParameters();

			/**
			 * Sets the sequence's intrinsic calibration parameters for the infrared sensor.
			 * @param params the new sequence's intrinsic calibration parameters for the infrared sensor.
			 */
			void setIntrinsicIRCalibrationParameters(IntrinsicCalibrationParametersSet params);

			/**
			 * Removes the intrinsic calibration parameters for the infrared sensor from the sequence.
			 */
			void removeIntrinsicIRCalibrationParameters();

			/**
			 * Gets the sequence's intrinsic calibration parameters for the RGB sensor.
			 */
			IntrinsicCalibrationParametersSet* getIntrinsicRGBCalibrationParameters();

			/**
			 * Sets the sequence's intrinsic calibration parameters for the RGB sensor.
			 * @param params the new sequence's intrinsic calibration parameters for the RGB sensor.
			 */
			void setIntrinsicRGBCalibrationParameters(IntrinsicCalibrationParametersSet params);

			/**
			 * Removes the intrinsic calibration parameters for the RGB sensor from the sequence.
			 */
			void removeIntrinsicRGBCalibrationParameters();

			/**
			 * Gets the sequence's extrinsic calibration parameters for the infrared sensor.
			 */
			ExtrinsicCalibrationParametersSet* getExtrinsicIRCalibrationParameters();

			/**
			 * Sets the sequence's extrinsic calibration parameters for the infrared sensor.
			 * @param params the new sequence's extrinsic calibration parameters for the infrared sensor.
			 */
			void setExtrinsicIRCalibrationParameters(ExtrinsicCalibrationParametersSet params);

			/**
			 * Removes the extrinsic calibration parameters for the infrared sensor from the sequence.
			 */
			void removeExtrinsicIRCalibrationParameters();

			/**
			 * Gets the sequence's extrinsic calibration parameters for the RGB sensor.
			 */
			ExtrinsicCalibrationParametersSet* getExtrinsicRGBCalibrationParameters();

			/**
			 * Sets the sequence's extrinsic calibration parameters for the RGB sensor.
			 * @param params the new sequence's extrinsic calibration parameters for the RGB sensor.
			 */
			void setExtrinsicRGBCalibrationParameters(ExtrinsicCalibrationParametersSet params);

			/**
			 * Removes the extrinsic calibration parameters for the RGB sensor from the sequence.
			 */
			void removeExtrinsicRGBCalibrationParameters();

			/**
			 * Gets the list of all frames file paths for the color image stream.
			 */
			std::vector<FramePath> getImageFramesList();

			/**
			 * Gets the list of all frames file paths for the infrared stream.
			 */
			std::vector<FramePath> getIRFramesList();

			/**
			 * Gets the list of all frames file paths for the depth stream.
			 */
			std::vector<FramePath> getDepthFramesList();

			/**
			 * Destructor.
			 */
			~Sequence();

		protected:

			/**
			 * Base folder in wich all the sequence's data is stored
			 */
			boost::filesystem::path rootDirectory;

			/**
			 * Index of all the files corresponding to the image stream frames
			 */
			std::vector<FramePath> imageFramesList;

			/**
			 * Index of all the files corresponding to the infrared stream frames
			 */
			std::vector<FramePath> irFramesList;

			/**
			 * Index of all the files corresponding to the depth stream frames
			 */
			std::vector<FramePath> depthFramesList;

			/**
			 * Sequence duration, in milliseconds
			 */
			long long duration;

			/**
			 * Intrinsic calibration parameters for the InfraRed sensor.
			 */
			IntrinsicCalibrationParametersSet* intrinsicIRCalibrationParameters;

			/**
			 * Intrinsic calibration parameters for the RGB sensor.
			 */
			IntrinsicCalibrationParametersSet* intrinsicRGBCalibrationParameters;

			/**
			 * Extrinsic calibration parameters for the InfraRed sensor.
			 */
			ExtrinsicCalibrationParametersSet* extrinsicIRCalibrationParameters;

			/**
			 * Extrinsic calibration parameters for the RGB sensor.
			 */
			ExtrinsicCalibrationParametersSet* extrinsicRGBCalibrationParameters;

			/**
			 * A pointer to the sequence's Kinect calibration file.
			 */
			KinectCalibrationFile* calibrationFile;
		};
	} // namespace datalib
} // namespace kocca

#endif // KOCCA_DATALIB_SEQUENCE_H
