#ifndef KOCCA_WIDGETS_SELECTABLE_CV_DRAWING_AREA_H
#define	KOCCA_WIDGETS_SELECTABLE_CV_DRAWING_AREA_H

#include "CvDrawingArea.h"
#include <atomic>

namespace kocca {
	namespace widgets {
		
		/**
		 * A Gtk compatible widget capable of displaying cv::Mat images, and also of being "selected" or not, visually indicating it's selection state, and emitting notifications when it gets selected or deselected.
		 */
		class SelectableCvDrawingArea: public CvDrawingArea {
		protected:

			/**
			 * Selected state of the widget : true when the widget is selected, false otherwise.
			 */
			std::atomic<bool> selected;

			/**
			 * "Selected" event notifier.
			 * @see Gtk documentation
			 */
			sigc::signal<void> m_signal_selected_event;

			/**
			 * "Deselected" event notifier.
			 * @see Gtk documentation
			 */
			sigc::signal<void> m_signal_deselected_event;

			/**
			 * Callback function called when the users click the widget
			 * @param event Gtk event, check GTK documentation
			 * @return always false, to let other objects handle the signal_button_press_event if needed
			 */
			bool onClick(GdkEventButton* event);

			/**
			 * Callback function called when the users moves the mouse cursor over the widget
			 * @param event Gtk event, check GTK documentation
			 * @return always false, to let other objects handle the signal_motion_notify_event if needed
			 */
			bool onMotion(GdkEventMotion* event);

			/**
			 * Gets the image to be drawn in the widget area : it calls cvDrawingArea::getDrawImage() and adds a border if the widget is selected.
			 * @return the image to be drawn in the widget area
			 */
			cv::Mat getDrawImage();

		public:

			/**
			 * Widget constructor
			 * @see Gtk and libglade documentation
			 */
			SelectableCvDrawingArea(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

			/**
			 * Gets the selected state of the widget
			 * @return true if the widget is selected, false otherwise
			 */
			bool get_selected();

			/**
			 * Sets the selected state of the widget
			 * @param _selected the new "selected" state for the widget
			 */
			void set_selected(bool _selected);

			/**
			 * Gets m_signal_selected_event, so other objects can connect it to be notfied when the widget gets selected
			 */
			sigc::signal<void> signal_selected_event();

			/**
			 * Gets m_signal_deselected_event, so other objects can connect it to be notfied when the widget gets deselected
			 */
			sigc::signal<void> signal_deselected_event();
		};
	} // namespace widgets
} // namespace kocca

#endif // KOCCA_WIDGETS_SELECTABLE_CV_DRAWING_AREA_H
