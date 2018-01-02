//
// Created by li danyang on 2017/12/31.
//

#ifndef AP_VERIFIER_ROUTER_H
#define AP_VERIFIER_ROUTER_H

#include <cstdint>
#include <map>
#include <vector>
#include "ap_verifier_utils.h"
#include "predicate_node.h"
#include "ap_node.h"
#include "ap_nodeS.h"
#include "ap_nodeV.h"

class Router {
public:
    const uint32_t router_id;

    //TODO: map's value better be pointer!
    // map from inport to its predicate map
    std::map< uint32_t, std::map<Json::Value, PredicateNode*> > predicate_map;

    //TODO: also revised to pointer!
    // map from inport to the packet header space already dealt with on this in_port
    std::map< uint32_t, bdd > dealt_bdd_map;

    //TODO: also revised to pointer!
    // map from inport to its ap represented in bool vector;
    std::map< uint32_t, std::map<Json::Value, APNodeV*> > ap_vec_map;

    // constructor
    Router(uint32_t id);

    // destructor
    ~Router();

    void print_router();

    void convert_to_ap(AP_TYPE type, std::vector< bdd* >* ap_list);
};
#endif //AP_VERIFIER_ROUTER_H
