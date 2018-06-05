#include "Layout.h"
Dimensions AbsoluteLayout::resize(LPARAM l, Dimensions dim) {
	int width = LOWORD(l);
	int height = HIWORD(l);
	Dimensions newDim = dim;
	Dimensions wndDimensions = Window::getBoundWindow()->getOriginalDimensions();
	if (props & abs_posProportional) {
		float newX = (dim.x * width) / (float)wndDimensions.width;
		float newY = (dim.y * height) / (float)wndDimensions.height;
		newDim.x = round(newX);
		newDim.y = round(newY);
	}
	if (props & abs_sizeProportional) {
		float newLength = (dim.width * width) / (float)wndDimensions.width;
		float newHeight = (dim.height * height) / (float)wndDimensions.height;
		newDim.width = round(newLength);
		newDim.height = round(newHeight);

	}
	return newDim;
}

Dimensions AbsoluteLayout::addControl(Dimensions dim)
{
	Dimensions wndDimensions = Window::getBoundWindow()->getOriginalDimensions();
	Dimensions newDim = dim;
	if (props & abs_posProportional) {
		float newX = (dim.x / 10000.0) * wndDimensions.width;
		float newY = (dim.y / 10000.0) * wndDimensions.height;
		newDim.x = round(newX);
		newDim.y = round(newY);

	}
	if (props & abs_sizeProportional) {
		float newW = (dim.width / 10000.0) * wndDimensions.width;
		float newH = (dim.height / 10000.0) * wndDimensions.height;
		newDim.width = round(newW);
		newDim.height = round(newH);
	}
	return newDim;
}

BOOL StackLayout::enumChildProc(HWND hwnd, LPARAM lp)
{
	unsigned int address = (unsigned int)lp;
	StackLayout * layout = (StackLayout*)address;
	RECT dimensions;
	GetWindowRect(hwnd, &dimensions);
	Dimensions dim = { dimensions.left, dimensions.top, dimensions.right - dimensions.left, dimensions.bottom - dimensions.top };
	Dimensions wndDimensions = Window::getBoundWindow()->getDimensions();

	if (layout->stackPointer == 0 && layout->props < 5) layout->stackPointer = wndDimensions.height - dim.y;
	else if (layout->stackPointer == 0 && layout->props >= 5) layout->stackPointer = wndDimensions.width - dim.x;

	switch (layout->props) {
	case st_center:
	{
		dim.x = (wndDimensions.width / 2.0) - (layout->maxWidth / 2.0);
		dim.y = layout->stackPointer;
		layout->stackPointer += dim.height + layout->spacing;
		break;
	}
	case st_left:
	{
		dim.x = layout->spacing;
		dim.y = layout->stackPointer;
		layout->stackPointer += dim.height + layout->spacing;
		break;
	}
	case st_right:
	{
		dim.x = wndDimensions.width - layout->spacing - layout->maxWidth;
		dim.y = layout->stackPointer;
		layout->stackPointer += dim.height + layout->spacing;
		break;
	}
	case st_middle:
	{
		dim.y = (wndDimensions.height / 2.0) - (layout->maxHeight / 2.0);
		dim.x = layout->stackPointer;
		layout->stackPointer += dim.width + layout->spacing;
		break;
	}
	case st_top:
	{
		dim.y = layout->spacing;
		dim.x = layout->stackPointer;
		layout->stackPointer += dim.width + layout->spacing;
		break;
	}
	case st_bottom:
	{
		dim.y = wndDimensions.height - layout->maxHeight - layout->spacing;
		dim.x = layout->stackPointer;
		layout->stackPointer += dim.width + layout->spacing;
		break;
	}
	}
	MoveWindow(hwnd, dim.x, dim.y, dim.width, dim.height, TRUE);
	return TRUE;
}

BOOL StackLayout::enumMax(HWND hwnd, LPARAM lp)
{
	RECT dim;
	GetWindowRect(hwnd, &dim);
	unsigned int address = (unsigned int)lp;
	StackLayout * layout = (StackLayout*)address;
	layout->maxWidth = max(layout->maxWidth, dim.right - dim.left);
	layout->maxHeight = max(layout->maxHeight, dim.right - dim.left);
	return TRUE;
}

StackLayout::StackLayout(stackLayoutProperties p, HWND window) : props(p), maxWidth(INT_MIN), maxHeight(INT_MIN)
{
	formatCurrentControls(window);
}

Dimensions StackLayout::resize(LPARAM size, Dimensions originalDim)
{
	return{ 0, 0, 0, 0 };
}

Dimensions StackLayout::addControl(Dimensions dim)
{
	if (stackPointer == 0 && props < 5) stackPointer = dim.y;
	else if (stackPointer == 0) stackPointer = dim.x;
	int oldMWidth = maxWidth;
	int oldMHeight = maxHeight;
	maxWidth = max(dim.width, maxWidth);
	maxHeight = max(dim.height, maxHeight);
	Dimensions wndDimensions = Window::getBoundWindow()->getDimensions();
	switch (props) {
	case st_center:
	{
		float x = (wndDimensions.width / 2.0) - (maxWidth / 2.0);
		float y = stackPointer;
		stackPointer += dim.height + spacing;
		return Dimensions{ round(x), round(y), dim.width, dim.height };
	}
	case st_left:
	{
		float x = spacing;
		float y = stackPointer;
		stackPointer += dim.height + spacing;
		return Dimensions{ round(x), round(y), dim.width, dim.height };

	}
	case st_right:
	{
		float x = wndDimensions.width - spacing - maxWidth;
		float y = stackPointer;
		stackPointer += dim.height + spacing;
		return Dimensions{ round(x), round(y), dim.width, dim.height };
	}
	case st_middle:
	{
		float y = (wndDimensions.height / 2.0) - (maxHeight / 2.0);
		float x = stackPointer;
		stackPointer += dim.width + spacing;
		return Dimensions{ round(x), round(y), dim.width, dim.height };
	}
	case st_top:
	{
		float y = spacing;
		float x = stackPointer;
		stackPointer += dim.width + spacing;
		return Dimensions{ round(x), round(y), dim.width, dim.height };
	}
	case st_bottom:
	{
		float y = wndDimensions.height - maxHeight - spacing;
		float x = stackPointer;
		stackPointer += dim.width + spacing;
		return Dimensions{ round(x), round(y), dim.width, dim.height };
	}
	}
}

void StackLayout::formatCurrentControls(HWND window)
{
	stackPointer = 0;
	unsigned int address = (unsigned int)this;
	EnumChildWindows(window, enumMax, address);
	EnumChildWindows(window, enumChildProc, address);
}
