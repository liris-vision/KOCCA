#ifndef KOCCA_WIDGETS_MAIN_WINDOW_H
#define KOCCA_WIDGETS_MAIN_WINDOW_H

#include <atomic>

#include <gtkmm.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/entry.h>

#include "SelectableCvDrawingArea.h"
#include "KoccaMonitoringWidget.h"
#include "CalibrationResultCell.h"
#include "MocapMarkersTreeView.h"
#include "WaitMessagePopup.h"

namespace kocca {
	namespace widgets {

		/**
		 * The main UI window of KOCCA
		 */
		class MainWindow: public Gtk::Window {

		protected:

			/**
			 * Position (in milliseconds) for the "playhead" slider scale
			 */
			long long playheadPosition;

			/**
			 * Event dispatcher emitted when a new value is set for playheadPosition. It will schedule a call to on_set_playhead_position_event() in Gtk main loop
			 */
			Glib::Dispatcher playheadPositionEventDispatcher;

			/**
			 * Callback function called after playheadPosition has been changed. It moves the slider scale widget to the new position and updates sensitivity for the playback control button widgets (play, stop, previous, next ...)
			 */
			void on_set_playhead_position_event();

			/**
			 * Event dispatch meant to be emitted when we want to disable (= make insensitive) all the widgets in the window. It will schedule a call to on_disable_all_widgets_event() in Gtk main loop
			 */
			Glib::Dispatcher disableAllWidgetsEventDispatcher;

			/**
			 * Callback function called after disableAllWidgetsEventDispatcher has been emitted. It sets all the widgets to insensitive (= disabled)
			 */
			void on_disable_all_widgets_event();

			/**
			 * Indicates if a sequence is currently being played
			 */
			bool sequenceIsPlaying;

			/**
			 * Event dispatcher emitted when the value of sequenceIsPlaying has been changed. It will schedule a call to on_set_sequence_is_playing_event() in Gtk's main loop.
			 */
			Glib::Dispatcher sequenceIsPlayingEventDispatcher;

			/**
			 * Callback function called after sequenceIsPlaying has been changed. It updates the sensitive state of play and stop buttons.
			 */
			void on_set_sequence_is_playing_event();

			/**
			 * Indicates if the application is connected to a NatNet server (= Optitrack Motive).
			 */
			bool natNetIsConnected;

			/**
			 * Event dispatcher emitted when the value of natNetIsConnected is changed. It will schedule a call to on_set_natnet_is_connected_event() in Gtk's main loop.
			 */
			Glib::Dispatcher setNatNetIsConnectedEventDispatcher;

			/**
			 * Callback function called after natNetIsConnected has been changed. It updates the content and color of the natNetConnectionStatusStatusLabel label widget, the sensitive state of natNetConnectButton button widget, and clears the content of the mocapMarkersListView widget if the new value for natNetIsConnected is false. 
			 */
			void on_set_natnet_is_connected_event();

			/**
			 * The duration of the currently loaded sequence
			 */
			unsigned long long sequenceDuration;

			/**
			 * Event dispatcher meant to be emitted when the sequence reading widgets (play, stop, previous, next etc...) need to be enabled. This will schedule a call to on_enable_sequence_reading_widgets_event() in Gtk's main loop.
			 */
			Glib::Dispatcher enableSequenceReadingWidgetsEventDispatcher;

			/**
			 * Callback function called after the sequence reading widgets (play, stop, previous, next etc...) have been requested to be enabled. This actually enables them.
			 */
			void on_enable_sequence_reading_widgets_event();

			/**
			 * Event dispatcher meant to be emitted when the monitoring widgets need to be enabled. This will schedule a call to on_enable_monitoring_widgets_event() in Gtk's main loop.
			 */
			Glib::Dispatcher enableMonitoringWidgetsEventDispatcher;

			/**
			 * Callback function called after monitoring widgets have been requested to be enabled. This actually enables them.
			 */
			void on_enable_monitoring_widgets_event();

			/**
			 * Whether the "Save" calibration button should be enabled or not.
			 */
			bool saveCalibrationButtonEnabled;

			/**
			 * Event dispactcher meant to be emitted when saveCalibrationButtonEnabled is set to true. This will schedule a call to on_enable_save_calibration_button_event() in Gtk's main loop.
			 */
			Glib::Dispatcher enableSaveCalibrationButtonEventDispatcher;

			/**
			 * Callback function called after saveCalibrationButtonEnabled has been set to true. This actually enables the "Save" calibration button.
			 */
			void on_enable_save_calibration_button_event();

			/**
			 * Whether the "Clear" calibration button should be enabled or not.
			 */
			bool clearCalibrationButtonEnabled;

			/**
			 * Event dispactcher meant to be emitted when clearCalibrationButtonEnabled is set to true. This will schedule a call to on_enable_clear_calibration_button_event() in Gtk's main loop.
			 */
			Glib::Dispatcher enableClearCalibrationButtonEventDispatcher;

			/**
			 * Callback function called after clearCalibrationButtonEnabled has been set to true. This actually enables the "Clear" calibration button.
			 */
			void on_enable_clear_calibration_button_event();

			/**
			 * Event dispatcher meant to be emitted when the "Stop" button needs to be disabled. This will schedule a call to on_disable_stop_button_event() in Gtk's main loop.
			 */
			Glib::Dispatcher disableStopButtonEventDispatcher;

			/**
			 * Callback function called after the "Stop" button has been requested to be disabled. This actually disables it.
			 */
			void on_disable_stop_button_event();

			/**
			 * Event dispatcher meant to be emitted when the main window enters the "Calibration" mode. This will schedule a call to on_enter_calibration_mode_event() in Gtk's main loop.
			 */
			Glib::Dispatcher enterCalibrationModeEventDispatcher;

			/**
			 * Callback function called after main window has entered calibration mode.
			 */
			void on_enter_calibration_mode_event();

			/**
			 * Indicates if the playhead has reached the end of the currently played sequence duration (in sequence reading mode only, obviously)
			 */
			bool playHeadHasReachedEnd();

			/**
			 * Memorises if a Kinect sensor is available or not.
			 */
			std::atomic<bool> kinectIsAvailable;

			/**
			 * Event dispatcher meant to be emitted when the kinectIsAvailable value has been changed. This will schedule a call to on_set_kinect_availability_event() in Gtk's main loop.
			 */
			Glib::Dispatcher setKinectAvailabilityEventDispatcher;

			/**
			 * Callback function called after kinectIsAvailable value has been changed.
			 */
			void on_set_kinect_availability_event();

			/**
			 * Callback function called when the user presses a key on the keyboard.
			 * @param event the Gtk event
			 * @return true if the pressed key combination is a known shortcut that has been handled, false if we must let the event propagate.
			 */
			bool on_key_press_event_custom_handler(GdkEventKey* event);

			/**
			 * The current frame time (in milliseconds) that must be displayed above the "monitor" widget.
			 */
			std::atomic<unsigned long long*> monitorFrameTime;

			/**
			 * Event dispatcher meant to be emitted when the value of monitorFrameTime is changed. This will schedule a call to on_set_monitor_frame_time_event() in Gtk's main loop.
			 */
			Glib::Dispatcher setMonitorFrameTimeEventDispatcher;

			/**
			 * Callback function called after monitorFrameTime has been changed. This actually updates the content of the "monitorFrameLabel" Label widget.
			 */
			void on_set_monitor_frame_time_event();

			/**
			 * The current frame time (in milliseconds) that must be displayed above the "Mocap Markers" ListView widget.
			 */
			std::atomic<unsigned long long*> mocapFrameTime;

			/**
			 * Event dispatcher meant to be called when the value of mocapFrameTime is changed. This will schedule a call to on_set_mocap_frame_time_event() in Gtk's main loop.
			 */
			Glib::Dispatcher setMocapFrameTimeEventDispatcher;

			/**
			 * Callback function called after the value of mocapFrameTime has been changed. This actually updates the content of the "mocapMarkersFrameLabel" Label widget.
			 */
			void on_set_mocap_frame_time_event();

			/**
			 * The current end (in milliseconds) of the reading buffer.
			 */
			std::atomic<unsigned long long> bufferEndingPoint;

			/**
			 * Event dispatcher meant to be called when value of bufferEndingPoint is changed. This will schedule a call to on_set_buffer_ending_point_event() in Gtk's main loop.
			 */
			Glib::Dispatcher setBufferEndingPointEventDispatcher;

			/**
			 * Callback function called after bufferEndingPoint value has been changed. This actually updates the buffer indicator display.
			 */
			void on_set_buffer_ending_point_event();

			/**
			 * Event notification to trigger callback function when the user presses the "Space" key.
			 */
			sigc::signal<void> m_signal_shortcuts_Space_event;

			/**
			 * Event notification to trigger callback function when the user presses the "Ctrl+S" keys.
			 */
			sigc::signal<void> m_signal_shortcuts_CtrlS_event;

			/**
			 * Event notification to trigger callback function when the user presses the "Ctrl+O" keys.
			 */
			sigc::signal<void> m_signal_shortcuts_CtrlO_event;

			/**
			 * Event notification to trigger callback function when the user presses the "Ctrl+Q" keys.
			 */
			sigc::signal<void> m_signal_shortcuts_CtrlQ_event;

			/**
			 * Event notification to trigger callback function when the user presses the "Ctrl+W" keys.
			 */
			sigc::signal<void> m_signal_shortcuts_CtrlW_event;

			/**
			 * Event notification to trigger callback function when the user presses the "left arrow" key.
			 */
			sigc::signal<void> m_signal_shortcuts_Left_event;

			/**
			 * Event notification to trigger callback function when the user presses the "right arrow" key.
			 */
			sigc::signal<void> m_signal_shortcuts_Right_event;

			/**
			 * Event notification to trigger callback function when the user presses the "page up" key.
			 */
			sigc::signal<void> m_signal_shortcuts_PageUp_event;

			/**
			 * Event notification to trigger callback function when the user presses the "page down" key.
			 */
			sigc::signal<void> m_signal_shortcuts_PageDown_event;

		public:

			/**
			 * Widget used to display a selectable thumbnail for the "RGB" kinect stream.
			 */
			SelectableCvDrawingArea* kinectRGBStreamThumbnail;

			/**
			 * Widget used to display a selectable thumbnail for the "Infrared" kinect stream.
			 */
			SelectableCvDrawingArea* kinectInfraredStreamThumbnail;

			/**
			 * Widget used to display a selectable thumbnail for the "Depth" kinect stream.
			 */
			SelectableCvDrawingArea* kinectDepthStreamThumbnail;

			/**
			 * Widget used to display the monitored stream frame time.
			 */
			Gtk::Label* monitorFrameLabel;

			/**
			 * Widget used to display the monitored stream and 2D-projected mocap markers.
			 */
			KoccaMonitoringWidget* monitor;

			/**
			 * Widget used as a container for the rgbBrightnessScale and rgbContrastScale widgets.
			 */
			Gtk::VBox* RGBAdjustmentsVBox;

			/**
			 * Widget used to tune the RGB stream brightness adjustment.
			 */
			Gtk::HScale* rgbBrightnessScale;

			/**
			 * Widget used to tune the RGB stream contrast adjustment.
			 */
			Gtk::HScale* rgbContrastScale;

			/**
			 * Widget used as a container for the irBrightnessScale and irContrastScale widgets.
			 */
			Gtk::VBox* infraredAdjustmentsVBox;

			/**
			 * Widget used to tune the infrared stream brightness adjustment.
			 */
			Gtk::HScale* irBrightnessScale;

			/**
			 * Widget used to tune the infrared stream contrast adjustment.
			 */
			Gtk::HScale* irContrastScale;

			/**
			 * Widget used to display the NatNet connection status ("Connected" in green, or "Disconnected" in red).
			 */
			Gtk::Label* natNetConnectionStatusStatusLabel;

			/**
			 * Widget for the NatNet "Connect" button.
			 */
			Gtk::Button* natNetConnectButton;

			/**
			 * Widget used for the "NatNet client" address entry.
			 */
			Gtk::Entry* natNetClientAddressEntry;

			/**
			 * Widget used for the "NatNet server" address entry.
			 */
			Gtk::Entry* natNetServerAddressEntry;

			/**
			 * Widget used to display the mocap markers frame time.
			 */
			Gtk::Label* mocapMarkersFrameLabel;

			/**
			 * Widget used to display the list of mocap markers with their coordinates.
			 */
			MocapMarkersTreeView* mocapMarkersListView;

			/**
			 * Widget for the "Calibration" and "Capture" tabs.
			 */
			Gtk::Notebook* tabs;

			/**
			 * Widget for the "Load" calibration file button.
			 */
			Gtk::Button* loadCalibrationFileButton;

			/**
			 * Widget for the infrared sensor intrinsic calibration result indicator.
			 */
			CalibrationResultCell* intrinsicIRResultCell;

			/**
			 * Widget for the infrared sensor extrinsic calibration result indicator.
			 */
			CalibrationResultCell* extrinsicIRResultCell;

			/**
			 * Widget for the RGB sensor intrinsic calibration result indicator.
			 */
			CalibrationResultCell* intrinsicRGBResultCell;

			/**
			 * Widget for the RGB sensor extrinsic calibration result indicator.
			 */
			CalibrationResultCell* extrinsicRGBResultCell;

			/**
			 * Widget for the "Clear" calibration button.
			 */
			Gtk::Button* clearCalibrationDataButton;

			/**
			 * Widget for the "Save" calibration button.
			 */
			Gtk::Button* saveCalibrationToFileButton;

			/**
			 * Widget for the "Chessboard width" setting spin button.
			 */
			Gtk::SpinButton* chessBoardWidthSpinButton;

			/**
			 * Widget for the "Chessboard height" setting spin button.
			 */
			Gtk::SpinButton* chessBoardHeightSpinButton;

			/**
			 * Widget for the "Chessboard square size" setting spin button.
			 */
			Gtk::SpinButton* chessBoardSquareSizeSpinButton;

			/**
			 * Widget for the "Snapshots count" setting spin button.
			 */
			Gtk::SpinButton* snapshotsCountSpinButton;

			/**
			 * Widget for the "Countdown duration" setting spin button.
			 */
			Gtk::SpinButton* countdownDurationSpinButton;

			/**
			 * Widget for the "Snapshot freeze duration" setting spin button.
			 */
			Gtk::SpinButton* snapshotFreezeDurationSpinButton;

			/**
			 * Widget for the "Start" calibration button.
			 */
			Gtk::Button* startCalibrationButton;

			/**
			 * Widget for the "Pause" calibration button.
			 */
			Gtk::Button* pauseCalibrationButton;

			/**
			 * Widget for the "Abort" calibration button.
			 */
			Gtk::Button* abortCalibrationButton;

			/**
			 * Widget for the "Play" button.
			 */
			Gtk::Button* startPlaybackButton;

			/**
			 * Widget for the "Go to begining" button.
			 */
			Gtk::Button* goToBeginingButton;

			/**
			 * Widget for the "Previous frame" button.
			 */
			Gtk::Button* previousButton;

			/**
			 * Widget for the "Stop" playing button.
			 */
			Gtk::Button* stopButton;

			/**
			 * Widget for the "Next frame" button.
			 */
			Gtk::Button* nextButton;

			/**
			 * Widget for the "Go to end" button.
			 */
			Gtk::Button* goToEndButton;

			/**
			 * Widget for the "Record" button.
			 */
			Gtk::Button* recordButton;

			/**
			 * Widget for the "Open" sequence button.
			 */
			Gtk::Button* openSequenceButton;

			/**
			 * Widget for the "Close" sequence button.
			 */
			Gtk::Button* closeSequenceButton;

			/**
			 * Widget for the "Export" sequence button.
			 */
			Gtk::Button* exportSequenceButton;

			/**
			 * Widget for the "playhead" slider scale.
			 */
			Gtk::HScale* videoTimeSliderScale;

			/**
			 * Widget for the "Sequence duration" Label.
			 */
			Gtk::Label* videoTimecodeLabel;

			/**
			 * Widget for the "Wait" message popup.
			 */
			WaitMessagePopup* waitMsgPopup;

			/**
			 * Constructor
			 * @see Gtk and libglade documentation
			 */
			MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

			/**
			 * Sets the current playhead position.
			 * @param new_position the new position for the playhead, in milliseconds
			 */
			void set_playhead_position(long long new_position);

			/**
			 * Sets the current sequence playing state.
			 * @param is_playing the new sequence playing state (true if a sequence is being played, false otherwise)
			 */
			void set_sequence_is_playing(bool is_playing);

			/**
			 * Sets the current "NatNet connection" state.
			 * @param is_connected the new connection state (true if the application is connected to a NatNet server, false otherwise).
			 */
			void set_natnet_is_connected(bool is_connected);

			/**
			 * Disables all widgets in the window.
			 */
			void disableAllWidgets();

			/**
			 * Enables the sequence reading widgets right after a sequence has been opened.
			 * @param _sequenceDuration the sequence duration (in milliseconds) of the newly opened sequence.
			 */
			void enableSequenceReadingWidgets(unsigned long long _sequenceDuration);

			/**
			 * Enables all the required widget for the "Monitoring" operation.
			 */
			void enableMonitoringWidgets();

			/**
			 * Sets a new state (enabled or disabled) for the "Save" calibration button.
			 * @param _saveCalibrationButtonEnabled whether the button should be enabled (true) or disabled (false)
			 * @todo change the name of this function to something like "setSaveCalibrationButtonState" as this can be used to enable OR disable the button.
			 */
			void enableSaveCalibrationButton(bool _saveCalibrationButtonEnabled);

			/**
			 * Sets a new state (enabled or disabled) for the "Clear" calibration button.
			 * @param _clearCalibrationButtonEnabled whether the button should be enabled (true) or disabled (false)
			 * @todo change the name of this function to something like "setClearCalibrationButtonState" as this can be used to enable OR disable the button.
			 */
			void enableClearCalibrationButton(bool _clearCalibrationButtonEnabled);

			/**
			 * Disables the "Stop" button.
			 */
			void disableStopButton();

			/**
			 * Enables the required widget for the "Calibration" operation.
			 */
			void enterCalibrationMode();

			/**
			 * Sets whether a Kinect sensor is available or not
			 * @param available whether a Kinect sensor is available (true) or not (false)
			 */
			void setKinectAvailability(bool available);

			/**
			 * Sets the frame time for the currently monitored Kinect stream.
			 * @param _frameTime the new frame time in milliseconds
			 */
			void setMonitorFrameTime(unsigned long long _frameTime);

			/**
			 * Sets the frame time for the mocap markers stream.
			 * @param _mocapFrameTime the new frame time in milliseconds
			 */
			void setMocapFrameTime(unsigned long long _mocapFrameTime);

			/**
			 * Sets the ending point of the sequence reading buffer.
			 * @param _bufferEndingPoint the new value (in milliseconds) of the reading buffer ending point.
			 */
			void setBufferEndingPoint(unsigned long long _bufferEndingPoint);

			/**
			 * Gets m_signal_shortcuts_Space_event
			 */
			sigc::signal<void> signal_shortcuts_Space_event();

			/**
			 * Gets m_signal_shortcuts_CtrlS_event
			 */
			sigc::signal<void> signal_shortcuts_CtrlS_event();

			/**
			 * Gets m_signal_shortcuts_CtrlO_event
			 */
			sigc::signal<void> signal_shortcuts_CtrlO_event();

			/**
			 * Gets m_signal_shortcuts_CtrlQ_event
			 */
			sigc::signal<void> signal_shortcuts_CtrlQ_event();

			/**
			 * Gets m_signal_shortcuts_CtrlW_event
			 */
			sigc::signal<void> signal_shortcuts_CtrlW_event();

			/**
			 * Gets m_signal_shortcuts_Left_event
			 */
			sigc::signal<void> signal_shortcuts_Left_event();

			/**
			 * Gets m_signal_shortcuts_Right_event
			 */
			sigc::signal<void> signal_shortcuts_Right_event();

			/**
			 * Gets m_signal_shortcuts_PageUp_event
			 */
			sigc::signal<void> signal_shortcuts_PageUp_event();

			/**
			 * Gets m_signal_shortcuts_PageDown_event
			 */
			sigc::signal<void> signal_shortcuts_PageDown_event();
		};
	} // namespace widgets
} // namespace kocca

#endif // KOCCA_WIDGETS_MAIN_WINDOW_H
