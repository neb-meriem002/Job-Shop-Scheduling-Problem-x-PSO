#include <vector>
#include <list>
#include <algorithm>
#include <ranges>
#include "util.cpp"
#include <unordered_map>
#include <iostream>

struct task {
	int job;
	int time;
	int index;
	int machine;
	int priority; // higher value = higher priority
};

using schedule = std::vector<task>;

void sort_index(schedule& s) {
    // Stable sort to maintain priority order while grouping by job/index
	for (int i = 0; i < s.size()-1; ++i) {
		for (int j=i+1; j < s.size(); ++j) {
			if(s[i].job == s[j].job && s[i].index > s[j].index) {
				std::rotate(s.begin() + i, s.begin() + i + 1, s.begin() + j + 1);
			}
		}
	}
    
}

int makespan(const schedule& s) {
    std::unordered_map<int, int> machine_time;
    std::unordered_map<int, int> job_progress;
    std::unordered_map<int, int> job_time;

    schedule sorted_schedule = s;
    
    // Single sort operation that handles both priority and sequence
    std::sort(sorted_schedule.begin(), sorted_schedule.end(),
        [](const task& a, const task& b) {
            // First by index (ascending), then by priority (descending)
            return a.priority > b.priority;
        });
	sort_index(sorted_schedule);

    for (const auto& t : sorted_schedule) {
        // Verify operation sequence

        int start = std::max(machine_time[t.machine], job_time[t.job]);
        int end = start + t.time;
        
        util::println("Job {} Op {} (priority {}) on M{}: {} -> {}",
                     t.job, t.index, t.priority, t.machine, start, end);
        
        machine_time[t.machine] = end;
        job_time[t.job] = end;
        job_progress[t.job] = t.index;
    }

    int makespan = 0;
    for (const auto& [machine, time] : machine_time) {
        makespan = std::max(makespan, time);
    }
    return makespan;
}
// 1 : 1,2 2,1
// 2 : 2,2 1,1

int main() {

	schedule schedule {
		{0, 2, 1, 0,2}, {1, 3, 1, 1,2}, {1, 4, 2, 0,1}, {0, 5, 2, 1,1}
	};
	// schedule = {
	// 	{0, 2, 2, 0}, {1, 3, 1, 0}, {1, 4, 2, 1}, {0, 5, 1, 1}
	// };
	util::println("makespan {}",makespan(schedule));
	
	return 0;

}