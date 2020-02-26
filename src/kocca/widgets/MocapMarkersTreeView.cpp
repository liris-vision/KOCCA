#include "MocapMarkersTreeView.h"
#include "../utils.h"
#include <sstream>

namespace kocca {
	namespace widgets {

		MocapMarkersTreeView::MocapMarkersTreeView(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder) : Gtk::TreeView(cobject) {
			nextFrame = NULL;
			refListStore = Gtk::ListStore::create(columnRecord);
			set_model(refListStore);

			Gtk::TreeViewColumn* pTreeViewMIDColumn = new MyTreeViewColumn("Marker ID", columnRecord.markerIDColumn);
			Gtk::TreeViewColumn *const pViewMIDColumn = Gtk::manage(pTreeViewMIDColumn);
			append_column(*pViewMIDColumn);

			Gtk::TreeViewColumn* pTreeViewXColumn = new MyTreeViewColumn("X", columnRecord.xColumn);
			Gtk::TreeViewColumn *const pViewXColumn = Gtk::manage(pTreeViewXColumn);
			append_column(*pViewXColumn);

			Gtk::TreeViewColumn* pTreeViewYColumn = new MyTreeViewColumn("Y", columnRecord.yColumn);
			Gtk::TreeViewColumn *const pViewYColumn = Gtk::manage(pTreeViewYColumn);
			append_column(*pViewYColumn);

			Gtk::TreeViewColumn* pTreeViewZColumn = new MyTreeViewColumn("Z", columnRecord.zColumn);
			Gtk::TreeViewColumn *const pViewZColumn = Gtk::manage(pTreeViewZColumn);
			append_column(*pViewZColumn);

			get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
			set_can_focus(true);
			drawEventDispatcher.connect(sigc::mem_fun(*this, &MocapMarkersTreeView::updateMarkersDisplay));
			setInitMarkersNamesEventDispatcher.connect(sigc::mem_fun(*this, &MocapMarkersTreeView::on_set_init_markers_names_event));
			onSelectedMarkersChanged = NULL;
			get_selection()->signal_changed().connect(sigc::mem_fun(*this, &MocapMarkersTreeView::on_selection_changed_event));
		}

		void MocapMarkersTreeView::on_set_init_markers_names_event() {
			for(int i = 0; i < initMarkersNames.size(); i++) {
				Gtk::TreeModel::iterator iter = refListStore->append();
				Gtk::TreeModel::Row row = *iter;
				row[columnRecord.markerIDColumn] = initMarkersNames.at(i);
				row[columnRecord.xColumn] = "?";
				row[columnRecord.yColumn] = "?";
				row[columnRecord.zColumn] = "?";
			}
		}

		void MocapMarkersTreeView::setInitMarkersNames(std::vector<std::string> _initMarkersNames) {
			initMarkersNames = _initMarkersNames;
			setInitMarkersNamesEventDispatcher.emit();
		}

		void MocapMarkersTreeView::setSelectedMarkersIds(std::vector<std::string> selectedMarkersIDs) {
			Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
			selection->unselect_all();

			for(Gtk::TreeNodeChildren::iterator rowIterator = refListStore->children().begin(); rowIterator != refListStore->children().end(); rowIterator++) {
				std::string currentLineMarkerID = (*rowIterator)->get_value(columnRecord.markerIDColumn);
				bool markerIDisSelected = false;

				for(int j = 0; (j < selectedMarkersIDs.size()) && !markerIDisSelected; j++) {
					if(selectedMarkersIDs.at(j) == currentLineMarkerID) {
						markerIDisSelected = true;
						selection->select(rowIterator);
					}
				}
			}
		}

		void MocapMarkersTreeView::on_selection_changed_event() {
			if(onSelectedMarkersChanged != NULL) {
				std::vector<std::string> selectedMarkersIDs;
				std::vector<Gtk::TreeModel::Path> selectedRowPaths = get_selection()->get_selected_rows();

				for(int i = 0; i < selectedRowPaths.size(); i++) {
					Gtk::TreeModel::Path rowPath = selectedRowPaths.at(i);
					Gtk::TreeModel::iterator rowIterator = refListStore->get_iter(rowPath);
					std::string markerID = (*rowIterator)->get_value(columnRecord.markerIDColumn);
					selectedMarkersIDs.push_back(markerID);
				}

				onSelectedMarkersChanged(selectedMarkersIDs);
			}
		}

		MocapMarkersTreeView::~MocapMarkersTreeView(){
			nextFrame_mutex.lock();

			if(nextFrame != NULL) {
				delete nextFrame;
				nextFrame = NULL;
			}

			nextFrame_mutex.unlock();
		}

		void MocapMarkersTreeView::updateMarkersDisplay() {
			nextFrame_mutex.lock();

			if(nextFrame != NULL) {
				for(int i = 0; i < nextFrame->getMarkersCount(); i++) {
					kocca::datalib::MocapMarker* pMarker = nextFrame->getMarkerByRank(i);

					if(pMarker != NULL) {
						if(!pMarker->name.empty() && strisprint(pMarker->name)) {
							bool newRowAlloc = true;
							Gtk::TreeNodeChildren::iterator* pRowIterator = getRowWithMarkerID(pMarker->name);

							if(pRowIterator == NULL) {
								pRowIterator = &(refListStore->append());
								(*pRowIterator)->set_value(columnRecord.markerIDColumn, (Glib::ustring)pMarker->name);
								newRowAlloc = false;
							}

							std::stringstream posX, posY, posZ;
							posX << pMarker->coords.x;
							posY << pMarker->coords.y;
							posZ << pMarker->coords.z;

							(*pRowIterator)->set_value(columnRecord.xColumn, (Glib::ustring)posX.str());
							(*pRowIterator)->set_value(columnRecord.yColumn, (Glib::ustring)posY.str());
							(*pRowIterator)->set_value(columnRecord.zColumn, (Glib::ustring)posZ.str());

							if(newRowAlloc)
								delete pRowIterator;
						}
					}
				}

				delete nextFrame;
				nextFrame = NULL;
			}

			nextFrame_mutex.unlock();

			queue_draw();
		}

		Gtk::TreeNodeChildren::iterator* MocapMarkersTreeView::getRowWithMarkerID(std::string id) {
			Gtk::TreeNodeChildren::iterator* row = NULL;

			if(!refListStore->children().empty()) {
				Gtk::TreeNodeChildren::iterator rowIterator = refListStore->children().begin();

				while((row == NULL) && (rowIterator != refListStore->children().end())) {
					if(rowIterator->get_value(columnRecord.markerIDColumn) == id)
						row = new Gtk::TreeNodeChildren::iterator(rowIterator);
					else
						rowIterator++;
				}
			}

			return row;
		}

		bool MocapMarkersTreeView::hasLineWithMarkerID(std::string id) {
			Gtk::TreeNodeChildren::iterator* row = getRowWithMarkerID(id);

			if(row != NULL) {
				delete row;
				return true;
			}
			else
				return false;
		}

		void MocapMarkersTreeView::setMarkerFrame(kocca::datalib::MocapMarkerFrame frame) {
			if(nextFrame_mutex.try_lock()) {
				if(nextFrame != NULL)
					delete nextFrame;

				nextFrame = new kocca::datalib::MocapMarkerFrame(frame);
				nextFrame_mutex.unlock();

				drawEventDispatcher.emit();
			}
		}

		void MocapMarkersTreeView::clear_items() {
			refListStore->clear();
		}
	} // namespace widgets
} // namespace kocca
