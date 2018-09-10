/*
 * OutputManager.h
 *
 *  Created on: Oct 2, 2013
 *      Author: zivlit
 */

#ifndef OUTPUTMANAGER_H_
#define OUTPUTMANAGER_H_

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#include <vector>
#include <deque>

#include "Poco/Semaphore.h"

#include <libnetfilter_queue/libnetfilter_queue.h>

struct pkt_data {
	u_int32_t _id;
	struct timeval _inTS;
	struct timeval _outTS;
};

typedef std::deque<struct pkt_data*> PktQ;

class OutputManager {

	public:
		OutputManager();
		~OutputManager();
		void clear();

		void insert(struct pkt_data* pkt, std::string type);
		struct pkt_data* remove(std::string type);
		bool isEmpty(std::string type);

		Poco::Semaphore _mutex1;
		Poco::Semaphore _mutex2;
		Poco::Semaphore _emptyQ; // remane egQisEmpty

	private:
		PktQ _egressQ;
		PktQ _reorderQ;

};

#endif /* OUTPUTMANAGER_H_ */
