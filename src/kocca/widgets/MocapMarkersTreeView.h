#ifndef KOCCA_WIDGETS_MOCAP_MARKERS_TREE_VIEW_H
#define KOCCA_WIDGETS_MOCAP_MARKERS_TREE_VIEW_H

#include <gtkmm.h>
#include <gtkmm/treeview.h>
#include "../datalib/MocapMarkerFrame.h"

#include <mutex>

#include <gtkmm/treeviewcolumn.h>

namespace kocca {
	namespace widgets {

		/**
		 * Column type for the MocapMarkersTreeView widget
		 */
		class MyTreeViewColumn : public Gtk::TreeViewColumn {
		public:

			/**
			 * Constructor
			 */
			template<class T_ModelColumnType>
			MyTreeViewColumn(
				const Glib::ustring& title,
				const Gtk::TreeModelColumn<T_ModelColumnType>& column
			) : Gtk::TreeViewColumn(title, column) {}
		};

		/**
		 * Column record model for the MocapMarkersTreeView
		 */
		class MocapMarkersColumnRecord : public Gtk::TreeModelColumnRecord {

		public:

			/**
			 * Constructor
			 */
			MocapMarkersColumnRecord() {
				add(markerIDColumn);
				add(xColumn);
				add(yColumn);
				add(zColumn);
			}

			/**
			 * Markers ID column
			 */
			Gtk::TreeModelColumn<Glib::ustring> markerIDColumn;

			/**
			 * Markers X coordinate column
			 */
			Gtk::TreeModelColumn<Glib::ustring> xColumn;

			/**
			 * Markers Y coordinate column
			 */
			Gtk::TreeModelColumn<Glib::ustring> yColumn;

			/**
			 * Markers Z coordinate column
			 */
			Gtk::TreeModelColumn<Glib::ustring> zColumn;
		};

		/**
		 * A Gtk compatible widget to display MocapMarker list with their respective ID and X,Y,Z coordinates. It also allows to select lines corresponding to markers and emits notifications when the selection changes.
		 */
		class MocapMarkersTreeView: public Gtk::TreeView {

		public:

			/**
			 * Widget constructor
			 * @see Check Gtk and libglade documentation for more detail
			 */
			MocapMarkersTreeView(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

			/**
			 * Widget destructor
			 */
			virtual ~MocapMarkersTreeView();

			/**
			 * Updates the widget display after change in it's markers content
			 */
			void updateMarkersDisplay();

			/**
			 * Sets the next MocapMarkerFrame to be displayed when the widget refreshes it's display
			 * @param frame the new MocapMarkerFrame to display
			 */
			void setMarkerFrame(kocca::datalib::MocapMarkerFrame frame);

			/**
			 * Sets the names of the initial list of markers. A line will be created in the widget for each marker, with it's name and ?,?,? as coordinates as these coordinates have not been set yet
			 * @param _initMarkersNames the names of the initial list of markers
			 */
			void setInitMarkersNames(std::vector<std::string> _initMarkersNames);

			/**
			 * Sets the list of markers that are selected
			 * @param selectedMarkersIDs the IDs of the newly selected markers
			 */
			void setSelectedMarkersIds(std::vector<std::string> selectedMarkersIDs);

			/**
			 * Callback function that will be called when the user changes the selected markers in the widget, to notify other objects of selection changes.
			 * @param std::vector<std::string> the IDs of the newly selected markers
			 * @todo rather use Gtk's sigc::signal (see code of SelectableCvDrawingArea for an example)
			 */
			void (*onSelectedMarkersChanged)(std::vector<std::string>);

			/**
			 * Clears all the markers displayed in the widget
			 */
			void clear_items();

		protected:

			/**
			 * The column record for the widget
			 * @see Gtk documentation
			 */
			MocapMarkersColumnRecord columnRecord;

			/**
			 * The list store for the widget
			 * @see Gtk documentation
			 */
			Glib::RefPtr<Gtk::ListStore> refListStore;

			/**
			 * The next MocapMarkerFrame to be displayed when the widget refreshes it's display
			 */
			kocca::datalib::MocapMarkerFrame* nextFrame;

			/**
			 * Names of markers initially displayed 
			 */
			std::vector<std::string> initMarkersNames;

			/**
			 * Event dispatcher emitted when initMarkersNames is set. It will schedule a call to on_set_init_markers_names_event() in Gtk's main loop
			 */
			Glib::Dispatcher setInitMarkersNamesEventDispatcher;

			/**
			 * Callback function called after initMarkersNames has been set. It fills the widget with initial markers names, and ?,?,? as their X,Y,Z coordinates
			 */
			void on_set_init_markers_names_event();

			/**
			 * Event dispatcher meant to be emitted every time the widget needs to refresh it's display. It will schedule a call to updateMarkersDisplay() in Gtk's main loop.
			 */
			Glib::Dispatcher drawEventDispatcher;

			/**
			 * Checks if the widget has a line correspondig to the marker ID given in argument
			 * @param id the marker ID to look for
			 * @return true if a line with the id marker ID has been found in the widget, false otherwise
			 */
			bool hasLineWithMarkerID(std::string id);

			/**
			 * Gets an iterator pointing to the widget's row that correspond to the id given in argument
			 * @param id the marker ID to look for
			 * @return an iterator pointing to the appropriate row, or NULL if none has been found
			 */
			Gtk::TreeNodeChildren::iterator* getRowWithMarkerID(std::string id);

			/**
			 * A mutex to lock nextFrame in a multi-threaded context
			 */
			std::mutex nextFrame_mutex;

			/**
			 * ??? Not used anymore ?
			 * @todo check if this is still used and remove it if it isn't
			 */
			guint refreshingThreadId;

			/**
			 * Callback function triggered when the markers selection has changed
			 */
			void on_selection_changed_event();
		};
	} // namespace widgets
} // namespace kocca

#endif // KOCCA_WIDGETS_MOCAP_MARKERS_TREE_VIEW_H
