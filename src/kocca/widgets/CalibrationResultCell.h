#ifndef KOCCA_WIDGETS_CALIBRATION_RESULT_CELL_H
#define KOCCA_WIDGETS_CALIBRATION_RESULT_CELL_H

#include <gtkmm.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/button.h>

#include "../datalib/CalibrationParametersSet.h"

namespace kocca {
	namespace widgets {

		/**
		 * A Gtk compatible widget that contains a status icon and a details button to display information about a CalibrationParametersSet.
		 */
		class CalibrationResultCell: public Gtk::HBox {
		public:

			/**
			 * Constructor
			 * @see Gtk and libglade documentation
			 */
			CalibrationResultCell(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

			/**
			 * Allows to set the CalibrationParametersSet the object refers to. When calling this method, statusIcon and detailsButton are updated.
			 * @param result_ the new result to be displayed
			 */
			void setResult(kocca::datalib::CalibrationParametersSet* result_);

			/**
			 * Removes the CalibrationParametersSet the object refers to. When calling this method, statusIcon and detailsButton are deactivated.
			 */
			void removeResult();

		protected:

			/**
			 * The CalibrationParametersSet refered to by the widget
			 */
			kocca::datalib::CalibrationParametersSet* result;

			/**
			 * Icon widget that will be automatically set when the result pointer is set. It will display either a green up thumb, a yellow warning sign or a red thumb down, according to the "error" value of the CalibrationParametersSet
			 */
			Gtk::Image statusIcon;

			/**
			 * A Button with no text showing a magnifier icon. The button will trigger
			 */
			Gtk::Button detailsButton;

			/**
			 * Displays a message dialog containing all the CalibrationParametersSet data (if available)
			 */
			void showResultDetails();

			/**
			 * Event dispatcher meant to be emitted when result is removed. This will schedule a call to on_remove_result_event() in Gtk's main loop.
			 */
			Glib::Dispatcher removeResultEventDispatcher;

			/**
			 * Callback function called after result has been removed.
			 */
			void on_remove_result_event();

			/**
			 * Event dispatcher meant to be emitted when result is changed. This will schedule a call to on_set_result_event() in Gtk's main loop.
			 */
			Glib::Dispatcher setResultEventDispatcher;

			/**
			 * Callback function called after result has been set.
			 */
			void on_set_result_event();
		};
	} // namespace widgets
} // namespace kocca

#endif // KOCCA_WIDGETS_CALIBRATION_RESULT_CELL_H
