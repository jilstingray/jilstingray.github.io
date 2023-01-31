#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "status.h"

using namespace std;

void init_grid(int a[], string filename);
void print_result(bool result, vector<char> path, clock_t timer, string filename);
bool insert_open(Status S, list<Status>& open);
int insert_close(Status S, list<Status>& close);
void get_path(Status S, vector<Status> close, vector<char> &path);
void expand(list<Status> &open, vector<Status> &close, int goal[], int type);

#endif