#include "search.h"

using namespace std;

int bound = 0;	// IDA* max depth
char path_str[INF];	// IDA* path

// A* (using utils/expand())
// Returns true if open.front() == goal.
bool a_star(int init[], int goal[], vector<char>& path, int type) {
	int step = 0;
	Status S(init, goal, type);
	Status G(goal, goal);
	list<Status> open;
	vector<Status> close;
	open.push_back(S);
	while (!open.empty()) {
		// open.front()==goal，返回
		if(open.front().compare(G)) {
			printf("Search successed. Steps = %d\n", step);
			get_path(open.front(), close, path);
			open.clear();
			close.clear();
			return true;
		}
		/*
		// TIMEOUT
		if(step >= 30000) {
			break;
		}
		*/
		// 否则，扩展当前状态
		expand(open, close, goal, type);
		step++;
		if (step % 500 == 0) {
			printf("Steps: %d\n", step++);
			open.front().print();
		}
	}
	//printf("Failed to find result within %d steps.\n", step);
	return false;
}

// Deep-first search.
bool depth_first(Status S, int goal[], int depth, char act, int type) {
	Status G(goal, goal);
	// S==G，返回
	if(S.compare(G)) {
		path_str[depth] = '\0';
		return true;
	}
	// 超出限制深度，剪枝
	if (S.f - S.cost + depth > bound) {
		return false;
	}
	// 搜索
	for (int i = 0; i < 4; i++) {
		if (actions[i] == act) {
			continue;	// 回到前一方向，剪枝
		}
		Status N = S;
		if (!N.move(i)) {
			continue;	// 跳过非法方向
		}
		// 获取合法方向的后继N
		N.prev = actions[i];
		N.cost = S.cost + 1;
		N.f = N.cost + N.h(goal, type);
		path_str[depth] = actions[i];
		// 递归
		if (depth_first(N, goal, depth + 1, opposite[i], type)) {
			return true;
		}
	}
	return false;
}

// IDA* (using depth_first())
bool ida_star(int init[], int goal[], vector<char> &path, int type) {
	Status S(init, goal, type);
	bound = S.h(goal, type);
	memset(path_str, 'X', sizeof(char[INF]));
	// DFS，搜索失败时增大限制深度
	while(!depth_first(S, goal, 0, 'X', type)) {
		bound++;
	}
	printf("Search successed.\n");
	// 存储最优路径
	for (int i = 0; i < INF && path_str[i] != '\0'; i++) {
		path.push_back(path_str[i]);
	}
	return true;
}