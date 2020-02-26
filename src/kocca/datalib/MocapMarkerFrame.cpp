#include "MocapMarkerFrame.h"
#include <string>
#include "../Exceptions.h"

namespace kocca {
	namespace datalib {
		bool MocapMarkerFrame::compareFrameTimes(MocapMarkerFrame frameA, MocapMarkerFrame frameB) {
			return (frameA.time < frameB.time);
		}

		MocapMarkerFrame::MocapMarkerFrame(long long time_) {
			time = time_;
		}

		/**
		 * @throws DuplicateMarkerNameException
		 */
		void MocapMarkerFrame::add_marker(MocapMarker marker_) {
			if(getMarkerByName(marker_.name) == NULL)
				markers.push_back(marker_);
			else
				throw DuplicateMarkerNameException("Frame already has a marker with this name");
		}

		MocapMarker* MocapMarkerFrame::getMarkerByName(std::string name) {
			MocapMarker* marker = NULL;

			for(int i = 0; (i < markers.size()) && (marker == NULL); i++)
				if(name == markers.at(i).name)
					marker = &(markers.at(i));

			return(marker);
		}

		bool MocapMarkerFrame::hasMarkerWithName(std::string name) {
			bool markerFound = false;

			for(int i = 0; (i < markers.size()) && !markerFound; i++)
				markerFound = (name == markers.at(i).name);

			return(markerFound);
		}

		MocapMarker* MocapMarkerFrame::getMarkerByRank(int rank) {
			MocapMarker* marker = NULL;

			if(rank < markers.size())
				marker = &(markers.at(rank));

			return(marker);
		}

		int MocapMarkerFrame::getMarkersCount() {
			return markers.size();
		}

		std::vector<std::string> MocapMarkerFrame::getMarkerNames() {
			std::vector<std::string> names;

			for(int i = 0; i < markers.size(); i++)
				names.push_back(markers.at(i).name);

			return names;
		}

		std::vector<cv::Point3d> MocapMarkerFrame::getCvPoint3dVector() {
			std::vector<cv::Point3d> pointsVector;

			for(int i = 0; i < markers.size(); i++) {
				MocapMarker marker = markers.at(i);
				cv::Point3d point(marker.coords.x, marker.coords.y, marker.coords.z);
				pointsVector.push_back(point);
			}

			return pointsVector;
		}
	} // namespace datalib
} // namespace kocca