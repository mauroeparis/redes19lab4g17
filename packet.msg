packet Packet {
    int source;
    int destination;
    int hopCount;
	bool isFeedback;
	bool isPathFinder;
	list <int> route;
}

packet Feedback extends Packet{
	int feedbackHopCount;
}

packet PathFinder extends Packet {
    int sourceGate;
}