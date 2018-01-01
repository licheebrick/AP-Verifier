//
// Created by li danyang on 2017/12/31.
//

#ifndef AP_VERIFIER_ROUTER_H
#define AP_VERIFIER_ROUTER_H

#include <cstdint>
#include <map>
#include <vector>
#include "predicate_node.h"
#include "ap_node.h"
class Router {
public:
    const uint32_t router_id;

    // map from inport to its predicate list
    std::map< uint32_t, std::vector<PredicateNode*>* > predicate_vec;

    // map from inport to its ap set;
    std::map< uint32_t, std::vector<APNode*>* > ap_vec;

    // constructor
    Router(uint32_t id);

    // destructor
    virtual ~Router();
};
#endif //AP_VERIFIER_ROUTER_H
