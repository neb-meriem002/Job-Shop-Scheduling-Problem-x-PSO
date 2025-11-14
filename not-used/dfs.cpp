#include <vector>
#include <list>
#include <algorithm>
#include <ranges>
#include <random>
#include <fstream>

#include "../parse.cpp"
#include "../util.cpp"
#include "../jssp.cpp"

int num;
struct task {
	int job;
	int time;
	int index;
	int machine;
};

using schedule = std::vector<task>;

int makespan(schedule& schedule, int j, int m) {
	using task_queeu_type = std::vector<task>;
	std::vector<task_queeu_type> job_waiting_tasks(j);
	std::vector<task_queeu_type> machine_waiting_tasks(m);
	std::vector<int> machine_times(m, 0);
	std::vector<int> job_times(j, 0);


     // Affichage ?????????????????????

     for (const auto& task : schedule) {
         std::cout << "(" << task.job << ", " << task.time << ", " << task.index << ", " << task.machine << ") ";
     }
     std::cout << std::endl;

	for (auto& task : schedule) {
		auto& job_queue = job_waiting_tasks[task.job];
		job_queue.push_back(task);
		std::sort(job_queue.begin(), job_queue.end(), [](struct task& a, struct task& b) {
			return a.index < b.index;
		});
		machine_waiting_tasks[task.machine].push_back(task);
	}

	// util::println("Job waiting tasks:");
	// for (auto [i, job_queue] : std::ranges::views::enumerate(job_waiting_tasks)) {
	// 	util::println("Job queue: {}", i);
	// 	for (const auto& task : job_queue) {
	// 		util::println("  Task: job {}, machine {}, index {}, time {}", task.job, task.machine, task.index, task.time);
	// 	}
	// }
	
	util::println("Machine waiting tasks:");
	for (auto [i, machine_queue] : std::ranges::views::enumerate(machine_waiting_tasks)) {
		util::println("Machine queue: {}", i);
		for (const auto& task : machine_queue) {
			util::println("  Task: job {}, machine {}, index {}, time {}", task.job, task.machine, task.index, task.time);
		}
	}


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

				// util::println("Processing task: job {}, machine {}, index {}, time {}", task.job, task.machine, task.index, task.time);
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
					// 	task.job, task.machine, task.index, task.time, machine_times[task.machine]);
				}
			}
		}

	} while (completed_task);
	util::println("Final machine times:");
	for (auto [i, time] : std::ranges::views::enumerate(machine_times)) {
		util::println("Machine {}: {}", i, time);
	}

    return *std::max_element(machine_times.begin(), machine_times.end());

}

void dfs(schedule& sched, int depth, int& best_makespan, schedule& best_schedule, int num_machines) {
    
    if (depth == sched.size()) {  
        

        int current_makespan = makespan(sched, sched.size(), num_machines);
        std::cout << "************************** " << num << std::endl;
        num=num+1;
        std::cout << "Checking permutation with makespan: " << current_makespan << std::endl;

        if (current_makespan < best_makespan) {

            best_makespan = current_makespan;

            best_schedule = sched; // MAJ
        }
        return;
    }

    
    
    for (int i = depth; i < sched.size(); ++i) {
        std::swap(sched[depth], sched[i]);  // Échanger les tâches
        dfs(sched, depth + 1, best_makespan, best_schedule, num_machines);
        std::swap(sched[depth], sched[i]);  // Annuler l'échange (backtracking)
    }
}
