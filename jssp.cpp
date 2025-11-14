
#pragma once
#include <vector>
#include <ranges>
#include <algorithm>
#include <limits>
#include <tuple>
#include <vector>
#include "util.cpp"

namespace jssp {

	struct Task {
		int job;
		int time;
		int index;
		int machine;
		// float priority = 0.0f;
	};
	
	using Schedule = std::vector<Task>;
	using Jobs = std::vector<std::vector<Task>>;
	using Machines = Jobs;
	
	int makespan(Schedule& schedule, int j, int m) {
		using task_queeu_type = std::vector<Task>;
		std::vector<task_queeu_type> job_waiting_tasks(j);
		std::vector<task_queeu_type> machine_waiting_tasks(m);
		std::vector<int> machine_times(m, 0);
		std::vector<int> job_times(j, 0);
	
		// util::stopwatch stopwatch;
		// stopwatch.init();
		for (auto& task : schedule) {
			auto& job_queue = job_waiting_tasks[task.job];
			job_queue.push_back(task);
			std::sort(job_queue.begin(), job_queue.end(), [](struct Task& a, struct Task& b) {
				return a.index < b.index;
			});
			machine_waiting_tasks[task.machine].push_back(task);
		}
		// util::println("Initializing machine job queues took: {}", stopwatch.elapsed<util::microseconds>());
		
		// util::println("Job waiting tasks:");
		// for (auto [i, job_queue] : std::ranges::views::enumerate(job_waiting_tasks)) {
		// 	util::println("Job queue: {}", i);
		// 	for (const auto& task : job_queue) {
		// 		util::println("  Task: job {}, machine {}, index {}, time {}", task.job, task.machine, task.index, task.time);
		// 	}
		// }
		
		// util::println("Machine waiting tasks:");
		// for (auto [i, machine_queue] : std::ranges::views::enumerate(machine_waiting_tasks)) {
		// 	util::println("Machine queue: {}", i);
		// 	for (const auto& task : machine_queue) {
		// 		util::println("  Task: job {}, machine {}, index {}, time {}", task.job, task.machine, task.index, task.time);
		// 	}
		// }
	
		// stopwatch.init();
		bool completed_task, empty_job_queue;
		do {
			completed_task = false;
			empty_job_queue = true;
			for (auto& job_queue : job_waiting_tasks) {
				if (not job_queue.empty()) {
					empty_job_queue = false;
					auto task = job_queue.front();
					auto& machine_queue = machine_waiting_tasks[task.machine];
					auto machine_task = machine_queue.front();
	
					// util::println("Processing task: job {}, machine {}, index {}, time {}, Machine task: job {}, machine {}, index {}, time {}", 
						// task.job, task.machine, task.index, task.time, machine_task.job, machine_task.machine, machine_task.index, machine_task.time);
	
					// check if the task can be completed
					if (task.job == machine_task.job) {
						completed_task = true;
	
						// remove task from job queue and machine queue
						job_queue.erase(job_queue.begin());
						machine_queue.erase(machine_queue.begin());
	
						// update machine and job times
						int max_time = std::max(machine_times[task.machine], job_times[task.job]) + task.time;
						machine_times[task.machine] = max_time;
						job_times[task.job] = max_time;
						// util::println("Completed task: job {}, machine {}, index {}, time {}, new machine and job times: {}",
							// task.job, task.machine, task.index, task.time, machine_times[task.machine]);
					}
				}
			}
	
		} while (completed_task);
		// util::println("Calculating makespan took: {}", stopwatch.elapsed<util::microseconds>());
		
		// util::println("Final machine times:");
		// for (auto [i, time] : std::ranges::views::enumerate(machine_times)) {
			// util::println("Machine {}: {}", i, time);
		// }
	
		int makespan = 0;
		if (empty_job_queue) {
			makespan = *std::max_element(job_times.begin(), job_times.end());
			// util::println("All tasks completed, makespan: {}", makespan);
		} else {
			makespan = std::numeric_limits<int>::max();
			// util::println("Not all tasks completed, makespan: {}", makespan);
		}
		
	
		return makespan;
	}
	
	int makespan_optimized(Jobs& jobs, Machines& machine_queues) {
		int m = machine_queues.size();
		int j = jobs.size() * jobs[0].size();
	
		std::vector<int> job_indecies(jobs.size(), 0);
		std::vector<int> machine_indecies(m, 0);
	
		std::vector<int> machine_times(m, 0);
		std::vector<int> job_times(jobs.size(), 0);
	
		std::vector<Task> tasks;
		tasks.reserve(j);
		int tasks_index = 0;
	
		auto init_tasks = [&]() {
			for (auto& job : jobs) {
				auto& task = job.front();
				auto& machine_task = machine_queues[task.machine].front();
				// check if the first task in the job queue is the same as the first task in the machine queue and has the same index
				if (machine_task.job == task.job and machine_task.index == task.index) {
					tasks.push_back(task);
				}
			}
		};
		auto add_next_task_to_queue = [&](int last_job, int last_machine) {
			auto& machine_queue = machine_queues[last_machine];
			auto& job = jobs[last_job];
			
			// check if the current task in the machine queue can be added to the task queue by checking if it is the head of it's job queue
			if (machine_indecies[last_machine] < machine_queue.size()) {
				auto& task = machine_queue[machine_indecies[last_machine]];
				if (job[job_indecies[task.job]].index == task.index) {
					tasks.push_back(task);
				}
			}
			// check if the current task in the job of the last job processed can be added to the task queue by checking if it is the head of any machine queue
			if (job_indecies[last_job] < job.size()) {
				auto& task = job[job_indecies[last_job]];
				auto& machine_queue = machine_queues[task.machine];
				auto& machine_task = machine_queue[machine_indecies[task.machine]];
				if (machine_task.job == task.job) {
					tasks.push_back(task);
				}
			}
			
		}; 
		
		// initialize the tasks queue
		init_tasks();
	
		while (tasks_index < tasks.size()) {
			auto& task = tasks[tasks_index];
	
			// go to next task in job and machine queue
			job_indecies[task.job]++;
			machine_indecies[task.machine]++;
	
			// update machine and job times
			int max_time = std::max(machine_times[task.machine], job_times[task.job]) + task.time;
			machine_times[task.machine] = max_time;
			job_times[task.job] = max_time;
	
			// add next tasks to queue
			tasks_index++;
			add_next_task_to_queue(task.job, task.machine);
		}
	
		int makespan = 0;
		if (tasks.size() == j) 
			makespan = *std::max_element(job_times.begin(), job_times.end());
		else
			makespan = std::numeric_limits<int>::max();
	
		return makespan;
	} 

	int makespan_schedule(Schedule& schedule, int j, int m) {
		std::vector<int> machine_times(m, 0);
		std::vector<int> job_times(j, 0);
	
		for (auto& task : schedule) {
			int max_time = std::max(machine_times[task.machine], job_times[task.job]) + task.time;
			machine_times[task.machine] = max_time;
			job_times[task.job] = max_time;
		}
	
		return *std::max_element(job_times.begin(), job_times.end());
	}

	void print_jobs(const jssp::Jobs& jobs) {
		for (const auto& job : jobs) {
			for (const auto& task : job) {
				util::println("Job {}, Machine {}, Index {}, Time {}", task.job, task.machine, task.index, task.time);
			}
		}
	} 

	void print_schedule(const jssp::Schedule& schedule) {
		for (const auto& task : schedule) {
			util::println("Job {}, Machine {}, Index {}, Time {}", task.job, task.machine, task.index, task.time);
		}
	}

	void sort_schedule(Schedule& schedule, int j, int m) {
				
		// initialize current_index of all jobs
		// for each i, task in schedule
		//  job = task.job
		// 	job_index& = current_index[job]
		//  tasks[job][task.index] = task
		//  new_index[job][job_index] = i


		std::vector<int> current_index(j, 0);
		std::vector<std::vector<Task>> jobs(j, std::vector<Task>(m));
		std::vector<std::vector<int>> new_index(j, std::vector<int>(m, 0));

		for (int i = 0; i < schedule.size(); ++i) {
			auto& task = schedule[i];
			int job = task.job;
			int& job_index = current_index[job];
			// put the task in it's correct position, in it's task.index
			jobs[job][task.index] = task;
			// store the old task position in schedule following it's order of discovering
			new_index[job][job_index] = i;
			job_index++;
		}

		for (auto [job, job_indices] : std::ranges::views::zip(jobs, new_index)) {
			for (auto [task, index] : std::ranges::views::zip(job, job_indices)) {
				schedule[index] = task;
			}
		}

	}

	jssp::Machines generate_random_machines(jssp::Schedule& schedule, int m) {
		jssp::Machines machines(m);
		for (auto& task : schedule) 
			machines[task.machine].push_back(task);
	
		return machines;
	}
		
	jssp::Schedule generate_random_schedule(jssp::Jobs& jobs) {
		jssp::Schedule schedule;
		std::random_device rd;
		std::mt19937 g(rd());
		std::uniform_int_distribution<int> dist(0, schedule.size() - 1);
		for (auto& job : jobs) {
			for (auto& task : job) {
				schedule.push_back(task);
			}
		}
	
		std::shuffle(schedule.begin(), schedule.end(), g);
		return schedule;
	}
	
	jssp::Jobs generate_random_jobs(int j, int m) {
		std::mt19937 gen(std::random_device{}());
		std::uniform_int_distribution<int> dist(1, 4);

		jssp::Jobs jobs(j);
		std::vector<int> machines(m);
		for (int i = 0; i < m; ++i)
			machines[i] = i;
	
		for (int i = 0; i < j; ++i) {
			std::shuffle(machines.begin(), machines.end(), gen);
			for (int k = 0; k < m; ++k) {
				jobs[i].push_back({i, dist(gen), k, machines[k]});
			}
		}
		return jobs;
	}




	Schedule generate_schedule_shortest_completion_time(jssp::Jobs& jobs, int m) {
		Schedule schedule;
		std::vector<int> job_indices(jobs.size(), 0);

		auto get_next_task = [&] {
			int min_time = std::numeric_limits<int>::max();
			Task min_task {-1, -1, -1, -1};
			for (int i = 0; i < jobs.size(); ++i) {
				if (job_indices[i] < jobs[i].size()) {
					auto& task = jobs[i][job_indices[i]];
					if (task.time < min_time) {
						min_task = task;
					}
				}
			}
			return min_task;
		};
		while (true) {
			auto task = get_next_task();
			if (task.job == -1) break;
			schedule.push_back(task);
			job_indices[task.job]++;
		}
		return schedule;
	}
		



	Schedule generate_schedule_shortest_starting_time(Jobs& jobs, int m) {
		Schedule schedule;
		std::vector<int> job_indices(jobs.size(), 0);
		std::vector<int> machine_times(m, 0);
		std::vector<int> job_times(jobs.size(), 0);

		auto get_next_task = [&] {
			int min_start_time = std::numeric_limits<int>::max();
			Task min_task {-1, -1, -1, -1};

			for (auto [job, index] : std::ranges::views::zip(jobs, job_indices)) {
				if (index < job.size()) {
					auto& task = job[index];
					int start_time = std::max(machine_times[task.machine], job_times[task.job]);
					if (start_time < min_start_time) {
						min_start_time = start_time;
						min_task = task;
					}
				}
			}
			return min_task;
		};
		while (true) {
			auto task = get_next_task();
			if (task.job == -1) break;
			schedule.push_back(task);
			job_indices[task.job]++;
			machine_times[task.machine] = std::max(machine_times[task.machine], job_times[task.job]) + task.time;
			job_times[task.job] = machine_times[task.machine];
		}
		return schedule;
	}	
	



	Schedule generate_schedule_shortest_finishing_time(Jobs& jobs, int m) {
		Schedule schedule;
		std::vector<int> job_indices(jobs.size(), 0);
		std::vector<int> machine_times(m, 0);
		std::vector<int> job_times(jobs.size(), 0);

		auto get_next_task = [&] {
			int min_finish_time = std::numeric_limits<int>::max();
			Task min_task {-1, -1, -1, -1};

			for (auto [job, index] : std::ranges::views::zip(jobs, job_indices)) {
				if (index < job.size()) {
					auto& task = job[index];
					int finish_time = std::max(machine_times[task.machine], job_times[task.job]) + task.time;
					if (finish_time < min_finish_time) {
						min_finish_time = finish_time;
						min_task = task;
					}
				}
			}
			return min_task;
		};
		while (true) {
			auto task = get_next_task();
			if (task.job == -1) break;
			schedule.push_back(task);
			job_indices[task.job]++;
			machine_times[task.machine] = std::max(machine_times[task.machine], job_times[task.job]) + task.time;
			job_times[task.job] = machine_times[task.machine];
		}
		return schedule;
	}


}
