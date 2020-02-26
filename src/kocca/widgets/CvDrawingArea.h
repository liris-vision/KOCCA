#ifndef KOCCA_WIDGETS_CV_DRAWING_AREA_H
#define	KOCCA_WIDGETS_CV_DRAWING_AREA_H

#include <gtkmm.h>
#include <gtkmm/drawingarea.h>

#include <opencv2/opencv.hpp>

#include <mutex>
#include <atomic>

namespace kocca {
	namespace widgets {

		/**
		 * A Gtk compatible widget that can display openCV's "cv:Mat" images.
		 * It is thread-safe and behaves well at high frame rates, so it can be used to display videos.
		 */
		class CvDrawingArea: public Gtk::DrawingArea {
		public:
			/**
			 * Widget constructor
			 * @see Gtk and libglade documentation
			 */
			CvDrawingArea(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

			/**
			 * Sets the next frame to be displayed.
			 * @param frame the next frame to be displayed.
			 */
			void setNextFrame(cv::Mat frame);

			/**
			 * Removes the currently displayed frame. After calling this function, if no "next frame" is set before the widget refreshes its display, it will show a black area with the text message "Waiting for image data input ...".
			 */
			void invalidateLastFrame();

		protected:

			/**
			 * Next frame to be displayed when updateDisplay() is called
			 */
			cv::Mat nextFrame;

			/**
			 * Mutex to lock nextFrame
			 */
			std::mutex nextFrame_mutex;

			/**
			 * Mutex to thread-safely call queue_draw()
			 */
			std::mutex queueDraw_mutex;

			/**
			 * When "emitted", this dispatcher will schedule a call to threadSafeQueueDraw() in the Gtk main loop.
			 */
			Glib::Dispatcher drawEventDispatcher;

			/**
			 * Gets the horizontal offset to draw the given image frame centered within the widget area.
			 * @param frame the image we want to center
			 */
			int getDrawingImageOffsetX(cv::Mat frame);

			/**
			 * Gets the vertical offset to draw a given image frame centered within the widget area.
			 * @param frame the image we want to center
			 */
			int getDrawingImageOffsetY(cv::Mat frame);

			/**
			 * Gets the ratio at wich a given image frame must be resized to fit in the widget area.
			 * @param frame the image we want to fit
			 */
			double getDrawingImageResizeRatio(cv::Mat frame);

			/**
			 * Resize a given image frame so its dimensions fit in the widget area
			 * @param frame the image we want to fit
			 * @return the resized image
			 */
			cv::Mat getFrameResizedToFit(cv::Mat frame);

			/**
			 * Formats a cv:Mat data so it corresponds to the format expected by Gdk::Pixbuf::create_from_data()
			 * @param frame the image to be formatted
			 * @return the formatted image
			 */
			static cv::Mat get8BitsRGBFormattedFrame(cv::Mat frame);

			/**
			 * Gets the image to be drawn in the widget area : either a copy of nextFrame formatted and resized to fit in the widget area, or a black image with the text message "Waiting for image data input ..." if nextFrame is not defined.
			 * @return the image to be drawn in the widget area
			 */
			virtual cv::Mat getDrawImage();

			/**
			 * Draws the widget content
			 * @param cr ??? Unclear : check Gtk documentation
			 * @return always false
			 */
			bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

			/**
			 * Thread-safley calls queue_draw()
			 */
			void threadSafeQueueDraw();
		};
	} // namespace widgets
} // namespace kocca

#endif // KOCCA_WIDGETS_CV_DRAWING_AREA_H
