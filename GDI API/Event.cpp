#include "Event.h"
void EventListener::setCallback(void(*func)(EventParams), Event eventType)
{
	function = func;
	message = eventType;
}
int EventListener::operator()(Event msg, EventParams ep)
{
	if (msg.getEventType() == message) {
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
