// #include <ctime>
// #include <vector>
// #include <utils/globals.h>

// #include <ai/eval.h>

// #include <ai/utils.h>
// #include <algorithm>
// #include <cstring>
// #include <cstdio>
// #include <climits>
// #include <iostream>
// #include <cstdlib>
// #include <time.h>
// #include <string>

// class Node {
//     public:
//         char state;
//         int win_count;
//         int visit_count = 0;
// 		double value = std::numeric_limits<float>::max();
// 		Node* parent = nullptr;
//         int last_move_r;
//         int last_move_c;
// 		std::vector<Node*> children;
// 		int node_who;

//         ~Node(){}
// };

// // value = win_count / visit_vount + 1.41 * UCB
// void compute_value(Node* node, int total_visit_count){
//     node->value = ((double)node->win_count/node->visit_count) + 1.41 * sqrt(log((double)total_visit_count)/node->visit_count);
// }

// // select a leaf node by value(greedy)
// Node* select(Node* node){
//     while(node->children.empty() == false){
//         double max_value = 0;
//         int select_index = 0; 
//         for(size_t i=0; i < node->children.size(); i++){
//             if (max_value < node->children[i]->value){
//                 max_value = node->children[i]->value;
//                 select_index = i;
//             }
//         }
//         node = node->children[select_index];
//     }
//     return node;
// }
// // add all avaiable children for a node
// void expand(Node* parent_node){
//     int child_who = (parent_node->node_who == 1 ? 2 : 1);
//     int move_r;
//     int move_c;
//     for(int i = 0; i < g_board_size; i++){
//         for(int j = 0; j < g_board_size; j++){
//             char after = parent_node->state;
//             if (after[g_board_size * i + j] == "0"){
//                 after[g_board_size * i + j] = std::to_string(child_who);
//                 Node* child_node = new Node;
//                 child_node->node_who = child_who;
//                 child_node->state = after;
//                 child_node->parent = parent_node;
//                 child_node->last_move_r = i;
//                 child_node->last_move_c = j;
//                 parent_node->children.push_back(child_node);
//             }
//         }
//     }
// }

// // random play until a game finishes. 
// // return winner
// int simulation(Node* node){
//     char virtual_state = node->state;
//     int who = (node->node_who == 1 ? 2 : 1 );
//     int random_r;
//     int random_c;
//     while(RenjuAIEval::winningPlayer((const char*) virtual_state) != 0){
//         random_r = rand() % g_board_size + 1;
//         random_c = rand() % g_board_size + 1;
//         if (virtual_state[g_board_size * random_r + random_c] == "0"){
//             virtual_state[g_board_size * random_r + random_c] = std::to_string(who);
//             who = (who == 1 ? 2 : 1);
//         }
//     }
//     return RenjuAIEval::winningPlayer((const char*) virtual_state);
// }

// // update node info on path which we selected.
// void backpropogation(Node* root, Node* node, int winner, int total_visit_count){
//     bool win = true;
//     if (winner == root->node_who) win = false;
//     while (node != nullptr){
//         node->visit_count = node->visit_count + 1;
//         if (win == true)
//             node->win_count = node->win_count + 1;
//         compute_value(node, total_visit_count);
//         node = node->parent;
//     }
// }

// void delete_tree(Node* node){
//     if(node->children.empty() == false){
//         for(size_t i = 0; i < node->children.size(); i++){
//             delete_tree(node->children[i]);
//             if (node->children[i] != nullptr)
//                 free(node->children[i]);	
//         }
//         node->children.clear();
//     }
// }

// void greedy_select(Node* node, int* move_r, int* move_c){
//     int child_index = -1;
//     double max_value = 0;
//     for(int i = 0; i < node->children.size(); i++){
//         if (node->children[i]->value > max_value){
//             max_value = node->children[i]->value;
//             child_index = i;
//         }
//     }
//     *move_r = node->children[child_index]->last_move_r;
//     *move_c = node->children[child_index]->last_move_c; 
// }

// void MCTS(const char *gs, int player, int depth, 
//          int time_limit, int num_threads, 
// 		 int *move_r, int *move_c){

//     Node* root = new Node;
//     clock_t start_time, end_time;
// 	double total_time = 0;
//     int total_visit_count = 0;
//     // init tree and root
//     root->state = *gs;
//     root->node_who = (player == 1 ? 2 : 1);
//     expand(root);
//     // select -> expand -> simulation -> backpropogation
//     start_time = clock();
//     while(total_time < 0.98 * time_limit){
//         Node* greedy_node;
//         int winner;
//         greedy_node = select(root);
//         expand(greedy_node);
//         winner = simulation(greedy_node);
//         total_visit_count = total_visit_count + 1;
//         backpropogation(root, greedy_node, winner, total_visit_count);
//         end_time = clock();
// 		total_time = (double)(end_time - start_time)/CLOCKS_PER_SEC;
//     }
//     greedy_select(root, move_r, move_c);
//     delete_tree(root);
//     free(root);
// }