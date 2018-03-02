
#ifndef AP_VERIFIER_AP_NODEV_H
#define AP_VERIFIER_AP_NODEV_H

#include <vector>
#include "ap_node.h"
#include "predicate_node.h"

using namespace std;

class APNodeV : public APNode {
public:
    vector<bool>* match;

    APNodeV(PredicateNode* pn, vector< bdd >* ap_list);

    ~APNodeV();

    void print_apv_node();
};

#endif //AP_VERIFIER_AP_NODEV_H
