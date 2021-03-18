/**
 *  Utility functions and data types 
 */
#ifndef UTILITY_H

#define UTILITY_H

#include <string.h>
#include <stdlib.h>
#include <errno.h>


#define VERSION "TSP 0.2"


// ================ Weight types =====================
#define EUC_2D 0 // weights are Euclidean distances in 2-D
#define MAX_2D 1 // weights are maximum distances in 2-D
#define MAN_2D 2 // weights are Manhattan distances in 2-D
#define CEIL_2D 3 // weights are Euclidean distances in 2-D rounded up
#define GEO 4 // weights are geographical distances
#define ATT 5 // special distance function for problems att48 and att532 (pseudo-Euclidean)

// ================ Edge types =======================
#define UDIR_EDGE 0
#define DIR_EDGE 1


// ==================== STRUCTS ==========================

// Struct which stores the parameters of the problem
typedef struct {
    int type;  // Describes if the graph is directed or undirected
    int num_threads; 
    int time_limit;
    int verbose; // Verbose level of debugging printing
    int integer_cost;
    char *file_path;
} instance_params;

// Definition of Point
typedef struct {
    double x;
    double y;
} point;

// Instance data structure where all the information of the problem are stored
typedef struct {
    instance_params params;

    int num_nodes;
    int weight_type;
    char *name;
    char *comment;
    point *nodes;
} instance;

// ===================== FUNCTIONS =============================

// Error print function
void print_error(const char *err) { printf("\n\n ERROR: %s \n\n", err); fflush(NULL); exit(1); } 

double dmax(double d1, double d2) {
    return d1 > d2 ? d1 : d2;
}

// Parses the input from the comand line
void parse_comand_line(int argc, const char *argv[], instance *inst) {

    if (argc <= 1) {
        printf("Type \"%s --help\" to see available comands\n", argv[0]);
        exit(1);
    }

    inst->params.type = DIR_EDGE; //Default edge type is directed 
    inst->params.time_limit = -1; //Default time limit value. -1 means no constraints in time limit 
    inst->params.num_threads = 1; //Default value is one thread
    inst->params.file_path = NULL;
    inst->params.verbose = 1; //Default verbose level of 1
    inst->params.integer_cost = 1; // Default integer costs
    int need_help = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp("-f", argv[i]) == 0) { 
            const char* path = argv[++i];
            inst->params.file_path = (char *) calloc(strlen(path), sizeof(char));
            strcpy(inst->params.file_path, path); 
            continue; 
        } // Input file
        if (strcmp("-t", argv[i]) == 0) { inst->params.time_limit = atoi(argv[++i]); continue; }
        if (strcmp("-threads", argv[i]) == 0) { inst->params.num_threads = atoi(argv[++i]); continue; }
        if (strcmp("-verbose", argv[i]) == 0) { inst->params.verbose = atoi(argv[++i]); continue; }
        if (strcmp("--fcost", argv[i]) == 0) { inst->params.integer_cost = 0; continue; }
        if (strcmp("--udir", argv[i]) == 0) { inst->params.type = UDIR_EDGE; continue; }
        if (strcmp("--v", argv[i]) == 0 || strcmp("--version", argv[i]) == 0) { printf("Version %s\n", VERSION); exit(0);} //Version of the software
        if (strcmp("--help", argv[i]) == 0) { need_help = 1; continue; } // For comands documentation
        need_help = 1;
    }

    // Print the functions available
    if (need_help) {
        printf("-f <file's path>          To pass the problem's path\n");
        printf("-t <time>                 The time limit\n");
        printf("-threads <num threads>    The number of threads to use\n");
        printf("-verbose <level>          The verbosity level of the debugging printing\n");
        printf("--fcost                   Whether you want float costs in the problem\n");
        printf("--udir                    Whether the edges shoul be treated as undirected in the graph\n");
        printf("--v, --version            Software's current version\n");
        exit(0);
    }
}

void free_instance(instance *inst) {
    free(inst->params.file_path);
    free(inst->name);
    free(inst->comment);
    free(inst->nodes);
}

// Parses the problem data
void parse_instance(instance *inst) {
    if (inst->params.file_path == NULL) { print_error("You didn't pass any file!"); }

    //Default values
    inst->num_nodes = -1;
    inst->weight_type = -1;

    // Open file
    FILE *fp = fopen(inst->params.file_path, "r");
    if(fp == NULL) { print_error("Unable to open file!"); }
   
    char line[128];          // 1 line of the file
    char *par_name;          // name of the parameter in the readed line
    char *token1;            // value of the parameter in the readed line
    char *token2;            // second value of the parameter in the readed line (used for reading coordinates)
    int active_section = 0;  // 0=reading parameters, 1=NODE_COORD_SECTION, 2=EDGE_WEIGHT_SECTION
    char sep[] = " :\n\t\r"; // separator for parsing

    // Read the file line by line
    while(fgets(line, sizeof(line), fp) != NULL) {
        if (strlen(line) <= 1 ) continue; // skip empty lines
        par_name = strtok(line, sep);

        if(strncmp(par_name, "NAME", 4) == 0){
			active_section = 0;
            token1 = strtok(NULL, sep);
            inst->name = (char *) calloc(strlen(token1), sizeof(char));         
            strncpy(inst->name,token1, strlen(token1));
			continue;
		}

		if(strncmp(par_name, "COMMENT", 7) == 0){
			active_section = 0;   
            //We don't do nothing with this parameter because we don't care about the comment  
			continue;
		}   

        if(strncmp(par_name, "TYPE", 4) == 0){
            token1 = strtok(NULL, sep);  
            if(strncmp(token1, "TSP", 3) != 0) print_error(" format error:  only TYPE == TSP implemented so far!!!!!!");
            active_section = 0;
            continue;
		}

        if(strncmp(par_name, "DIMENSION", 9) == 0 ){
            token1 = strtok(NULL, sep);
            inst->num_nodes = atoi(token1);
            inst->nodes = (point *) calloc(inst->num_nodes, sizeof(point));
            active_section = 0;  
            continue;
		}

        if(strncmp(par_name, "EOF", 3) == 0 ){
			active_section = 0;
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
            if (strncmp(token1, "EXPLICIT", 8) == 0) print_error("Wrong edge weight type, this program resolve only 2D TSP case with coordinate type.");
            active_section = 0;  
            continue;
		}

        if (strncmp(par_name, "NODE_COORD_SECTION", 18) == 0){
            active_section = 1;
            continue;
        }

        if (strncmp(par_name, "EDGE_WEIGHT_SECTION", 19) == 0){
            active_section = 2;
            continue;
        }

        // NODE_COORD_SECTION
        if(active_section == 1){ 
            int i = atoi(par_name) - 1; // Nodes in problem's file start from index 1
			if ( i < 0 || i >= inst->num_nodes) print_error(" ... unknown node in NODE_COORD_SECTION section");     
			token1 = strtok(NULL, sep);
			token2 = strtok(NULL, sep);
            point p = {atof(token1), atof(token2)};
			inst->nodes[i] = p;
            continue;
        }
        
        // EDGE_WEIGHT_SECTION
        if (active_section == 2) { // Are we going to use this??

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
            printf("======== PARAMS PASSED =========\n");
            const char* edge;
            switch (inst.params.type) {
            case UDIR_EDGE:
                edge = "Undirected";
                break;
            default:
                edge = "Directed";
                break;
            }
            printf("Edge type: %s\n", edge);
            printf("Time Limit: %d\n", inst.params.time_limit);
            printf("Threads: %d\n", inst.params.num_threads);
            printf("Verbose: %d\n", inst.params.verbose);
            printf("File path: %s\n", inst.params.file_path);
            printf("\n");
        }
        
        printf("\n");
        printf("======== Instance ========\n");
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
        if (inst.params.verbose >= 3) {
            for (int i = 0; i < inst.num_nodes; i++) {
                point node = inst.nodes[i];
                printf("node %d: %0.2f, %0.2f\n", i+1, node.x, node.y);
            }
        }
        
    
        printf("\n");
    }
}

#endif