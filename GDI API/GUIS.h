#pragma once
#include <Windows.h>
#include <string>
#include <fstream>
namespace gui {
	namespace {
		std::string loadFromTextFileToWin32ControlTextFormat(char * file);
	}
	struct Resource {
		std::string data;
		int size;
		~Resource() { /*delete[] data;*/ }
	};
	class GUI {
	private:
		GUI() {};
		static HWND window;
		static int _width;
		static int _height;
	public:
		inline static void bindWindow(HWND wnd) { window = wnd; }
		inline static HWND useWindow() { return window; }
		inline static char * textFieldLoadFromFile(char * file) { return (char*)loadFromTextFileToWin32ControlTextFormat(file).c_str(); }
		static Resource loadResource(int file, int type, HMODULE handle = GetModuleHandle(NULL));
		inline static void setDimensions(int width, int height) {
			_width = width;
			_height = height;
		}
		inline static std::pair<int, int> getDimensions() {
			return std::make_pair(_width, _height);
		}
	};
}