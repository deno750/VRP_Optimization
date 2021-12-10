#include "utility.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <math.h>
#include <time.h>

#include "plot.h"

double dmax(double d1, double d2) {
    return d1 > d2 ? d1 : d2;
}

int x_udir_pos(int i, int j, int num_nodes) {
    if (i == j) { 
        LOG_E("Indexes passed are equal!"); 
    }
    if (i > num_nodes - 1 || j > num_nodes -1 ) {
        LOG_E("Indexes passed greater than the number of nodes");
    }
    // Since the problem has undirected edges that connects two nodes,
    // if we have i > j means that we have already the edge that connects j
    // to i. (i.e. we have already edge (j, i) so we switch index j with index i
    // to obtain that edge)
    if (i > j) return x_udir_pos(j, i, num_nodes);  
    return i * num_nodes + j - ((i + 1) * (i + 2)) / 2;
}

int x_dir_pos(int i, int j, int num_nodes) {
    if (i > num_nodes - 1 || j > num_nodes -1 ) {
        LOG_E("Indexes passed greater than the number of nodes");
    }
    return i * num_nodes + j;
}

static int check_input_index_validity(int index, int argc, int *need_help) {
    if (index >= argc - 1) {
        *need_help = 1;
        return 1;
    }
    return 0;
}

void parse_comand_line(int argc, const char *argv[], instance *inst) {

    if (argc <= 1) {
        printf("Type \"%s --help\" to see available comands\n", argv[0]);
        exit(1);
    }

    inst->params.method.id = SOLVE_DEFAULT; // Default solver
    inst->params.method.edge_type = DEFAULT_EDGE; //Default edge type
    inst->params.method.name = SOLVER_DEFAULT_NAME;
    inst->params.time_limit = -1; //Default time limit value. -1 means no constraints in time limit 
    inst->params.num_threads = -1; //Default value -1. Means no limit on number of threads
    inst->params.file_path = NULL;
    inst->params.verbose = 1; //Default verbose level of 1
    inst->params.integer_cost = 1; // Default integer costs
    inst->params.seed = time(NULL); // We want to specify the random seed as the current time in order to have a real randomness when user doesn't explicitly choose the seed
    inst->params.perf_prof = 0;
    inst->params.callback_2opt = 0;
    inst->name = NULL;
    inst->comment = NULL;
    inst->nodes = NULL;
    inst->ind = NULL;
    inst->thread_seeds = NULL;
    inst->solution.edges = NULL;
    inst->solution.xbest = NULL;
    int need_help = 0;
    int show_methods = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp("-f", argv[i]) == 0) { 
            if (check_input_index_validity(i, argc, &need_help)) continue;
            const char* path = argv[++i];
            inst->params.file_path = CALLOC(strlen(path), char);
            strncpy(inst->params.file_path, path, strlen(path)); 
            continue; 
        } // Input file
        if (strcmp("-t", argv[i]) == 0) { 
            if (check_input_index_validity(i, argc, &need_help)) continue;
            inst->params.time_limit = atoi(argv[++i]); continue; 
        }
        if (strcmp("-threads", argv[i]) == 0) { 
            if (check_input_index_validity(i, argc, &need_help)) continue;
            inst->params.num_threads = atoi(argv[++i]); continue; 
        }
        if (strcmp("-verbose", argv[i]) == 0) { 
            if (check_input_index_validity(i, argc, &need_help)) continue;
            inst->params.verbose = atoi(argv[++i]); continue; 
        }
        if (strcmp("-method", argv[i]) == 0) {
            if (check_input_index_validity(i, argc, &need_help)) continue;
            const char* method = argv[++i];
            
            // Directed graph methods
            if (strncmp(method, "MTZ", 3) == 0) {
                inst->params.method.id = SOLVE_MTZ;
                inst->params.method.edge_type = DIR_EDGE;
                inst->params.method.name = "MTZ Static";
                inst->params.method.use_cplex = 1;
            }
            if (strncmp(method, "MTZL", 4) == 0) {
                inst->params.method.id = SOLVE_MTZL;
                inst->params.method.edge_type = DIR_EDGE;
                inst->params.method.name = "MTZ Lazy";
                inst->params.method.use_cplex = 1;
            }
            if (strncmp(method, "MTZI", 4) == 0) {
                inst->params.method.id = SOLVE_MTZI;
                inst->params.method.edge_type = DIR_EDGE;
                inst->params.method.name = "MTZ with SEC of degree 2";
                inst->params.method.use_cplex = 1;
            }
            if (strncmp(method, "MTZLI", 5) == 0) {
                inst->params.method.id = SOLVE_MTZLI;
                inst->params.method.edge_type = DIR_EDGE;
                inst->params.method.name = "MTZ lazy with SEC of degree 2";
                inst->params.method.use_cplex = 1;
            }
            if (strncmp(method, "MTZ_IND", 5) == 0) {
                inst->params.method.id = SOLVE_MTZ_IND;
                inst->params.method.edge_type = DIR_EDGE;
                inst->params.method.name = "MTZ with indicator constraints";
                inst->params.method.use_cplex = 1;
            }
            if (strncmp(method, "GG", 2) == 0) {
                inst->params.method.id = SOLVE_GG;
                inst->params.method.edge_type = DIR_EDGE;
                inst->params.method.name = "GG";
                inst->params.method.use_cplex = 1;
            }

            // Undirected graph methods
            if (strncmp(method, "LOOP", 4) == 0) {
                inst->params.method.id = SOLVE_LOOP;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "BENDERS' LOOP";
                inst->params.method.use_cplex = 1;
            }
            if (strncmp(method, "CALLBACK", 8) == 0) {
                inst->params.method.id = SOLVE_CALLBACK;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "INCUBEMENT CALLBACK";
                inst->params.method.use_cplex = 1;
            }
            if (strncmp(method, "USER_CUT", 9) == 0) {
                inst->params.method.id = SOLVE_UCUT;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "USER CUT CALLBACK";
                inst->params.method.use_cplex = 1;
            }
            if (strncmp(method, "CALLBACK_2OPT", 14) == 0) {
                inst->params.method.id = SOLVE_CALLBACK;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "INCUBEMENT CALLBACK WITH 2OPT";
                inst->params.method.use_cplex = 1;
                inst->params.callback_2opt = 1;
            }
            if (strncmp(method, "USER_CUT_2OPT", 13) == 0) {
                inst->params.method.id = SOLVE_UCUT;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "USER CUT CALLBACK WITH 2OPT";
                inst->params.method.use_cplex = 1;
                inst->params.callback_2opt = 1;
            }
            if (strncmp(method, "HARD_FIX", 8) == 0) {
                inst->params.method.id = SOLVE_HARD_FIXING;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "HARD FIXING HEURISTIC FIXED PROB";
                inst->params.method.use_cplex = 1;
            }
            if (strncmp(method, "HARD_FIX2", 9) == 0) {
                inst->params.method.id = SOLVE_HARD_FIXING2;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "HARD FIXING HEURISTIC VARIABLE PROB";
                inst->params.method.use_cplex = 1;
            }
            if (strncmp(method, "SOFT_FIX", 8) == 0) {
                inst->params.method.id = SOLVE_SOFT_FIXING;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "SOFT FIXING HEURISTIC";
                inst->params.method.use_cplex = 1;
            }
            if (strncmp(method, "GREEDY", 6) == 0) {
                inst->params.method.id = SOLVE_GREEDY;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "GREEDY HEURISTIC";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "GREEDY_ITER", 11) == 0) {
                inst->params.method.id = SOLVE_GREEDY_ITER;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "GREEDY ITERATIVE HEURISTIC";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "EXTR_MIL", 6) == 0) {
                inst->params.method.id = SOLVE_EXTR_MIL;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "EXTRA MILEAGE HEURISTIC";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "GRASP", 5) == 0) {
                inst->params.method.id = SOLVE_GRASP;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "GRASP HEURISTIC";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "GRASP_ITER", 10) == 0) {
                inst->params.method.id = SOLVE_GRASP_ITER;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "GRASP ITERATIVE HEURISTIC";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "2OPT_GRASP", 9) == 0) {
                inst->params.method.id = SOLVE_2OPT_GRASP;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "2-OPT HEURISTIC WITH GRASP INITIALIZATION";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "2OPT_GRASP_ITER", 15) == 0) {
                inst->params.method.id = SOLVE_2OPT_GRASP_ITER;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "2-OPT HEURISTIC WITH ITERATIVE GRASP INITIALIZATION";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "2OPT_GREEDY", 11) == 0) {
                inst->params.method.id = SOLVE_2OPT_GREEDY;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "2-OPT HEURISTIC WITH GREEDY INITIALIZATION";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "2OPT_GREEDY_ITER", 16) == 0) {
                inst->params.method.id = SOLVE_2OPT_GREEDY_ITER;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "2-OPT HEURISTIC WITH ITERATIVE GREEDY INITIALIZATION";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "2OPT_EXTR_MIL", 13) == 0) {
                inst->params.method.id = SOLVE_2OPT_EXTR_MIL;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "2-OPT HEURISTIC WITH EXTRA MILEAGE INITIALIZATION";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "VNS", 3) == 0) {
                inst->params.method.id = SOLVE_VNS;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "VNS META-HEURISTIC";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "TABU_STEP", 9) == 0) {
                inst->params.method.id = SOLVE_TABU_STEP;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "TABU SEARCH META-HEURISTIC WITH STEP POLICY";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "TABU_LIN", 8) == 0) {
                inst->params.method.id = SOLVE_TABU_LIN;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "TABU SEARCH META-HEURISTIC WITH LINEAR POLICY";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "TABU_RAND", 9) == 0) {
                inst->params.method.id = SOLVE_TABU_RAND;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "TABU SEARCH META-HEURISTIC WITH RANDOM POLICY";
                inst->params.method.use_cplex = 0;
            }
            if (strncmp(method, "GENETIC", 7) == 0) {
                inst->params.method.id = SOLVE_GENETIC;
                inst->params.method.edge_type = UDIR_EDGE;
                inst->params.method.name = "GENETIC ALGORITHM META-HEURISTIC";
                inst->params.method.use_cplex = 0;
            }
            continue;
        }
        if (strcmp("-seed", argv[i]) == 0) {
            if (check_input_index_validity(i, argc, &need_help)) continue;
            inst->params.seed = atoi(argv[++i]);
            continue;
        }
        if (strcmp("--fcost", argv[i]) == 0) { inst->params.integer_cost = 0; continue; }
        if (strcmp("--methods", argv[i]) == 0) {show_methods = 1; continue;}
        if (strcmp("--perfprof", argv[i]) == 0) {inst->params.perf_prof = 1; continue;}
        if (strcmp("--v", argv[i]) == 0 || strcmp("--version", argv[i]) == 0) { printf("Version %s\n", VERSION); exit(0);} //Version of the software
        if (strcmp("--help", argv[i]) == 0) { need_help = 1; continue; } // For comands documentation
        need_help = 1;
    }

    if (show_methods) {
        printf("MTZ                MTZ with static constraints\n");
        printf("MTZL               MTZ with lazy constraints\n");
        printf("MTZI               MTZ with static constraints and subtour elimination of degree 2\n");
        printf("MTZLI              MTZ with lazy constraints and subtour elimination of degree 2\n");
        printf("MTZ_IND            MTZ with indicator constraints\n");
        printf("GG                 GG constraints\n");
        printf("LOOP               Benders Method\n");
        printf("CALLBACK           Callback Method\n");
        printf("CALLBACK_2OPT      Callback Method with 2opt refinement\n");
        printf("USER_CUT           Callback Method using usercuts\n");
        printf("USER_CUT_2OPT      Callback Method using usercuts with 2opt refinement\n");
        printf("HARD_FIX           Hard fixing heuristic method with fixed prob\n");
        printf("HARD_FIX2          Hard fixing heuristic method with variable prob\n");
        printf("SOFT_FIX           Soft fixing heuristic method\n");
        printf("GREEDY             Greedy algorithm method\n");
        printf("GREEDY_ITER        Iterative Greedy algorithm method\n");
        printf("EXTR_MILE          Extra mileage method\n");
        printf("GRASP              GRASP method\n");
        printf("GRASP_ITER         Iterative GRASP method\n");
        printf("2OPT_GRASP         2-OPT with GRASP initialization\n");
        printf("2OPT_GRASP_ITER    2-OPT with iterative GRASP initialization\n");
        printf("2OPT_GREEDY        2-OPT with Greedy initialization\n");
        printf("2OPT_GREEDY_ITER   2-OPT with iterative Greedy initialization\n");
        printf("2OPT_EXTR_MIL      2-OPT with extra mileage initialization\n");
        printf("VNS                VNS method\n");
        printf("TABU_STEP          TABU Search method with step policy\n");
        printf("TABU_LIN           TABU Search method with linear policy\n");
        printf("TABU_RAND          TABU Search method with random policy\n");
        printf("GENETIC            GENETIC Algorithm\n");
        exit(0);
    }

    // Print the functions available
    if (need_help) {
        printf("-f <file's path>          To pass the problem's path\n");
        printf("-t <time>                 The time limit in seconds\n");
        printf("-threads <num threads>    The number of threads to use\n");
        printf("-verbose <level>          The verbosity level of the debugging printing\n");
        printf("-method <type>            The method used to solve the problem. Use \"--methods\" to see the list of available methods\n");
        printf("-seed <seed>              The seed for random generation\n");
        printf("--fcost                   Whether you want float costs in the problem\n");
        printf("--v, --version            Software's current version\n");
        exit(0);
    }
}

void free_instance(instance *inst) {
    FREE(inst->params.file_path);
    FREE(inst->name);
    FREE(inst->comment);
    FREE(inst->nodes);
    FREE(inst->ind);
    FREE(inst->thread_seeds);
    FREE(inst->solution.edges);
    FREE(inst->solution.xbest);
}

void parse_instance(instance *inst) {
    if (inst->params.file_path == NULL) { LOG_E("You didn't pass any file!"); }

    //Default values
    inst->num_nodes = -1;
    inst->weight_type = -1;
    inst->num_columns = -1;
    inst->is_vrp = false;

    // Open file
    FILE *fp = fopen(inst->params.file_path, "r");
    if(fp == NULL) { LOG_E("Unable to open file!"); }
   
    char line[128];          // 1 line of the file
    char *par_name;          // name of the parameter in the readed line
    char *token1;            // value of the parameter in the readed line
    char *token2;            // second value of the parameter in the readed line (used for reading coordinates)
    active_section active_section = PARAM_SECTION; 
    char sep[] = " :\n\t\r"; // separator for parsing

    // Read the file line by line
    while(fgets(line, sizeof(line), fp) != NULL) {
        if (strlen(line) <= 1 ) continue; // skip empty lines
        par_name = strtok(line, sep);

        if(strncmp(par_name, "NAME", 4) == 0){
			active_section = PARAM_SECTION;
            token1 = strtok(NULL, sep);
            inst->name = CALLOC(strlen(token1), char);   
            strncpy(inst->name, token1, strlen(token1));
			continue;
		}

		if(strncmp(par_name, "COMMENT", 7) == 0){
			active_section = PARAM_SECTION;   
            inst->comment = NULL; // Need to set this null in order to avoid crashes in free_instance function
            //We don't do anything with this parameter because we don't care about the comment  
			continue;
		}   

        if(strncmp(par_name, "TYPE", 4) == 0){
            token1 = strtok(NULL, sep);  
            if(strncmp(token1, "CVRP", 4) == 0) { 
                inst->is_vrp = true; 
            };
            active_section = PARAM_SECTION;
            continue;
		}

        if(strncmp(par_name, "DIMENSION", 9) == 0 ){
            token1 = strtok(NULL, sep);
            inst->num_nodes = atoi(token1);
            inst->nodes = CALLOC(inst->num_nodes, node);
            active_section = PARAM_SECTION;  
            continue;
		}

        if(strncmp(par_name, "CAPACITY", 8) == 0 ){
            token1 = strtok(NULL, sep);
            inst->capacity = atoi(token1);
            active_section = PARAM_SECTION;  
            continue;
		}

        if(strncmp(par_name, "EOF", 3) == 0 ){
			active_section = PARAM_SECTION;
			break;
		}

        if(strncmp(par_name, "EDGE_WEIGHT_TYPE", 16) == 0 ){
            token1 = strtok(NULL, sep);
            if (strncmp(token1, "EUC_2D", 6) == 0) inst->weight_type = EUC_2D;
            if (strncmp(token1, "MAX_2D", 6) == 0) inst->weight_type = MAX_2D;
            if (strncmp(token1, "MAN_2D", 6) == 0) inst->weight_type = MAN_2D;
            if (strncmp(token1, "CEIL_2D", 7) == 0) inst->weight_type = CEIL_2D;
            if (strncmp(token1, "GEO", 3) == 0) inst->weight_type = GEO;
            if (strncmp(token1, "ATT", 3) == 0) inst->weight_type = ATT;
            if (strncmp(token1, "EXPLICIT", 8) == 0) LOG_E("Wrong edge weight type, this program resolve only 2D TSP case with coordinate type.");
            active_section = PARAM_SECTION;  
            continue;
		}

        if (strncmp(par_name, "NODE_COORD_SECTION", 18) == 0){
            active_section = NODE_COORD_SECTION;
            continue;
        }

        if (strncmp(par_name, "EDGE_WEIGHT_SECTION", 19) == 0){
            active_section = EDGE_WEIGHT_SECTION;
            continue;
        }

        if (strncmp(par_name, "DEMAND_SECTION", 14) == 0){
            active_section = DEMAND_SECTION;
            continue;
        }

        if (strncmp(par_name, "DEPOT_SECTION", 13) == 0){
            active_section = DEPOT_SECTION;
            continue;
        }

        // NODE_COORD_SECTION
        if(active_section == NODE_COORD_SECTION){ 
            int i = atoi(par_name) - 1; // Nodes in problem's file start from index 1
			if ( i < 0 || i >= inst->num_nodes) LOG_E(" ... unknown node in NODE_COORD_SECTION");     
			token1 = strtok(NULL, sep);
			token2 = strtok(NULL, sep);
            node p = {atof(token1), atof(token2), 0, false};
			inst->nodes[i] = p;
            continue;
        }
        
        // EDGE_WEIGHT_SECTION
        if (active_section == EDGE_WEIGHT_SECTION) { // Are we going to use this??

            continue;
        }

        if (active_section == DEMAND_SECTION) {
            int i = atoi(par_name) - 1; // Nodes in problem's file start from index 1
			if ( i < 0 || i >= inst->num_nodes) LOG_E(" ... unknown node in DEMAND_SECTION"); 
            token1 = strtok(NULL, sep);
			token2 = strtok(NULL, sep);
            //int node_idx = atoi(token1);
            int demand = atoi(token1);
            inst->nodes[i].demand = demand;
            continue;
        }

        if (active_section == DEPOT_SECTION) {
            int i = atoi(par_name) - 1; // Nodes in problem's file start from index 1
            if (i >= inst->num_nodes) LOG_E(" ... unknown node in DEPOT_SECTION"); 
            if (i == -1) { continue; }
            inst->nodes[i].is_depot = true;
            continue;
        }
        
    }

    // close file
    fclose(fp);
}

void print_instance(instance inst) {
    if (inst.params.verbose >= 1) {
        if (inst.params.verbose >= 2) {
            printf("\n");
            printf("======== PARAMS =========\n");
            const char* edge;
            switch (inst.params.method.edge_type) {
            case UDIR_EDGE:
                edge = "Undirected";
                break;
            default:
                edge = "Directed";
                break;
            }
            
            printf("Edge type: %s\n", edge);
            printf("Solver method: %s\n", inst.params.method.name);
            if (inst.params.time_limit > 0) printf("Time Limit: %d\n", inst.params.time_limit);
            if (inst.params.num_threads > 0) printf("Threads: %d\n", inst.params.num_threads);
            if (inst.params.seed >= 0) printf("Seed: %d\n", inst.params.seed);
            printf("Cost: %s\n", inst.params.integer_cost ? "Integer" : "Floating point");
            printf("Verbose: %d\n", inst.params.verbose);
            printf("File path: %s\n", inst.params.file_path);
            printf("\n");
        }
        
        printf("\n");
        printf("======== INSTANCE ========\n");
        printf("name: %s\n", inst.name);
        printf("n° nodes: %d\n", inst.num_nodes);
        const char* weight;

        switch (inst.weight_type) {
        case EUC_2D:
            weight = "EUC_2D";
            break;
        case MAX_2D:
            weight = "MAX_2D";
            break;
        case MAN_2D:
            weight = "MAN_2D";
            break;
        case CEIL_2D:
            weight = "CEIL_2D";
            break;
        case GEO:
            weight = "GEO";
            break;
        case ATT:
            weight = "ATT";
            break;
        default:
            weight = "UNKNOWN";
            break;
        }
        printf("weight type: %s\n", weight);
        /*if (inst.params.verbose >= 3) {
            for (int i = 0; i < inst.num_nodes; i++) {
                point node = inst.nodes[i];
                printf("node %d: %0.2f, %0.2f\n", i+1, node.x, node.y);
            }
        }*/
        
    
        printf("\n");
    }
}

void export_tour(instance *inst) {
    if (inst->params.perf_prof) return;
    const char *dir = "../tour";
    mkdir(dir, 0777);

    char path[1024];
    sprintf(path, "%s/%s.tour", dir, inst->name);
    FILE* tour = fopen(path, "w"); 
    if (tour == NULL) {
        printf("Unable to save the tour file\n");
        return;
    }
    
    fprintf(tour, "NAME : %s.tour\n", inst->name);
    fprintf(tour, "TYPE : TOUR\n");
    fprintf(tour, "DIMENSION : %d\n", inst->num_nodes);
    fprintf(tour, "OBJECTIVE : %f\n", inst->solution.obj_best);
    fprintf(tour, "TIME : %f\n", inst->solution.time_to_solve);
    fprintf(tour, "TOUR_SECTION\n");

    edge e = inst->solution.edges[0]; // Starting node
    
    for (int i = 0; i < inst->num_nodes; i++) { // The tour is composed by the number of nodes
        fprintf(tour, "%d\n", e.i + 1);
        e = inst->solution.edges[e.j];
    }
    
    fprintf(tour, "-1\n");
    fprintf(tour, "EOF");

    fflush(tour);
    fclose(tour);
}

int count_components(instance *inst, double* xstar, int* successors, int* comp) {
    return count_components_adv(inst, xstar, successors, comp, NULL, NULL);
}

int count_components_adv(instance *inst, double* xstar, int* successors, int* comp, edge* close_cycle_edges, int* num_closed_cycles) {

    int num_comp = 0;

    for (int i = 0; i < inst->num_nodes; i++ ) {
		if ( comp[i] >= 0 ) continue; 

		// a new component is found
		num_comp++;
		int current_node = i;
		int visit_comp = 0; // 
        int comp_members = 1;
		while ( !visit_comp ) { // go and visit the current component
			comp[current_node] = num_comp;
			visit_comp = 1; // We set the flag visited to true until we find the successor
			for ( int j = 0; j < inst->num_nodes; j++ ) {
                if (current_node == j || comp[j] >= 0) continue;
				if (fabs(xstar[x_udir_pos(current_node,j,inst->num_nodes)]) >= EPS ) {
					successors[current_node] = j;
					current_node = j;
					visit_comp = 0;
                    comp_members++;
					break;
				}
			}
		}	
		successors[current_node] = i;  // last arc to close the cycle
        if (close_cycle_edges && num_closed_cycles && comp_members > 2) {
            edge e = {i, current_node};
            close_cycle_edges[(*num_closed_cycles)++] = e; 
        }
	}
    
    return num_comp;
}

void save_solution_edges(instance *inst, double *xstar) {
    inst->solution.edges = CALLOC(inst->num_nodes, edge);

    if (inst->params.method.edge_type == UDIR_EDGE) {

        int *succ = MALLOC(inst->num_nodes, int);
        int *comp = MALLOC(inst->num_nodes, int);
        MEMSET(succ, -1, inst->num_nodes, int);
        MEMSET(comp, -1, inst->num_nodes, int);
        count_components(inst, xstar, succ, comp);
        for (int i = 0; i < inst->num_nodes; i++) {
            edge *e = &(inst->solution.edges[i]);
            int successor = succ[i];
            e->i = i;
            e->j = succ[i];
        }
        FREE(succ);
        FREE(comp);

    } else {
        int k = 0;
        // Stores the directed model's edges
        for ( int i = 0; i < inst->num_nodes; i++ ){
            for ( int j = 0; j < inst->num_nodes; j++ ){
                // Zero is considered when the absolute value of number is <= EPS. 
                // One is considered when the absolute value of number is > EPS
                if ( fabs(xstar[x_dir_pos(i,j,inst->num_nodes)]) > EPS )  {
                    edge *e = &(inst->solution.edges[k++]);
                    e->i = i;
                    e->j = j;
                }              
            }
        }


    }
}

int plot_solution(instance *inst) {
    if (inst->params.perf_prof) return 0;
    PLOT gnuplotPipe = plot_open();
    if (gnuplotPipe == NULL) {
        printf("GnuPlot is not installed. Make sure that you have installed GnuPlot in your system and it's added in your PATH");
        return 1;
    }
    plot_in_file(gnuplotPipe, inst->name);
    if (inst->num_nodes <= 5000) {
        add_plot_param(gnuplotPipe, "plot '-' using 1:2 w linespoints pt 7");
    } else {
        add_plot_param(gnuplotPipe, "plot '-' using 1:2 w lines");
    }
    

    for (int i = 0; i < inst->num_nodes; i++) {
        edge e = inst->solution.edges[i];
        plot_edge(gnuplotPipe, inst->nodes[e.i], inst->nodes[e.j]);
    }
    
    plot_end_input(gnuplotPipe);

    plot_free(gnuplotPipe);

    return 0;
}

void save_lp(CPXENVptr env, CPXLPptr lp, char *name) {
    char modelPath[1024];
    sprintf(modelPath, "../model/%s.lp", name);
    
    CPXwriteprob(env, lp, modelPath, NULL);
}

int prepare_SEC(instance *inst, int tour, int *comp, char *sense, int *indexes, double *values, double *rhs) {
    int nnz = 0; // Number of variables to add in the constraint
    int num_nodes = 0; // We need to know the number of nodes due the vincle |S| - 1
    *sense = 'L'; // Preparing here sense in order that the caller of this function does not care about the underling constraints
    
    // Could it be faster if we use the successors array?? Nope.
    for (int i = 0; i < inst->num_nodes; i++) {
        if (comp[i] != tour) continue;
        num_nodes++;

        for (int j = i+1; j < inst->num_nodes; j++) {
            if (comp[j] != tour) continue;
            indexes[nnz] = x_udir_pos(i, j, inst->num_nodes);
            values[nnz] = 1.0;
            nnz++;
        }
    }
    
    *rhs = num_nodes - 1; // |S| - 1

    return nnz;
}

void save_cplex_log(CPXENVptr env, instance *inst) {
    if (inst->params.perf_prof) return;

    mkdir("../logs", 0777);
    char log_path[1024];
    sprintf(log_path, "../logs/%s.log", inst->name);
    CPXsetlogfilename(env, log_path, "w");
}

double get_elapsed_time(struct timeval start, struct timeval end) {
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;
    return elapsed;
}

void reverse_path(instance *inst, int start_node, int end_node, int *prev) {
    int currnode = start_node;
    while (1) {
        int node = prev[currnode];
        inst->solution.edges[currnode].j = node;
        currnode = node;
        if (node == end_node) {
            break;
        }
    }

    for (int k = 0; k < inst->num_nodes; k++) {
        prev[inst->solution.edges[k].j] = k;
    }
}

void copy_instance(instance *dst, instance *src) {
    memcpy(dst, src, sizeof(instance));
    dst->name = NULL;
    dst->params.file_path = NULL;
    dst->comment = NULL;
    dst->params.method.name = NULL;
    if (src->nodes) {
        dst->nodes = MALLOC(src->num_nodes, node);
        memcpy(dst->nodes, src->nodes, sizeof(node) * src->num_nodes);
    }
    if (src->ind) {
        dst->ind = MALLOC(src->num_columns, int);
        memcpy(dst->ind, src->ind, sizeof(int) * src->num_columns);
    }
    if (src->solution.edges) {
        dst->solution.edges = MALLOC(src->num_nodes, edge);
        memcpy(dst->solution.edges, src->solution.edges, sizeof(edge) * src->num_nodes);
    }
    dst->thread_seeds = NULL;
}

/**
 * Choses a random number in between [from, to)
 * 
 * @param from The left bound 
 * @param to The right bound
 * @returns Random integer between [from, to)
 */
int rand_choice(int from, int to) {
    return from + ((int) (URAND() * (to - from)));
}