
#include <vector>
#include <algorithm>
#include <map>
#include <stdlib.h>
#include <ranges>
#include "../util.cpp"

struct process {
	int job;
	int time;
	int op;
	int machine;
	int priority = 0;

	process(int job, int time, int operation, int machine) : job(job), time(time), op(operation), machine(machine) {}
};

using Schedule = std::vector<process>;

int makespan(Schedule& schedule) {
    std::map<int, int> machine_time; // Last finish time per machine
    std::map<int, int> job_time; // Last finish time per job
    std::map<int, std::vector<process>> machine_ops;

    // Group operations by machine
    for (const auto& op : schedule) {
        machine_ops[op.machine].push_back(op);
    }

	for (const auto& [machine, ops] : machine_ops) {
		int machine_finish_time = 0;
		for (const auto& op : ops) {
			int job_finish_time = job_time[op.job];
			int start_time = std::max(machine_finish_time, job_finish_time);
			machine_finish_time = start_time + op.time;
			job_time[op.job] = machine_finish_time;
			machine_time[machine] = machine_finish_time;
		}
	}
	int makespan = 0;
	for (const auto& [machine, finish_time] : machine_time) {
		makespan = std::max(makespan, finish_time);
	}
	// util::println("Max job time: {}, Max machine time: ({}, {})", max_job_time, max_machine - machine_times.begin(), max_machine_time);

    return makespan;
}



Schedule schedule {
	{0, 2, 1, 0}, {1, 3, 2, 0}, {1, 4, 1, 1}, {0, 5, 2, 1}
	
};

int main() {
	int time = makespan(schedule);
	util::println("Makespan: {}", time);
	return 0;
}

