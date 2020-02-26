#include "TimeCodedFrameBuffer.h"
#include "../Exceptions.h"

namespace kocca {
	/**
	 * @throws FrameNotInBufferException
	 */
	datalib::TimeCodedFrame TimeCodedFrameBuffer::getFrameAtTime(unsigned long long time) {
		datalib::TimeCodedFrame* pFrame = NULL;
	
		if(size() > 1) {
			for(int i = 1; (pFrame == NULL) && (i < size()); i++)
				if(at(i).time > time)
					pFrame = &(at(i-1));
		}
		else
			if((size() == 1) && (at(0).time <= time))
				pFrame = &(at(0));

		if(pFrame != NULL)
			return(*pFrame);
		else
			throw FrameNotInBufferException("No frame available in the buffer at requested time");
	}

	void TimeCodedFrameBuffer::purgeBefore(long long time) {
		while((size() >= 2) && (at(1).time < time))
			pop_front();
	}

	unsigned long long TimeCodedFrameBuffer::getStartingPoint() {
		if(size() > 0)
			return(at(0).time);
		else
			return(0);
	}

	unsigned long long TimeCodedFrameBuffer::getEndingPoint() {
		if(size() > 0)
			return(at(size() - 1).time);
		else
			return(0);
	}
} // namespace kocca