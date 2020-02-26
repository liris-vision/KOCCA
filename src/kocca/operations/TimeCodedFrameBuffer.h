#ifndef KOCCA_TIME_CODED_FRAME_BUFFER_H
#define KOCCA_TIME_CODED_FRAME_BUFFER_H

#include "../datalib/TimeCodedFrame.h"
#include <deque>

namespace kocca {

	/**
	 * A buffer to store time coded frames and access/manage them easily
	 */
	class TimeCodedFrameBuffer: public std::deque<datalib::TimeCodedFrame> {
	public:

		/**
		 * Get the frame that should be visible at the time specified as a parameter.
		 * @param time the time for wich we want the corresponding frame, in milliseconds
		 * @return the frame visible at time
		 * @throws OutOfSequenceException
		 */
		datalib::TimeCodedFrame getFrameAtTime(unsigned long long time);

		/**
		 * Delete all frames whose time is before the one specified as a parameter.
		 * @param time the time before when we want to purge the buffer, in milliseconds 
		 */
		void purgeBefore(long long time);

		/**
		 * Gets the time (in milliseconds) associated with the FIRST frame stored in the buffer, of 0 if there isn't any.
		 */
		unsigned long long getStartingPoint();

		/**
		 * Gets the time (in milliseconds) associated with the LAST frame stored in the buffer, of 0 if there isn't any.
		 */
		unsigned long long getEndingPoint();
	};
} // namespace kocca

#endif // KOCCA_TIME_CODED_FRAME_BUFFER_H
