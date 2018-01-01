//
// Created by li danyang on 2017/12/30.
//

#ifndef AP_VERIFIER_AP_VERIFIER_UTILS_H
#define AP_VERIFIER_AP_VERIFIER_UTILS_H

#include <cstdint>

#define PACKED __attribute__ ((__packed__))
enum PREDICATE_TYPE {
    FWD = 0,
    RW,
};
struct PACKED List_t {
    uint32_t size;
    uint32_t *list;
    bool shared;
};
#endif //AP_VERIFIER_AP_VERIFIER_UTILS_H
