#ifndef AP_VERIFIER_AP_NODEB_H
#define AP_VERIFIER_AP_NODEB_H

#include <bitset>
#include "ap_node.h"
#include "predicate_node.h"

using namespace std;
const int BITSETLEN = 16500;

class APNodeB : public APNode {
public:
    bitset<BITSETLEN> match;

    APNodeB(PredicateNode* pn, vector< bdd >* ap_list);

    void print_apb_node();
};

#endif //AP_VERIFIER_AP_NODEB_H
