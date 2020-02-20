#include "Event.h"
void EventListener::setCallback(std::function<void(EventParams)> func, Event eventType)
{
	function = func;
	message = eventType;
}
void EventListener::setCallback2(std::function<int(EventParams)> func, Event eventType)
{
	function2 = func;
	message = eventType;
}
int EventListener::operator()(Event msg, EventParams ep)
{
	if (msg.getEventType() == message) {
		if (function2) return function2(ep);
		function(ep);
		return 1;
	}
	return 0;
}

MSG Event::toMsg()
{
	MSG ev;
	ev.message = msg;
	ev.lParam = param.getParaml();
	ev.wParam = param.getParam3();
	return ev;
}
