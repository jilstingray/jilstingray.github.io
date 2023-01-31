import bisect
import copy
import math
import operator
import re
import sys
import time

class Problem(object):
    """Define the abstract problem in the search module.

    Attributes:
        init: input.txt
        goal: goal.txt
    """

    def __init__(self, init, goal):
        self.init = init
        self.goal = goal

    def actions(self, state):
        """Return the actions (directions) that can be execute in the current state."""
        raise NotImplementedError

    def result(self, state, action):
        """Return the state that results from executing the action in the current state."""
        raise NotImplementedError

    def goal_test(self, status):
        """If status == goal, return True"""
        return operator.eq(self.goal.grid, status.state.grid)

    def path_cost(self, pre_cost, prestate, action, currstate):
        """Return the cost of a solution path that arrives at the current state from the previous one via action."""
        return pre_cost + 1

    def value(self, state):
        """For optimization problems, each state has a value."""
        raise NotImplementedError


class PriorityQueue():
    def __init__(self, order = 'min', f = lambda x: x):
        self.items = []
        self.order = order
        self.f_func = f

    def __len__(self):
        return len(self.items)

    def __contains__(self, item):
        return any(operator.eq(item.state.grid, i.state.grid) for i in self.items)

    def __getitem__(self, item):
        """Rewrite '[]' operator like dict(): self[item] <-- item"""
        for i in self.items:
            if i == item:
                return i

    def __delitem__(self, item):
        for i in enumerate(self.items):
            if i == item:
                self.items.pop(i)

    def put(self, item):
        item.f_func = self.f_func(item)
        bisect.insort(self.items, item)
 
    def get(self):
        if self.order == 'max':
            return self.items.pop()
        else:
            return self.items.pop(0)

    def top(self):
        return self.items[0]


class NNumberProblem(Problem):
    """Define the specific N puzzle problem (N = 24)

    Attributes:
        init: initial state
        goal: goal state
    """
    def __init__(self, init = None, goal = None):
        self.init = init
        self.goal = goal

    def actions(self, state):
        """Return allowed actions list at the current state."""
        [row, col] = state.location(0)                          # The coordinate of '0'
        allowed_actions = [[-1, 0], [1, 0], [0, -1], [0, 1]]    # Possible actions: U, D, L, R
        """Restrict some actions at the border of the 5*5 matrix."""
        if row == 0 and col != 2:
            if [-1, 0] in allowed_actions:
                allowed_actions.remove([-1, 0]) # 'U' is not allowed.
        if row == 4 and col != 2:
            if [1, 0] in allowed_actions:
                allowed_actions.remove([1, 0])  # 'D' is not allowed.
        if row != 2 and col == 0:
            if [0, -1] in allowed_actions:
                allowed_actions.remove([0, -1]) # 'L' is not allowed.
        if row != 2 and col == 4:
            if [0, 1] in allowed_actions:
                allowed_actions.remove([0, 1])  # 'R' is not allowed.
        return allowed_actions

    def result(self, state, action):
        """Return the grid results from the given action."""
        [row, col] = state.location(0)
        actr, actc = action[0], action[1]
        temp = state.grid[(row + actr) % 5][(col + actc) % 5]
        next_state = copy.deepcopy(state)
        next_state.grid[(row + actr) % 5][(col + actc) % 5] = 0    # Move '0'.
        next_state.grid[row][col] = temp
        return next_state

    def path_cost(self, status):
        """The path cost of the expanded child status."""
        return status.path_cost + 1

    def h_func(self, status, func):
        """h(S) = number of misplaced tiles"""
        if not self.init or not self.goal:
            return float('inf')
        return func(status.state, self.goal)


class Status:
    """Define the status of the graph.

    Attributes:
        state: See "class State" below.
        parent: The parent status of the current status.
        action: The direction of the movement of '0' (U/D/L/R).
        path_cost: The path cost value of current status.
        f_func: The value of the search funtion (f(S) = cost(S) + h(S)).
        depth: The depth of the current status.
    """

    def __init__(self, state, parent = None, action = None, path_cost = 0, depth = 0):
        """Initialize class Status."""
        self.state = state
        self.parent = parent
        self.action = action
        self.path_cost = path_cost
        self.f_func = 0
        self.depth = depth
        if parent:
            self.depth = parent.depth + 1

    def __repr__(self):
        """Define the string format to describe (print) the state of the status."""
        return "{}".format(self.state.grid)

    def __lt__(self, status):
        """Rewrite '<' operator."""
        return self.f_func < status.f_func

    def __eq__(self, status):
        """Rewrite '==' operator."""
        return isinstance(status, Status) and operator.eq(self.state.grid, status.state.grid)

    def __hash__(self):
        value = 0
        for x in range(5):
            for y in range(5):
                value = value * 10 + self.state.grid[x][y]
        return value

    def get_child(self, problem, action):
        """The child status of the current status."""
        next_state = problem.result(self.state, action)
        return Status(next_state, self, action, problem.path_cost(self))

    def expand(self, problem):
        """Return the list of the statuss reachable in one step from the current status."""
        child_list = []
        for action in problem.actions(self.state):
            child_list.append(self.get_child(problem, action))
        return child_list

    def path(self):
        """Return the path from the root to the current status."""
        status, repath = self, []
        while status and status.action:
            repath.append(status.action)
            status = status.parent
        repath.append([self.depth])
        return list(reversed(repath))


class State:
    """Define the state of each status.
    Attributes:
        grid
        action
    """

    def __init__(self, grid = None, action = None):
        self.grid = grid
        self.action = action

    def location(self, number):
        for row in range(5):
            for col in range(5):
                if self.grid[row][col] == number:
                    return [row, col]
        return []


def best_first_graph_search(problem, f):
    steps = 0
    status = Status(problem.init)
    status.f_func = f(status)
    open_list = PriorityQueue('min', f)
    open_list.put(status)
    close_list = set()
    while open_list.items != []:
        if steps > 100000:
            print("\nFailed to find the path within 100000 steps.")
            sys.exit()
        status = open_list.get()
        if problem.goal_test(status) == True:
            print("\nsteps: %d\n" % steps)
            return status
        steps += 1
        close_list.add(status.state)
        for child in status.expand(problem):
            if child.state not in close_list and child not in open_list:
                open_list.put(child)
            elif child in open_list:
                if f(child) < f(open_list[child]):
                    del open_list[child]
                    open_list.put(child)
    return None


def astar_search(problem, func):
    """A* search is a revised best-first graph search with f(S) = cost(S) + h(S)."""
    h = problem.h_func
    return best_first_graph_search(problem, lambda S: S.path_cost + h(S, func))


def read_state_grid(filename):
    """Read the initial/goal grid of the graph from txt files."""
    state_grid = []
    file = open(filename, "r")    
    count = 0
    for l in file.readlines():
        count += 1
        if count > 5:
            break
        temp = []
        line = []
        temp = re.split(r'[\s]', l)[:5]
        for i in temp:
            line.append(int(i))
        state_grid.append(line)
    file.close()
    return state_grid


def show_result(timecount, path):
    print(len(path) - 1)
    for i in path[1:]:
        if i == [-1, 0]:
            print("U", end = "")
        elif i == [1, 0]:
            print("D", end = "")
        elif i == [0, -1]:
            print("L", end = "")
        elif i == [0, 1]:
            print("R", end = "")
    print("\n%.6fs\n" % timecount)
    return

def h_func(state, goal):
    value = 0
    for row in range(5):
        for col in range(5):
            if state.grid[row][col] != goal.grid[row][col]:
                value += 1
    return value


def main():
    start_time = time.perf_counter()
    init = State(read_state_grid('input.txt'))
    goal = State(read_state_grid('goal.txt'))
    init.action = NNumberProblem().actions(init)
    goal.action = NNumberProblem().actions(goal)
    problem = NNumberProblem(init, goal)
    result = astar_search(problem, h_func)
    stop_time = time.perf_counter()
    show_result(stop_time - start_time, result.path())
    return


if __name__ == '__main__':
    main()