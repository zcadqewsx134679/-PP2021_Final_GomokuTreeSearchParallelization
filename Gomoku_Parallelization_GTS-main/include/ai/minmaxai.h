#ifndef INCLUDE_AI_MINMAXAI_H_
#define INCLUDE_AI_MINMAXAI_H_

#include <vector>

struct Moverment {
	int move_r, move_c;
	int v;

	bool operator < (const Moverment& rhs) const {
		return v > rhs.v;
	}
};


class MinMaxAI {

private:
	int numThread;

public:
	MinMaxAI(); // numThread, maxDepth, player=[1, 2]
	~MinMaxAI();

	static void searchMovement(const char*gs, int player, std::vector<Moverment>& v, int num_threads);
	static void searchMovementAll(const char*gs, int player, std::vector<Moverment>& v, int num_threads);

	static void dfs_first_floor(const char *gs, int player, int depth, int time_limit, int num_threads, 
								int *move_r, int *move_c);

	static int normal_dfs(const char* gs, int player, int depth, int time_limit, int time_start, 
							int alpha, int beta, int num_threads, int& num_nodes);
};



#endif  // INCLUDE_AI_MINMAXAI_H_
