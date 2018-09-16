/*
 * Rule.cpp
 *
 *  Created on: Sep 6, 2013
 *      Author: zivlit
 */

#include "../include/Rule.h"
using namespace std;

Rule::Rule(struct LinkProperties link, struct Metrics metrics) :
		_qh(0), _reorderQ(), _egress(new OutputManager()), _maxBWcounterSize(0) {
	timerclear(&_maxBWlastTS);

	_link._flowID = link._flowID;
	_link._chain = link._chain;
	_link._protocol = link._protocol;
//	_link._src = link._src;
//	_link._dst = link._dst;
	_link._dstHost = link._dstHost;
	_link._dstPort = link._dstPort;
	_link._srcHost = link._srcHost;
	_link._srcPort = link._srcPort;

	_metrices._maxThroughput = metrics._maxThroughput;
	_metrices._delay = metrics._delay;
	_metrices._loss_ratio = metrics._loss_ratio;
	_metrices._reorder_ratio = metrics._reorder_ratio;
}

Rule::~Rule(){
	if (_qh != 0) {
		destroy();
		this->clear();
	}
}

void Rule::clear() {
	delete _qh;
	_egress->~OutputManager();
}

/**
 * copy link properties structure
 * @param struct LinkProperties
 */
void Rule::setLinkProperties(struct LinkProperties link) {
	_link._flowID = link._flowID;
	_link._chain = link._chain;
	_link._protocol = link._protocol;
//	_link._src = link._src;
//	_link._dst = link._dst;
	_link._dstHost = link._dstHost;
	_link._dstPort = link._dstPort;
	_link._srcHost = link._srcHost;
	_link._srcPort = link._srcPort;
}

/**
 * copy metrics structure
 * @param struct Metrics
 */
void Rule::setMetrics(struct Metrics metrics) {
	_metrices._maxThroughput = metrics._maxThroughput;
	_metrices._delay = metrics._delay;
	_metrices._loss_ratio = metrics._loss_ratio;
	_metrices._reorder_ratio = metrics._reorder_ratio;
}

int Rule::getFlow() {
	return _link._flowID;
}

/**
 * construct dedicated queue
 * @param struct nfq_handle* netlink queue
 */
void Rule::create(struct nfq_handle* handler) {
	std::cout << "binding this socket to queue " << _link._flowID << endl << endl;
	if ((_qh = nfq_create_queue(handler, _link._flowID, &task, this)) < 0) { // the bug is here
		std::cout << "error during nfq_create_queue()" << endl << endl;
		throw new Poco::Exception("A queue handle can't create");
	}

	std::cout << "setting copy_packet mode" << endl << endl;
	if (nfq_set_mode(_qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		std::cout << "can't set packet_copy mode " << endl << endl;
		nfq_destroy_queue(_qh);
		throw new Poco::Exception("can't set packet_copy mode");
	}

	std::cout << "register this rule into iptables " << _link._flowID << endl << endl;
	registerRule("A");
	_egress->start(_qh);
}

/**
 * Register a rule into Iptable, using system command
 *
 * @param string desirable action - Append or Delete
 */
void Rule::registerRule(std::string action) {
	std::ostringstream command, tmp;

	command << "iptables"
			<< " -" << action << (_link._chain.empty() ? " FORWARD" : " ") << _link._chain
			<< (_link._protocol.empty() ? "" : " -p ") << _link._protocol;

	command << (_link._srcHost.empty() ? "" : " -s ") << _link._srcHost;
	tmp << " --sport " << _link._srcPort;
	command	<< (_link._srcPort>0 ? tmp.str().c_str() : "");

	command << (_link._dstHost.empty() ? "" : " -d ") << _link._dstHost;
	tmp << " --dport " << _link._dstPort;
	command	<< (_link._dstPort>0 ? tmp.str().c_str() : "");

/*	command << (_link._src.host().toString().empty() ? "" : " -s ") << _link._src.host().toString();
	tmp1 << " --sport " << _link._src.port();
	command	<< (_link._src.port()>0 ? tmp1.str().c_str() : "");

	command << (_link._dst.host().toString().empty() ? "" : " -s ") << _link._dst.host().toString();
	tmp1 << " --dport " << _link._dst.port();
	command	<< (_link._dst.port()>0 ? tmp1.str().c_str() : "");	*/

	command << " -j NFQUEUE"
			<< " --queue-num " << _link._flowID;

	std::cout << command.str() << endl << endl;
	system(command.str().c_str());
	sleep(10);

//	sudo iptables -A OUTPUT -p icmp -j NFQUEUE --queue-num 0
//	sudo iptables -A INPUT -p tcp -s localhost -d localhost -dport 4444 -j NFQUEUE --queue-num 1
//	sudo iptables -A OUTPUT -p icmp -d 212.199.219.242 -j NFQUEUE --queue-num 0
//	sudo iptables -A INPUT -p tcp -s localhost -d localhost --dport 4444 -j NFQUEUE --queue-num 1
}

/**
 * destroy the proper queue and evacuate associated queues
 */
void Rule::destroy() {
	std::cout << "Unregister this rule into iptables " << _link._flowID << endl << endl;
	registerRule("D");
	_egress->stop();

	struct pkt_data *pkt;
	while (!_reorderQ.empty()){
		pkt = _reorderQ.front();
		nfq_set_verdict(_qh, pkt->_id, NF_ACCEPT, 0, NULL);
		_reorderQ.pop_front();
		free(pkt);
	}
	_reorderQ.clear();

	std::cout << "unbinding from queue " << 0 << endl << endl;
	nfq_destroy_queue(this->_qh);
}

// callback function
int Rule::task(struct nfq_q_handle *qh,
				  struct nfgenmsg *nfmsg,
				  struct nfq_data *nfa,
				  void *data)
{
	Rule* rule = (Rule*)data;
	return rule->_task(qh,nfmsg,nfa,data);
}

int Rule::_task(struct nfq_q_handle *qh,
				  struct nfgenmsg *nfmsg,
				  struct nfq_data *nfa,
				  void *data)
{
	u_int32_t id, verdict;
//	std::cout << "Rule: entering callback, Flow: " << _link._flowID << endl << endl;

//	struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(nfa);
//	id = ntohl(ph->packet_id);
//	id = print_pkt(nfa);
	id = handle_pkt(nfa, &verdict);
//	std::cout << "Rule: packet " << id << " is " << verdict << endl << endl;
	return nfq_set_verdict(qh, id, verdict, 0, NULL);
//	ans = nfq_set_verdict(qh, id, NF_ACCEPT, 0, NULL);
}

/**
 * manipulating the packet
 *
 * @param struct nfq_data* payload of packet
 * @param u_int32_t* pointer to verdict
 * @return u_int32_t ID assigned to packet by netfilter
 */
u_int32_t Rule::handle_pkt(struct nfq_data *tb, u_int32_t *verdict) {
//	u_int32_t id;
	unsigned char *data;
	struct pkt_data *pkt = new struct pkt_data;
	struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(tb);
	pkt->_id = ntohl(ph->packet_id);
	// std::cout << "Rule: entering callback, Flow: " << _link._flowID << ", packet: " << pkt->_id << endl << endl;

	// TODO : define delay
	gettimeofday(&(pkt->_inTS), NULL);
//	int ansTS = nfq_get_timestamp(tb, &(pkt->_inTS));	// TODO : retrieve timestamp
//	std::cout << "Rule: packet " << pkt->_id << " : result " << ansTS << endl;
//	std::cout << "Rule: packet " << pkt->_id << " came at "; _egress->printTS(pkt->_outTS);
	(pkt->_outTS).tv_sec = pkt->_inTS.tv_sec;
	(pkt->_outTS).tv_usec = pkt->_inTS.tv_usec;

	std::cout << "Rule: entering callback"
			<< ", Flow: " << _link._flowID
			<< ", packet " << pkt->_id
			<< " enter at "; _egress->printTS(pkt->_inTS);

	// packet loss
	if (isPacketLoss()) {
	//	std::cout << "Rule: packet " << pkt->_id << " is dropped" << endl << endl;
		*verdict = NF_DROP;
	}

	// Max throughput
	else if (isMaxLimitExceeded(&(pkt->_inTS), nfq_get_payload(tb, &data))) {
		std::cout << "Rule: packet " << pkt->_id << " is dropped" << endl << endl;
		*verdict = NF_DROP;
	}

	// packet reorder
	else if (isPacketReoder()) {
	//	std::cout << "Rule: packet " << pkt->_id << " is reordered" << endl << endl;
		_reorderQ.push_back(pkt); // LIFO order
		*verdict = NF_STOLEN;
	}

	// packet forward
	else {
	//	std::cout << "Rule: packet " << pkt->_id << " is queued now" << endl << endl;
		setDelay(pkt, NULL);
		while (!_reorderQ.empty()) {
			_egress->insert(_reorderQ.back()); // LIFO order
			setDelay(_reorderQ.back(), &(pkt->_outTS));
			_reorderQ.pop_back();
		}
		_egress->insert(pkt);
		*verdict = NF_STOLEN;
//		*verdict = NF_ACCEPT;
	}

//	return nfq_set_verdict(qh, id, verdict, 0, NULL);
	return pkt->_id;
}

/** Verdict methods can return the following values :
 * 0 if the packet is to be accepted without modification,
 * 1 if the packet is to be accepted with modification(s),
 * 2 if the packet is to be changed.
 */

// Method that decides whether a packet should be dropped
bool Rule::isPacketLoss() {
	if (rand()/(float)RAND_MAX < _metrices._loss_ratio)
		return true;
	return false;
}

// Method that decides whether a packet should be reorder
// TODO: add a limit of reorder queue - constant or defined by user
// TODO : enforce relative constraint
bool Rule::isPacketReoder() {
	if (_reorderQ.size() < 10 && rand()/(float)RAND_MAX < _metrices._reorder_ratio)
		return true;
	return false;
}

bool Rule::isMaxLimitExceeded(struct timeval *timestamp, size_t size) {
//	_egress->printTS(*timestamp);
	if (timercmp(timestamp, &_maxBWlastTS, >) > 0) {
		_maxBWcounterSize = 0;
		timerclear(&_maxBWlastTS);
		_maxBWlastTS.tv_sec = 1;
		timeradd(timestamp, &_maxBWlastTS, &_maxBWlastTS);
	}
//	std::cout << "size : " << size << endl;
	_maxBWcounterSize += size;
//	std::cout << "accSize : " << _maxBWcounterSize << endl;
	if (_maxBWcounterSize > _metrices._maxThroughput)
		return true;
	return false;
}

/**
 * define output timestamp including delay
 *
 * @param struct pkt_data* packet structure
 * @param struct timeval* new timestamp
 */
void Rule::setDelay(struct pkt_data *pkt, struct timeval *newOutTS) {
	if (newOutTS == NULL) {
		int tTSinUsec = (pkt->_outTS).tv_usec + int(_metrices._delay*1000L);
		(pkt->_outTS).tv_usec = tTSinUsec%1000000L;
		(pkt->_outTS).tv_sec += tTSinUsec/1000000L;
	}

	else {
		(pkt->_outTS).tv_sec = newOutTS->tv_sec;
		(pkt->_outTS).tv_usec = newOutTS->tv_usec;
	}
}

// For debugging
void Rule::printData() {
//	std::cout << "Print rule" << endl;
	std::cout << "Flow no. " << _link._flowID << endl
			<< "Chain: " << _link._chain << endl
			<< "Source: Host " << _link._srcHost << ", Port " << _link._srcPort << endl
			<< "Destination: Host " << _link._dstHost << ", Port " << _link._dstPort << endl
//			<< "Source: " << _link._src.toString() << endl
//			<< "Destination: " << _link._dst.toString() << endl
			<< "Protocol: " << _link._protocol << endl;

	std::cout << "Flow metrics:" << endl
			<< "Max Throughput: "<< _metrices._maxThroughput << " Bytes per second" << endl
			<< "Delay: " << _metrices._delay << " Millisecond" << endl
			<< "Packet Loss: " << _metrices._loss_ratio << endl
			<< "Reorder: " << _metrices._reorder_ratio << endl;

	std::cout << endl;
}

// returns packet id
u_int32_t Rule::print_pkt (struct nfq_data *tb) {
	int id = 0;
	struct nfqnl_msg_packet_hdr *ph;
	struct nfqnl_msg_packet_hw *hwph;
	u_int32_t mark,ifi;
	int ret;
	unsigned char *data;

	ph = nfq_get_msg_packet_hdr(tb);
	if (ph) {
		id = ntohl(ph->packet_id);
		std::cout << "hw_protocol=" << ntohs(ph->hw_protocol)
				<< " hook=" << ph->hook << "id=" << id << endl << endl;
	}

	hwph = nfq_get_packet_hw(tb);
	if (hwph) {
		int i, hlen = ntohs(hwph->hw_addrlen);

		for (i = 0; i < hlen-1; i++)
			std::cout << "hw_src_addr=" << hwph->hw_addr[i] << endl << endl;
		std::cout << "hw_src_addr=" << hwph->hw_addr[hlen-1] << endl << endl;
	}

	mark = nfq_get_nfmark(tb);
	if (mark)
		std::cout << "mark=" << mark << " ";

	ifi = nfq_get_indev(tb);
	if (ifi)
		std::cout << "indev=" << ifi << " ";

	ifi = nfq_get_outdev(tb);
	if (ifi)
		std::cout << "outdev=" << ifi << " ";

	ifi = nfq_get_physindev(tb);
	if (ifi)
		std::cout << "physindev=" << ifi << " ";

	ifi = nfq_get_physoutdev(tb);
	if (ifi)
		std::cout << "physoutdev=" << ifi << " ";

	ret = nfq_get_payload(tb, &data);
	if (ret >= 0)
		std::cout << "payload_len=" << ret << " ";

	std::cout << endl << endl;

	return id;
}
