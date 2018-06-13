#include "guiMainEx.h"
bool gui::Common::enabledCommon = false;
void gui::Common::enable()
{
	if (!enabledCommon) {
		enabledCommon = true;
		InitCommonControls();
	}
}

gui::Statusbar::Statusbar(const char * name, int sections, int * widths, HWND parent)
{
	Common::enable();
	selfHandle = true;
	handle = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, parent, NULL, GetModuleHandle(NULL), NULL);
	this->parent = parent;
	this->name = name;
	SendMessage(handle, SB_SETPARTS, sections, (LPARAM)widths);
}

void gui::Statusbar::setText(const char * txt, int section)
{
	SendMessage(handle, SB_SETTEXT, section, (LPARAM)txt);
}

void gui::Statusbar::handleMsg(MSG * msg)
{
	if (msg->message == WM_SIZE)
		SendMessage(handle, WM_SIZE, 0, 0);
}

gui::Toolbar::Toolbar(const char * name, std::initializer_list<ToolbarControls *> controls, HWND parent)
{
	Common::enable();
	handle = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, parent, NULL, GetModuleHandle(NULL), NULL);
	this->parent = parent;
	selfHandle = true;
	SendMessage(handle, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	TBADDBITMAP tbit;
	tbit.hInst = HINST_COMMCTRL;
	tbit.nID = IDB_STD_SMALL_COLOR;
	SendMessage(handle, TB_ADDBITMAP, 0, (LPARAM)&tbit);
	TBBUTTON * buttons = new TBBUTTON[controls.size()];
	memset(buttons, 0, sizeof(TBBUTTON) * controls.size());
	int i = 0;
	for (auto it = controls.begin(); it != controls.end(); it++) {
		buttons[i].iBitmap = (*it)->bitmap;
		buttons[i].fsState = TBSTATE_ENABLED;
		buttons[i].fsStyle = (*it)->controlType;
		buttons[i].idCommand = (*it)->command;
		i++;
	}
	SendMessage(handle, TB_ADDBUTTONS, controls.size(), (LPARAM)buttons);
	delete[] buttons;
}

void gui::Toolbar::handleMsg(MSG * msg)
{
	if (msg->message == WM_SIZE)
		SendMessage(handle, TB_AUTOSIZE, 0, 0);
}

gui::ToolbarControls::ToolbarControls(int command, int bitmap, toolbarControlTypes tcs) : command(command), bitmap(bitmap), controlType(tcs) {}
