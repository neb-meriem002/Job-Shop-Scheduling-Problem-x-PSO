#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <limits>
#include "../util.cpp"

struct process {
    int job;
    double priority; // Real-valued priority for sorting
    int time;
};

using Machine = std::vector<process>;

struct Particle {
    std::vector<Machine> position;  // Current job sequence (machine-oriented)
    std::vector<Machine> velocity;  // Velocity matrix (small priority shifts)
    std::vector<Machine> best_position; // Best solution found by this particle
    double best_fitness; // Best makespan found by this particle
};

// Random number generator
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> rand_priority(0.0, 1.0);
std::uniform_real_distribution<> rand_velocity(-0.1, 0.1);

const int NUM_PARTICLES = 30;
const int NUM_ITERATIONS = 100;
const double w = 0.5;  // Inertia weight
const double c1 = 1.5; // Personal acceleration coefficient
const double c2 = 1.5; // Global acceleration coefficient

/*
// Function to compute makespan (same as your corrected implementation)
int makespan(const std::vector<Machine>& machines) {
    int num_machines = machines.size();
    std::vector<int> job_times(100, 0);
    std::vector<int> machine_times(num_machines, 0);

    for (int m = 0; m < num_machines; m++) {
        for (const auto& job : machines[m]) {
            int job_id = job.id;
            int start_time = std::max(machine_times[m], job_times[job_id]);
            int finish_time = start_time + job.time;

            machine_times[m] = finish_time;
            job_times[job_id] = finish_time;
        }
    }
    return *std::max_element(machine_times.begin(), machine_times.end());
}
*/

int makespan(const std::vector<std::vector<process>>& machines) {
	std::vector<int> machine_times(machines.size(), 0);
	std::vector<int> job_times(machines[0].size(), 0);
	for (int j = 0; j < machines[0].size(); j++) {
		for (int i = 0; i < machines.size(); i++) {
			const auto& process = machines[i][j];
			if (machine_times[i] < job_times[process.job]) 
				machine_times[i] = job_times[process.job];
			machine_times[i] += process.time;
			job_times[process.job] = machine_times[i];
		}
	}
	int max_job_time = *std::max_element(job_times.begin(), job_times.end());
	auto max_machine = std::max_element(machine_times.begin(), machine_times.end());
	int max_machine_time = *max_machine;
	// util::println("Max job time: {}, Max machine time: ({}, {})", max_job_time, max_machine - machine_times.begin(), max_machine_time);
	return max_machine_time;
}


// Initialize particles randomly
void initialize_particles(std::vector<Particle>& swarm, int num_machines, int num_jobs) {
    for (auto& particle : swarm) {
        particle.position.resize(num_machines);
        particle.velocity.resize(num_machines);
        particle.best_position.resize(num_machines);

        for (int m = 0; m < num_machines; m++) {
            for (int j = 0; j < num_jobs; j++) {
                process job = {j, rand_priority(gen), (rand() % 10) + 1}; // Random processing time
                particle.position[m].push_back(job);

                process velocity = {j, rand_velocity(gen), 0};
                particle.velocity[m].push_back(velocity);
            }
            // Sort jobs by initial priority
            std::sort(particle.position[m].begin(), particle.position[m].end(),
                      [](const process& a, const process& b) { return a.priority < b.priority; });
        }

        // Save as personal best
        particle.best_position = particle.position;
        particle.best_fitness = makespan(particle.position);
    }
}

// Update particle positions and velocities
void update_particles(std::vector<Particle>& swarm, const std::vector<Machine>& global_best_position) {
    for (auto& particle : swarm) {
        for (int m = 0; m < particle.position.size(); m++) {
            for (int j = 0; j < particle.position[m].size(); j++) {
                double r1 = rand_priority(gen);
                double r2 = rand_priority(gen);

                double v_new = w * particle.velocity[m][j].priority +
                               c1 * r1 * (particle.best_position[m][j].priority - particle.position[m][j].priority) +
                               c2 * r2 * (global_best_position[m][j].priority - particle.position[m][j].priority);

                // Update velocity and position
                particle.velocity[m][j].priority = v_new;
                particle.position[m][j].priority += v_new;
            }

            // Re-sort jobs based on updated priorities
            std::sort(particle.position[m].begin(), particle.position[m].end(),
                      [](const process& a, const process& b) { return a.priority < b.priority; });
        }

        // Evaluate new fitness (makespan)
        double new_fitness = makespan(particle.position);
        if (new_fitness < particle.best_fitness) {
            particle.best_position = particle.position;
            particle.best_fitness = new_fitness;
        }
    }
}

// PSO algorithm
std::vector<Machine> pso(int num_machines, int num_jobs) {
    std::vector<Particle> swarm(NUM_PARTICLES);
    initialize_particles(swarm, num_machines, num_jobs);

    // Find global best solution
    std::vector<Machine> global_best_position = swarm[0].best_position;
    double global_best_fitness = swarm[0].best_fitness;

    for (const auto& particle : swarm) {
        if (particle.best_fitness < global_best_fitness) {
            global_best_position = particle.best_position;
            global_best_fitness = particle.best_fitness;
        }
    }

    // PSO Iterations
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        update_particles(swarm, global_best_position);

        for (const auto& particle : swarm) {
            if (particle.best_fitness < global_best_fitness) {
                global_best_position = particle.best_position;
                global_best_fitness = particle.best_fitness;
            }
        }

        // std::cout << "Iteration " << iter + 1 << " - Best Makespan: " << global_best_fitness << "\n";
    }
	std::cout << "Final Best Makespan: " << global_best_fitness << "\n";

    return global_best_position;
}

// Display schedule
void print_schedule(const std::vector<Machine>& schedule) {
    for (int m = 0; m < schedule.size(); m++) {
        std::cout << "Machine " << m << ": ";
        for (const auto& job : schedule[m]) {
            std::cout << "[J" << job.job << ", T" << job.time << "] ";
        }
        std::cout << "\n";
    }
}

// Main function
int main() {
    int num_machines = 100;
    int num_jobs = 100;

    std::vector<Machine> best_schedule = pso(num_machines, num_jobs);

    std::cout << "\nBest Found Schedule:\n";
    print_schedule(best_schedule);
}
