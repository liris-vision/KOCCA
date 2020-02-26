#ifndef KOCCA_DATALIB_MOCAP_MARKER_H
#define KOCCA_DATALIB_MOCAP_MARKER_H

#include <opencv2/opencv.hpp>

namespace kocca {
	namespace datalib {

		/**
		 * Represents one MoCap marker position with it's coordinates
		 */
		class MocapMarker {
		public:

			/**
			 * The marker name
			 */
			std::string name;

			/**
			 * The marker 3D coordinates
			 */
			cv::Point3d coords;

			/**
			 * Constructor
			 * @param x_ the X coordinate value for the new marker
			 * @param y_ the Y coordinate value for the new marker
			 * @param z_ the Z coordinate value for the new marker
			 * @param name_ the marker name for the new marker
			 */
			MocapMarker(double x_, double y_, double z_, std::string name_);
		};
	} // namespace datalib
} // namespace kocca

#endif // KOCCA_DATALIB_MOCAP_MARKER_H
