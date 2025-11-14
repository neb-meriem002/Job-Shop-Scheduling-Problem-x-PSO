#include <vector>
#include <list>
#include <algorithm>
#include <ranges>
#include <random>
#include <fstream>

#include "parse.cpp"
#include "util.cpp"
#include "jssp.cpp"



/* dfs optimized
- store the last task index of each job in a vector initialized to 0
- create an empty schedule
- call dfs with the initial schedule and the last task index vector
- in the dfs function:
  - if the schedule is full, evaluate it and update the best schedule if necessary
  - else
	- for each job
	- get the last task index of the job
	- get the task at that index
	- add the task to the schedule
	- increment the last task index of the job
	- evaluate the schedule
	- if the schedule is worse than the best schedule, remove the task from the schedule and decrement the last task index of the job and continue
	- call dfs with the new schedule and the new last task index of the job
	*/


/*
util::minutes max_time(30);
util::stopwatch sw;
std::string filename = util::format("experiments/results/dfs-optimized-benchmark.csv");
auto write = [](int j, int m, int makespan, auto time) {
	util::write(filename, util::format("{},{},{},\n", util::format("{} {}", j, m), makespan, time), std::ios::app);
};


void _dfs_optimized(const jssp::Jobs& jobs, jssp::Schedule& schedule, int number_of_machines, int& best_makespan, jssp::Schedule& best_schedule, std::vector<int>& last_task_indices) {
	if (schedule.size() == jobs.size() * number_of_machines) {
		int current_makespan = jssp::makespan_schedule(schedule, jobs.size(), number_of_machines);
		if (current_makespan < best_makespan) {
			best_makespan = current_makespan;
			best_schedule = schedule;
		}
		if (sw.elapsed<util::minutes>() > max_time) {
			// write(jobs.size(), number_of_machines, best_makespan, sw.elapsed<util::minutes>());
			util::println("Best makespan: {}", best_makespan);
			std::exit(0);
		}
		return;
	}

	for (int job = 0; job < jobs.size(); ++job) {
		if (sw.elapsed<util::minutes>() > max_time) {
			// write(jobs.size(), number_of_machines, best_makespan, sw.elapsed<util::minutes>());
			util::println("Best makespan: {}", best_makespan);
			std::exit(0);
		}
		if (last_task_indices[job] < jobs[job].size()) {
			auto task = jobs[job][last_task_indices[job]];
			schedule.push_back(task);
			last_task_indices[job]++;
			int current_makespan = jssp::makespan_schedule(schedule, jobs.size(), number_of_machines);
			if (current_makespan < best_makespan) {
				_dfs_optimized(jobs, schedule, number_of_machines, best_makespan, best_schedule, last_task_indices);
			}
			schedule.pop_back();
			last_task_indices[job]--;
		}
	}

}

int dfs_optimized(const jssp::Jobs& jobs, int number_of_machines) {
	jssp::Schedule best_schedule, current_schedule;
	int best_makespan = std::numeric_limits<int>::max();
	std::vector<int> last_task_indices(jobs.size(), 0);
	_dfs_optimized(jobs, current_schedule, number_of_machines, best_makespan, best_schedule, last_task_indices);
	util::println("Best makespan: {}", best_makespan);
	jssp::print_schedule(best_schedule);
	return best_makespan;
}

void benchmark_dfs_optimized(int j, int m) {
	// util::write(filename, "jobs machines,makespan,time,\n", std::ios::out | std::ios::trunc);
	dfs_optimized(load_jobs(util::format("experiments/benchmarks/tai{}_{}.txt", j, m))[0], m);
}

void benchmark_dfs() {
	// calculate the time taken to run the dfs function on given jobs and save it in experiments/results/dfs-optimized.csv
	std::string filename = "experiments/results/dfs-optimized.csv";

	auto write = [&](int j, int m, int makespan, int time) {
		util::write(filename, util::format("{},{},{},{}\n", j, m, makespan, time), std::ios::app);
	};
	int max_jobs = 6;
	int max_machines = 6;
	util::stopwatch sw;
	for (int j = 2; j <= max_jobs; j++) {
		for (int m = 2; m <= max_machines; m++) {
			jssp::Jobs jobs = jssp::generate_random_jobs(j, m);
			sw.init();
			int makespan = dfs_optimized(jobs, m);
			int time = sw.elapsed<util::microseconds>().count();
			write(j, m, makespan, time);
		}
	}
}

*/



void _dfs_optimized(const jssp::Jobs& jobs, jssp::Schedule& schedule, int number_of_machines, int& best_makespan, jssp::Schedule& best_schedule, std::vector<int>& last_task_indices) {
	if (schedule.size() == jobs.size() * number_of_machines) {
		int current_makespan = jssp::makespan_schedule(schedule, jobs.size(), number_of_machines);
		if (current_makespan < best_makespan) {
			best_makespan = current_makespan;
			best_schedule = schedule;
		}
		return;
	}

	for (int job = 0; job < jobs.size(); ++job) {
		if (last_task_indices[job] < jobs[job].size()) {
			auto task = jobs[job][last_task_indices[job]];
			schedule.push_back(task);
			last_task_indices[job]++;
			int current_makespan = jssp::makespan_schedule(schedule, jobs.size(), number_of_machines);
			if (current_makespan < best_makespan) {
				_dfs_optimized(jobs, schedule, number_of_machines, best_makespan, best_schedule, last_task_indices);
			}
			schedule.pop_back();
			last_task_indices[job]--;
		}
	}

}

int dfs_optimized(const jssp::Jobs& jobs, int number_of_machines) {
	jssp::Schedule best_schedule, current_schedule;
	int best_makespan = std::numeric_limits<int>::max();
	std::vector<int> last_task_indices(jobs.size(), 0);
	_dfs_optimized(jobs, current_schedule, number_of_machines, best_makespan, best_schedule, last_task_indices);
	util::println("Best makespan: {}", best_makespan);
	jssp::print_schedule(best_schedule);
	return best_makespan;
}

void test_dfs() {
	int j = 4;
	int m = 4;
	jssp::Jobs jobs = jssp::generate_random_jobs(j, m);
	jssp::print_jobs(jobs);
	dfs_optimized(jobs, m);
}



int main() {

	test_dfs();

    return 0;
}