//
// Created by li danyang on 2017/12/30.
//

#include "ap_verifier.h"
#include <assert.h>
#include <set>

using namespace std;
using namespace log4cxx;

LoggerPtr APVerifier::logger(Logger::getLogger("APVerifier"));

APVerifier::APVerifier(int length, AP_TYPE type) {
    this->length = length;
    this->ap_type = type;
}

APVerifier::~APVerifier() {
    printf("Into ~APVerifier().\n");
    // clear topology
    std::map< uint32_t, std::vector<uint32_t>* >::iterator tpit;
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

void APVerifier::add_link(uint32_t from_port, uint32_t to_port) {
    if (topology.count(from_port) == 0) {
        topology[from_port] = new std::vector<uint32_t>(1, to_port);
    } else {
        topology[from_port]->push_back(to_port);
    }
}

void APVerifier::print_topology() {
    std::map< uint32_t, std::vector<uint32_t>* >::iterator it;
    printf("Now printing the topology...\n");
    for (it = topology.begin(); it != topology.end(); it++) {
        printf("%u --> ( ", (*it).first);
        for (size_t i = 0; i < (*it).second->size(); i++) {
            printf("%u ", (*it).second->at(i));
        }
        printf(")\n");
    }
}

void APVerifier::add_then_load_router(uint32_t router_id, Json::Value *root) {
    if (id_to_router.count(router_id) == 0 && router_id > 0) {
        Router* router = new Router(router_id);
        id_to_router[router_id] = router;
        Json::Value rules = (*root)["rules"];
        // Json::Value ports = (*root)["ports"];
        for (unsigned i = 0; i < rules.size(); i++) {
            // rule_counter++;
            string action = rules[i]["action"].asString();
            if (action =="fwd") {
                string match = rules[i]["match"].asString();
                assert(match.length() == length);
                List_t in_ports;
                in_ports = val_to_list(rules[i]["in_ports"]);
                for (uint32_t idx = 0; idx < in_ports.size; idx++) {
                    uint32_t inport = in_ports.list[idx];
                    //TODO: remove predicate with false BDD
                    if (router->predicate_map.count(inport) == 0) {
                        // We have never encountered any rule on this inport before;
                        PredicateNode* predicate_node = new PredicateNode(inport, match, FWD, this->length);
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
                            PredicateNode* predicate_node = new PredicateNode(inport, match, FWD, this->length);
                            predicate_node->out_ports = val_to_list(rules[i]["out_ports"]);
                            predicate_node->predicate -= router->dealt_bdd_map[inport];
                            router->dealt_bdd_map[inport] |= predicate_node->predicate;
                            router->predicate_map[inport]->insert(make_pair(rules[i]["out_ports"], predicate_node));
                        } else {
                            // We encountered rule on this inport with this action (outports)
                            PredicateNode* predicate_node = (router->predicate_map[inport])->at(rules[i]["out_ports"]);
                            bdd new_add = match2bdd(match, this->length) - router->dealt_bdd_map[inport];
                            predicate_node->predicate |= new_add;
                            router->dealt_bdd_map[inport] |= new_add;
                        }
                    }
                }
            } else { // "rw"
                //TODO::ADD REWRITE IMPLEMENTATION
                LOG4CXX_WARN(logger, "No rewrite rule supported yet, just ignore.\n");
            }
        }
        // router->print_router();
    } else if (router_id == 0) {
        LOG4CXX_ERROR(logger, "Cannot create table with ID 0.\n");
    } else {
        stringstream error_msg;
        error_msg << "Table " << router_id << " already exist. Can't add it again.";
        LOG4CXX_ERROR(logger,error_msg.str());
    }
}

void APVerifier::make_atomic_predicates() {
    // generate ap_bdd_list
    std::map< uint32_t, Router* >::iterator it;
    std::list< bdd > ap_list;
    bdd true_bdd = bdd_true();
    ap_list.push_back(true_bdd);
    for (it = id_to_router.begin(); it != id_to_router.end(); it++) { // it is router iterator
        std::map< uint32_t, std::map<Json::Value, PredicateNode*>* >::iterator port_it;
        for (port_it = (*it).second->predicate_map.begin(); port_it != (*it).second->predicate_map.end(); port_it++) {
            // port_it is port predicate map iterator
            std::map< Json::Value, PredicateNode* >::iterator pn_it;
            for (pn_it = (*port_it).second->begin(); pn_it != (*port_it).second->end(); pn_it++) {
                // pn_it is predicate node iterator
                bdd P = (*pn_it).second->predicate;
                if (P != bddfalse && P != bddtrue) {
                    int ori_size = int(ap_list.size());
                    for (int i = 0; i < ori_size; i++) {
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
    }
    //TODO: make sure here is safe... and valid...
    printf("Finish generate atomic predicates, total %ld predicates for this network.\n", ap_list.size());
    vector<bdd>* ap_bdd_vec = new vector<bdd>{make_move_iterator(begin(ap_list)), make_move_iterator(end(ap_list))};
    this->ap_bdd_list = ap_bdd_vec;
    printf("These atomic predicates are as follows:\n");
    for (size_t i = 0; i < this->ap_bdd_list->size(); i++) {
        bdd_allsat(this->ap_bdd_list->at(i), allsatPrintHandler);
    }
}

void APVerifier::convert_router_to_ap(AP_TYPE type) {
    // convert every router to ap;
    std::map< uint32_t, Router* >::iterator it;
    for (it = id_to_router.begin(); it != id_to_router.end(); it++) {
        (*it).second->convert_to_ap(type, this->ap_bdd_list);
    }
    printf("Finished converting all router's predicates to ap-vector...\n");

    for (it = id_to_router.begin(); it != id_to_router.end(); it++) {
        (*it).second->print_router_ap_map(type);
    }
}

void APVerifier::query_reachability(uint32_t from_port, uint32_t to_port) {

}