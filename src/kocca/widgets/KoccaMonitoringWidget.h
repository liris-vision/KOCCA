#ifndef KOCCA_WIDGETS_KOCCA_MONITORING_WIDGET_H
#define KOCCA_WIDGETS_KOCCA_MONITORING_WIDGET_H

#include <gtkmm.h>
#include "CvDrawingArea.h"
#include "../datalib/MocapMarkerFrame.h"
#include <mutex>
#include <atomic>
#include "../datalib/ExtrinsicCalibrationParametersSet.h"
#include "../datalib/IntrinsicCalibrationParametersSet.h"

namespace kocca {
	namespace widgets {

		/**
		 * A Gtk compatible widget designed for KOCCA, that can display cv::Mat images, draw selectable MocapMarkers on these images (2D-projected according to sensor calibration parameters), and also write a "REC[ ]" indicator and elapsed time during recording.
		 */
		class KoccaMonitoringWidget: public CvDrawingArea {
		protected:

			/**
			 * Wether the widget should display or not the "Record" indicator and elapsed time on the image
			 */
			std::atomic<bool> showRecordingIndications;

			/**
			 * Elapsed time since the begining of recording
			 */
			std::atomic<unsigned long long> recordingTime;

			/**
			 * Next MocapMarkerFrame to be projected and drawn when the widget refreshes it's display
			 */
			kocca::datalib::MocapMarkerFrame* nextMocapMarkerFrame;

			/**
			 * Mutex to lock nextMocapMarkerFrame in a multi-thread context
			 */
			std::mutex nextMocapMarkerFrame_mutex;

			/**
			 *  Mocap Markers positions, mapped to the actual widget's coordinates (after openCV 2d projection and image frame resizing and centering). This could be re-calculated on the fly at widget draw time, but for performance reason we store it until markers are changed or widget is resized.
			 */
			std::vector<cv::Point2d>* mappedMarkersCoordinates;

			/**
			 * Mutex to lock mappedMarkersCoordinates in a multi-thread context
			 */
			std::mutex mappedMarkersCoordinates_mutex;

			/**
			 * Extrinsic calibration parameters to use for 2D-projection of MocapMarkers
			 */
			kocca::datalib::ExtrinsicCalibrationParametersSet* extrinsicCalibrationParams;

			/**
			 * Intrinsic calibration parameters to use for 2D-projection of MocapMarkers
			 */
			kocca::datalib::IntrinsicCalibrationParametersSet* intrinsicCalibrationParams;

			/**
			 * IDs of the MocapMarkers that have been selected by the user
			 */
			std::vector<std::string> selectedMarkersIDs;

			/**
			 * ID of the MocapMarker that is currently hovered by the mouse cursor (there can be only one at the same time)
			 */
			std::string hoveredMarkerId;

			/**
			 * true when the user is currently holding down the 'Ctrl' key, in order to select several markers
			 */
			bool multipleSelectionActivated;

			/**
			 * Gets the list of 2D points corresponding to the MocapMarkers in nextMocapMarkerFrame, after 2D projection (according to calibration parameters) and resizing of nextFrame (to fit in the widget area)
			 */
			std::vector<cv::Point2d>* getMappedMarkersCoordinates();

			/**
			 * Gets the image to be drawn in the widget area : nextFrame is resized and centered, then the mapped markers are drawn on it, finally the recording indications are incrusted
			 */
			cv::Mat getDrawImage();

			/**
			 * Checks if one given marker is currently selected
			 * @param markerID the ID of the marker we want to know if it's selected
			 * @return true if the marker identified by markerID is currently selected, false otherwise
			 */
			bool markerIDisSelected(std::string markerID);

			/**
			 * Callback function called when the user clicks in the widget area. Used to determine when a marker is clicked.
			 * @param event Gtk event. Check Gtk documentation for more detail.
			 * @return always false, so other objects can handle the event too
			 */
			bool onClick(GdkEventButton* event);

			/**
			 * Callback function called when the user hovers the widget area with the mouse cursor. Used to determine when a marker is hovered.
			 * @param event Gtk event. Check Gtk documentation for more detail.
			 * @return always false, so other objects can handle the event too
			 */
			bool onMotion(GdkEventMotion* event);

			/**
			 * Callback function called after the widget has been instanciated (that is just after application startup). This is useful to know the widget parent window, wich is required to handle the key press and key release events.
			 */
			void onRealize();

			/**
			 * Callback function called when the user presses down a key on it's keyboard. This is usefull to know when the 'Ctrl' key is pressed down so multiple marker selection is activated.
			 * @param event Gtk event. Check Gtk documentation for more detail.
			 * @return always false, so other objects can handle the event too
			 */
			bool onKeyPress(GdkEventKey* event);

			/**
			 * Callback function called when the user releases key on it's keyboard. This is usefull to know when the 'Ctrl' key is released so multiple marker selection is de-activated.
			 * @param event Gtk event. Check Gtk documentation for more detail.
			 * @return always false, so other objects can handle the event too
			 */
			bool onKeyRelease(GdkEventKey* event);

			/**
			 * Callback function called when the widget is resized. We need to know it because we store mappedMarkersCoordinates for performance reason, so mappedMarkersCoordinates must be re-calculated when the widget is resized.
			 * @param allocation ??? check Gtk documentation for more detail. We don't use it here anyway ...
			 */
			void onResize(Gtk::Allocation& allocation);

			/**
			 * Checks if the blinking point in the "REC[ ]" indicator should be on or off. This is cyclic (2 seconds period) and based on elapsed recording time.
			 * @return true if the blinking point should be on, false if it should be off.
			 */
			bool blinkingRecPointOn();

			/**
			 * Incrusts the "REC[ ]" indicator in a cv::Mat image 
			 * @param frame the image to incrust the indicator into
			 */
			void incrustRecIndicator(cv::Mat& frame);

			/**
			 * Incrusts the elapsed recording time in a cv::Mat image 
			 * @param frame the image to incrust the time into
			 */
			void incrustTimeStamp(cv::Mat& frame);

		public:

			/**
			 * Widget contructor
			 * @see Check Gtk and libglade documentation for more details
			 */
			KoccaMonitoringWidget(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

			/**
			 * Widget destructor.
			 */
			~KoccaMonitoringWidget();

			/**
			 * Sets the next MocapMarkerFrame to be mapped and drawn when the widget refreshes its display.
			 */
			void setNextMocapMarkerFrame(kocca::datalib::MocapMarkerFrame _nextMocapMarkerFrame);

			/**
			 * Sets the intrinsic calibration parameters to use for MocapMarkers projection
			 */
			void setExtrinsicCalibrationParams(kocca::datalib::ExtrinsicCalibrationParametersSet* _extrinsicCalibrationParams);

			/**
			 * Sets the extrinsic calibration parameters to use for MocapMarkers projection
			 */
			void setIntrinsicCalibrationParams(kocca::datalib::IntrinsicCalibrationParametersSet* _intrinsicCalibrationParams);

			/**
			 * Sets the list of markers that are selected
			 * @param _selectedMarkersIDs IDs of the markers that are selected
			 */
			void setSelectedMarkersIds(std::vector<std::string> _selectedMarkersIDs);

			/**
			 * Dynamic callback method that will be called to notify of markers selection changes.
			 * @param IDs of the markers that have been selected
			 * @todo rather use Gtk's sigc::signal (see code of SelectableCvDrawingArea for an example)
			 */
			void (*onSelectedMarkersChanged)(std::vector<std::string>);

			/**
			 * Removes the currently displayed cv::Mat AND MocapMarkersFrame.
			 * @see CvDrawingArea::invalidateLastFrame()
			 */
			void invalidateLastFrame();

			/**
			 * Tells the widget we are entering in recording mode. This initialises recordingTime to 0 and sets showRecordingIndications to true.
			 */
			void enterRecordingMode();

			/**
			 * Tells the widget we are leaving from recording mode. This sets showRecordingIndications to false.
			 */
			void exitRecordingMode();

			/**
			 * Updates the elapsed recording time.
			 * @param time the new elapsed recording time
			 */
			void setRecordingTime(unsigned long long time);
		};
	} // namespace widgets
} // namespace kocca

#endif // KOCCA_WIDGETS_KOCCA_MONITORING_WIDGET_H
