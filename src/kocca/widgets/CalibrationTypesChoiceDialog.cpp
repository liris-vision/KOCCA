#include "CalibrationTypesChoiceDialog.h"

namespace kocca {
	namespace widgets {
		CalibrationTypesChoiceDialog::CalibrationTypesChoiceDialog(
				CALTYPECHOICE_DIALOG_TYPE dialogType, bool inIRAvailable,
				bool inRGBAvailable, bool exIRAvailable, bool exRGBAvailable,
				bool inIRDefaultChecked, bool inRGBDefaultChecked,
				bool exIRDefaultChecked, bool exRGBDefaultChecked) {
			std::string headQuestionString, checkBoxesLabelString;

			// depending of the dialog action type, a few words will change in the label widgets
			switch (dialogType) {
				case CALTYPECHOICE_DIALOG_IMPORT:
					headQuestionString =
							"Please choose which calibration parameters to import";
					checkBoxesLabelString = "Import";
					break;
				case CALTYPECHOICE_DIALOG_EXPORT:
					headQuestionString =
							"Please choose which calibration parameters to export";
					checkBoxesLabelString = "Export";
					break;
				case CALTYPECHOICE_DIALOG_CALIBRATE:
					headQuestionString = "Please choose what to calibrate";
					checkBoxesLabelString = "Calibrate";
					break;
				case CALTYPECHOICE_DIALOG_CLEAR:
					headQuestionString = "Please choose what to clear";
					checkBoxesLabelString = "Clear";
					break;
			}

			// initialise dialog's window ...

			set_title(headQuestionString);
			set_border_width(50);
			set_position(Gtk::WIN_POS_CENTER);
			set_resizable(false);

			// intitalize contained widgets ...

			get_vbox()->pack_start(headQuestionLabel, Gtk::PACK_SHRINK, 20);
			headQuestionLabel.set_label(headQuestionString);
			headQuestionLabel.show();

			get_vbox()->pack_start(calibrationChoiceTable, Gtk::PACK_SHRINK, 20);
			IRLabel.set_label("IR");
			calibrationChoiceTable.attach(IRLabel, 1, 2, 2, 3);
			IRLabel.show();

			RGBLabel.set_label("RGB");
			calibrationChoiceTable.attach(RGBLabel, 1, 2, 3, 4);
			RGBLabel.show();

			intrinsicLabel.set_label("Intrinsic");
			calibrationChoiceTable.attach(intrinsicLabel, 2, 3, 1, 2);
			intrinsicLabel.show();

			extrinsicLabel.set_label("Extrinsic");
			calibrationChoiceTable.attach(extrinsicLabel, 3, 4, 1, 2);
			extrinsicLabel.show();

			calibrationChoiceTable.attach(inIRCell, 2, 3, 2, 3, Gtk::SHRINK,
					Gtk::SHRINK);
			inIRCell.pack_start(inIRCheckBox, Gtk::PACK_SHRINK, 20);
			inIRCheckBox.set_label(checkBoxesLabelString);

			inIRCell.pack_start(inIRNALabel, Gtk::PACK_SHRINK, 20);
			inIRNALabel.set_label("Not available");
			inIRCell.show();

			calibrationChoiceTable.attach(inRGBCell, 2, 3, 3, 4, Gtk::SHRINK,
					Gtk::SHRINK);
			inRGBCell.pack_start(inRGBCheckBox, Gtk::PACK_SHRINK, 20);
			inRGBCheckBox.set_label(checkBoxesLabelString);

			inRGBCell.pack_start(inRGBNALabel, Gtk::PACK_SHRINK, 20);
			inRGBNALabel.set_label("Not available");
			inRGBCell.show();

			calibrationChoiceTable.attach(exIRCell, 3, 4, 2, 3, Gtk::SHRINK,
					Gtk::SHRINK);
			exIRCell.pack_start(exIRCheckBox, Gtk::PACK_SHRINK, 20);
			exIRCheckBox.set_label(checkBoxesLabelString);

			exIRCell.pack_start(exIRNALabel, Gtk::PACK_SHRINK, 20);
			exIRNALabel.set_label("Not available");
			exIRCell.show();

			calibrationChoiceTable.attach(exRGBCell, 3, 4, 3, 4, Gtk::SHRINK,
					Gtk::SHRINK);
			exRGBCell.pack_start(exRGBCheckBox, Gtk::PACK_SHRINK, 20);
			exRGBCheckBox.set_label(checkBoxesLabelString);

			exRGBCell.pack_start(exRGBNALabel, Gtk::PACK_SHRINK, 20);
			exRGBNALabel.set_label("Not available");
			exRGBCell.show();
			calibrationChoiceTable.show();

			// apply availability parameters ...

			if (exIRAvailable) {
				exIRCheckBox.set_active(exIRDefaultChecked);
				exIRCheckBox.show();
				exIRNALabel.hide();
			} else {
				exIRNALabel.show();
				exIRCheckBox.hide();
			}

			if (inIRAvailable) {
				inIRCheckBox.set_active(inIRDefaultChecked);
				inIRCheckBox.show();
				inIRNALabel.hide();

				// when action type is calibration, if Intrinsic IR is default checked, this means there is no Intrinsic IR calibration data available
				// in that case, make Intrinsic IR calibration required in order to be able to perform Extrinsic IR calibration
				if ((dialogType == CALTYPECHOICE_DIALOG_CALIBRATE)
						&& inIRDefaultChecked) {
					exIRCheckBox.signal_toggled().connect(
							sigc::mem_fun(this,
									&CalibrationTypesChoiceDialog::onClickExIRCheckBoxDependantFromIn));
					inIRCheckBox.set_sensitive(false);
				}
			} else {
				inIRNALabel.show();
				inIRCheckBox.hide();
			}

			if (exRGBAvailable) {
				exRGBCheckBox.set_active(exRGBDefaultChecked);
				exRGBCheckBox.show();
				exRGBNALabel.hide();
			} else {
				exRGBNALabel.show();
				exRGBCheckBox.hide();
			}

			if (inRGBAvailable) {
				inRGBCheckBox.set_active(inRGBDefaultChecked);
				inRGBCheckBox.show();
				inRGBNALabel.hide();

				// when action type is calibration, if Intrinsic RGB is default checked, this means there is no Intrinsic RGB calibration data available
				// in that case, make Intrinsic RGB calibration required in order to be able to perform Extrinsic RGB calibration
				if ((dialogType == CALTYPECHOICE_DIALOG_CALIBRATE)
						&& inRGBDefaultChecked) {
					inRGBCheckBox.set_sensitive(false);
					exRGBCheckBox.signal_toggled().connect(
							sigc::mem_fun(this,
									&CalibrationTypesChoiceDialog::onClickExRGBCheckBoxDependantFromIn));
				}
			} else {
				inRGBNALabel.show();
				inRGBCheckBox.hide();
			}

			// when action type is calibration, if user wants to perform Extrinsic calibration both IR and RGB must be performed together
			// so make them depend of each other
			if (dialogType == CALTYPECHOICE_DIALOG_CALIBRATE) {
				exRGBCheckBox.signal_toggled().connect(
						sigc::mem_fun(this,
								&CalibrationTypesChoiceDialog::onClickExRGBCheckBoxDependantFromIR));
				exIRCheckBox.signal_toggled().connect(
						sigc::mem_fun(this,
								&CalibrationTypesChoiceDialog::onClickExIRCheckBoxDependantFromRGB));
			}

			// add OK and cancel buttons at the bottom of the dialog
			CancelButton = add_button(GTK_STOCK_CANCEL, Gtk::RESPONSE_CANCEL);
			OKButton = add_button(GTK_STOCK_OK, Gtk::RESPONSE_OK);

			// at any checkbox state change, update OK Button state
			inIRCheckBox.signal_toggled().connect(
					sigc::mem_fun(this,
							&CalibrationTypesChoiceDialog::onClickAnyCheckBox));
			inRGBCheckBox.signal_toggled().connect(
					sigc::mem_fun(this,
							&CalibrationTypesChoiceDialog::onClickAnyCheckBox));
			exIRCheckBox.signal_toggled().connect(
					sigc::mem_fun(this,
							&CalibrationTypesChoiceDialog::onClickAnyCheckBox));
			exRGBCheckBox.signal_toggled().connect(
					sigc::mem_fun(this,
							&CalibrationTypesChoiceDialog::onClickAnyCheckBox));
		}

		void CalibrationTypesChoiceDialog::onClickExIRCheckBoxDependantFromIn() {
			if (exIR_checked()) {
				inIRCheckBox.set_active(true);
				inIRCheckBox.set_sensitive(false);
			} else
				inIRCheckBox.set_sensitive(true);
		}

		void CalibrationTypesChoiceDialog::onClickExRGBCheckBoxDependantFromIn() {
			if (exRGB_checked()) {
				inRGBCheckBox.set_active(true);
				inRGBCheckBox.set_sensitive(false);
			} else
				inRGBCheckBox.set_sensitive(true);
		}

		void CalibrationTypesChoiceDialog::onClickExRGBCheckBoxDependantFromIR() {
			exIRCheckBox.set_active(exRGBCheckBox.get_active());
		}

		void CalibrationTypesChoiceDialog::onClickExIRCheckBoxDependantFromRGB() {
			exRGBCheckBox.set_active(exIRCheckBox.get_active());
		}

		void CalibrationTypesChoiceDialog::onClickAnyCheckBox() {
			// OK button is enabled only if at least onf of the checkboxes is checked
			OKButton->set_sensitive(
					inIR_checked() || inRGB_checked() || exIR_checked()
							|| exRGB_checked());
		}

		bool CalibrationTypesChoiceDialog::inIR_checked() {
			return (inIRCheckBox.is_visible() && inIRCheckBox.get_active());
		}

		bool CalibrationTypesChoiceDialog::inRGB_checked() {
			return (inRGBCheckBox.is_visible() && inRGBCheckBox.get_active());
		}

		bool CalibrationTypesChoiceDialog::exIR_checked() {
			return (exIRCheckBox.is_visible() && exIRCheckBox.get_active());
		}

		bool CalibrationTypesChoiceDialog::exRGB_checked() {
			return (exRGBCheckBox.is_visible() && exRGBCheckBox.get_active());
		}
	} // namespace widgets
} // namespace kocca
