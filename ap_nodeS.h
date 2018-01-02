//
// Created by li danyang on 2018/1/2.
//

#ifndef AP_VERIFIER_AP_NODES_H
#define AP_VERIFIER_AP_NODES_H

#include <vector>
#include "ap_node.h"

using namespace std;

class APNodeS : public APNode {
public:
    vector<bool>* match;
};

#endif //AP_VERIFIER_AP_NODES_H
