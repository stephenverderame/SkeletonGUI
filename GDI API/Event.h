#pragma once
#include <windows.h>
class EventParams {
private:
	LPARAM l;
	WPARAM w;
public:
	EventParams() :l(0), w(0) {};
	EventParams(short param1, short param2, unsigned int param3) : l(MAKEWORD(param1, param2)), w(param3) {};
	EventParams(LPARAM l) : l(l), w(0) {};
	EventParams(LPARAM l, WPARAM w) : l(l), w(w) {};
	operator LPARAM() { return l; }
	operator WPARAM() { return w; }
	short getParam1() { return LOWORD(l); }
	short getParam2() { return HIWORD(l); }
	long getParaml() { return l; }
	unsigned int getParam3() { return w; }
	void setParam1(short x) { l = MAKEWORD(x, HIWORD(l)); }
	void setParam2(short y) { l = MAKEWORD(LOWORD(l), y); }
	void setParam3(unsigned int z) { w = z; }
};
class Event {
private:
	UINT msg;
	EventParams param;
public:
	Event(UINT eventType) : msg(eventType) {}
	Event(UINT eventType, EventParams param) : msg(eventType), param(param) {}
	UINT getEventType() { return msg; }
	EventParams getParams() { return param; }
	operator UINT() { return msg; }
	MSG toMsg();
};
class EventListener {
private:
	void(*function)(EventParams);
	UINT message;
public:
	EventListener() {};
	EventListener(void(*f)(EventParams), Event e) : function(f), message(e) {};
	void setCallback(void(*func)(EventParams), Event e);
	bool operator==(EventListener & other) { return message == other.message && function == other.function; };
	int operator()(Event msg, EventParams ep);
};