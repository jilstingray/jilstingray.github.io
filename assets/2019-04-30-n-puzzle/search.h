#ifndef SEARCH_H_
#define SEARCH_H_

#include <iostream>
#include <list>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "status.h"
#include "utils.h"

using namespace std;

bool a_star(int init[], int goal[], vector<char>& path, int type);
bool depth_first(Status S, int goal[], int depth, char act, int type);
bool ida_star(int init[], int goal[], vector<char> &path, int type);

#endif