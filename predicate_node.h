//
// Created by li danyang on 2017/12/30.
//

#ifndef AP_VERIFIER_PREDICATE_NODE_H
#define AP_VERIFIER_PREDICATE_NODE_H

#include <bdd.h>
#include "ap_verifier_utils.h"

class PredicateNode;
class PredicateNode {
public:
    bdd predicate;
    uint32_t out_port;
    uint32_t in_port;
    PREDICATE_TYPE type;
};
#endif //AP_VERIFIER_PREDICATE_NODE_H
