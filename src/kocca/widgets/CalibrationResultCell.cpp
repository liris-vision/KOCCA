#include "CalibrationResultCell.h"
#include <gtkmm/messagedialog.h>
#include "../utils.h"

namespace kocca {
	namespace widgets {
		void CalibrationResultCell::showResultDetails() {
			if ((result != NULL))
				Gtk::MessageDialog(result->toString()).run();
		}

		CalibrationResultCell::CalibrationResultCell(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): Gtk::HBox(cobject) {
			// add status icon
			pack_start(statusIcon, Gtk::PACK_SHRINK, 5);
			statusIcon.show();

			// add details button
			pack_start(detailsButton, Gtk::PACK_SHRINK, 5);

			detailsButton.add_pixlabel(get_img_path("magnifier.png"), "");// @TODO : sortir l'appel à get_img_path pour rendre la classe indépendante de KOCCA => passer en paramètre du constructeur
			detailsButton.show();

			// connect details button click event to protected method showResultDetails
			detailsButton.signal_clicked().connect(sigc::mem_fun(*this, &CalibrationResultCell::showResultDetails));

			removeResultEventDispatcher.connect(sigc::mem_fun(*this, &CalibrationResultCell::on_remove_result_event));

			setResultEventDispatcher.connect(sigc::mem_fun(*this, &CalibrationResultCell::on_set_result_event));

			// intitalize as empty
			removeResult();
		}

		void CalibrationResultCell::on_set_result_event() {
			detailsButton.set_sensitive(true);
			detailsButton.set_tooltip_text("Click to view calibration data details");

			// set different icon and tooltip, according to the error parameter
			if (result->error < 1) {
				statusIcon.set_tooltip_text("Calibration quality : good");
				statusIcon.set(get_img_path("good.png")); // @TODO : sortir l'appel à get_img_path pour rendre la classe indépendante de KOCCA => passer en paramètre du constructeur
			} else if (result->error < 5) {
				statusIcon.set_tooltip_text("Calibration quality : medium");
				statusIcon.set(get_img_path("warning.png")); // @TODO : sortir l'appel à get_img_path pour rendre la classe indépendante de KOCCA => passer en paramètre du constructeur
			} else {
				statusIcon.set_tooltip_text("Calibration quality : bad");
				statusIcon.set(get_img_path("bad.png")); // @TODO : sortir l'appel à get_img_path pour rendre la classe indépendante de KOCCA => passer en paramètre du constructeur
			}
		}

		void CalibrationResultCell::setResult(kocca::datalib::CalibrationParametersSet* result_) {
			if(result_ != NULL) {
				result = result_;
				setResultEventDispatcher.emit();
			}
		}
		
		void CalibrationResultCell::on_remove_result_event() {
			statusIcon.set(get_img_path("missing.png")); // @TODO : sortir l'appel à get_img_path pour rendre la classe indépendante de KOCCA => passer en paramètre du constructeur
			statusIcon.set_tooltip_text("Calibration data not available");
			detailsButton.set_tooltip_text("Calibration data not available");
			detailsButton.set_sensitive(false);
		}

		void CalibrationResultCell::removeResult() {
			result = NULL;
			removeResultEventDispatcher.emit();
		}
	} // namespace widgets
} // namespace kocca
