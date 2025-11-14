#include <vector>
#include <algorithm>
#include <random>
#include <unordered_map>
#include <vector>
#include <list>
#include <ranges>
#include<format>
#include "../util.cpp"
#include "../main.cpp"

schedule best_schedule;
int min_makespan = 1000;

int number_jobs=5;
int number_machines=5;

void dfs(schedule& current, const std::vector<task>& all_tasks, 
         std::vector<bool>& used, 
         std::unordered_map<int, int>& next_op_needed) {
    
    // Base case: complete schedule
    if (current.size() == all_tasks.size()) {
        int current_makespan = makespan(current,number_jobs,number_machines);
        util::println("makespan {}", current_makespan);
        if (current_makespan < min_makespan) {
            min_makespan = current_makespan;
            best_schedule = current;
        }
        return;
    }

    // Recursive case: try all possible next tasks
    for (size_t i = 0; i < all_tasks.size(); ++i) {
        const task& t = all_tasks[i];
        // Check if task is available:
        // 1. Not already used
        // 2. Is the next required operation for its job
        if (!used[i] && next_op_needed[t.job] == t.index) {
            // Prune if current partial makespan already exceeds best
            schedule partial = current;
            partial.push_back(t);
            int partial_makespan = makespan(partial,number_jobs,number_machines);
            util::println("partial makespan {}",partial_makespan);
            if (partial_makespan >= min_makespan) continue;
            
            // Explore this branch
            used[i] = true;
            next_op_needed[t.job]++;
            
            current.push_back(t);
            dfs(current, all_tasks, used, next_op_needed);
            current.pop_back();
            
            // Backtrack
            used[i] = false;
            next_op_needed[t.job]--;
        }
    }
}

schedule solve_jssp(const std::vector<task>& all_tasks) {
    schedule current;
    std::vector<bool> used(all_tasks.size(), false);
    
    // Initialize next_op_needed: each job starts needing operation 1
    std::unordered_map<int, int> next_op_needed;
    for (const auto& t : all_tasks) {
        next_op_needed[t.job] = 1;
    }
    
    dfs(current, all_tasks, used, next_op_needed);
    return best_schedule;
}


int main(){
    std::vector<task> all_jobs = {
        // Job 0 (5 operations)
        {0, 85, 1, 3},  // Op1: 85 time units on Machine 4
          {0, 64, 2, 0},  // Op2: 64 on M1
          {0, 31, 3, 2},  // Op3: 31 on M3
          {0, 44, 4, 4},  // Op4: 44 on M5
          {0, 66, 5, 1},// Op5: 66 on M2

        // Job 1
        {1,  7, 1, 0},
          {1, 14, 2, 3},
          {1, 69, 3, 1},
          {1, 18, 4, 4},
          {1, 68, 5, 2},

        // Job 2
        {2,  1, 1, 3},
          {2, 74, 2, 0},
          {2, 70, 3, 1},
          {2, 90, 4, 4},
          {2, 60, 5, 2},

        // Job 3
         {3, 45, 1, 1},
          {3, 76, 2, 3},
          {3, 13, 3, 4},
          {3, 98, 4, 2},
          {3, 54, 5, 0},

        // Job 4
        {4, 80, 1, 0},
          {4, 15, 2, 3},
          {4, 45, 3, 1},
          {4, 91, 4, 4},
          {4, 10, 5, 2}
    };
    schedule best_schedule = solve_jssp(all_jobs);
    util::println("best schedule : ");
    for(auto t:best_schedule){
      util::println("job {} , time {} , index {} , machine {} ",t.job,t.time,t.index,t.machine);
    }
    util::print("makespan {}",min_makespan);
    return 0;
}
