/*
 * Aggregator.h
 *
 *  Created on: Oct 2, 2013
 *      Author: zivlit
 */

#ifndef AGGREGATOR_H_
#define AGGREGATOR_H_

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

#include <string>
#include <deque>

#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>		/* for NF_ACCEPT */
#include <libnetfilter_queue/libnetfilter_queue.h>

#include "Poco/Exception.h"
#include "Poco/Runnable.h"
#include "Poco/Timer.h"
#include "Poco/Thread.h"

#include "../include/OutputManager.h"

using Poco::Timer;
using Poco::TimerCallback;
using namespace std;

class Aggregator : public Poco::Runnable {

	public:

		Aggregator(OutputManager* egress);
		~Aggregator();

		void setQueue(struct nfq_q_handle *qh);
		void setDelay(struct timeval output_ts);
		void printTS(struct timeval ts);
		void run();

	private:
		struct nfq_q_handle *_qh;
		OutputManager* _egress;

};

#endif /* AGGREGATOR_H_ */
