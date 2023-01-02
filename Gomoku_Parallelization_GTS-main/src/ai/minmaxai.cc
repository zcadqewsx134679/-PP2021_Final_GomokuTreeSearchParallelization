#include <ai/minmaxai.h>
#include <ctime>
#include <vector>
#include <utils/globals.h>
// #include <ai/eval.h>
#include <ai/myeval.h>
#include <ai/utils.h>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <climits>

// parallel 
#include <omp.h>

void MinMaxAI::dfs_first_floor(const char *gs, int player, 
                            int depth, int time_limit, int num_threads, 
						    int *move_r, int *move_c) {
    omp_set_dynamic(0);
	//omp_set_num_threads(num_threads);

	// cout << player << ' ' << depth <<  ' ' << time_limit << ' ' << num_threads << '\n';

 	//clock_t time_start = clock();
    float time_start = omp_get_wtime();

	char *_gs = new char[g_gs_size];
    std::memcpy(_gs, gs, g_gs_size);
 	
    int opponent = (player == 1 ? 2 : 1);

 	std::vector<Moverment> play_movs, oppo_movs;
 	MinMaxAI::searchMovement(_gs, player, play_movs, num_threads);
 	MinMaxAI::searchMovement(_gs, opponent, oppo_movs, num_threads);
	// MinMaxAI::searchMovementAll(_gs, player, play_movs, num_threads);
 	// MinMaxAI::searchMovementAll(_gs, opponent, oppo_movs, num_threads);
	

 	if (play_movs.size() == 0) return;
 	if (play_movs.size() == 1 || play_movs[0].v >= kMyAIEvalWinningScore) {
 		auto now = play_movs[0];
 		*move_r = now.move_r;
 		*move_c = now.move_c;
 		return;
 	}

 	// If AI needs to block player
	bool block_opponent = false;
    int tmp_size = std::min((int) oppo_movs.size(), 10);
    int blk_post = play_movs.size();

    if (oppo_movs[0].v >= kMyAIEvalThreateningScore) {
        block_opponent = true;

        Moverment* opo_mov = (Moverment*) calloc(tmp_size, sizeof(Moverment));
        
        #pragma omp parallel for num_threads(num_threads)
        for (int i = 0; i < tmp_size; ++i) {    
            opo_mov[i].move_r = oppo_movs[i].move_r;
            opo_mov[i].move_c = oppo_movs[i].move_c;
            opo_mov[i].v      = MyAIEval::evalMove(gs, opo_mov[i].move_r, opo_mov[i].move_c, player);
        }
        for (int i = 0; i < tmp_size; ++i) {
            play_movs.push_back(opo_mov[i]); 
        }
        free(opo_mov);
    }
    
    int n = (int) play_movs.size();
    int max_score = INT_MIN;

    int score[361], rr[361], cc[361];
    int num_node[361];
    auto new_g = new char[361][361];

    #pragma omp parallel for schedule(guided, 1) num_threads(num_threads)
    for (int i = 0; i < n; i++) {
    	auto move = play_movs[i];

    	char* now_g = new_g[i];
    	std::memcpy(now_g, _gs, g_gs_size);

    	// Execute move
        RenjuAIUtils::setCell(now_g, move.move_r, move.move_c, static_cast<char>(player));

        num_node[i] = 0;
        score[i] = MinMaxAI::normal_dfs((const char*) now_g, opponent, depth - 1, time_limit, time_start,
        								-(INT_MAX / 2), -(INT_MIN / 2) - move.v, num_threads, num_node[i]);
        
        // Nega-Max 
        score[i] = move.v - score[i];

        rr[i] = move.move_r;
        cc[i] = move.move_c;

		// Restore
        RenjuAIUtils::setCell(now_g, move.move_r, move.move_c, 0);
    }
    
    for (int i = 0; i < n; i++) {
    	if (score[i] > max_score) {
    		max_score = score[i];
    		*move_r = rr[i];
    		*move_c = cc[i];
    	}
    	g_node_count += num_node[i];
    }
	if (block_opponent && max_score < 0) {
        auto blocking_move = play_movs[blk_post];
        int b_score = blocking_move.v;

        if (b_score == 0) b_score = 1;
        if ((max_score - b_score) / static_cast<float>(std::abs(b_score)) < 0.2) {
			*move_r = blocking_move.move_r;
            *move_c = blocking_move.move_c;
            max_score = blocking_move.v;
        }
    }
    free(new_g);
}

int MinMaxAI::normal_dfs(const char* gs, int player, int depth, int time_limit, int time_start, 
						int alpha, int beta, int num_threads, int& num_nodes) {

	if (depth == 0) return 0;
	if (depth < 0 && (omp_get_wtime() - time_start) >= time_limit) return 0;
	
	++ num_nodes;

	char* _gs = new char[g_gs_size];
    for (int i = 0; i < g_gs_size; i++) _gs[i] = gs[i];

    int opponent = (player == 1 ? 2 : 1);

	std::vector<Moverment> play_movs, oppo_movs;
 	MinMaxAI::searchMovement(_gs, player, play_movs, num_threads);
 	MinMaxAI::searchMovement(_gs, opponent, oppo_movs, num_threads);
	// MinMaxAI::searchMovementAll(_gs, player, play_movs, num_threads);
 	// MinMaxAI::searchMovementAll(_gs, opponent, oppo_movs, num_threads);


	if (play_movs.size() == 0) return 0;
 	if (play_movs.size() == 1 || play_movs[0].v >= kMyAIEvalWinningScore) {
 		free(_gs);
 		return play_movs[0].v;
 	}

 	// If AI needs to block player
    bool block_opponent = false;
    int tmp_size = std::min((int) oppo_movs.size(), 10);
    int blk_post = play_movs.size();

    if (oppo_movs[0].v >= kMyAIEvalThreateningScore) {
        block_opponent = true;

        Moverment* opo_mov = (Moverment*) calloc(tmp_size, sizeof(Moverment));
        
        //#pragma omp parallel for
        for (int i = 0; i < tmp_size; ++i) {    
            opo_mov[i].move_r = oppo_movs[i].move_r;
            opo_mov[i].move_c = oppo_movs[i].move_c;
            opo_mov[i].v      = MyAIEval::evalMove(gs, opo_mov[i].move_r, opo_mov[i].move_c, player);
        }
        for (int i = 0; i < tmp_size; ++i) {
            play_movs.push_back(opo_mov[i]); 
        }
        free(opo_mov);
    }

    int max_score = alpha;
	int n = (int) play_movs.size();
    char* new_g = new char[g_gs_size];

    for (int i = 0; i < n; i++) {
    	auto move = play_movs[i];

    	std::memcpy(new_g, _gs, g_gs_size);

    	// Execute move
        RenjuAIUtils::setCell(new_g, move.move_r, move.move_c, static_cast<char>(player));


        int score = MinMaxAI::normal_dfs(new_g, opponent, depth - 1, time_limit, time_start,
        								-beta, -max_score + move.v, num_threads, num_nodes);
        move.v = move.v - score;
        // if (score >= 2) score = (int) (0.95 * score);

		// Restore
        RenjuAIUtils::setCell(new_g, move.move_r, move.move_c, 0);

        max_score = std::max(max_score, move.v);
		
		// int max_score_decayed = max_score;
        // if (max_score >= 2) max_score_decayed = (int) (0.95 * max_score_decayed);
        // alpha = max(alpha, max_score);
        // if (max_score_decayed >= beta) break;
		if (max_score >= beta) break;
    }
    if (block_opponent && max_score < 0) {
        auto blocking_move = play_movs[blk_post];
        int b_score = blocking_move.v;

        if (b_score == 0) b_score = 1;
        if ((max_score - b_score) / static_cast<float>(std::abs(b_score)) < 0.2) {
            max_score = blocking_move.v;
        }
    }
    free(new_g);
    free(_gs);

    return max_score;
}

void MinMaxAI::searchMovementAll(const char*gs, int player, std::vector<Moverment>& v, int num_threads) {
    v.clear();

    Moverment* all = (Moverment*) calloc(361, sizeof(Moverment));
    
    // #pragma omp parallel for collapse(2)
    for (int r = 0; r < g_board_size; ++r) {
        for (int c = 0; c < g_board_size; ++c) {
            if (gs[g_board_size * r + c] != 0) continue;

            all[g_board_size * r + c].move_r = r;
            all[g_board_size * r + c].move_c = c;
            all[g_board_size * r + c].v      = MyAIEval::evalMove(gs, r, c, player);
        }
    }
    for (int i = 0; i < 361; i++) if (gs[i] == 0) {
        v.push_back(all[i]);
    }
    std::sort(v.begin(), v.end());

    free(all);
}

void MinMaxAI::searchMovement(const char*gs, int player, std::vector<Moverment>& v, int num_threads) {

	v.clear();

	// Find an extent to reduce unnecessary calls to RenjuAIUtils::remoteCell	
	int min_r = INT_MAX, min_c = INT_MAX, max_r = INT_MIN, max_c = INT_MIN;
	for (int r = 0; r < g_board_size; ++r) {
		for (int c = 0; c < g_board_size; ++c) {
			if (gs[g_board_size * r + c] != 0) {
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

    std::vector<Moverment> tmp[g_board_size];

	// Loop through all cells
	//#pragma omp parallel for num_threads(num_threads)
    for (int r = min_r - 2; r <= max_r + 2; ++r) {
    	tmp[r].clear();

        for (int c = min_c - 2; c <= max_c + 2; ++c) {
            // Consider only empty cells
            if (gs[g_board_size * r + c] != 0) continue;

            // Skip remote cells (no pieces within 2 cells)
            if (RenjuAIUtils::remoteCell(gs, r, c)) continue;

            Moverment m;
            m.move_r = r;
			m.move_c = c;
			m.v      = MyAIEval::evalMove(gs, r, c, player);

            // Add move
            tmp[r].push_back(m);
        }
    }
    for (int r = min_r - 2; r <= max_r + 2; ++r) {
    	for (auto j : tmp[r]) v.push_back(j);
    }
    std::sort(v.begin(), v.end());
}
