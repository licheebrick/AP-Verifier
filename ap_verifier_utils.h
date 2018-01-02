//
// Created by li danyang on 2017/12/30.
//

#ifndef AP_VERIFIER_AP_VERIFIER_UTILS_H
#define AP_VERIFIER_AP_VERIFIER_UTILS_H

#include <cstdint>
#include <json/json.h>

#define PACKED __attribute__ ((__packed__))
enum PREDICATE_TYPE {
    FWD = 0,
    RW,
};

enum AP_TYPE {
    VECTOR = 0,
    NUM_SET,
    BITSET
};

struct PACKED List_t {
    uint32_t size;
    uint32_t *list;
    bool shared;
};

List_t make_sorted_list_from_array (uint32_t count, uint32_t elems[]);
List_t val_to_list(const Json::Value &val);
std::string list_to_string(List_t p);

#endif //AP_VERIFIER_AP_VERIFIER_UTILS_H
