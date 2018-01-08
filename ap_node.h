//
// Created by li danyang on 2017/12/31.
//

#ifndef AP_VERIFIER_AP_NODE_H
#define AP_VERIFIER_AP_NODE_H

#include "ap_verifier_utils.h"

class APNode {
public:
    List_t out_ports;
    uint64_t in_port;
    AP_TYPE type;
};
#endif //AP_VERIFIER_AP_NODE_H
