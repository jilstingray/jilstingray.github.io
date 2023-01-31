#ifndef STATUS_H_
#define STATUS_H_

#include <list>
#include <vector>
#include <cstdio>

using namespace std;

const char actions[] = { 'U', 'D', 'L', 'R' };	// 移动方向
const char opposite[] = { 'D', 'U', 'R', 'L' };	// 反方向
const int INF = 1000;
const int TYPE_STD = 1;							// 启发式函数: 错位棋子数
const int TYPE_MIX = 2;							// 启发式函数: 当前与下一步错位数的加权和

class Status {
private:
	int grid[25];	// 5*5棋盘
public:
	char prev;		// 上一步移动的方向
	int cost;		// 路径耗散
	double f;		// 优先级=路径耗散+启发式函数
	int parent;		// close表中的前一状态
	Status(int b[], int goal[], int type = TYPE_STD, char pv = 'X', int c = 0, int p = -1):prev(pv), cost(c), parent(p) {
		for (int i = 0; i < 25; i++) {
            grid[i] = b[i];
        }
		f = cost + h(goal, type);
	}
	double h(int goal[], int type = TYPE_STD);	// 启发式函数
	void swap(int i, int j);					// 交换数字
	bool move(int m);							// 移动空格
	void print();								// 打印grid
	bool compare(Status S);						// 比较grid
	bool is_open(list<Status> open);			// status是否在open表
	bool is_close(vector<Status> close);		// status是否在close表
};

#endif