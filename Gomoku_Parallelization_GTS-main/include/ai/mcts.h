#ifndef INCLUDE_AI_MCTS_H_
#define INCLUDE_AI_MCTS_H_

#include <cmath>
#include <ctime>
#include <vector>
#include <utils/globals.h>
#include <ai/eval.h>
#include <ai/utils.h>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <climits>
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <string>
#include <omp.h>

class Node {
    public:
        char state[361];
        int win_count;
        int visit_count = 0;
		double value = std::numeric_limits<float>::max();
		Node* parent = nullptr;
        int last_move_r;
        int last_move_c;
		std::vector<Node*> children;
		int node_who;

        ~Node(){}
};

// value = win_count / visit_vount + 0.5 * UCB
void compute_value(Node* node, int total_visit_count){
    node->value = ((double)node->win_count/node->visit_count) + 0.5 * sqrt(log((double)total_visit_count)/node->visit_count);
}

// select a leaf node by value(greedy)
Node* select(Node* node, int total_visit_count){
    while(node->children.empty() == false){
        double max_value = 0;
        int select_index = 0; 
        for(size_t i=0; i < node->children.size(); i++){
            // node->children[i]->value = 
			compute_value(node->children[i], total_visit_count);
            if (max_value < node->children[i]->value){
                max_value = node->children[i]->value;
                select_index = i;
            }
        }
        node = node->children[select_index];
    }
    return node;
}
// add all avaiable children for a node
void expand(Node* parent_node){
    int child_who = (parent_node->node_who == 1 ? 2 : 1);
    int move_r;
    int move_c;
    char after[361];	
    int min_r = INT_MAX, min_c = INT_MAX, max_r = INT_MIN, max_c = INT_MIN;
	// reduce children
    for (int r = 0; r < g_board_size; ++r) {
		for (int c = 0; c < g_board_size; ++c) {
			if (parent_node->state[g_board_size * r + c] != 0) {
				if (r < min_r) min_r = r;
				if (c < min_c) min_c = c;
				if (r > max_r) max_r = r;
				if (c > max_c) max_c = c;
            }
        }
    }
	if (min_r - 2 < 0) min_r = 2;
    if (min_c - 2 < 0) min_c = 2;
    if (max_r + 2 >= g_board_size) max_r = g_board_size - 3;
    if (max_c + 2 >= g_board_size) max_c = g_board_size - 3;

    for(int i = min_r; i <= max_r; i++){
        for(int j = min_c; j <= max_c; j++){
            std::memcpy(after, parent_node->state, g_gs_size);
            if (after[g_board_size * i + j] == 0){
                after[g_board_size * i + j] = child_who;
                Node* child_node = new Node;
                child_node->node_who = child_who;
                // child_node->state = after;
                std::memcpy(child_node -> state, after, g_gs_size);
                child_node->parent = parent_node;
                child_node->last_move_r = i;
                child_node->last_move_c = j;
                parent_node->children.push_back(child_node);
            }
        }
    }
	// std::cout << parent_node -> children.size() << '\n';
}

// random play until a game finishes. 
// return winner
int simulation(Node* node){
    char virtual_state[361];
    // = node->state;
    std::memcpy(virtual_state, node->state, g_gs_size);
    int who = (node->node_who == 1 ? 2 : 1 );
    int random_r;
    int random_c;
    int real_chess_count = 0;
    int virtual_chess_count = 0;
    int min_r = INT_MAX, min_c = INT_MAX, max_r = INT_MIN, max_c = INT_MIN;
    for (int r = 0; r < g_board_size; ++r) {
		for (int c = 0; c < g_board_size; ++c) {
			if (virtual_state[g_board_size * r + c] != 0) {
				if (r < min_r) min_r = r;
                if (r > max_r) max_r = r;
				if (c < min_c) min_c = c;
				if (c > max_c) max_c = c;
                real_chess_count = real_chess_count + 1;
            }
        }
    }
	if (min_r - 2 < 0) min_r = 2;
    if (min_c - 2 < 0) min_c = 2;
    if (max_r + 2 >= g_board_size) max_r = g_board_size - 3;
    if (max_c + 2 >= g_board_size) max_c = g_board_size - 3;

    while(RenjuAIEval::winningPlayer((const char*) &virtual_state) != 0 && 
        (real_chess_count + virtual_chess_count == (max_r - min_r) * (max_c - min_c))){
        random_r = rand() % (max_r - min_r);
        random_c = rand() % (max_c - min_c);
        if (virtual_state[g_board_size * (min_r + random_r) + (min_c + random_c)] == 0){
            virtual_state[g_board_size * (min_r + random_r) + (min_c + random_c)] = who;
            virtual_chess_count = virtual_chess_count + 1;
            who = (who == 1 ? 2 : 1);
        }
    }
    return RenjuAIEval::winningPlayer((const char*) &virtual_state);
}

// update node info on path which we selected.
void backpropogation(Node* root, Node* node, int win_count,int num_threads){
    while (node != nullptr){
        node->visit_count = node->visit_count + num_threads;
        if (win_count > 0)
            node->win_count = node->win_count + win_count;
        node = node->parent;
    }
}

void delete_tree(Node* node){
    if(node->children.empty() == false){
        for(size_t i = 0; i < node->children.size(); i++){
            delete_tree(node->children[i]);
            if (node->children[i] != nullptr)
                free(node->children[i]);	
        }
        node->children.clear();
    }
}

// #include <iostream>

void greedy_select(Node* node, int* move_r, int* move_c){
    int child_index = -1;
    double max_value = 0;
    for(int i = 0; i < node->children.size(); i++) {
        if (node->children[i]->value > max_value){
            max_value = node->children[i]->value;
            child_index = i;
        }
		// std::cout << node -> children.size() << '\n';
    }
    *move_r = node->children[child_index]->last_move_r;
    *move_c = node->children[child_index]->last_move_c; 
}
/*
void MCTS(const char *gs, int player, int depth, 
         int time_limit, int num_threads, 
		 int *move_r, int *move_c){

    clock_t start_time, end_time;
    start_time = clock();
    Node* root = new Node;
	double total_time = 0;
    int total_visit_count = 0;
    // init tree and root
    
    // root->state = *gs;
    std::memcpy(root->state, gs, g_gs_size);
	
    root->node_who = (player == 1 ? 2 : 1);
    expand(root);

    // select -> expand -> simulation -> backpropogation
    while(total_time < 0.98 * time_limit){
        Node* greedy_node;
        int winner;
        greedy_node = select(root, total_visit_count);
        expand(greedy_node);
        winner = simulation(greedy_node);
        total_visit_count = total_visit_count + 1;
        backpropogation(root, greedy_node, winner, num_threads);
        end_time = clock();
		total_time = (double)(end_time - start_time)/CLOCKS_PER_SEC;
	}
    greedy_select(root, move_r, move_c);
    delete_tree(root);
    free(root);
    g_node_count = total_visit_count;
}
*/
void MCTS(const char *gs, int player, int depth, 
         int time_limit, int num_threads, 
		 int *move_r, int *move_c){
    
    omp_set_num_threads(num_threads);

    double start_time, end_time;
    start_time = omp_get_wtime();
    Node* root = new Node;
	double total_time = 0;
    int total_visit_count = 0;
    // init tree and root
    
    // root->state = *gs;
    std::memcpy(root->state, gs, g_gs_size);
	
    root->node_who = (player == 1 ? 2 : 1);
    expand(root);

    // select -> expand -> simulation -> backpropogation
    while(total_time < 0.95 * time_limit){
        Node* greedy_node;
        int win_count = 0;
        greedy_node = select(root, total_visit_count);
        expand(greedy_node);
        #pragma omp parallel shared(win_count)
        {
        int winner;
        winner = simulation(greedy_node);
        if  (winner != root->node_who && winner != 0) win_count += 1;  
        }
        total_visit_count = total_visit_count + num_threads;
        backpropogation(root, greedy_node, win_count, num_threads);
        end_time = omp_get_wtime();
		total_time = (double)(end_time - start_time);
	}

    greedy_select(root, move_r, move_c);
    delete_tree(root);
    free(root);
    g_node_count = total_visit_count;
}

#endif
