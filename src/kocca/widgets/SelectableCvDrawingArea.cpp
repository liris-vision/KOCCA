#include "SelectableCvDrawingArea.h"

namespace kocca {
	namespace widgets {
		SelectableCvDrawingArea::SelectableCvDrawingArea(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): CvDrawingArea(cobject, builder) {
			selected = false;
			add_events(Gdk::BUTTON_PRESS_MASK);
			signal_button_press_event().connect(sigc::mem_fun(this, &SelectableCvDrawingArea::onClick));
			add_events(Gdk::POINTER_MOTION_MASK);
			signal_motion_notify_event().connect(sigc::mem_fun(this, &SelectableCvDrawingArea::onMotion));
		}

		bool SelectableCvDrawingArea::get_selected() {
			return selected;
		}

		void SelectableCvDrawingArea::set_selected(bool _selected) {
			if (selected != _selected) {
				selected = _selected;

				if (selected)
					m_signal_selected_event.emit();
				else
					m_signal_deselected_event.emit();

				drawEventDispatcher.emit();
			}
		}

		cv::Mat SelectableCvDrawingArea::getDrawImage() {
			cv::Mat drawImage = CvDrawingArea::getDrawImage();

			if (selected) {
				cv::line(drawImage, cv::Point(0, 0), cv::Point(get_width()-1, 0), cv::Scalar(255, 0, 0), 2);
				cv::line(drawImage, cv::Point(get_width()-1, 0), cv::Point(get_width()-1, get_height() - 1), cv::Scalar(255, 0, 0), 2);
				cv::line(drawImage, cv::Point(get_width() - 1, get_height() - 1), cv::Point(0, get_height() - 1), cv::Scalar(255, 0, 0), 2);
				cv::line(drawImage, cv::Point(0, get_height() - 1), cv::Point(0, 0), cv::Scalar(255, 0, 0), 2);
			}

			return drawImage;
		}

		bool SelectableCvDrawingArea::onClick(GdkEventButton* event) {
			set_selected(true);
			return false;
		}

		bool SelectableCvDrawingArea::onMotion(GdkEventMotion* event) {
			if (selected)
				get_window()->set_cursor(Gdk::Cursor::create(Gdk::CursorType::ARROW));
			else
				get_window()->set_cursor(Gdk::Cursor::create(Gdk::CursorType::HAND1));

			return false;
		}

		sigc::signal<void> SelectableCvDrawingArea::signal_selected_event() {
			return m_signal_selected_event;
		}

		sigc::signal<void> SelectableCvDrawingArea::signal_deselected_event() {
			return m_signal_deselected_event;
		}
	} // namespace widgets
} // namespace kocca
