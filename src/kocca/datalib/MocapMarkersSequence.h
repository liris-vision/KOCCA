#ifndef KOCCA_DATALIB_MOCAP_MARKERS_SEQUENCE_H
#define KOCCA_DATALIB_MOCAP_MARKERS_SEQUENCE_H

#include "MocapMarkerFrame.h"
#include <vector>
#include "TaskProgress.h"

namespace kocca {
	namespace datalib {

		/**
		 * All the MoCap data of one KOCCA sequence.
		 */
		class MocapMarkersSequence {
		public:

			/**
			 * Adds a MocapMarkerFrame to the sequence
			 * @param frame the MocapMarkerFrame to add
			 */
			void addFrame(MocapMarkerFrame frame);

			/**
			 * Gets the names of all the MocapMarkers found in all the sequence
			 */
			std::vector<std::string> getAllMarkerNames();

			/**
			 * Gets the rank of the frame by it's time within the sequence
			 * @param time the time for wich we want the frame.
			 * @return the rank as an int, or -1 if no frame correspond to that time
			 */
			int getFrameRankAtTime(unsigned long long time);

			/**
			 * Gets the time of the next frame that will occur just after the time in argument. Useful for frame-by-frame navigation.
			 * @param time the time for wich we want the next sequence time after that
			 * @return the time of the next sequence found after the time argument
			 * @throws EmptySequenceStreamException if the current MocapMarkersSequence doesn't have any MocapMarkerFrame
			 */
			int getNextFrameTime(unsigned long long time);

			/**
			 * Gets the time of the previous frame that occured just before the time in argument. Useful for frame-by-frame navigation.
			 * @param time the time for wich we want the previous sequence time before that
			 * @return the time of the previous sequence found before the time argument
			 * @throws EmptySequenceStreamException if the current MocapMarkersSequence doesn't have any MocapMarkerFrame
			 */
			int getPreviousFrameTime(unsigned long long time);

			/**
			 * Gets the MocapMarkerFrame that should be displayed at the time given in argument
			 * @param time the time for wich we want the frame
			 * @return the frame that should be displayed at time
			 * @throws OutOfSequenceException if no frame has been found for that time
			 */
			MocapMarkerFrame getFrameAtTime(unsigned long long time);

			/**
			 * Gets the "rank-th" MocapMarkerFrame from the sequence
			 * @param rank the rank of the desired frame
			 * @return the frame at rank
			 * @throws std::out_of_range if no frame has been found for that rank
			 */
			MocapMarkerFrame getFrameAtRank(int rank);

			/**
			 * Parses a .CSV file complying to the KOCCA MoCap data file format, and loads it's data in the current MocapMarkerSequence object
			 * @param filePath the full path of the file on the filesystem
			 * @param taskProgress a TaskProgress* pointer, allowing to monitor the progress of long threaded operations
			 * @param progressIncrement the amount of progress (in percentage) we must add at the end of parsing
			 * @throws FileReadingException if the file couldn't have been read
			 * @throws InvalidMocapDataFileException if the file does not comply to the KOCCA MoCap data file format
			 */
			void readFromFile(const char* filePath, TaskProgress* taskProgress = NULL, float progressIncrement = 100.0);

			/**
			 * Gest all the current MocapMarkerSequence object data encoded in the "comma-seperated values" format used by KOCCA
			 */
			std::string getCSVContent();

			/**
			 * Exports all the current MocapMarkerSequence object data as a .CSV file
			 * @param filePath the path on the filesystem to export the data to
			 */
			void writeToFile(const char* filePath);

			/**
			 * Checks if the current MocapMarkerSequence object has at least one MocapMarkerFrame containing at least one MocapMarker
			 * @return true if the sequence has data, false otherwise
			 */
			bool hasData();

			/**
			 * The vector where all the sequence's MocapMarkerFrame are stored
			 */
			std::vector<MocapMarkerFrame> markersData;
		};
	} // namespace datalib
} // namespace kocca

#endif // KOCCA_DATALIB_MOCAP_MARKERS_SEQUENCE_H
