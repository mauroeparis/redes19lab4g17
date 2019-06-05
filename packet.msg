cplusplus {{
#include <list>
typedef std::list<int> IntList;
}}

class noncobject IntList;

packet Packet {
    int source;
    int destination;
    int hopCount = 0;
	bool isFeedback = false;
	bool isPathFinder = false;
	bool isApp = false;
	IntList route;
	IntList backRoute;
}

packet Feedback extends Packet {
	int feedbackHopCount;
	IntList feedbackRoute;
}

packet PathFinder extends Packet {
    int sourceGate;
}
