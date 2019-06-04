#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>

using namespace omnetpp;

class Net: public cSimpleModule {
private:
    map<int, int> routes;
public:
    Net();
    virtual ~Net();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Net);

#endif /* NET */

Net::Net() {
}

Net::~Net() {
}

void Net::initialize() {
}

void Net::finish() {
}

void Net::handleAppMessage(Packet *pkt) {
    if (routes.find(pkt->getDestination()) == routes.end()){ // Route not found
        for(int i=0;i <= this->getParentModule()->getlnk(); i = i + 1) {
            PathFinder *pFndr = (PathFinder *) pkt;
            pFndr->setSourceGate(i);
            send(pFndr, "toLnk$o", i);
        }
    } else {
        pkt->setRoute(routes.find(pkt->getDestination()));
        send(pkt, "toLnk$o", pkt->getRoute().pop_front());
    }
}

void Net::handlePathFinder(Packet *pFndr) {
    if (pFndr->getDestination() == this->getParentModule()->getIndex()) {
        Feedback *fbk = new Feedback;
        fbk->setSource(this->getParentModule()->getIndex());
        fbk->setDestination(pFndr->getSource());
        fbk->setFeedbackHopCount(pFndr->getHopCount());
        fbk->setIsFeedback(true);
        fbk->setRoute(pFndr->getRoute());
        fbk->setFeedbackRoute(pFndr->getRoute().reverse());
        fbk->setHopCount(0);
    } else {
        for(int i=0;i <= this->getParentModule()->getlnk(); i = i + 1) {
            if (i != pFndr.getSourceGate()) {
                PathFinder *pFndr = (PathFinder *) pkt;
                pFndr->setRoute(
                    pFndr->getRoute().push_back(i)
                );
                pFndr->setSourceGate(i);
                pFndr->setIsPathFinder(true);
                send(pFndr, "toLnk$o", i);
            }
        }
    }
}

void Net::handleFeedback(Packet *fbk) {
    if (fbk->getDestination() == this->getParentModule()->getIndex()) {
        // check if better route exists if not set this one
    } else {
        send(fbk, "toLnk$o", fbk->getFeedbackRoute().pop_front());
    }
}

void Net::handlePacket(Packet *pkt) {
    if (pkt->getDestination() == this->getParentModule()->getIndex()) {
        send(msg, "toApp$o"); // TODO msg??
    } else {
        send(pkt, "toLnk$o", pkt->getRoute().pop_front());
    }
}

void Net::handleMessage(cMessage *msg) {

    // All msg (events) on net are packets
    Packet *pkt = (Packet *) msg;

    if (pkt->getHopCount() == 0){
        this->handleAppMessage(pkt);
    } else if (pkt->getIsPathFinder()) {
        this->handlePathFinder(pkt);
    } else if (pkt->getIsFeedback()) {
        this->handleFeedback(pkt);
    } else {
        this->handlePacket(pkt);
    }
}
