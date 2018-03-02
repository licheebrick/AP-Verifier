

#ifndef AP_VERIFIER_AP_VERIFIER_UTILS_H
#define AP_VERIFIER_AP_VERIFIER_UTILS_H

#include <cstdint>
#include <vector>
#include <list>
#include <json/json.h>
#include <log4cxx/logger.h>

#define PACKED __attribute__ ((__packed__))
enum PREDICATE_TYPE {
    FWD = 0,
    RW,
};

enum AP_TYPE {
    VECTOR = 0,
    NUM_SET,
    BITSET,
    NONE,
    ALL
};

struct PACKED List_t {
    uint32_t size;
    uint64_t *list;
    bool shared;
};

extern log4cxx::LoggerPtr flogger;
extern log4cxx::LoggerPtr clogger;
extern log4cxx::LoggerPtr rlogger;

extern bool show_detail;

List_t make_sorted_list_from_array (uint32_t count, uint64_t elems[]);
List_t val_to_list(const Json::Value &val);
std::string list_to_string(List_t p);
void print_bool_vector(std::vector< bool > packet_header);
void print_passed_port(std::list< uint64_t > passed_port);


#endif //AP_VERIFIER_AP_VERIFIER_UTILS_H
