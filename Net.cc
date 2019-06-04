#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>

using namespace omnetpp;

class Net: public cSimpleModule {
private:
    cQueue buffer;
    cMessage *endServiceEvent;
    std::map<int, std::list<int>> routes;
public:
    Net();
    virtual ~Net();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);

    virtual void handleAppMessage(Packet *pkt);
    virtual void handleFeedback(Packet *pkt);
    virtual void handlePathFinder(Packet *pkt);
    virtual void handlePacket(Packet *pkt);
};

Define_Module(Net);

#endif /* NET */

Net::Net() {
}

Net::~Net() {
}

void Net::initialize() {
    endServiceEvent = new cMessage("endService");
}

void Net::finish() {
}

void Net::handleAppMessage(Packet *pkt) {
    pkt->setIsApp(false);

    if (this->routes.find(pkt->getDestination()) == this->routes.end()){
        // Route not found
        int interfaces = this->getParentModule()->par("interfaces");
        for(int i=0;i < interfaces; i = i + 1) {
            PathFinder *pFndr = new PathFinder("pathFinder",this->getParentModule()->getIndex());
            pFndr->setIsPathFinder(true);
            pFndr->setDestination(pkt->getDestination());
            pFndr->setSource(pkt->getSource());
            send(pFndr, "toLnk$o", i);
            this->bubble("IF");
        }

        buffer.insert(pkt);
    } else {
        this->bubble("ELSE");
        // TODO: esto anda?
        std::list<int> r = this->routes.find(pkt->getDestination())->second;

        int interface = r.front();
        r.pop_front();

        pkt->setRoute(r);

        send(pkt, "toLnk$o", interface);
    }
}

void Net::handlePathFinder(Packet *pkt) {
    PathFinder *pFndr = (PathFinder *) pkt;

    if (pFndr->getDestination() == this->getParentModule()->getIndex()) {
        this->bubble("Arrived!");
        Feedback *fbk = new Feedback("feedback", pFndr->getKind());
        fbk->setSource(this->getParentModule()->getIndex());
        fbk->setDestination(pFndr->getSource());
        fbk->setFeedbackHopCount(pFndr->getHopCount());
        fbk->setIsFeedback(true);
        fbk->setFeedbackRoute(pFndr->getRoute());
        fbk->setRoute(pFndr->getBackRoute());
        send(fbk, "toLnk$o", pFndr->getSourceGate());
        delete pFndr;
    } else {
        int interfaces = this->getParentModule()->par("interfaces");
        for(int i=0;i < interfaces; i = i + 1) {
            if (i != pFndr->getSourceGate()) {
                PathFinder *pFdup = pFndr->dup();

                this->bubble("I'm different");
                std::list<int> r = pFndr->getRoute();
                r.push_back(i);
                pFdup->setRoute(r);

                std::list<int> br = pFndr->getBackRoute();
                br.push_back(pFdup->getSourceGate());
                pFdup->setBackRoute(br);

                pFdup->setSourceGate(i);
                pFdup->setIsPathFinder(true);
                send(pFdup, "toLnk$o", i);
            }
        }
    }
}

void Net::handleFeedback(Packet *pkt) {
    Feedback *fbk = (Feedback *) pkt;

    if (fbk->getDestination() == this->getParentModule()->getIndex()) {
        // check if better route exists if not set this one
        int dest = pkt->getDestination();

        if (this->routes.find(dest) == this->routes.end()) {
            this->routes.insert({fbk->getSource(), fbk->getFeedbackRoute()});
        } //TODO: else

        delete fbk;
        if (!endServiceEvent->isScheduled()) {
            // start the service
            scheduleAt(simTime() + 0, endServiceEvent);
        }

    } else {
        std::list<int> r = fbk->getRoute();

        int interface = r.front();
        r.pop_front();
        fbk->setRoute(r);

        send(fbk, "toLnk$o", interface);
    }
}

void Net::handlePacket(Packet *pkt) {
    if (pkt->getDestination() == this->getParentModule()->getIndex()) {
        send(pkt, "toApp$o");
    } else {
        std::list<int> r = pkt->getRoute();

        int interface = r.front();
        r.pop_front();

        send(pkt, "toLnk$o", interface);
    }
}

void Net::handleMessage(cMessage *msg) {
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            Packet* pkt = (Packet*) buffer.pop();

            std::list<int> r = this->routes.find(pkt->getDestination())->second;
            int interface = r.front();
            r.pop_front();
            pkt->setRoute(r);

            // send
            send(pkt, "toLnk$o", interface);

            // start new service
            simtime_t serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
    } else {
        // All msg (events) on net are packets
        Packet *pkt = (Packet *) msg;

        if (pkt->getIsApp()){
            this->bubble("APP");
            this->handleAppMessage(pkt);
        } else if (pkt->getIsPathFinder()) {
            this->bubble("PATH");
            this->handlePathFinder(pkt);
        } else if (pkt->getIsFeedback()) {
            this->bubble("FEED");
            this->handleFeedback(pkt);
        } else {
            this->bubble("PKT");
            this->handlePacket(pkt);
        }
    }
}
