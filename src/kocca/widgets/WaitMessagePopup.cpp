#include "WaitMessagePopup.h"
#include "../utils.h"
#include <gdkmm/cursor.h>

namespace kocca {
	namespace widgets {
		WaitMessagePopup::WaitMessagePopup(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): Gtk::Window(cobject) {
			override_background_color(Gdk::RGBA("#FFFFFF"));

			messagesVBox = NULL;
			builder->get_widget("messagesVBox", messagesVBox);

			progressBar = NULL;
			builder->get_widget("progressBar", progressBar);

			setProgressEventDispatcher.connect(sigc::mem_fun(*this, &WaitMessagePopup::on_set_progress_event));
			hideProgressEventDispatcher.connect(sigc::mem_fun(*this, &WaitMessagePopup::on_hide_progress_event));
		}

		Gtk::Label* WaitMessagePopup::addOperationMessage(std::string msg) {
			Gtk::Label* pLabel = Gtk::manage(new Gtk::Label(msg));
			pLabel->set_alignment(Gtk::ALIGN_START, Gtk::ALIGN_CENTER);
			messagesVBox->pack_end(*pLabel);
			pLabel->show();
			show();
			get_window()->set_cursor(Gdk::Cursor::create(Gdk::CursorType::WATCH));
			get_transient_for()->get_window()->set_cursor(Gdk::Cursor::create(Gdk::CursorType::WATCH));
			return(pLabel);
		}

		void WaitMessagePopup::removeOperationMessage(Gtk::Label** pMsgLabel) {
			if(*pMsgLabel != NULL) {
				messagesVBox->remove(**pMsgLabel);
				delete *pMsgLabel;
				*pMsgLabel = NULL;
			}

			if(messagesVBox->get_children().size() == 0) {
				get_transient_for()->get_window()->set_cursor(Gdk::Cursor::create(Gdk::CursorType::ARROW));
				hide();
			}
		}

		void WaitMessagePopup::on_set_progress_event() {
			progressBar->set_fraction(progress / 100.0);
	
			std::ostringstream displayedText;
			displayedText << (int)progress << "%";

			progressBar->set_text(displayedText.str());

			if(!progressBar->get_visible())
				progressBar->show();
		}

		void WaitMessagePopup::setProgress(float _newProgress) {
			progress = _newProgress;
			setProgressEventDispatcher.emit();
		}

		void WaitMessagePopup::on_hide_progress_event() {
			progress = 0;
			progressBar->set_fraction(0);
			progressBar->set_text("");
			progressBar->hide();
		}

		void WaitMessagePopup::hideProgress() {
			usleep(50);
			hideProgressEventDispatcher.emit();
		}
	} // namespace widgets
} // namespace kocca
