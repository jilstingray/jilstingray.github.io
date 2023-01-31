#include "utils.h"

using namespace std;

// 初始化棋盘
void init_grid(int a[], string filename) {
	FILE *f = fopen(filename.c_str(), "r");
	if (!f) {
		printf("Failed to open file %s.\n", filename.c_str());
		exit(0);
	}
	for (int i = 0; i < 25; i++) {
		fscanf(f, "%d", &a[i]);
	}
}

// 输出结果
void print_result(bool result, vector<char> path, clock_t timer, string filename) {
	if (!result) {
		printf("Search failed.\n");
	}
	FILE* f = fopen(filename.c_str(), "w+");
	if (!f) {
		printf("Failed to open file %s.\n", filename.c_str());
		exit(0);
	}
	printf("%d\n", (int)path.size());
	fprintf(f, "%d\n", (int)path.size());
	for (int i = path.size() - 1; i >= 0; i--) {
		printf("%c", path[i]);
		fprintf(f, "%c", path[i]);
	}
	printf("\n%fs\n\n", double(timer / CLOCKS_PER_SEC));
	fprintf(f, "\n%fs", double(timer / CLOCKS_PER_SEC));
}

// 插入open
bool insert_open(Status S, list<Status>& open) {
	for (auto it = open.begin(); it != open.end(); it++) {
		if (S.f < (*it).f) {
			open.insert(it, S);
			return true;
		}
	}
	open.push_back(S);
	return true;
}

// 插入close
int insert_close(Status S, vector<Status>& close) {
	close.push_back(S);
	return close.size() - 1;
}

// 从close获取路径
void get_path(Status S, vector<Status> close, vector<char>& path) {
	if (S.parent == -1) {
		return;
	}
	path.push_back(S.prev);
	return get_path(close[S.parent], close, path);
}

// 扩展当前状态
void expand(list<Status> &open, vector<Status> &close, int goal[], int type) {
	Status S = open.front();
	int index = insert_close(S, close);	// 将open的head加入close并得到下标
	open.pop_front();					// 弹出open队首
	// 尝试S的四个方向的后继状态N
	for (int i = 0; i < 4; i++) {
		Status N = S;
		if(!N.move(i)) {
			continue;	// 跳过非法状态
		}
		N.prev = actions[i];
		N.cost = S.cost + 1;
		N.f = N.cost + N.h(goal, type);
		N.parent = index;
		// 若N在open中
		if (N.is_open(open)) {
			for (auto it = open.begin(); it != open.end(); it++) {
				if (N.compare(*it)) {
					// N优先级低，不替换
					if (N.f > (*it).f) {
						break;
					}
					// 若新的N优先级更高，将open中的状态删除...
					else {
						open.erase(it);
						break;
					}
				}
				// ...然后将N插入open
				if (N.f < (*it).f) {
					open.insert(it, N);
				}
			}
		}
		// 若N不在open中而在close中（已扩展过）
		else if (N.is_close(close)) {
			for (int j = 0; j < close.size(); j++) {
				// 若新的N优先级更高，替换，并将N插入open等待重新扩展
				if(N.compare(close[j]) && N.f < close[j].f) {
					close[j].cost = N.cost;
					close[j].f = N.f;
					insert_open(N, open);
					break;
				}
			}
		}
		// 若N既不在open中也不在close中，将N插入open
		else {
			insert_open(N, open);
		}
	}
}