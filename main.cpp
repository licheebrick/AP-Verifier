#include <iostream>
#include <bdd.h>
#include "json/json.h"
#include <string.h>
#include <fstream>
#include <dirent.h>
#include <sys/time.h>
#include "ap_verifier.h"
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include "log4cxx/propertyconfigurator.h"
#include <bitset>

log4cxx::LoggerPtr flogger;
log4cxx::LoggerPtr clogger;
log4cxx::LoggerPtr rlogger;

bool show_detail = false;

using namespace std;

void testBddBasic() {
    int var_num = 8;
    bdd bdd3 = match2bdd("xxxxxx11", var_num);
    bdd bdd1 = match2bdd("xx11xxxxxxxx1111", var_num);
    bdd bdd2 = match2bdd("10xxxxxx", var_num);
    printf("Sizeof bdd: %zu\n", sizeof(bdd1));

    printf("BDD1's all sat count: %lf\n", bdd_satcount(bdd1));

    cout << bdd_satcount(bdd1) << endl;

    bdd_allsat(bdd1 & bdd2, allsatPrintHandler);
    bdd_allsat((bdd1 & bdd2 - bdd3), allsatPrintHandler);

    cout << bddtable << bdd1 << endl;
    cout << bdd_nodecount(bdd1) << endl;
    cout << bddtable << (bdd1 & bdd2) << endl;

    cout << bddtable << bdd_satone(bdd2) << endl;
}

list<long> load_apverifier_from_dir(string json_file_path, APVerifier *A) {
    struct timeval start,end;
    ifstream jsfile;

    Json::Value root;
    Json::Reader reader;
    list<long> t_list;
    int router_counter = 0;
    long total_run_time = 0;

    // read topology
    string file_name = json_file_path + "/" + "topology.json";
    jsfile.open(file_name.c_str());
    if (!jsfile.good()) {
        stringstream err_msg;
        err_msg << "Error opening the file " << file_name << "\n";
        LOG4CXX_ERROR(rlogger, err_msg.str());
        return t_list;
    }
    reader.parse(jsfile, root, false);
    Json::Value topology = root["topology"];
    for (unsigned i = 0; i < topology.size(); i++) {
        A->add_link(topology[i]["src"].asUInt64(), topology[i]["dst"].asUInt64());
    }

    LOG4CXX_DEBUG(flogger, A->topology_to_string());
    jsfile.close();

    // read other json file
    struct dirent *ent;
    DIR *dir = opendir(json_file_path.c_str());
    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            long run_time = 0;
            file_name = string(ent->d_name);
            if (file_name.find(".rules.json") != string::npos ||
                    file_name.find(".tf.json") != string::npos) {
                file_name = json_file_path + "/" + file_name;
                stringstream msg;
                msg << "=== Loading rule file " << file_name << " to APVerifier ===";
                LOG4CXX_INFO(rlogger, msg.str());
                jsfile.open(file_name.c_str());
                reader.parse(jsfile, root, false);
                uint32_t router_id = root["id"].asUInt();
                run_time = A->add_then_load_router(router_id, &root);
                total_run_time += run_time;
                t_list.push_back(run_time);
                jsfile.close();
                router_counter++;
            }
        }
    }
    // printf("%s\n", string(40, '=').c_str());
    // printf("Total loading time: %ld us, %d routers, average loading time: %ld us\n", total_run_time, router_counter,
    stringstream msg;
    msg << "Total loading time: " << to_string(total_run_time) << " us, " << to_string(router_counter) <<
        " routers, average load time: " << to_string(total_run_time / router_counter) << " us.";
    LOG4CXX_INFO(rlogger, msg.str());
    closedir(dir);

    A->make_atomic_predicates();

//    A->convert_router_to_ap(BITSET);
//    A->convert_router_to_ap(NUM_SET);
//    A->convert_router_to_ap(VECTOR);

    return t_list;
}

void load_action_file(string json_action_file, APVerifier *A) {
    stringstream msg;
    msg << "Loading action file " << json_action_file;
    LOG4CXX_INFO(rlogger, msg.str());

    ifstream jsfile;
    Json::Value root;
    Json::Reader reader;

    jsfile.open(json_action_file.c_str());
    if (!jsfile.good()) {
        stringstream err_msg;
        err_msg << "Error opening the file " << json_action_file;
        LOG4CXX_ERROR(rlogger, err_msg.str());
        return;
    }

    reader.parse(jsfile, root, false);
    Json::Value actions = root["actions"];
    for (int i = 0; i < actions.size(); i++) {
        string type = actions[i]["method"].asString();
        if (type == "query_reach") {
            uint64_t from_port = actions[i]["params"]["from_port"].asUInt64();
            uint64_t to_port = actions[i]["params"]["to_port"].asUInt64();
            A->query_reachability(from_port, to_port, NONE);
//            A->query_reachability(from_port, to_port, BITSET);
//            A->query_reachability(from_port, to_port, NUM_SET);
//            A->query_reachability(from_port, to_port, VECTOR);

        } else {
            stringstream msg;
            msg << "Query " << type << " not supported yet...";
            LOG4CXX_INFO(rlogger, msg.str());
        }
    }
    jsfile.close();
}

int main(int argc, char* argv[]) {
    bool do_run_test = true;
    bool do_load_json_files = false;
    bool do_load_action = false;
    int hdr_len = 16;
    int var_num = 128;

    string json_files_path = "";
    string action_json_file = "";

    for (int i = 1; i < argc; i++) {
        if ( strcmp(argv[i] , "--help") == 0 ) {
            printf("Usage: ap-verifier [run option(s)][settings]\n");
            printf("  run options:\n");
            printf("\t --test  runs all the unit tests.\n");
            printf("\t --load <path> : load the rules from json files in the <path>.\n");
            printf("\t --action <file> : loads actions to be performed from a json policy <file>.\n");
            // printf("\t --filter <filter-wc> : cluster based on <wc-filter>.\n");

            printf("  settings:\n");
            printf("\t --hdr-len <length> : <length> of packet header (default is 1 byte).\n");
            printf("\t -detailed : show detailed router information in console.\n");
            break;
        }
        if ( strcmp(argv[i],"--test") == 0 ) {
            do_run_test = true;
        }

        if ( strcmp(argv[i],"--load") == 0)  {
            if (i+1 >= argc) {
                printf("Please specify path to json files after --load.\n");
                return -1;
            }
            do_load_json_files = true;
            json_files_path = string(argv[++i]);
        }

        if ( strcmp(argv[i],"--action") == 0)  {
            if (i+1 >= argc) {
                printf("Please specify policy file after --action.\n");
                return -1;
            }
            do_load_action = true;
            action_json_file = string(argv[++i]);
        }

        if ( strcmp(argv[i],"--hdr-len") == 0 ) {
            if (i+1 >= argc) {
                printf("Please specify length of header after --hdr-len.\n");
                return -1;
            }
            hdr_len = atoi(argv[++i]);
            var_num = 8 * hdr_len;
        }

        if ( strcmp(argv[i], "-detailed") == 0) {
            show_detail = true;
        }
    }

    // configure log4cxx.
    log4cxx::PropertyConfigurator::configure("./Log4cxxConfig.conf");
    flogger = log4cxx::Logger::getLogger("file");
    clogger = log4cxx::Logger::getLogger("console");
    rlogger = log4cxx::Logger::getLogger("root");

    APVerifier *A = new APVerifier(hdr_len, NONE);
    stringstream msg;
    msg << "Starting AP-Verifier with header length (in byte): " << hdr_len;
    LOG4CXX_INFO(rlogger, msg.str());

    // prepare bdd basics
    bdd_init(10000000, 1000000);
    bdd_setvarnum(var_num);
    // testBddBasic();

    if (do_load_json_files) {
        load_apverifier_from_dir(json_files_path, A);
    }

    if (do_load_action) {
        load_action_file(action_json_file, A);
    }

    delete A;
    bdd_done();
    return 0;
}