#include "KoccaMonitoringWidget.h"
#include "../utils.h"
#include <gdkmm/cursor.h>
#include <gtkmm/window.h>
#include <gdk/gdkkeysyms.h>

namespace kocca {
	namespace widgets {
		KoccaMonitoringWidget::KoccaMonitoringWidget(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): CvDrawingArea(cobject, builder) {
			showRecordingIndications = false;
			nextMocapMarkerFrame = NULL;
			mappedMarkersCoordinates = NULL;
			extrinsicCalibrationParams = NULL;
			intrinsicCalibrationParams = NULL;
			hoveredMarkerId = "";
			multipleSelectionActivated = false;
			add_events(Gdk::BUTTON_PRESS_MASK);
			add_events(Gdk::POINTER_MOTION_MASK);
			signal_button_press_event().connect(sigc::mem_fun(this, &KoccaMonitoringWidget::onClick));
			signal_motion_notify_event().connect(sigc::mem_fun(this, &KoccaMonitoringWidget::onMotion));
			signal_realize().connect(sigc::mem_fun(this, &KoccaMonitoringWidget::onRealize));
			signal_size_allocate().connect(sigc::mem_fun(this, &KoccaMonitoringWidget::onResize));
		}

		void KoccaMonitoringWidget::onResize(Gtk::Allocation& allocation) {
			mappedMarkersCoordinates_mutex.lock();

			if(mappedMarkersCoordinates != NULL) {
				delete mappedMarkersCoordinates;
				mappedMarkersCoordinates = NULL;
			}

			mappedMarkersCoordinates_mutex.unlock();
		}

		void KoccaMonitoringWidget::onRealize() {
			Gtk::Window* parentWindow = dynamic_cast<Gtk::Window *>(get_toplevel());

			if(parentWindow != NULL) {
				parentWindow->add_events(Gdk::KEY_PRESS_MASK);
				parentWindow->signal_key_press_event().connect(sigc::mem_fun(this, &KoccaMonitoringWidget::onKeyPress));
				parentWindow->add_events(Gdk::KEY_RELEASE_MASK);
				parentWindow->signal_key_release_event().connect(sigc::mem_fun(this, &KoccaMonitoringWidget::onKeyRelease));
			}
		}

		bool KoccaMonitoringWidget::onKeyPress(GdkEventKey* event) {
			if((event->keyval == GDK_KEY_Control_L) || (event->keyval == GDK_KEY_Control_L))
				multipleSelectionActivated = true;

			return false;
		}

		bool KoccaMonitoringWidget::onKeyRelease(GdkEventKey* event) {
			if((event->keyval == GDK_KEY_Control_L) || (event->keyval == GDK_KEY_Control_L))
				multipleSelectionActivated = false;

			return false;
		}

		std::vector<cv::Point2d>* KoccaMonitoringWidget::getMappedMarkersCoordinates() {
			if(mappedMarkersCoordinates == NULL) {
				mappedMarkersCoordinates = new std::vector<cv::Point2d>();

				nextMocapMarkerFrame_mutex.lock();

				if ((extrinsicCalibrationParams != NULL) && (intrinsicCalibrationParams != NULL) && (nextMocapMarkerFrame != NULL) && (nextMocapMarkerFrame->getMarkersCount() > 0)) {
					std::vector<cv::Point3d> rawMarkers = nextMocapMarkerFrame->getCvPoint3dVector();
					nextMocapMarkerFrame_mutex.unlock();

					nextFrame_mutex.lock();
					int offsetX = getDrawingImageOffsetX(nextFrame);
					int offsetY = getDrawingImageOffsetY(nextFrame);
					double resizeRatio = getDrawingImageResizeRatio(nextFrame);
					nextFrame_mutex.unlock();

					std::vector<cv::Point2d> projectedMarkers;
					cv::projectPoints(rawMarkers, extrinsicCalibrationParams->getRotationVector(), extrinsicCalibrationParams->getTranslationVector(), intrinsicCalibrationParams->getProjectionMatrix(), intrinsicCalibrationParams->getDistortionVector(), projectedMarkers);

					// for each marker, apply resize ratio, then add offsets(x & y)
					for (int i = 0; i < projectedMarkers.size(); i++) {
						cv::Point2d marker;
						marker.x = (double)offsetX + ((double)projectedMarkers.at(i).x * resizeRatio);
						marker.y = (double)offsetY + ((double)projectedMarkers.at(i).y * resizeRatio);
						mappedMarkersCoordinates->push_back(marker);
					}
				}
				else
					nextMocapMarkerFrame_mutex.unlock();
			}

			return mappedMarkersCoordinates;
		}

		bool KoccaMonitoringWidget::onMotion(GdkEventMotion* event) {
			mappedMarkersCoordinates_mutex.lock();
			std::vector<cv::Point2d> imageMappedMarkers = *(getMappedMarkersCoordinates());
			mappedMarkersCoordinates_mutex.unlock();

			nextMocapMarkerFrame_mutex.lock();

			if ((nextMocapMarkerFrame != NULL) && (imageMappedMarkers.size() > 0)) {
				std::vector<std::string> markersIDs = nextMocapMarkerFrame->getMarkerNames();
				nextMocapMarkerFrame_mutex.unlock();
				bool hoveredMarkerFound = false;
				std::string newlyHoveredMarkerId = "";

				for (int i = (imageMappedMarkers.size() - 1); !hoveredMarkerFound && (i >= 0); i--) {
					if (
						(event->x >= (imageMappedMarkers.at(i).x - 5))
						&& (event->x <= (imageMappedMarkers.at(i).x + 5))
						&& (event->y >= (imageMappedMarkers.at(i).y - 5))
						&& (event->y <= (imageMappedMarkers.at(i).y + 5))) {
						hoveredMarkerFound = true;
						newlyHoveredMarkerId = markersIDs.at(i);
					}
				}

				if (newlyHoveredMarkerId != hoveredMarkerId) {
					hoveredMarkerId = newlyHoveredMarkerId;

					Glib::RefPtr<Gdk::Window> win = get_window();

					if (hoveredMarkerId != "") {
						win->set_cursor(Gdk::Cursor::create(Gdk::CursorType::HAND1));
						set_tooltip_text(hoveredMarkerId);
					}
					else {
						win->set_cursor(Gdk::Cursor::create(Gdk::CursorType::ARROW));
						set_tooltip_text("");
					}

					drawEventDispatcher.emit();
				}
			}
			else
				nextMocapMarkerFrame_mutex.unlock();

			return false;
		}

		bool KoccaMonitoringWidget::onClick(GdkEventButton* event) {
			mappedMarkersCoordinates_mutex.lock();
			std::vector<cv::Point2d> imageMappedMarkers = *(getMappedMarkersCoordinates());
			mappedMarkersCoordinates_mutex.unlock();

			nextMocapMarkerFrame_mutex.lock();

			if ((nextMocapMarkerFrame != NULL) && (imageMappedMarkers.size() > 0)) {
				std::vector<std::string> markersIDs = nextMocapMarkerFrame->getMarkerNames();
				nextMocapMarkerFrame_mutex.unlock();

				if (!multipleSelectionActivated)
					selectedMarkersIDs.clear();

				bool clickedMarkerFound = false;

				for (int i = (imageMappedMarkers.size() - 1); !clickedMarkerFound && (i >= 0); i--) {
					if (
						(event->x >= (imageMappedMarkers.at(i).x - 5))
						&& (event->x <= (imageMappedMarkers.at(i).x + 5))
						&& (event->y >= (imageMappedMarkers.at(i).y - 5))
						&& (event->y <= (imageMappedMarkers.at(i).y + 5))) {
						clickedMarkerFound = true;
						std::string clickedMarkerID = markersIDs.at(i);

						if (multipleSelectionActivated) {
							std::vector<std::string>::iterator it = std::find(selectedMarkersIDs.begin(), selectedMarkersIDs.end(), clickedMarkerID);

							if (it != selectedMarkersIDs.end())
								selectedMarkersIDs.erase(it);
							else
								selectedMarkersIDs.push_back(clickedMarkerID);
						}
						else
							selectedMarkersIDs.push_back(clickedMarkerID);
					}
				}

				drawEventDispatcher.emit();

				if (onSelectedMarkersChanged != NULL)
					onSelectedMarkersChanged(selectedMarkersIDs);
			}
			else
				nextMocapMarkerFrame_mutex.unlock();

			return false;
		}

		KoccaMonitoringWidget::~KoccaMonitoringWidget() {
			nextMocapMarkerFrame_mutex.lock();

			if(nextMocapMarkerFrame != NULL)
				delete nextMocapMarkerFrame;

			nextMocapMarkerFrame_mutex.unlock();

			mappedMarkersCoordinates_mutex.lock();

			if(mappedMarkersCoordinates != NULL)
				delete mappedMarkersCoordinates;

			mappedMarkersCoordinates_mutex.unlock();
		}

		void KoccaMonitoringWidget::setNextMocapMarkerFrame(kocca::datalib::MocapMarkerFrame _nextMocapMarkerFrame) {
			if(nextMocapMarkerFrame_mutex.try_lock()) {
				if(nextMocapMarkerFrame != NULL)
					delete nextMocapMarkerFrame;

				nextMocapMarkerFrame = new kocca::datalib::MocapMarkerFrame(_nextMocapMarkerFrame);

				nextMocapMarkerFrame_mutex.unlock();

				mappedMarkersCoordinates_mutex.lock();

				if(mappedMarkersCoordinates != NULL) {
					delete mappedMarkersCoordinates;
					mappedMarkersCoordinates = NULL;
				}

				mappedMarkersCoordinates_mutex.unlock();

				drawEventDispatcher.emit();
			}
		}

		void  KoccaMonitoringWidget::setSelectedMarkersIds(std::vector<std::string> _selectedMarkersIDs) {
			selectedMarkersIDs = _selectedMarkersIDs;
			drawEventDispatcher.emit();
		}

		bool KoccaMonitoringWidget::markerIDisSelected(std::string markerID) {
			bool found = false;

			for(int i = 0; (i < selectedMarkersIDs.size()) && !found; i++)
				found = (selectedMarkersIDs.at(i) == markerID);

			return(found);
		}

		bool KoccaMonitoringWidget::blinkingRecPointOn() {
			int nbSec = (int)(recordingTime / 1000);
			return(!(nbSec % 2));
		}

		void KoccaMonitoringWidget::incrustRecIndicator(cv::Mat& frame) {
			cv::putText(frame, "REC[ ]", cv::Point(10, 25), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);

			if (blinkingRecPointOn())
				cv::circle(frame, cv::Point(52, 21), 6, cv::Scalar(255, 0, 0), -1);
		}

		void KoccaMonitoringWidget::incrustTimeStamp(cv::Mat& frame) {
			cv::putText(frame, kocca::format_timestamp(recordingTime), cv::Point(frame.cols - 120, 25), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 1);
		}

		cv::Mat KoccaMonitoringWidget::getDrawImage() {
			cv::Mat drawImage = CvDrawingArea::getDrawImage();

			if ((intrinsicCalibrationParams != NULL) && (extrinsicCalibrationParams != NULL)) {
				mappedMarkersCoordinates_mutex.lock();
				std::vector<cv::Point2d> imageMappedMarkers = *(getMappedMarkersCoordinates());
				mappedMarkersCoordinates_mutex.unlock();

				nextMocapMarkerFrame_mutex.lock();

				if ((nextMocapMarkerFrame != NULL) && (imageMappedMarkers.size() > 0)) {
					std::vector<std::string> markersIDs = nextMocapMarkerFrame->getMarkerNames();
					nextMocapMarkerFrame_mutex.unlock();

					for (int i = 0; i < imageMappedMarkers.size(); i++) {
						cv::Scalar color;
						std::string markerID = markersIDs.at(i);

						if (markerIDisSelected(markerID))
							color = cv::Scalar(255, 0, 0);
						else if (markerID == hoveredMarkerId)
							color = cv::Scalar(255, 0, 255);
						else
							color = cv::Scalar(0, 255, 0);

						cv::circle(drawImage, cv::Point(imageMappedMarkers.at(i).x, imageMappedMarkers.at(i).y), 4, color, -1);
					}
				}
				else
					nextMocapMarkerFrame_mutex.unlock();
			}

			if (showRecordingIndications) {
				incrustRecIndicator(drawImage);
				incrustTimeStamp(drawImage);
			}

			return drawImage;
		}

		void KoccaMonitoringWidget::setExtrinsicCalibrationParams(kocca::datalib::ExtrinsicCalibrationParametersSet* _extrinsicCalibrationParams) {
			extrinsicCalibrationParams = _extrinsicCalibrationParams;
			drawEventDispatcher.emit();
		}

		void KoccaMonitoringWidget::setIntrinsicCalibrationParams(kocca::datalib::IntrinsicCalibrationParametersSet* _intrinsicCalibrationParams) {
			intrinsicCalibrationParams = _intrinsicCalibrationParams;
			drawEventDispatcher.emit();
		}

		void KoccaMonitoringWidget::invalidateLastFrame() {
			nextMocapMarkerFrame_mutex.lock();

			if(nextMocapMarkerFrame != NULL) {
				delete nextMocapMarkerFrame;
				nextMocapMarkerFrame = NULL;
			}

			mappedMarkersCoordinates_mutex.lock();

			if(mappedMarkersCoordinates != NULL) {
				delete mappedMarkersCoordinates;
				mappedMarkersCoordinates = NULL;
			}

			mappedMarkersCoordinates_mutex.unlock();

			nextMocapMarkerFrame_mutex.unlock();

			CvDrawingArea::invalidateLastFrame();
		}

		void KoccaMonitoringWidget::enterRecordingMode() {
			showRecordingIndications = true;
			recordingTime = 0;
		}

		void KoccaMonitoringWidget::exitRecordingMode() {
			showRecordingIndications = false;
		}

		void KoccaMonitoringWidget::setRecordingTime(unsigned long long time) {
			recordingTime = time;
		}
	} // namespace widgets
} // namespace kocca
