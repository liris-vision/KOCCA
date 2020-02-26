#include "FramePath.h"
#include <sstream>

namespace kocca {
	namespace datalib {
		FramePath::FramePath() {
			path = std::string("");
			time = 0;
		}

		FramePath::FramePath(boost::filesystem::path boostPath) {
			path = boostPath.string();
			std::stringstream(boostPath.stem().string()) >> time;
		}

		bool FramePath::compareFramePaths(FramePath framePathA, FramePath framePathB) {
			return (framePathA.time < framePathB.time);
		}
	} // namespace datalib
} // namespace kocca