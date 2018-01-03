//
// Created by li danyang on 2017/12/30.
//

#ifndef AP_VERIFIER_AP_VERIFIER_H
#define AP_VERIFIER_AP_VERIFIER_H
#include <map>
#include <vector>
#include <list>
#include <json/json.h>
#include "ap_verifier_utils.h"
#include "predicate_node.h"
#include "router.h"
#include "log4cxx/logger.h"

class APVerifier;
class APVerifier {
private:
    static log4cxx::LoggerPtr logger;

public:
    // header length
    int length;

    AP_TYPE ap_type;

    // topology
    std::map< uint32_t, std::vector<uint32_t>* > topology;

    // router map: from router_id to predicate list;
    std::map< uint32_t, Router* > id_to_router;

    // router port map: from router_id to port list;
    // std::map< uint32_t, List_t> router_to_ports;

    // list to place atomic predicates represented in bdd;
    std::vector< bdd >* ap_bdd_list;

    APVerifier(int length, AP_TYPE type);
    int get_length() { return this->length; }

    ~APVerifier();

    void add_link(uint32_t from_port, uint32_t to_port);

    void print_topology();

    void add_then_load_router(uint32_t router_id, Json::Value *root);

    void make_atomic_predicates();

    void convert_router_to_ap(AP_TYPE type);

    void query_reachability(uint32_t from_port, uint32_t to_port);
};

bdd match2bdd(string match, int length);

#endif //AP_VERIFIER_AP_VERIFIER_H
