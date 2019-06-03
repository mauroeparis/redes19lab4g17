packet Packet {
    int source;
    int destination;
    int hopCount;
	bool isFeedback;
}

packet Feedback extends Packet{
	int feedbackHopCount;
	int route[];
	
}


packet hello {
 	int destination;  
}