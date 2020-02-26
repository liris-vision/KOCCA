#ifndef KOCCA_WIDGETS_CALIBRATION_TYPES_CHOICE_DIALOG_H
#define KOCCA_WIDGETS_CALIBRATION_TYPES_CHOICE_DIALOG_H

#include <gtkmm/dialog.h>
#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>

namespace kocca {
	namespace widgets {

		/**
		 * The type of action for which we want a dialog to choose calibrations parameters
		 */
		enum CALTYPECHOICE_DIALOG_TYPE {
			CALTYPECHOICE_DIALOG_IMPORT = 1,
			CALTYPECHOICE_DIALOG_EXPORT = 2,
			CALTYPECHOICE_DIALOG_CALIBRATE = 3,
			CALTYPECHOICE_DIALOG_CLEAR = 4
		};

		/**
		 * A Gtk compatible widget for a dialog popup window that allows users to choose wich calibrations parameters they want to perform an operation to.
		 */
		class CalibrationTypesChoiceDialog: public Gtk::Dialog {
		public:

			/**
			 * Constructor for the dialog
			 * @param dialogType the type of action for wich we display a dialog
			 * @param inIRAvailable wether the "Intrinsic IR" choice is available
			 * @param inRGBAvailable wether the "Intrinsic RGB" choice is available
			 * @param exIRAvailable wether the "Extrinsic IR" choice is available
			 * @param exRGBAvailable wether the "Extrinsic RGB" choice is available
			 * @param inIRDefaultChecked wether the "Intrinsic IR" checkbox is checked by default
			 * @param inRGBDefaultChecked wether the "Intrinsic RGB" checkbox is checked by default
			 * @param exIRDefaultChecked wether the "Extrinsic IR" checkbox is checked by default
			 * @param exRGBDefaultChecked wether the "Extrinsic RGB" checkbox is checked by default
			 */
			CalibrationTypesChoiceDialog(CALTYPECHOICE_DIALOG_TYPE dialogType,
					bool inIRAvailable = true, bool inRGBAvailable = true,
					bool exIRAvailable = true, bool exRGBAvailable = true,
					bool inIRDefaultChecked = true, bool inRGBDefaultChecked = true,
					bool exIRDefaultChecked = true, bool exRGBDefaultChecked = true);

			/**
			 * After dialog validation, allows to know if user has checked "Intrinsic IR" choice
			 */
			bool inIR_checked();

			/**
			 * After dialog validation, allows to know if user has checked "Intrinsic RGB" choice
			 */
			bool inRGB_checked();

			/**
			 * After dialog validation, allows to know if user has checked "Extrinsic IR" choice
			 */
			bool exIR_checked();

			/**
			 * After dialog validation, allows to know if user has checked "Extrinsic RGB" choice
			 */
			bool exRGB_checked();

			/**
			 * Main container widget
			 */
			//Gtk::VBox mainVBox;

			/**
			 * Label widget that displays the question asked to user on the top of the dialog
			 */
			Gtk::Label headQuestionLabel;

			/**
			 * The table widget that contains checkboxes
			 */
			Gtk::Table calibrationChoiceTable;

			/**
			 * Label widget that displays "IR" as a table's row header
			 */
			Gtk::Label IRLabel;

			/**
			 * Label widget that displays "RGB" as a table's row header
			 */
			Gtk::Label RGBLabel;

			/**
			 * Label widget that displays "Intrinsic" as a table's column header
			 */
			Gtk::Label intrinsicLabel;

			/**
			 * Label widget that displays "Extrinsic" as a table's column header
			 */
			Gtk::Label extrinsicLabel;

			/**
			 * Table cell container widget for "Intrinsic IR" choice
			 */
			Gtk::HBox inIRCell;

			/**
			 * CheckBox widget for Intrinsic IR choice
			 */
			Gtk::CheckButton inIRCheckBox;

			/**
			 * Label widget to display instead of the Intrinsic IR CheckBox when it's not available
			 */
			Gtk::Label inIRNALabel;

			/**
			 * Table cell container widget for "Intrinsic RGB" choice
			 */
			Gtk::HBox inRGBCell;

			/**
			 * CheckBox widget for Intrinsic RGB choice
			 */
			Gtk::CheckButton inRGBCheckBox;

			/**
			 * Label widget to display instead of the Intrinsic RGB CheckBox when it's not available
			 */
			Gtk::Label inRGBNALabel;

			/**
			 * Table cell container widget for "Extrinsic IR" choice
			 */
			Gtk::HBox exIRCell;

			/**
			 * CheckBox widget for Extrinsic IR choice
			 */
			Gtk::CheckButton exIRCheckBox;

			/**
			 * Label widget to display instead of the Extrinsic IR CheckBox when it's not available
			 */
			Gtk::Label exIRNALabel;

			/**
			 * Table cell container widget for "Extrinsic RGB" choice
			 */
			Gtk::HBox exRGBCell;

			/**
			 * CheckBox widget for Extrinsic RGB choice
			 */
			Gtk::CheckButton exRGBCheckBox;

			/**
			 * Label widget to display instead of the Extrinsic RGB CheckBox when it's not available
			 */
			Gtk::Label exRGBNALabel;

			/**
			 * Dialog's "OK" bottom button, to let user validate his choice
			 */
			Gtk::Button* OKButton;

			/**
			 * Dialog's "Cancel" bottom button, to let user cancel action
			 */
			Gtk::Button* CancelButton;

		protected:

			/**
			 * When "Extrinsic IR" choice is checked and "Intrinsic IR" is required in order to choose it, force intrinsic choice checked too. This is used when the dialog action type is "Calibration" and there is no data currently loaded for "Intrinsic IR", beacause Extrinsic calibrations REQUIRES Intrinsic parameters in order to be performed
			 */
			void onClickExIRCheckBoxDependantFromIn();

			/**
			 * When "Extrinsic RGB" choice is checked and "Intrinsic RGB" is required in order to choose it, force intrinsic choice checked too. This is used when the dialog action type is "Calibration" and there is no data currently loaded for "Intrinsic RGB", beacause Extrinsic calibrations REQUIRES Intrinsic parameters in order to be performed
			 */
			void onClickExRGBCheckBoxDependantFromIn();

			/**
			 * When "Extrinsic RGB" choice is checked and "Extrinsic IR and Extrinsic RGB" depend from each other, force "Extrinsic IR" checked too. This is used when the dialog action type is "Calibration" because both extrinsic calibrations MUST be performed together
			 */
			void onClickExRGBCheckBoxDependantFromIR();

			/**
			 * When "Extrinsic IR" choice is checked and "Extrinsic IR and Extrinsic RGB" depend from each other, force "Extrinsic RGB" checked too. This is used when the dialog action type is "Calibration" because both extrinsic calibrations MUST be performed together
			 */
			void onClickExIRCheckBoxDependantFromRGB();

			/**
			 * On any change made by user to the checkboxes, update "OK" button availability (set to enabled if at least on choice is checked, or disabled otherwise)
			 */
			void onClickAnyCheckBox();
		};
	} // namespace widgets
} // namespace kocca

#endif // KOCCA_WIDGETS_CALIBRATION_TYPES_CHOICE_DIALOG_H
