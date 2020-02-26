#ifndef KOCCA_DATALIB_TASK_PROGRESS_H
#define KOCCA_DATALIB_TASK_PROGRESS_H

#include <thread>
#include <atomic> 

namespace kocca {
	namespace datalib {

		/**
		 * A class that allows to track the progress of a long operation between several objects and emits notifications through callback functions.
		 */
		class TaskProgress {
		protected:

			/**
			 * The amount of progress, in percentage.
			 */
			std::atomic<float> progress;

		public:

			/**
			 * Constructor.
			 */
			TaskProgress();

			/**
			 * Sets a new value for progress.
			 */
			void setProgress(float _newProgress);

			/**
			 * Gets the current value of progress.
			 */
			float getProgress();

			/**
			 * Increments progress of the amount given as an argument.
			 * @param _increment the amount to add to progress.
			 */
			float incrementProgress(float _increment);

			/**
			 * Callback function called when the value of progress changes, to notify other objects of this new value.
			 * @param float the new value that was set for progress.
			 */
			void (*onSetProgress)(float);

			/**
			 *
			 */
			void onSetProgressThread();

		};
	} // namespace datalib
} // namespace kocca;

#endif // KOCCA_DATALIB_TASK_PROGRESS_H
