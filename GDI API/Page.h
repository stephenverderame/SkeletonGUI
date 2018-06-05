#pragma once
#include "guiMain.h"
namespace gui {
	class Page {
	protected:
		std::vector<Control*> controls;
		std::map<std::string, int> controlList;
		std::string name;
		bool shown;
		Layout * organization;
	public:
		Page(std::string name) : name(name), shown(false) {}
		Page(std::string name, std::initializer_list<Control*> list);
		~Page();
		void setLayout(Layout * layout) {organization = layout;}
		void resize(LPARAM newSize);
		virtual void addControl(Control * control);
		void showPage(bool show);
		virtual void handleMessages(MSG * msg);
		Control * getControl(int i);
		Control * getControl(std::string name) { return controls[controlList[name]]; }
		inline std::string getName() { return name; }
		inline bool isCurrentPage() { return shown; }
	};
	class MainPage {
	private:
		std::vector<Page*> pages;
		std::map<std::string, int> pageList;
	public:
		MainPage() {}
		MainPage(std::initializer_list<Page*> list);
		~MainPage();
		void addPage(Page* p);
		void navigateTo(int i);
		void navigateTo(std::string page);
		std::string getCurrentPageName();
		Page * getCurrentPage();
		inline Page * getPage(std::string name) { return pages[pageList[name]]; }
		void resize(LPARAM size);
		void handleMessage(MSG * msg);
	};
}