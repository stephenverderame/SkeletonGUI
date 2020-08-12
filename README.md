# SkeletonGUI
A simple wrapper library of the Win32 API to simplify creation of GUI's for Windows in C++. 
## General Overview:
This GUI library is based off a `MainPage` - `Page` - `Control` composition heirarchy. Every program needs a main page, which controls the navigation between pages. Controls added to pages are to be pointers and
the page will acquire the pointer resource and handle deleting the pointer when it is done. The same goes for pages and main pages. This library was initially designed to use raw pointers (i know, i know)

`MainPage`
* `addPage()`
* `getCurrentPage()`
* `navigateTo()`
* `handleMsg()` - send messages to Pages
* `GUI_CLEANUP()` - macro that should be called to cleanup the GUI library. Deletes the mainpage, which will delete all the child pages and child controls

`Page`
* `setLayout()` - sets the Page layout, which controls placement and resizing
* `addControl()` - adds controls, each Control has a string identifier which is stored in the Page hashmap
* `showPage()`
* `getControl()` - queries control hashmap based on string identifier of each control

`Control` - wrapper of a HWND
* ie. Button, Progressbar, TextField etc.

`Layout`
* `addControl()` - called when adding a control to a page
* `resize()` - handles resizing of page

A second main composition hierarchy is a `Window` - `Canvas` - `Drawable` composition hierarchy. This hierarchy is not neeeded. However every program does need a `Window` which can hold
both a `Canvas` and `MainPage`. A canvas is for more complex controls that have to be custom drawn such as a `BasicList`. This composition heirachy is a wrapper around the Gdi library

`Canvas`
* `addImage()`
* `addDrawable()`
* `draw()`
* `enableDoubleBuffering()`

`Bitmap`
* `setPixel()`
* `saveBmp()`
* `readBmp()`

`Window`
* `use()` - sets this as the current Window (needed for internal code)
* `addEventListener()`
* `fireEvent()`
* `getCanvas()`
* `setStyle()`
* `createWindow()` - call to actually create the window
* `show()`
* also, the window must be bound to the GUI with `GUI::bindWindow()`

`Dialog`
* `openSaveFileDialog()` - dialog to save a file
* `openOpenFileDialog()` - dialog to open a file

`Event`
* wrapper around native Win32 window events

Examples:
From CubeWorldProject
```C++
gui::GUI::bindWindow(game->getHwnd());
gui::TextField * commandBox = new gui::TextField("cmd", length * 0.005, height - (height * 0.01), length * 0.2, 40, ES_LOWERCASE | WS_THICKFRAME | WS_TABSTOP);
gui::TextField * displayBox = new gui::TextField("display", length * 0.005, height - (height * 0.2), length * 0.2, height * 0.15, ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | WS_BORDER);
gui::Page * page = new gui::Page("p1", { commandBox, displayBox });
mainPage = new gui::MainPage({ page, new gui::Page("emptyPage") });
mainPage->navigateTo("emptyPage");

//Remember to handle page messages in the main loop or message handler code
MSG msg;
if (PeekMessage(&msg, game->getHwnd(), 0, 0, PM_REMOVE)) {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
	mainPage->handleMessage(&msg);
}
```

Furthermore, there is also a `GLWindow` and `GLControl`. The GLWindow is an OpenGL enabled window and the GLControl is a custom control with a custom image
*Note this library was developed prior to gaining a better understanding of good programming practices and design patterns

