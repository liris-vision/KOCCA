#include "CvDrawingArea.h"

#include "../utils.h"
#include "../Exceptions.h"

#include "math.h"

namespace kocca {
	namespace widgets {
		CvDrawingArea::CvDrawingArea(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): Gtk::DrawingArea(cobject) {
			drawEventDispatcher.connect(sigc::mem_fun(this, &CvDrawingArea::threadSafeQueueDraw));
			drawEventDispatcher.emit();
		}

		int CvDrawingArea::getDrawingImageOffsetX(cv::Mat frame) {
			// first we calculate widget's width/height ratio
			int widgetWidth = get_width();
			int widgetHeight = get_height();
			double widgetRatio = (double)widgetWidth / (double)widgetHeight;

			// then we calculate frame's width/height ratio
			int originalFrameWidth = frame.cols;
			int originalFrameHeight = frame.rows;
			double originalFrameRatio = (double)originalFrameWidth / (double)originalFrameHeight;

			int resizedFrameWidth, resizedFrameHeight, offsetX;
		
			if (widgetRatio > originalFrameRatio) // widget ratio is wider than frame => we'll have black stripes at left and right
			{
				resizedFrameHeight = widgetHeight;
				resizedFrameWidth = resizedFrameHeight * originalFrameRatio;
				offsetX = (widgetWidth - resizedFrameWidth) / 2; // vertical black stripes width
			}
			else // widget ratio is higher than frame => we'll have black stripes at top and bottom
				offsetX = 0;

			return offsetX;
		}

		int CvDrawingArea::getDrawingImageOffsetY(cv::Mat frame) {
			// first we calculate widget's width/height ratio
			int widgetWidth = get_allocated_width();
			int widgetHeight = get_allocated_height();
			double widgetRatio = (double)widgetWidth / (double)widgetHeight;

			// then we calculate frame's width/height ratio
			int originalFrameWidth = frame.cols;
			int originalFrameHeight = frame.rows;
			double originalFrameRatio = (double)originalFrameWidth / (double)originalFrameHeight;

			int resizedFrameWidth, resizedFrameHeight, offsetX, offsetY;
		
			if (widgetRatio < originalFrameRatio) // widget ratio is higher than frame => we'll have black stripes at top and bottom
			{
				resizedFrameWidth = widgetWidth;
				resizedFrameHeight = resizedFrameWidth / originalFrameRatio;
				offsetY = (widgetHeight - resizedFrameHeight) / 2; // horizontal black stripes height
			}
			else
				offsetY = 0;

			return offsetY;
		}

		double CvDrawingArea::getDrawingImageResizeRatio(cv::Mat frame) {
			// first we calculate widget's width/height ratio
			int widgetWidth = get_allocated_width();
			int widgetHeight = get_allocated_height();
			double widgetRatio = (double)widgetWidth / (double)widgetHeight;

			// then we calculate frame's width/height ratio
			int originalFrameWidth = frame.cols;
			int originalFrameHeight = frame.rows;
			double originalFrameRatio = (double)originalFrameWidth / (double)originalFrameHeight;

			int resizedFrameWidth, resizedFrameHeight;
		
			if (widgetRatio > originalFrameRatio) // widget ratio is wider than frame => we'll have black stripes at left and right
				resizedFrameWidth = widgetHeight * originalFrameRatio;
			else // widget ratio is higher than frame => we'll have black stripes at top and bottom
				resizedFrameWidth = widgetWidth;

			double resizeRatio = (double)resizedFrameWidth / (double)originalFrameWidth;
			return resizeRatio;
		}

		cv::Mat CvDrawingArea::getFrameResizedToFit(cv::Mat frame) {
			if ((frame.rows != get_allocated_height()) || (frame.cols != get_allocated_width())) {
				int offsetX = getDrawingImageOffsetX(frame);
				int offsetY = getDrawingImageOffsetY(frame);
				double resizeRatio = getDrawingImageResizeRatio(frame);
				int resizedFrameWidth = frame.cols * resizeRatio;
				int resizedFrameHeight = frame.rows * resizeRatio;

				// we resize the frame to it's ideal size
				cv::Mat resizedFrame;
				cv::resize(frame, resizedFrame, cv::Size(resizedFrameWidth, resizedFrameHeight));

				// finally, we copy the resized frame on a 0-filled image that is the size of the widget
				cv::Mat drawImage = cv::Mat::zeros(get_allocated_height(), get_allocated_width(), CV_8UC3);
				resizedFrame.copyTo(drawImage(cv::Rect(offsetX, offsetY, resizedFrameWidth, resizedFrameHeight)));
				return drawImage;
			}
			else
				return frame;
		}

		cv::Mat CvDrawingArea::get8BitsRGBFormattedFrame(cv::Mat frame) {
			int depth;

			switch (frame.depth()) {
				case CV_8U:
					depth = 8;
					break;
				case CV_8S:
					depth = 8;
					break;
				case CV_16U:
					depth = 16;
					break;
				case CV_16S:
					depth = 16;
					break;
				case CV_32S:
					depth = 32;
					break;
				case CV_32F:
					depth = 32;
					break;
				case CV_64F:
					depth = 64;
					break;
			}
				
			if (depth != 8)
				frame.convertTo(frame, CV_8U, pow(2, 8) / pow(2, depth));

			if (frame.channels() == 1)
				cv::cvtColor(frame, frame, CV_GRAY2RGB);
			
			return frame;
		}

		cv::Mat CvDrawingArea::getDrawImage() {
			cv::Mat formattedImage;

			// lock nextFrame while we're working on it
			nextFrame_mutex.lock();
	
			// if there is an available frame to display
			if ((nextFrame.data))
				formattedImage = get8BitsRGBFormattedFrame(nextFrame);
			else {
				cv::Mat textImage = cv::Mat::zeros(get_allocated_height(),get_allocated_width(),CV_8UC3);
				cv::putText(textImage, "Waiting for image data input ...", cv::Point((int)((get_width() - 200) /2),(int)((get_height() - 10) /2)),cv::FONT_HERSHEY_SIMPLEX,0.4,cv::Scalar(255,255,255),1);
				formattedImage = textImage;
			}

			nextFrame_mutex.unlock();

			return getFrameResizedToFit(formattedImage);
		}

		bool CvDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr) {
			cv::Mat drawImage = getDrawImage();

			if(drawImage.data) {
				if ((drawImage.cols == get_allocated_width()) && (drawImage.rows == get_allocated_height())) {
					Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_data(drawImage.data,
						Gdk::COLORSPACE_RGB, false, 8, drawImage.cols,
						drawImage.rows, drawImage.step);

					if ((cr) && (pixbuf)) {
						Gdk::Cairo::set_source_pixbuf(cr, pixbuf, 0, 0);
						cr->paint();
					}
				}
			}

			return false;
		}

		void CvDrawingArea::threadSafeQueueDraw() {
			if (queueDraw_mutex.try_lock()) {
				queue_draw();
				queueDraw_mutex.unlock();
			}
		}

		void CvDrawingArea::setNextFrame(cv::Mat frame) {
			if(frame.data != NULL) {
				if(nextFrame_mutex.try_lock()) {
					if(nextFrame.data)
						nextFrame.release();

					nextFrame = frame;
					nextFrame_mutex.unlock();
					drawEventDispatcher.emit();
				}
			}
			else
				throw EmptyFrameException("Call to CvDrawingArea::setNextFrame with NULL frame.data");
		}

		void CvDrawingArea::invalidateLastFrame() {
			nextFrame_mutex.lock();

			if(nextFrame.data)
				nextFrame.release();

			nextFrame_mutex.unlock();
			drawEventDispatcher.emit();
		}
	} // namespace widgets
} // namespace kocca
