#ifndef KOCCA_WIDGETS_WAIT_MESSAGE_POPUP_H
#define KOCCA_WIDGETS_WAIT_MESSAGE_POPUP_H

#include <gtkmm.h>
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>

namespace kocca {
	namespace widgets {

		/**
		 * A Gtk compatible widget for a modal popup window that can display custom messages next to an hourglass icon, and also an optionnal progress bar.
		 */
		class WaitMessagePopup: public Gtk::Window {
		protected:

			/**
			 * The Gtk::VBox container that will hold the dynamically created Gtk::Labels for our custom messages.
			 */
			Gtk::VBox* messagesVBox;
			
			/**
			 * The progress bar widget.
			 */
			Gtk::ProgressBar* progressBar;

			/**
			 * Value of the current operation progress to be shown in the progress bar.
			 */
			float progress;

			/**
			 * When "emitted", this dispatcher will schedule a call to on_set_progress_event() during Gtk main loop.
			 * @see Glib::Dispatcher
			 */
			Glib::Dispatcher setProgressEventDispatcher;

			/**
			 * Callback function that is asynchoneously called after setProgress() has been called, to refresh the widget display.
			 */
			void on_set_progress_event();

			/**
			 * When "emitted", this dispatcher will schedule a call to on_hide_progress_event() during Gtk main loop.
			 * @see Glib::Dispatcher
			 */
			Glib::Dispatcher hideProgressEventDispatcher;

			/**
			 * Callback function that is asynchoneously called after hideProgress() has been called, to refresh the widget display.
			 */
			void on_hide_progress_event();

		public:

			/**
			 * Constructor
			 * @see Gtk and libglade documentation
			 */
			WaitMessagePopup(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

			/**
			 * Adds a message to be displayed in the popup. A new Gtk::Label object is dynamically created and it's reference is returned so we can remove it later. The popup is automatically showed if it wasn't already visible.
			 * @param msg the content of the message to be displayed in the popup
			 * @return a pointer to the new Gtk::Label created to display the message.
			 */
			Gtk::Label* addOperationMessage(std::string msg);

			/**
			 * Removes a message already displayed from the popup. If no visible message is left in the popup after that, the popup is automatically hidden.
			 * @param msgLabel a pointer to a pointer to the Gtk::Label that is used to display the message. The pointed Gtk::Label* will be set to NULL after the Gtk::Label has been deleted. 
			 */
			void removeOperationMessage(Gtk::Label** msgLabel);

			/**
			 * Sets a new value for the progress bar and automatically shows the progress bar if it wasn't already visible.
			 * @param _newProgress the new value for the progress bar, in percentage.
			 */
			void setProgress(float _newProgress);

			/**
			 * Hides the progress bar.
			 */
			void hideProgress();
		};
	} // namespace widgets
} // namespace kocca

#endif // KOCCA_WIDGETS_WAIT_MESSAGE_POPUP_H
