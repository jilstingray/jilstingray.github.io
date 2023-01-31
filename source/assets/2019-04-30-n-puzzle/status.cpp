#include "status.h"

using namespace std;

// 启发式函数
double Status::h(int goal[], int type) {
	double temp = 0, value = 0;
	for (int i = 0; i < 25; i++) {
		if (grid[i] != goal[i] && grid[i] != 0) {
			temp++;
		}
	}
	// TYPE_STD: 启发式函数 - 错位棋子数
	if (type == TYPE_STD) {
		value = temp;
	}
	// TYPE_MIX: 启发式函数 - 当前与下一步错位数的加权和
	else if (type == TYPE_MIX) {
		for (int i = 0; i < 4; i++) {
			Status N(grid, goal, 1);
			if(N.move(i)) {
				value += N.h(goal, 1) * 0.25;
			}
		}
		value = value * 0.5 +  temp * 0.5;
	}
	return value;
}

// 交换棋子位置
void Status::swap(int i, int j) {
	int temp = grid[i];
	grid[i] = grid[j];
	grid[j] = temp;
}

// 移动空格0
bool Status::move(int m) {
	int i;
	for (i = 0; grid[i] != 0; i++);	// 定位'0'的位置
	// 'U' - up
	if (m == 0) {
		if (i == 2) {
			swap(2, 22);
		}
		else if (i > 4) {
			swap(i, i - 5);
		}
		return true;
	}
	// 'D' - down
	else if (m == 1) {
		if (i == 22) {
			swap(22, 2);
		}
		else if (i < 20) {
			swap(i, i + 5);
		}
		return true;
	}
	// 'L' - left
	else if (m == 2) {
		if (i == 10) {
			swap(10, 14);
		}
		else if(i % 5 != 0) {
			swap(i, i - 1);
		}
		return true;
	}
	// 'R' - right
	else if (m == 3) {
		if (i == 14) {
			swap(14, 10);
		}
		else if((i + 1) % 5 != 0) {
			swap(i, i + 1);
		}
		return true;
	}
	// 无法移动
	return false;
}

// 打印grid
void Status::print() {
	for (int i = 0; i < 25;) {
		printf("%d", grid[i]);
		if (++i % 5 == 0) {
			printf("\n");
		}
		else {
			printf("\t");
		}
	}
	printf("\n");
}

// 比较grid
bool Status::compare(Status S) {
	for (int i = 0; i < 25; i++) {
		if (grid[i] != S.grid[i]) {
			return false;
		}
	}
	return true;
}

// 状态是否在open表中
bool Status::is_open(list<Status> open) {
	for (auto it = open.begin(); it != open.end(); it++) {
		if(this->compare(*it)) {
			return true;
		}
	}
	return false;
}

// 状态是否在close表中
bool Status::is_close(vector<Status> close) {
	for (int i = 0; i < close.size(); i++) {
		if (this->compare(close[i])) {
			return true;
		}
	}
	return false;
}