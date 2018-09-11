/*
 * OutputManager.h
 *
 *  Created on: Oct 2, 2013
 *      Author: zivlit
 */

#ifndef OUTPUTMANAGER_H_
#define OUTPUTMANAGER_H_

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

#include <string>
#include <queue>

#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>		/* for NF_ACCEPT */
#include <libnetfilter_queue/libnetfilter_queue.h>

#include "Poco/Semaphore.h"
#include "Poco/Runnable.h"
#include "Poco/Thread.h"

using namespace std;

struct pkt_data {
	u_int32_t _id;
	struct timeval _inTS;
	struct timeval _outTS;
};

typedef std::queue<struct pkt_data*> PktQ;

class OutputManager : public Poco::Runnable {

	public:
		OutputManager();
		~OutputManager();

		void start(struct nfq_q_handle *qh);
		void run();

		void insert(struct pkt_data* pkt);
		struct pkt_data* remove();

		void printTS(struct timeval ts);

	private:
		void clear();
		void delay(struct timeval output_ts);

		int n;
		Poco::Semaphore _QRdWrMutex;
		Poco::Semaphore _QisEmpty;

		struct nfq_q_handle *_qh;
		PktQ _egressQ;
		Poco::Thread _thread;

};

#endif /* OUTPUTMANAGER_H_ */
