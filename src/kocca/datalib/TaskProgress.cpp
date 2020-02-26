#include "TaskProgress.h"

namespace kocca {
	namespace datalib {
		
		void TaskProgress::onSetProgressThread() {
			if(onSetProgress != NULL)
				onSetProgress(progress);
		}

		TaskProgress::TaskProgress() {
			progress = 0.0;
			onSetProgress = NULL;
		}

		void TaskProgress::setProgress(float _newProgress) {
			progress = _newProgress;
			
			if(onSetProgress != NULL)
				new std::thread(&TaskProgress::onSetProgressThread, this);
		}

		float TaskProgress::getProgress() {
			return progress;
		}

		float TaskProgress::incrementProgress(float _increment) {
			float newProgress = progress + _increment;
			setProgress(newProgress);
			return newProgress;
		}
	} // namespace datalib
} // namespace kocca