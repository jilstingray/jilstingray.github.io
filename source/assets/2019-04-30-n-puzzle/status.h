#ifndef STATUS_H_
#define STATUS_H_

#include <list>
#include <vector>
#include <cstdio>

using namespace std;

// 移动方向
const char actions[] = { 'U', 'D', 'L', 'R' };
// 反方向
const char opposite[] = { 'D', 'U', 'R', 'L' };
// infinitiy
const int INF = 1000;
// 启发式函数: 错位棋子数
const int TYPE_STD = 1;
// 启发式函数: 当前与下一步错位数的加权和
const int TYPE_MIX = 2;

class Status {
private:
	// 5*5棋盘
	int grid[25];
public:
	// 上一步移动的方向
	char prev;
	// 路径耗散
	int cost;
	// 优先级=路径耗散+启发式函数
	double f;
	// close表中的前一状态
	int parent;
	Status(int b[], int goal[], int type = TYPE_STD, char pv = 'X', int c = 0, int p = -1):prev(pv), cost(c), parent(p) {
		for (int i = 0; i < 25; i++) {
            grid[i] = b[i];
        }
		f = cost + h(goal, type);
	}
	// 启发式函数
	double h(int goal[], int type = TYPE_STD);
	// 交换数字
	void swap(int i, int j);
	// 移动空格
	bool move(int m);
	// 打印grid
	void print();
	// 比较grid
	bool compare(Status S);
	// status是否在open表
	bool is_open(list<Status> open);
	// status是否在close表
	bool is_close(vector<Status> close);
};

#endif