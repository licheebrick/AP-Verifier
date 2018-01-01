//
// Created by li danyang on 2017/12/30.
//

#ifndef AP_VERIFIER_AP_VERIFIER_H
#define AP_VERIFIER_AP_VERIFIER_H
#include <map>
#include <vector>
#include <list>
#include "ap_verifier_utils.h"
#include "predicate_node.h"
#include "router.h"

class APVerifier;
class APVerifier {
private:
    // header length
    int length;

    // topology
    std::map< uint32_t, std::vector<uint32_t>* > topology;

    // router map: from router_id to predicate list;
    std::map< uint32_t, Router* > id_to_router;

    // router port map: from router_id to port list;
    std::map< uint32_t, List_t> router_to_ports;

public:
    APVerifier(int length);
    int get_length() { return this->length; }

    ~APVerifier();

    void add_link(uint32_t from_port, uint32_t to_port);

    void print_topology();

    void add_then_load_router(uint32_t id, std::string routerName);

    void make_atomic_predicates();
};


#endif //AP_VERIFIER_AP_VERIFIER_H
