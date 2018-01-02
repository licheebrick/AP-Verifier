#include <iostream>
#include <bdd.h>
#include "json/json.h"
#include <fstream>
#include <dirent.h>
#include <sys/time.h>
#include "ap_verifier.h"
#include <log4cxx/logger.h>
#include "log4cxx/propertyconfigurator.h"
#include <bitset>

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;

int var_num = 8;

void testBitsetBasic() {
    bitset<8> be(0xff);
    cout << be << endl;
}

void testBddBasic() {
    bdd bdd3 = match2bdd("xxxxxx11", var_num);
    bdd bdd1 = match2bdd("xx11xxxx", var_num);
    bdd bdd2 = match2bdd("10xxxxxx", var_num);

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
        printf("Error opening the file %s\n", file_name.c_str());
        return t_list;
    }
    reader.parse(jsfile, root, false);
    Json::Value topology = root["topology"];
    for (unsigned i = 0; i < topology.size(); i++) {
        A->add_link(topology[i]["src"].asInt(), topology[i]["dst"].asInt());
    }
    A->print_topology();
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
                printf("=== Loading rule file %s to APVerifier ===\n", file_name.c_str());
                jsfile.open(file_name.c_str());
                reader.parse(jsfile, root, false);
                uint32_t router_id = root["id"].asInt();
                gettimeofday(&start, NULL);
                A->add_then_load_router(router_id, &root);
                gettimeofday(&end, NULL);
                run_time = end.tv_usec - start.tv_usec;
                if (run_time < 0) {
                    run_time = 1000000 * (end.tv_sec - start.tv_sec);
                }
                printf("%d us used.\n", run_time);
                A->id_to_router[router_id]->print_router();
                total_run_time += run_time;
                t_list.push_back(run_time);
                jsfile.close();
                router_counter++;

            }
        }
    }
    printf("Total loading time: %ld us, %d routers, average loading time: %ld us", total_run_time, router_counter,
    total_run_time / router_counter);
    closedir(dir);
    return t_list;
}

int main(int argc, char* argv[]) {
    std::cout << "Hello, here is atomic-predicates Verifier." << std::endl;
    bool do_run_test = true;
    bool do_load_json_files = false;
    int hdr_len = 1;
    var_num = 8;
    testBitsetBasic();

    string json_files_path = "";

    for (int i = 1; i < argc; i++) {
        if ( strcmp(argv[i] , "--help") == 0 ) {
            printf("Usage: ap-verifier [run option(s)][settings]\n");
            printf("  run options:\n");
            printf("\t --test  runs all the unit tests.\n");
            printf("\t --load <path> : load the rules from json files in the <path>.\n");
            // printf("\t --policy <file> : loads the source and probe nodes from a json policy <file>.\n");
            // printf("\t --filter <filter-wc> : cluster based on <wc-filter>.\n");

            printf("  settings:\n");
            printf("\t --hdr-len <length> : <length> of packet header (default is 1 byte).\n");
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

        if ( strcmp(argv[i],"--hdr-len") == 0 ) {
            if (i+1 >= argc) {
                printf("Please specify length of header after --hdr-len.\n");
                return -1;
            }
            hdr_len = atoi(argv[++i]);
            var_num = 8 * hdr_len;
        }
    }

    // configure log4cxx.
    PropertyConfigurator::configure("../Log4cxxConfig.conf");

    APVerifier *A = new APVerifier(var_num);

    // prepare bdd basics
    bdd_init(10000, 1000);
    bdd_setvarnum(var_num);
//    testBddBasic();

    if (do_load_json_files) {
        load_apverifier_from_dir(json_files_path, A);
    }

    bdd_done();
    return 0;
}