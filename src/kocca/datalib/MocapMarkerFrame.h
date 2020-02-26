#ifndef KOCCA_DATALIB_MOCAP_MARKER_FRAME_H
#define KOCCA_DATALIB_MOCAP_MARKER_FRAME_H

#include <vector>
#include "MocapMarker.h"

namespace kocca {
	namespace datalib {

		/**
		 * Represents a MoCap markers "frame", wich is the set of all tracked markers with their names and 3D coordinates at a given time
		 */
		class MocapMarkerFrame {
		public:

			/**
			 * The time of the frame, in milliseconds
			 */
			long long time;

			/**
			 * Constructor
			 * @param time_ the time of the new frame
			 */
			MocapMarkerFrame(long long time_);

			/**
			 * Gets a marker from the frame, identified by it's name
			 * @param name the name of the marker we are looking for
			 * @return a pointer to the requested marker, or NULL if no marker with this name has been found in the frame
			 */
			MocapMarker* getMarkerByName(std::string name);

			/**
			 * Checks if the frame contains a marker with the specified name
			 * @param name the name of the marker we are looking for
			 * @return true if the frame contains a marker with the specified name, false otherwise
			 */
			bool hasMarkerWithName(std::string name);

			/**
			 * Gets a marker from the frame, identified by it's rank
			 * @param rank the rank of the marker we are looking for
			 * @return a pointer to the requested marker, or NULL if no marker with this rank has been found in the frame
			 */
			MocapMarker* getMarkerByRank(int rank);

			/**
			 * Adds a marker to the frame
			 * @param marker_ the marker to add
			 * @throws DuplicateMarkerNameException if a marker with the same name is already in the frame
			 */
			void add_marker(MocapMarker marker_);

			/**
			 * Gets the number of markers in the frame
			 */
			int getMarkersCount();

			/**
			 * Gets the names of all markers in the frame
			 */
			std::vector<std::string> getMarkerNames();

			/**
			 * Gets the X,Y,Z coordinates for all markers in the frame
			 */
			std::vector<cv::Point3d> getCvPoint3dVector();

			/**
			 * Compares two frames by their respective times. Useful for sorting.
			 * @param frameA the first frame to compare
			 * @param frameB the second frame to compare
			 * @return true if the time of frameA is strictly inferior to the time of frameB, false otherwise
			 */
			static bool compareFrameTimes(MocapMarkerFrame frameA, MocapMarkerFrame frameB);

		protected:

			/**
			 * All the markers in the frame
			 */
			std::vector<MocapMarker> markers;
		};
	} // namespace datalib
} // namespace kocca

#endif // KOCCA_DATALIB_MOCAP_MARKER_FRAME_H
