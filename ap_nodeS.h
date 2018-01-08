//
// Created by li danyang on 2018/1/2.
//

#ifndef AP_VERIFIER_AP_NODES_H
#define AP_VERIFIER_AP_NODES_H

#include <set>
#include "ap_node.h"
#include "predicate_node.h"

using namespace std;

class APNodeS : public APNode {
public:
    set<uint64_t >* match;

    APNodeS(PredicateNode* pn, vector< bdd >* ap_list);

    ~APNodeS();

    void print_aps_node();
};

#endif //AP_VERIFIER_AP_NODES_H
