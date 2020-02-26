#include "MainWindow.h"
#include <gtkmm/adjustment.h>
#include <gdk/gdkkeysyms.h>

#include "../utils.h"

namespace kocca {
	namespace widgets {
		MainWindow::MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): Gtk::Window(cobject) {
			// -------- Set pointers to widgets built by "builder" -------------

			kinectRGBStreamThumbnail = NULL;
			builder->get_widget_derived("kinectRGBStreamThumbnail", kinectRGBStreamThumbnail);

			kinectInfraredStreamThumbnail = NULL;
			builder->get_widget_derived("kinectInfraredStreamThumbnail", kinectInfraredStreamThumbnail);

			kinectDepthStreamThumbnail = NULL;
			builder->get_widget_derived("kinectDepthStreamThumbnail", kinectDepthStreamThumbnail);

			monitorFrameLabel = NULL;
			builder->get_widget("monitorFrameLabel", monitorFrameLabel);

			monitor = NULL;
			builder->get_widget_derived("monitor", monitor);

			RGBAdjustmentsVBox = NULL;
			builder->get_widget("RGBAdjustmentsVBox", RGBAdjustmentsVBox);

			rgbBrightnessScale = NULL;
			builder->get_widget("rgbBrightnessScale", rgbBrightnessScale);

			rgbContrastScale = NULL;
			builder->get_widget("rgbContrastScale", rgbContrastScale);

			infraredAdjustmentsVBox = NULL;
			builder->get_widget("infraredAdjustmentsVBox", infraredAdjustmentsVBox);

			irBrightnessScale = NULL;
			builder->get_widget("irBrightnessScale", irBrightnessScale);

			irContrastScale = NULL;
			builder->get_widget("irContrastScale", irContrastScale);

			natNetConnectionStatusStatusLabel = NULL;
			builder->get_widget("natNetConnectionStatusStatusLabel", natNetConnectionStatusStatusLabel);

			natNetConnectButton = NULL;
			builder->get_widget("natNetConnectButton", natNetConnectButton);

			natNetClientAddressEntry = NULL;
			builder->get_widget("natNetClientAddressEntry", natNetClientAddressEntry);

			natNetServerAddressEntry = NULL;
			builder->get_widget("natNetServerAddressEntry", natNetServerAddressEntry);

			mocapMarkersFrameLabel = NULL;
			builder->get_widget("mocapMarkersFrameLabel", mocapMarkersFrameLabel);

			mocapMarkersListView = NULL;
			builder->get_widget_derived("mocapMarkersListView", mocapMarkersListView);

			tabs = NULL;
			builder->get_widget("tabs", tabs);

			loadCalibrationFileButton = NULL;
			builder->get_widget("loadCalibrationFileButton", loadCalibrationFileButton);

			intrinsicIRResultCell = NULL;
			builder->get_widget_derived("intrinsicIRResultCell", intrinsicIRResultCell);

			extrinsicIRResultCell = NULL;
			builder->get_widget_derived("extrinsicIRResultCell", extrinsicIRResultCell);

			intrinsicRGBResultCell = NULL;
			builder->get_widget_derived("intrinsicRGBResultCell", intrinsicRGBResultCell);

			extrinsicRGBResultCell = NULL;
			builder->get_widget_derived("extrinsicRGBResultCell", extrinsicRGBResultCell);

			clearCalibrationDataButton = NULL;
			builder->get_widget("clearCalibrationDataButton", clearCalibrationDataButton);

			saveCalibrationToFileButton = NULL;
			builder->get_widget("saveCalibrationToFileButton", saveCalibrationToFileButton);

			chessBoardWidthSpinButton = NULL;
			builder->get_widget("chessBoardWidthSpinButton", chessBoardWidthSpinButton);

			chessBoardHeightSpinButton = NULL;
			builder->get_widget("chessBoardHeightSpinButton", chessBoardHeightSpinButton);

			chessBoardSquareSizeSpinButton = NULL;
			builder->get_widget("chessBoardSquareSizeSpinButton", chessBoardSquareSizeSpinButton);

			snapshotsCountSpinButton = NULL;
			builder->get_widget("snapshotsCountSpinButton", snapshotsCountSpinButton);

			countdownDurationSpinButton = NULL;
			builder->get_widget("countdownDurationSpinButton", countdownDurationSpinButton);

			snapshotFreezeDurationSpinButton = NULL;
			builder->get_widget("snapshotFreezeDurationSpinButton", snapshotFreezeDurationSpinButton);

			startCalibrationButton = NULL;
			builder->get_widget("startCalibrationButton", startCalibrationButton);

			pauseCalibrationButton = NULL;
			builder->get_widget("pauseCalibrationButton", pauseCalibrationButton);
	
			abortCalibrationButton = NULL;
			builder->get_widget("abortCalibrationButton", abortCalibrationButton);

			startPlaybackButton = NULL;
			builder->get_widget("startPlaybackButton", startPlaybackButton);

			goToBeginingButton = NULL;
			builder->get_widget("goToBeginingButton", goToBeginingButton);

			previousButton = NULL;
			builder->get_widget("previousButton", previousButton);

			stopButton = NULL;
			builder->get_widget("stopButton", stopButton);

			nextButton = NULL;
			builder->get_widget("nextButton", nextButton);

			goToEndButton = NULL;
			builder->get_widget("goToEndButton", goToEndButton);

			recordButton;
			builder->get_widget("recordButton", recordButton);
	
			openSequenceButton;
			builder->get_widget("openSequenceButton", openSequenceButton);
	
			closeSequenceButton;
			builder->get_widget("closeSequenceButton", closeSequenceButton);
	
			exportSequenceButton;
			builder->get_widget("exportSequenceButton", exportSequenceButton);

			videoTimeSliderScale = NULL;
			builder->get_widget("videoTimeSliderScale", videoTimeSliderScale);

			videoTimecodeLabel = NULL;
			builder->get_widget("videoTimecodeLabel", videoTimecodeLabel);

			waitMsgPopup = NULL;
			builder->get_widget_derived("waitMessagePopup", waitMsgPopup);
			// ------------------------------------------------------------------


			// -----------------
			rgbBrightnessScale->set_range(-100,100);
			rgbBrightnessScale->set_increments(1,10);
			rgbBrightnessScale->set_value(0);

			rgbContrastScale->set_range(-100, 100);
			rgbContrastScale->set_increments(1, 10);
			rgbContrastScale->set_value(0);
					
			irBrightnessScale->set_range(-100,100);
			irBrightnessScale->set_increments(1,10);
			irBrightnessScale->set_value(0);

			irContrastScale->set_range(-100, 100);
			irContrastScale->set_increments(1, 10);
			irContrastScale->set_value(0);

			natNetConnectionStatusStatusLabel->override_color(Gdk::RGBA("#FF0000"));
	
			chessBoardWidthSpinButton->set_range(3.0, 100.0);
			chessBoardWidthSpinButton->set_value(10.0);
			chessBoardWidthSpinButton->set_increments(1.0, 10.0);
								
			chessBoardHeightSpinButton->set_range(3.0, 100.0);
			chessBoardHeightSpinButton->set_value(7.0);
			chessBoardHeightSpinButton->set_increments(1.0, 10.0);

			chessBoardSquareSizeSpinButton->set_range(0.01, 1000.00);
			chessBoardSquareSizeSpinButton->set_value(34.0);
			chessBoardSquareSizeSpinButton->set_increments(1.0, 100.0);
			chessBoardSquareSizeSpinButton->set_digits(2);
								
			snapshotsCountSpinButton->set_range(1, 100);
			snapshotsCountSpinButton->set_value(10);
			snapshotsCountSpinButton->set_increments(1, 10);
								
			countdownDurationSpinButton->set_range(0, 120);
			countdownDurationSpinButton->set_value(5);
			countdownDurationSpinButton->set_increments(1, 10);
								
			snapshotFreezeDurationSpinButton->set_range(0.0, 60.0);
			snapshotFreezeDurationSpinButton->set_value(1.0);
			snapshotFreezeDurationSpinButton->set_increments(0.5, 2.5);
			snapshotFreezeDurationSpinButton->set_digits(1);

			videoTimeSliderScale->set_range(0,1000);
			videoTimeSliderScale->set_increments(10,100);
			videoTimeSliderScale->set_value(0);
			videoTimeSliderScale->signal_format_value().connect(sigc::ptr_fun(&format_timestamp));
			// -------------------------------------------------------------------


			// ----------------- set internal events -----------------------------
			playheadPosition = 0;
			playheadPositionEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_set_playhead_position_event));

			sequenceIsPlaying = false;
			sequenceIsPlayingEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_set_sequence_is_playing_event));

			natNetIsConnected = false;
			setNatNetIsConnectedEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_set_natnet_is_connected_event));

			disableAllWidgetsEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_disable_all_widgets_event));

			enableSequenceReadingWidgetsEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_enable_sequence_reading_widgets_event));

			enableMonitoringWidgetsEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_enable_monitoring_widgets_event));

			saveCalibrationButtonEnabled = false;
			enableSaveCalibrationButtonEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_enable_save_calibration_button_event));

			clearCalibrationButtonEnabled = false;
			enableClearCalibrationButtonEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_enable_clear_calibration_button_event));

			disableStopButtonEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_disable_stop_button_event));

			enterCalibrationModeEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_enter_calibration_mode_event));

			kinectIsAvailable = false;
			setKinectAvailabilityEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_set_kinect_availability_event));

			// make the whole Window accept drag&drop of files form 
			std::vector<Gtk::TargetEntry> listTargets;
			listTargets.push_back(Gtk::TargetEntry("text/uri-list"));
			drag_dest_set(listTargets, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_COPY);
			
			// handle key press events for hotkeys triggering
			add_events(Gdk::KEY_PRESS_MASK);
			signal_key_press_event().connect(sigc::mem_fun(*this, &MainWindow::on_key_press_event_custom_handler));

			monitorFrameTime = NULL;
			setMonitorFrameTimeEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_set_monitor_frame_time_event));

			mocapFrameTime = NULL;
			setMocapFrameTimeEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_set_mocap_frame_time_event));

			setBufferEndingPointEventDispatcher.connect(sigc::mem_fun(*this, &MainWindow::on_set_buffer_ending_point_event));
			// ---------------------------------------------------------------
		}
		
		void MainWindow::set_playhead_position(long long new_position) {
			playheadPosition = new_position;
			playheadPositionEventDispatcher.emit();
		}

		bool MainWindow::playHeadHasReachedEnd() {
			return(playheadPosition >= videoTimeSliderScale->get_adjustment()->get_upper());
		}

		void MainWindow::on_set_playhead_position_event() {
			if(videoTimeSliderScale->get_value() != playheadPosition)
				videoTimeSliderScale->set_value(playheadPosition);

			if((playheadPosition > 0) && !sequenceIsPlaying) {
				if(!goToBeginingButton->get_sensitive())
					goToBeginingButton->set_sensitive(true);

				if(!previousButton->get_sensitive())
					previousButton->set_sensitive(true);
			}
			else {
				if(goToBeginingButton->get_sensitive())
					goToBeginingButton->set_sensitive(false);

				if(previousButton->get_sensitive())
					previousButton->set_sensitive(false);
			}

			if(!playHeadHasReachedEnd() && !sequenceIsPlaying) {
				if(!goToEndButton->get_sensitive())
					goToEndButton->set_sensitive(true);

				if(!nextButton->get_sensitive())
					nextButton->set_sensitive(true);

				if(!startPlaybackButton->get_sensitive())
					startPlaybackButton->set_sensitive(true);
			}
			else {
				if(goToEndButton->get_sensitive())
					goToEndButton->set_sensitive(false);

				if(nextButton->get_sensitive())
					nextButton->set_sensitive(false);

				if(startPlaybackButton->get_sensitive())
					startPlaybackButton->set_sensitive(false);
			}
		}

		void MainWindow::on_set_sequence_is_playing_event() {
			if(sequenceIsPlaying) {
				startPlaybackButton->set_sensitive(false);
				stopButton->set_sensitive(true);
			}
			else {
				stopButton->set_sensitive(false);
				on_set_playhead_position_event();
			}
		}

		void MainWindow::set_sequence_is_playing(bool is_playing) {
			sequenceIsPlaying = is_playing;
			sequenceIsPlayingEventDispatcher.emit();
		}

		void MainWindow::on_disable_all_widgets_event() {
			rgbBrightnessScale->set_sensitive(false);
			rgbContrastScale->set_sensitive(false);
			irBrightnessScale->set_sensitive(false);
			irContrastScale->set_sensitive(false);
			natNetConnectButton->set_sensitive(false);
			natNetClientAddressEntry->set_sensitive(false);
			natNetServerAddressEntry->set_sensitive(false);
			loadCalibrationFileButton->set_sensitive(false);
			clearCalibrationDataButton->set_sensitive(false);
			saveCalibrationToFileButton->set_sensitive(false);
			chessBoardWidthSpinButton->set_sensitive(false);
			chessBoardHeightSpinButton->set_sensitive(false);
			chessBoardSquareSizeSpinButton->set_sensitive(false);
			snapshotsCountSpinButton->set_sensitive(false);
			countdownDurationSpinButton->set_sensitive(false);
			snapshotFreezeDurationSpinButton->set_sensitive(false);
			startCalibrationButton->set_sensitive(false);
			pauseCalibrationButton->set_sensitive(false);
			abortCalibrationButton->set_sensitive(false);
			startPlaybackButton->set_sensitive(false);
			goToBeginingButton->set_sensitive(false);
			previousButton->set_sensitive(false);
			stopButton->set_sensitive(false);
			nextButton->set_sensitive(false);
			goToEndButton->set_sensitive(false);
			recordButton->set_sensitive(false);
			openSequenceButton->set_sensitive(false);
			closeSequenceButton->set_sensitive(false);
			exportSequenceButton->set_sensitive(false);
			videoTimeSliderScale->set_value(0);
			videoTimeSliderScale->set_fill_level(0);
			videoTimeSliderScale->set_sensitive(false);
		}

		void MainWindow::disableAllWidgets() {
			disableAllWidgetsEventDispatcher.emit();
		}

		void MainWindow::on_set_natnet_is_connected_event() {
			if(natNetIsConnected) {
				natNetConnectionStatusStatusLabel->set_label("Connected");
				natNetConnectionStatusStatusLabel->override_color(Gdk::RGBA("#00AA00"));
				natNetConnectButton->remove();
				natNetConnectButton->add_pixlabel(get_img_path("disconnect.png"), "Disconnect"); // @TODO : sortir l'appel à get_img_path pour rendre la classe ind\E9pendante de KOCCA => passer le chemin du dossier img en parametre du constructeur

			}
			else {
				natNetConnectionStatusStatusLabel->set_label("Disconnected");
				natNetConnectionStatusStatusLabel->override_color(Gdk::RGBA("#FF0000"));
				natNetConnectButton->remove();
				natNetConnectButton->add_pixlabel(get_img_path("connect.png"), "Connect"); // @TODO : sortir l'appel à get_img_path pour rendre la classe ind\E9pendante de KOCCA => passer le chemin du dossier img en parametre du constructeur
			}

			mocapMarkersListView->clear_items();
		}

		void MainWindow::set_natnet_is_connected(bool is_connected) {
			natNetIsConnected = is_connected;
			setNatNetIsConnectedEventDispatcher.emit();
		}

		void MainWindow::on_enable_sequence_reading_widgets_event() {
			videoTimecodeLabel->set_label(format_timestamp(sequenceDuration));
			videoTimeSliderScale->set_range(0,sequenceDuration);
			videoTimeSliderScale->set_sensitive(true);
			closeSequenceButton->set_sensitive(true);
			loadCalibrationFileButton->set_sensitive(true);
			exportSequenceButton->set_sensitive(true);
		}

		void MainWindow::enableSequenceReadingWidgets(unsigned long long _sequenceDuration) {
			sequenceDuration = _sequenceDuration;
			enableSequenceReadingWidgetsEventDispatcher.emit();
		}

		void MainWindow::setKinectAvailability(bool available) {
			kinectIsAvailable = available;
			setKinectAvailabilityEventDispatcher.emit();
		}

		void MainWindow::on_set_kinect_availability_event() {
			rgbBrightnessScale->set_sensitive(kinectIsAvailable);
			irBrightnessScale->set_sensitive(kinectIsAvailable);
			startCalibrationButton->set_sensitive(kinectIsAvailable);
			recordButton->set_sensitive(kinectIsAvailable);
		}

		void MainWindow::on_enable_monitoring_widgets_event() {
			rgbBrightnessScale->set_sensitive(true);
			rgbContrastScale->set_sensitive(true);
			irBrightnessScale->set_sensitive(true);
			irContrastScale->set_sensitive(true);
			natNetConnectButton->set_sensitive(true);
			natNetClientAddressEntry->set_sensitive(true);
			natNetServerAddressEntry->set_sensitive(true);
			loadCalibrationFileButton->set_sensitive(true);
			clearCalibrationDataButton->set_sensitive(true);
			saveCalibrationToFileButton->set_sensitive(true);
			chessBoardWidthSpinButton->set_sensitive(true);
			chessBoardHeightSpinButton->set_sensitive(true);
			chessBoardSquareSizeSpinButton->set_sensitive(true);
			snapshotsCountSpinButton->set_sensitive(true);
			countdownDurationSpinButton->set_sensitive(true);
			snapshotFreezeDurationSpinButton->set_sensitive(true);
			startCalibrationButton->set_sensitive(true);
			recordButton->set_sensitive(true);
			openSequenceButton->set_sensitive(true);
			pauseCalibrationButton->set_sensitive(false);
			abortCalibrationButton->set_sensitive(false);
		}

		void MainWindow::enableMonitoringWidgets() {
			enableMonitoringWidgetsEventDispatcher.emit();
		}

		void MainWindow::enableSaveCalibrationButton(bool _saveCalibrationButtonEnabled) {
			saveCalibrationButtonEnabled = _saveCalibrationButtonEnabled;
			enableSaveCalibrationButtonEventDispatcher.emit();
		}

		void MainWindow::enableClearCalibrationButton(bool _clearCalibrationButtonEnabled) {
			clearCalibrationButtonEnabled = _clearCalibrationButtonEnabled;
			enableClearCalibrationButtonEventDispatcher.emit();
		}

		void MainWindow::on_enable_save_calibration_button_event() {
			saveCalibrationToFileButton->set_sensitive(saveCalibrationButtonEnabled);
		}

		void MainWindow::on_enable_clear_calibration_button_event() {
			clearCalibrationDataButton->set_sensitive(clearCalibrationButtonEnabled);
		}

		void MainWindow::disableStopButton() {
			disableStopButtonEventDispatcher.emit();
		}

		void MainWindow::on_disable_stop_button_event() {
			stopButton->set_sensitive(false);
		}

		void MainWindow::enterCalibrationMode() {
			enterCalibrationModeEventDispatcher.emit();
		}

		void MainWindow::on_enter_calibration_mode_event() {
			loadCalibrationFileButton->set_sensitive(false);
			saveCalibrationToFileButton->set_sensitive(false);
			chessBoardWidthSpinButton->set_sensitive(false);
			chessBoardHeightSpinButton->set_sensitive(false);
			chessBoardSquareSizeSpinButton->set_sensitive(false);
			snapshotsCountSpinButton->set_sensitive(false);
			startCalibrationButton->set_sensitive(false);
			pauseCalibrationButton->set_sensitive(true);
			abortCalibrationButton->set_sensitive(true);
		}

		sigc::signal<void> MainWindow::signal_shortcuts_Space_event() {
			return m_signal_shortcuts_Space_event;
		}

		sigc::signal<void> MainWindow::signal_shortcuts_CtrlS_event() {
			return m_signal_shortcuts_CtrlS_event;
		}

		sigc::signal<void> MainWindow::signal_shortcuts_CtrlO_event() {
			return m_signal_shortcuts_CtrlO_event;
		}

		sigc::signal<void> MainWindow::signal_shortcuts_CtrlQ_event() {
			return m_signal_shortcuts_CtrlQ_event;
		}

		sigc::signal<void> MainWindow::signal_shortcuts_CtrlW_event() {
			return m_signal_shortcuts_CtrlW_event;
		}

		sigc::signal<void> MainWindow::signal_shortcuts_Left_event() {
			return m_signal_shortcuts_Left_event;
		}

		sigc::signal<void> MainWindow::signal_shortcuts_Right_event() {
			return m_signal_shortcuts_Right_event;
		}

		sigc::signal<void> MainWindow::signal_shortcuts_PageUp_event() {
			return m_signal_shortcuts_PageUp_event;
		}

		sigc::signal<void> MainWindow::signal_shortcuts_PageDown_event() {
			return m_signal_shortcuts_PageDown_event;
		}

		bool MainWindow::on_key_press_event_custom_handler(GdkEventKey* event) {
			if(event->keyval == GDK_KEY_space) {
				m_signal_shortcuts_Space_event.emit();
				return true; // signal handled
			}
			else if((event->state == 4) && (event->keyval == GDK_KEY_s)) {
				m_signal_shortcuts_CtrlS_event.emit();
				return true; // signal handled
			}
			else if((event->state == 4) && (event->keyval == GDK_KEY_o)) {
				m_signal_shortcuts_CtrlO_event.emit();
				return true; // signal handled
			}
			else if((event->state == 4) && (event->keyval == GDK_KEY_q)) {
				m_signal_shortcuts_CtrlQ_event.emit();
				return true; // signal handled
			}
			else if((event->state == 4) && (event->keyval == GDK_KEY_w)) {
				m_signal_shortcuts_CtrlW_event.emit();
				return true; // signal handled
			}
			else if(event->keyval == GDK_KEY_Left) {
				m_signal_shortcuts_Left_event.emit();
				return true; // signal handled
			}
			else if(event->keyval == GDK_KEY_Right) {
				m_signal_shortcuts_Right_event.emit();
				return true; // signal handled
			}
			else if(event->keyval == GDK_KEY_Page_Up) {
				m_signal_shortcuts_PageUp_event.emit();
				return true; // signal handled
			}
			else if(event->keyval == GDK_KEY_Page_Down) {
				m_signal_shortcuts_PageDown_event.emit();
				return true; // signal handled
			}
			else
				return false; // allow propagation
		}

		void MainWindow::setMonitorFrameTime(unsigned long long _frameTime) {
			if (monitorFrameTime != NULL)
				delete monitorFrameTime;

			monitorFrameTime = new unsigned long long(_frameTime);
			setMonitorFrameTimeEventDispatcher.emit();
		}

		void MainWindow::on_set_monitor_frame_time_event() {
			std::ostringstream labelTextStream;
			labelTextStream << "Frame time: " << *monitorFrameTime;
			monitorFrameLabel->set_label(labelTextStream.str());
		}

		void MainWindow::setMocapFrameTime(unsigned long long _mocapFrameTime) {
			if(mocapFrameTime != NULL)
				delete mocapFrameTime;

			mocapFrameTime = new unsigned long long(_mocapFrameTime);
			setMocapFrameTimeEventDispatcher.emit();
		}

		void MainWindow::on_set_mocap_frame_time_event() {
			if (mocapFrameTime != NULL) {
				std::ostringstream labelTextStream;
				labelTextStream << "Frame time: " << *mocapFrameTime;
				mocapMarkersFrameLabel->set_label(labelTextStream.str());
			}
		}

		void MainWindow::setBufferEndingPoint(unsigned long long _bufferEndingPoint) {
			bufferEndingPoint = _bufferEndingPoint;
			setBufferEndingPointEventDispatcher.emit();
		}

		void MainWindow::on_set_buffer_ending_point_event() {
			videoTimeSliderScale->set_fill_level(bufferEndingPoint);
		}
	} // namespace widgets
} // namespace kocca
