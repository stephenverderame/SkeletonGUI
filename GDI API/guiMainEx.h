#pragma once
#include "guiMain.h"
#pragma comment(lib, "comctl32.lib")
#include <CommCtrl.h>
namespace gui{
	class Common {
	private:
		Common() {};
	private:
		static bool enabledCommon;
	public:
		static void enable();
	};

	class Statusbar : public Control {
	public:
		Statusbar(const char * name, int sections, int * widths, HWND parent = gui::GUI::useWindow());
		void setText(const char * txt, int section);
		void handleMsg(MSG * msg);
	};
	enum toolbarControlTypes {
		tcs_button = TBSTYLE_BUTTON,
		tcs_check = TBSTYLE_CHECK,
		tcs_dropdown = TBSTYLE_DROPDOWN
	};
	enum toolbarBitmaps {
		tbm_openFile = STD_FILEOPEN,
		tbm_saveFile = STD_FILESAVE,
		tbm_newFile = STD_FILENEW,
		tbm_copy = STD_COPY,
		tbm_cut = STD_CUT,
		tbm_help = STD_HELP,
		tbm_undo = STD_UNDO,
		tbm_redo = STD_REDOW,
		tbm_delete = STD_DELETE,
		tbm_paste = STD_PASTE,
		tbm_find = STD_FIND,
		tbm_print = STD_PRINT,
		tbm_printPreview = STD_PRINTPRE,
		tbm_replace = STD_REPLACE,
		tbm_properties = STD_PROPERTIES
	};
	struct ToolbarControls {
		int command;
		int bitmap;
		toolbarControlTypes controlType = tcs_button;
		ToolbarControls(int command, int bitmap, toolbarControlTypes tcs = tcs_button);
	};
	class Toolbar : public Control {
	public:
		Toolbar(const char * name, std::initializer_list<ToolbarControls *> controls, HWND parent = gui::GUI::useWindow());
		void handleMsg(MSG * msg);
	};
}