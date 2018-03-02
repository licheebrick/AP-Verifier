#include "ap_verifier.h"
#include <assert.h>
#include <set>
#include <json/json.h>
#include <sys/time.h>
#include <algorithm>

using namespace std;
using namespace log4cxx;

APVerifier::APVerifier(int length, AP_TYPE type) {
    this->hdr_len = length;
    this->ap_type = type;
}

APVerifier::~APVerifier() {
    // clear topology
    std::map< uint64_t, std::vector<uint64_t>* >::iterator tpit;
    for (tpit = topology.begin(); tpit != topology.end(); tpit++) {
        if (tpit->second != NULL) {
            delete (*tpit).second;
            tpit->second = NULL;
        }
    }
    topology.clear();

    // clear id_to_router
    std::map< uint32_t, Router* >::iterator rit;
    for (rit = id_to_router.begin(); rit != id_to_router.end(); rit++) {
        if (rit->second != NULL) {
            delete rit->second;
            rit->second = NULL;
        }
    }
    id_to_router.clear();

    // clear ap_bdd_list
    std::vector< bdd > tmp;
    ap_bdd_list->swap(tmp);
    delete ap_bdd_list;
}

void APVerifier::add_link(uint64_t from_port, uint64_t to_port) {
    if (topology.count(from_port) == 0) {
        topology[from_port] = new std::vector<uint64_t>(1, to_port);
    } else {
        topology[from_port]->push_back(to_port);
    }
}

string APVerifier::topology_to_string() {
    std::map< uint64_t, std::vector<uint64_t>* >::iterator it;
    stringstream result;
    char buf[40];
    result << "Now printing the topology...\n";
    for (it = topology.begin(); it != topology.end(); it++) {
        sprintf(buf, "%lu --> ( ", it->first);
        result << buf;
        for (size_t i = 0; i < (*it).second->size(); i++) {
            sprintf(buf, "%lu ", it->second->at(i));
            result << buf;
        }
        result << ")\n";
    }
    return result.str();
}

void APVerifier::print_topology() {
    std::map< uint64_t, std::vector<uint64_t>* >::iterator it;
    printf("Now printing the topology...\n");
    for (it = topology.begin(); it != topology.end(); it++) {
        printf("%lu --> ( ", (*it).first);
        for (size_t i = 0; i < (*it).second->size(); i++) {
            printf("%lu ", (*it).second->at(i));
        }
        printf(")\n");
    }
}

long APVerifier::add_then_load_router(uint32_t router_id, Json::Value *root) {
    struct timeval start, end;
    long run_time = 0;

    if (id_to_router.count(router_id) == 0 && router_id > 0) {
        Router* router = new Router(router_id);
        id_to_router[router_id] = router;
        Json::Value rules = (*root)["rules"];
        int rw_rule_count = 0;
        gettimeofday(&start, NULL);
        for (uint32_t id = rules.size(); id != 0; id--) {
            uint32_t i = id - 1;
            string action = rules[i]["action"].asString();
            if (action =="fwd") {
                string match = rules[i]["match"].asString();
                // assert(match.length() == length);
                List_t in_ports;
                in_ports = val_to_list(rules[i]["in_ports"]);
                for (uint32_t idx = 0; idx < in_ports.size; idx++) {
                    uint64_t inport = in_ports.list[idx];

                    // add to inport_to_router
                    if (this->inport_to_router.count(inport) == 0) {
                        this->inport_to_router.insert(make_pair(inport, router_id));
                    } else {
                        if (this->inport_to_router[inport] != router_id) {
                            stringstream err_msg;
                            err_msg << "Wrong in configuration: inport " << to_string(inport) <<
                                    ", mapped to multiple router: " << to_string(router_id) << " and " <<
                                    to_string(inport_to_router[inport]) << ".";
                            LOG4CXX_ERROR(rlogger, err_msg.str());
                        }
                    }

                    // add predicate
                    //TODO: remove predicate with false BDD
                    if (router->predicate_map.count(inport) == 0) {
                        // We have never encountered any rule on this inport before;
                        PredicateNode* predicate_node = new PredicateNode(inport, match, FWD, this->hdr_len);
                        predicate_node->out_ports = val_to_list(rules[i]["out_ports"]);
                        map<Json::Value, PredicateNode*>* port_map = new map<Json::Value, PredicateNode*>;
                        port_map->insert(make_pair(rules[i]["out_ports"], predicate_node));
                        router->predicate_map.insert(make_pair(inport, port_map));
                        router->dealt_bdd_map[inport] = bdd_false();
                        router->dealt_bdd_map[inport] = router->dealt_bdd_map[inport] | predicate_node->predicate;
                    } else {
                        if (router->predicate_map[inport]->count(rules[i]["out_ports"]) == 0) {
                            // We encountered rule on this inport, but never this action (outports)
                            // So we need to subtract the already dealt bdd
                            PredicateNode* predicate_node = new PredicateNode(inport, match, FWD, this->hdr_len);
                            predicate_node->out_ports = val_to_list(rules[i]["out_ports"]);
                            predicate_node->predicate -= router->dealt_bdd_map[inport];
                            router->dealt_bdd_map[inport] |= predicate_node->predicate;
                            router->predicate_map[inport]->insert(make_pair(rules[i]["out_ports"], predicate_node));
                        } else {
                            // We encountered rule on this inport with this action (outports)
                            PredicateNode* predicate_node = (router->predicate_map[inport])->at(rules[i]["out_ports"]);
                            bdd new_add = match2bdd(match, this->hdr_len) - router->dealt_bdd_map[inport];
                            predicate_node->predicate |= new_add;
                            router->dealt_bdd_map[inport] |= new_add;
                        }
                    }
                }
            } else { // "rw"
                //TODO::ADD REWRITE IMPLEMENTATION
                rw_rule_count++;
            }
        }
        gettimeofday(&end, NULL);
        run_time = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
//        stringstream msg;
//        msg << "Finish load router " << to_string(router_id) << ", " << to_string(run_time) << " us used, total "
//            << to_string(rules.size()) << " rules, " << rw_rule_count << " rewrite rules exclude.";
//        LOG4CXX_INFO(rlogger, msg.str());
//        msg.str("");
//        msg << "This router has " << to_string(router->predicate_map.size()) << " inport predicates list, and each of them has [";
//        for (auto it = router->predicate_map.begin(); it != router->predicate_map.end(); it++) {
//            msg << to_string(it->second->size()) << ", ";
//        }
//        msg << "]";
//        LOG4CXX_INFO(rlogger, msg.str());

    } else if (router_id == 0) {
        LOG4CXX_ERROR(rlogger, "Cannot create table with ID 0.\n");
    } else {
        stringstream error_msg;
        error_msg << "Table " << to_string(router_id) << " already exist. Can't add it again.";
        LOG4CXX_ERROR(rlogger, error_msg.str());
    }
    if (show_detail) {
        this->id_to_router[router_id]->print_router();
    }
    return run_time;
}

void APVerifier::make_atomic_predicates() {
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);
    stringstream msg;
    msg << "Start make atomic predicates...";
    LOG4CXX_INFO(rlogger, msg.str());
    std::map< uint32_t, Router* >::iterator it;
    std::list< bdd > ap_list;
    bdd true_bdd = bdd_true();
    ap_list.push_back(true_bdd);
    for (it = id_to_router.begin(); it != id_to_router.end(); it++) { // it is router iterator
        std::map< uint64_t, std::map<Json::Value, PredicateNode*>* >::iterator port_it;
        msg.str("");
        msg << "Now dealing with router " << to_string(it->second->router_id);
        //LOG4CXX_INFO(rlogger, msg.str());
        for (port_it = (*it).second->predicate_map.begin(); port_it != (*it).second->predicate_map.end(); port_it++) {
            // port_it is port predicate map iterator
            std::map< Json::Value, PredicateNode* >::iterator pn_it;
            for (pn_it = (*port_it).second->begin(); pn_it != (*port_it).second->end(); pn_it++) {
                // pn_it is predicate node iterator
                bdd P = (*pn_it).second->predicate;
                if (P != bddfalse && P != bddtrue) {
                    uint64_t ori_size = ap_list.size();
                    for (uint64_t i = 0; i < ori_size; i++) {
                        bool del_flag = false;
                        bdd bdd_now = ap_list.back();
                        bdd truesect = bdd_now & P;
                        bdd falsesect = bdd_now & (!P);
                        if (truesect != bddfalse) {
                            ap_list.push_front(truesect);
                            del_flag = true;
                        }
                        if (falsesect != bddfalse) {
                            ap_list.push_front(falsesect);
                            del_flag = true;
                        }
                        if (del_flag) {
                            ap_list.pop_back();
                        }
                    }
                }
            }
        }
//        msg.str("");
//        msg << "Atomic predicate number for now: " << to_string(ap_list.size());
//        LOG4CXX_INFO(rlogger, msg.str());
    }
    gettimeofday(&end_time, NULL);

    msg.str("");
    long run_time = 1000000 * (end_time.tv_sec - start_time.tv_sec) + end_time.tv_usec - start_time.tv_usec;

    msg << "Finish generating atomic predicates, " << to_string(run_time) << " us used, total " <<
        to_string(ap_list.size()) << " predicates for this network.";
    LOG4CXX_INFO(rlogger, msg.str());

    vector<bdd>* ap_bdd_vec = new vector<bdd>{make_move_iterator(begin(ap_list)), make_move_iterator(end(ap_list))};
    this->ap_bdd_list = ap_bdd_vec;
    this->ap_size = ap_list.size();
    if (show_detail) {
        printf("These %d atomic predicates are as follows:\n", this->ap_size);
        for (size_t i = 0; i < this->ap_bdd_list->size(); i++) {
            printf("Predicate %lu:\n", i);
            bdd_allsat(this->ap_bdd_list->at(i), allsatPrintHandler);
        }
    }
}

void APVerifier::convert_router_to_ap(AP_TYPE type) {
    // convert every router to ap;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    std::map< uint32_t, Router* >::iterator it;
    for (it = id_to_router.begin(); it != id_to_router.end(); it++) {
        (*it).second->convert_to_ap(type, this->ap_bdd_list);
    }
    gettimeofday(&end, NULL);
    long run_time = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    stringstream msg;
    msg << "Finished converting all router's predicates to ap, " << to_string(run_time) << " us used.";
    LOG4CXX_INFO(rlogger, msg.str());

    if (show_detail) {
        for (it = id_to_router.begin(); it != id_to_router.end(); it++) {
            (*it).second->print_router_ap_map(this->ap_type);
        }
    }
}

void APVerifier::query_reachability(uint64_t from_port, uint64_t to_port, AP_TYPE type) {
    // create empty passed_port list
    struct timeval start, end;
    long run_time;
    gettimeofday(&start, NULL);
    std::list< uint64_t > passed_port;
    switch (type) {
        case VECTOR: {
            // create full packet_header
            std::vector< bool > packet_header = std::vector<bool>(ap_size, true);
            propagate_vec(packet_header, passed_port, from_port, to_port);
            break;
        }
        case BITSET: {
            std::bitset< BITSETLEN > packet_header;
            packet_header.flip();
            propagate_bset(packet_header, passed_port, from_port, to_port);
            break;
        }
        case NUM_SET: {
            std::set<uint64_t> packet_header;
            for (uint64_t i = 0; i < this->ap_bdd_list->size(); i++) {
                packet_header.insert(i);
            }
            propagate_numset(packet_header, passed_port, from_port, to_port);
            break;
        }
        case NONE: {
            bdd packet_header = bdd_true();
            propagate_bdd(packet_header, passed_port, from_port, to_port);
            break;
        }
    }
    gettimeofday(&end, NULL);
    run_time = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    stringstream msg;
    msg << "Finished reachability query between " << to_string(from_port) << " and " << to_string(to_port) <<
        ", time used: " << to_string(run_time) << " us";
    LOG4CXX_INFO(rlogger, msg.str());
}

void APVerifier::propagate_bdd(bdd packet_header, std::list< uint64_t > passed_port, uint64_t from_port,
                   uint64_t dst_port) {
    passed_port.push_back(from_port);

    // find from_port's router
    if (inport_to_router.count(from_port) == 0) {
        LOG4CXX_ERROR(rlogger, "Wrong in action configuration: no port found!\n");
        return;
    }
    uint32_t in_router_id = inport_to_router[from_port];
    if (id_to_router.count(in_router_id) == 0) {
        LOG4CXX_ERROR(rlogger, "Wrong somewhere that no according router is found!\n");
        return;
    }
    Router* in_router = id_to_router[in_router_id];

    // iterate on from_port's predicate list to find outports;
    std::map<Json::Value, PredicateNode *>::iterator it;
    for (it = in_router->predicate_map[from_port]->begin(); it != in_router->predicate_map[from_port]->end(); it++) {
        // whether there is any packet that can pass:
        bool continue_ppgt = false;
        bdd intersect = packet_header & (it->second->predicate);

        continue_ppgt = (intersect != bddfalse);
        if (continue_ppgt) {
            uint64_t outport;
            for (uint32_t i = 0; i < it->second->out_ports.size; i++) {
                outport = it->second->out_ports.list[i];
                // printf("Now, we can continue propagate on one match, with outport %lu.\n", outport);
                if (outport == dst_port) { // we finally reach where we need...
                    passed_port.push_back(outport);
                    printf("One path found: now print it: Match: \n");
                    bdd_allsat(intersect, allsatPrintHandler);
                    printf("Path: ");
                    print_passed_port(passed_port);
                } else {
                    // we encounter a loop...
                    bool looped = false;
                    for (auto itr = passed_port.begin(); itr != passed_port.end();
                         itr++) {
                        if (*itr == outport) {
                            looped = true;
                            break;
                        }
                    }
                    if (looped) {
                        printf("Looped! The loop is: ");
                        print_passed_port(passed_port);
                        printf("Loop's match: \n");
                        bdd_allsat(intersect, allsatPrintHandler);
                        return;
                    } else {
                        // no loop, then continue propagate on all outports...
                        std::vector<uint64_t>::iterator port_it;
                        passed_port.push_back(outport);
                        if (topology.count(outport) == 0) {
                            passed_port.pop_back();
                            continue;
                        }
                        for (port_it = topology[outport]->begin(); port_it != topology[outport]->end();
                             port_it++) {
                            propagate_bdd(intersect, passed_port, *port_it,  dst_port);
                        }
                        passed_port.pop_back();
                    }
                }
            }
        }
    }
}

void APVerifier::propagate_vec(std::vector<bool> packet_header, std::list<uint64_t> passed_port,
                       uint64_t from_port, uint64_t dst_port) {
    passed_port.push_back(from_port);

    // find from_port's router
    if (inport_to_router.count(from_port) == 0) {
        LOG4CXX_ERROR(rlogger, "Wrong in action configuration: no port found!\n");
        return;
    }
    uint32_t in_router_id = inport_to_router[from_port];
    if (id_to_router.count(in_router_id) == 0) {
        LOG4CXX_ERROR(rlogger, "Wrong somewhere that no according router is found!\n");
        return;
    }
    Router* in_router = id_to_router[in_router_id];

    // iterate on from_port's predicate list to find outports;
    std::map<Json::Value, APNodeV *>::iterator it;
    for (it = in_router->ap_vec_map[from_port]->begin(); it != in_router->ap_vec_map[from_port]->end(); it++) {
        // whether there is any packet that can pass:
        bool continue_ppgt = false;
        std::vector< bool > intersect = std::vector< bool >(ap_size, false);
        for (uint32_t i = 0; i < ap_size; i++) {
            if (packet_header[i] & (*it->second->match)[i]) {
                intersect[i] = true;
                continue_ppgt = true;
            }
        }

        if (continue_ppgt) {
            uint64_t outport;
            for (uint32_t i = 0; i < it->second->out_ports.size; i++) {
                outport = it->second->out_ports.list[i];
                // printf("Now, continue propagate on one match, with outport %lu.\n", outport);
                if (outport == dst_port) { // we finally reach where we need...
                    passed_port.push_back(outport);
                    printf("One path found: now print it: Match: ");
                    print_bool_vector(intersect);
                    printf("Path: ");
                    print_passed_port(passed_port);
                } else {
                    // we encounter a loop...
                    //TODO: maybe use algorithm's find? anyway...
                    bool looped = false;
                    for (auto itr = passed_port.begin(); itr != passed_port.end();
                         itr++) {
                        if (*itr == outport) {
                            looped = true;
                            break;
                        }
                    }
                    if (looped) {
                        printf("Looped! The loop is: ");
                        print_passed_port(passed_port);
                    } else {
                        // no loop, then continue propagate on all outports...
                        std::vector<uint64_t>::iterator port_it;
                        passed_port.push_back(outport);
                        if (topology.count(outport) == 0) {
                            passed_port.pop_back();
                            continue;
                        }
                        for (port_it = topology[outport]->begin(); port_it != topology[outport]->end();
                             port_it++) {
                            propagate_vec(intersect, passed_port, *port_it,  dst_port);
                        }
                        passed_port.pop_back();
                    }
                }
            }
        }
    }
}

void print_set(std::set< uint64_t > packet_header) {
    cout << "(";
    for (auto it = packet_header.begin(); it != packet_header.end(); it++) {
        cout << *it << ", ";
    }
    cout << ")" << endl;
}

void APVerifier::propagate_numset(std::set< uint64_t > packet_header, std::list<uint64_t> passed_port,
                               uint64_t from_port, uint64_t dst_port) {
    passed_port.push_back(from_port);

    // find from_port's router
    if (inport_to_router.count(from_port) == 0) {
        LOG4CXX_ERROR(rlogger, "Wrong in action configuration: no port found!\n");
        return;
    }
    uint32_t in_router_id = inport_to_router[from_port];
    if (id_to_router.count(in_router_id) == 0) {
        LOG4CXX_ERROR(rlogger, "Wrong somewhere that no according router is found!\n");
        return;
    }
    Router* in_router = id_to_router[in_router_id];

    // iterate on from_port's predicate list to find outports;
    std::map<Json::Value, APNodeS *>::iterator it;
    for (it = in_router->ap_nset_map[from_port]->begin(); it != in_router->ap_nset_map[from_port]->end(); it++) {
        // whether there is any packet that can pass:
        bool continue_ppgt = false;
        std::set< uint64_t > intersect;
        set_intersection(packet_header.begin(), packet_header.end(), it->second->match->begin(),
                         it->second->match->end(), std::inserter(intersect, intersect.begin()));
        continue_ppgt = !intersect.empty();

        if (continue_ppgt) {
            uint64_t outport;
            for (uint32_t i = 0; i < it->second->out_ports.size; i++) {
                outport = it->second->out_ports.list[i];
                // printf("Now, continue propagate on one match, with outport %lu.\n", outport);
                if (outport == dst_port) { // we finally reach where we need...
                    passed_port.push_back(outport);
                    printf("One path found: now print it: Match: ");
                    print_set(intersect);
                    printf("Path: ");
                    print_passed_port(passed_port);
                } else {
                    // we encounter a loop...
                    //TODO: maybe use algorithm's find? anyway...
                    bool looped = false;
                    for (auto itr = passed_port.begin(); itr != passed_port.end();
                         itr++) {
                        if (*itr == outport) {
                            looped = true;
                            break;
                        }
                    }
                    if (looped) {
                        printf("Looped! The loop is: ");
                        print_passed_port(passed_port);
                    } else {
                        // no loop, then continue propagate on all outports...
                        std::vector<uint64_t>::iterator port_it;
                        passed_port.push_back(outport);
                        if (topology.count(outport) == 0) {
                            passed_port.pop_back();
                            continue;
                        }
                        for (port_it = topology[outport]->begin(); port_it != topology[outport]->end();
                             port_it++) {
                            propagate_numset(intersect, passed_port, *port_it,  dst_port);
                        }
                        passed_port.pop_back();
                    }
                }
            }
        }
    }
}

void APVerifier::propagate_bset(std::bitset<BITSETLEN> packet_header, std::list<uint64_t> passed_port,
                               uint64_t from_port, uint64_t dst_port) {
    passed_port.push_back(from_port);

    // find from_port's router
    if (inport_to_router.count(from_port) == 0) {
        LOG4CXX_ERROR(rlogger, "Wrong in action configuration: no port found!\n");
        return;
    }
    uint32_t in_router_id = inport_to_router[from_port];
    if (id_to_router.count(in_router_id) == 0) {
        LOG4CXX_ERROR(rlogger, "Wrong somewhere that no according router is found!\n");
        return;
    }
    Router* in_router = id_to_router[in_router_id];

    // iterate on from_port's predicate list to find outports;
    std::map<Json::Value, APNodeB *>::iterator it;
    for (it = in_router->ap_bset_map[from_port]->begin(); it != in_router->ap_bset_map[from_port]->end(); it++) {
        // whether there is any packet that can pass:
        bool continue_ppgt = false;
        std::bitset<BITSETLEN> intersect = packet_header & (it->second->match);

        // printf("After match with this rule, we got packet header left as: %s\n", intersect.to_string().c_str());
        continue_ppgt = intersect.any();
        if (continue_ppgt) {
            uint64_t outport;
            for (uint32_t i = 0; i < it->second->out_ports.size; i++) {
                outport = it->second->out_ports.list[i];
                // printf("Now, continue propagate on one match, with outport %lu.\n", outport);
                if (outport == dst_port) { // we finally reach where we need...
                    passed_port.push_back(outport);
                    printf("One path found: now print it: Match: %s\n", intersect.to_string().c_str());
                    printf("Path: ");
                    print_passed_port(passed_port);
                } else {
                    // we encounter a loop...
                    bool looped = false;
                    for (auto itr = passed_port.begin(); itr != passed_port.end();
                         itr++) {
                        if (*itr == outport) {
                            looped = true;
                            break;
                        }
                    }
                    if (looped) {
                        printf("Looped! The loop is: ");
                        print_passed_port(passed_port);
                    } else {
                        // no loop, then continue propagate on all outports...
                        passed_port.push_back(outport);
                        if (topology.count(outport) == 0) {
                            passed_port.pop_back();
                            continue;
                        }
                        for (auto port_it = topology[outport]->begin(); port_it != topology[outport]->end();
                             port_it++) {
                            propagate_bset(intersect, passed_port, *port_it,  dst_port);
                        }
                        passed_port.pop_back();
                    }
                }
            }
        }
    }
}