#pragma once
#include <Windows.h>
#include <string>
class Drawable {
public:
	virtual void draw(HDC hdc) = 0;
	virtual void eventHandle(UINT msg, WPARAM w, LPARAM l) = 0;
//	virtual std::string getId() = 0;
};