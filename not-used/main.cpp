#include <vector>
#include <algorithm>
#include <ranges>
#include "../util.cpp"

struct job_process {
	int job;
	int time;
};

std::vector<std::vector<job_process>> machines {
	{ { 0, 4 }, { 1, 7 }},
	{ { 0, 1 }, { 1, 1 }},
};



int makespan(const std::vector<std::vector<job_process>>& machines) {
	std::vector<int> machine_times(machines.size(), 0);
	std::vector<int> job_times(machines[0].size(), 0);
	for (int j = 0; j < machines[0].size(); j++) {
		for (int i = 0; i < machines.size(); i++) {
			const auto& job_process = machines[i][j];

			if (machine_times[i] < job_times[job_process.job]) 
				machine_times[i] = job_times[job_process.job];
			machine_times[i] += job_process.time;
			job_times[job_process.job] = machine_times[i];
		}
	}
	int max_job_time = *std::max_element(job_times.begin(), job_times.end());
	auto max_machine = std::max_element(machine_times.begin(), machine_times.end());
	int max_machine_time = *max_machine;
	util::println("Max job time: {}, Max machine time: ({}, {})", max_job_time, max_machine - machine_times.begin(), max_machine_time);
	return *std::max_element(machine_times.begin(), machine_times.end());
}


int main() {
	util::println("Hello, World!");
	for (const auto& row : machines) {
		for (const auto& elem : row) {
			util::print("({} {}) ", elem.job, elem.time);
		}
		util::println("");
	}
	util::println("Makespan: {}", makespan(machines));
}