#include "Page.h"
gui::Page::Page(std::string name, std::initializer_list<Control*> list) : name(name), shown(false) {
	for (Control * c : list) {
		controls.push_back(c);
		controlList.insert(std::pair<std::string, int>(c->getName(), controls.size() - 1));
	}
}
gui::Page::~Page() {
	for (int i = 0; i < controls.size(); i++) {
		delete controls[i];
	}
	if (organization != NULL) delete organization;
}
void gui::Page::resize(LPARAM newSize) {
	if (organization != NULL) {
		for (auto it = controls.begin(); it != controls.end(); it++) {
			if ((*it)->getControlType() != nullptr) {
				if (*(*it)->getControlType() == scrollHorz || *(*it)->getControlType() == scrollVert) return;
			}
			Dimensions d = organization->resize(newSize, (*it)->getOriginalDimensions());
			if (d.width != 0 && d.height != 0 && d.x != 0 && d.y != 0)
				(*it)->setPos(d.x, d.y, d.width, d.height);
		}
	}
}
void gui::Page::addControl(Control * control) {
	if (organization != NULL) {
		for (auto it = controls.begin(); it != controls.end(); it++) {
			Dimensions d = organization->addControl((*it)->getOriginalDimensions());
			(*it)->setPos(d.x, d.y, d.width, d.height);
		}
	}
	controls.push_back(control);
	controlList.insert(std::pair<std::string, int>(control->getName(), controls.size() - 1));
}
void gui::Page::showPage(bool show) {
	shown = show;
	if (show) {
		for (int i = 0; i < controls.size(); i++) {
			controls[i]->showComponent();
		}
	}
	else {
		for (int i = 0; i < controls.size(); i++) {
			controls[i]->hideComponent();
		}
	}
}
void gui::Page::handleMessages(MSG * msg) {
	for (int i = 0; i < controls.size(); i++) {
		if (controls[i]->doesDefaultMsgHandle()) {
			controls[i]->handleMsg(msg);
		}
	}
	if (msg->message == WM_SIZE && organization != NULL)
		resize(msg->lParam);
}
gui::Control * gui::Page::getControl(int i) {
	if (i < controls.size()) return controls[i];
	else return NULL;
}

gui::MainPage::MainPage(std::initializer_list<Page*> list) {
	for (Page* p : list) {
		pages.push_back(p);
		pageList.insert(std::pair<std::string, int>(p->getName(), pages.size() - 1));
	}
}
gui::MainPage::~MainPage() {
	for (int i = 0; i < pages.size(); i++) {
		delete pages[i];
	}
}
void gui::MainPage::addPage(Page* p) {
	pages.push_back(p);
	pageList.insert(std::pair<std::string, int>(p->getName(), pages.size() - 1));
}
void gui::MainPage::navigateTo(int i) {
	for (int j = 0; j < pages.size(); j++) {
		if (j == i) {
			pages[j]->showPage(true);
		}
		else pages[j]->showPage(false);
	}
}
void gui::MainPage::navigateTo(std::string page) {
	int i = pageList[page];
	navigateTo(i);
}
std::string gui::MainPage::getCurrentPageName() {
	for (int i = 0; i < pages.size(); i++) {
		if (pages[i]->isCurrentPage()) {
			for (auto it = pageList.begin(); it != pageList.end(); it++) {
				if ((*it).second == i) return (*it).first;
			}
			break;
		}
	}
	return "";
}
gui::Page * gui::MainPage::getCurrentPage() {
	for (int i = 0; i < pages.size(); i++) {
		if (pages[i]->isCurrentPage()) return pages[i];
	}
}
void gui::MainPage::resize(LPARAM size) {
	for (int i = 0; i < pages.size(); i++) {
		pages[i]->resize(size);
	}
	GUI::setDimensions(LOWORD(size), HIWORD(size));
}
void gui::MainPage::handleMessage(MSG * msg) {
	for (int i = 0; i < pages.size(); i++) {
		if (pages[i]->isCurrentPage()) {
			pages[i]->handleMessages(msg);
		}
	}
}

