#include <iostream>
#include <list>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "status.h"
#include "utils.h"
#include "search.h"

using namespace std;

int main() {
	int init[25], goal[25];
	vector<char> path;
	bool result;
	clock_t timer, start_timer;
	int _switch;

	init_grid(init, "input.txt");
	init_grid(goal, "goal.txt");

	printf("1 - A* search\n2 - A* search (revised)\n");
	printf("3 - IDA * search\n4 - IDA * search(revised)\n");
	printf("Input:");
	scanf("%d", &_switch);
	printf("\n");

	switch (_switch) {
	// A*
	case 1:
		start_timer = clock();
		result = a_star(init, goal, path, TYPE_STD);
		timer = clock() - start_timer;
		print_result(result, path, timer, "A1_solution.txt");
		break;

	// A* with the mixed heuristic function
	case 2:
		start_timer = clock();
		result = a_star(init, goal, path, TYPE_MIX);
		timer = clock() - start_timer;
		print_result(result, path, timer, "A2_solution.txt");
		break;

	// IDA*
	case 3:
		start_timer = clock();
		result = ida_star(init, goal, path, TYPE_STD);
		timer = clock() - start_timer;
		reverse(path.begin(), path.end());
		print_result(result, path, timer, "IDA1_solution.txt");
		break;

	// IDA* with the mixed heuristic function
	case 4:
		start_timer = clock();
		result = ida_star(init, goal, path, TYPE_MIX);
		timer = clock() - start_timer;
		reverse(path.begin(), path.end());
		print_result(result, path, timer, "IDA2_solution.txt");
		break;

	// incorrect input
	default:
		printf("Incorrect input. Try again.\n\n");
		main();
	}

	return 0;
}