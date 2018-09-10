/*
 * Aggregator.cpp
 *
 *  Created on: Oct 2, 2013
 *      Author: zivlit
 */

#include "../include/Aggregator.h"
using namespace std;

Aggregator::Aggregator(OutputManager* egress) : _qh(0), _egress(egress) {}

Aggregator::~Aggregator() {}

void Aggregator::setQueue(struct nfq_q_handle *qh) {
	_qh = qh;
}

// TODO : Setting up delay
void Aggregator::setDelay(struct timeval output_ts) {
	struct timeval curr_time, delay_time;
	memset(&delay_time, 0, sizeof(struct timeval));
	gettimeofday(&curr_time,NULL);
	if (timercmp(&output_ts, &curr_time, >) > 0) {
		//delay the packet for the difference between output_ts and curr_time
		timersub(&output_ts, &curr_time, &delay_time);
		select(0, 0, 0, 0, &delay_time);
	}
}

void Aggregator::printTS(struct timeval ts) {
	std::cout << ts.tv_sec << " sec, "
			<< ts.tv_usec/1000 << "milisec, "
			<< ts.tv_usec << " microsec";
}

void Aggregator::run() {
	struct pkt_data* pkt;

	while (1) {
		std::cout << "Agg: entering . . . " << endl << endl;
		_egress->_emptyQ.wait();
		if (!_egress->isEmpty("output")) {
			_egress->_mutex1.wait();
			pkt = _egress->remove("output");
			_egress->_mutex1.set();
//			setDelay(pkt->_outTS);
			std::cout << "Agg: packet " << pkt->_id << " is sending now" << endl;	// printTS(pkt->_outTS);
			std::cout << endl << endl;
			nfq_set_verdict(_qh, pkt->_id, NF_ACCEPT, 0, NULL);
		}

		_egress->_mutex2.wait();
		while (!_egress->isEmpty("reorder")) {
			pkt = _egress->remove("reorder");
//			setDelay(pkt->_outTS);
			std::cout << "Agg: packet " << pkt->_id << " is sending now" << endl;	// printTS(pkt->_outTS);
			std::cout << endl << endl;
			nfq_set_verdict(_qh, pkt->_id, NF_ACCEPT, 0, NULL);
		}
		_egress->_mutex2.set();
	}
}
