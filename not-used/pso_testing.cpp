#include <vector>
#include <random>
#include <algorithm>
#include <unordered_set>
#include <thread>
#include <mutex>
#include "../util.cpp"
#include "../jssp.cpp"

auto print_positions = [](const std::vector<float>& positions) {
	util::print("Positions: ");
	for (const auto& pos : positions)
		util::print("{}, ", pos);
	util::println();
};

namespace pso {

	struct Particle {
        using Position = std::vector<float>;
        using Velocity = Position;
        
        Position position;
        Velocity velocity;
        Position pbest_position;
        int pbest_makespan;
        int stagnation_count = 0;
    };

    struct Pso {
        jssp::Jobs& jobs;
        const int number_of_machines;
        const int number_of_jobs;
        const int number_of_operations;
        
        // PSO parameters
        int iterations = 2000;
        int number_of_particles = 40;
        float w_init = 0.9f;
        float w_end = 0.4f;
        float c1 = 0.5f;  // cognitive
        float c2 = 0.5f;  // social
        float cl = 1.5f;  // local
        float cn = 1.5f;  // neighborhood
        float constriction = 0.729f;
        float max_velocity = 0.25f;
        
        // Schedule parameters
        float delta_init = 0.7f;
        float delta_end = 0.3f;
        float current_delta;
        
        std::vector<Particle> swarm;        
        Particle::Position gbest_position;
        int gbest_makespan = std::numeric_limits<int>::max();
        
        std::mt19937 random_engine;
        std::uniform_real_distribution<float> uniform_dist;
        std::uniform_int_distribution<int> int_dist;

        Pso(jssp::Jobs& jobs, int num_machines) : 
            jobs(jobs),
            number_of_machines(num_machines),
            number_of_jobs(jobs.size()),
            number_of_operations(jobs.size() * num_machines),
            random_engine(std::random_device()()),
            uniform_dist(0, 1),
            int_dist(0, number_of_particles-1) {
            
            current_delta = delta_init;
        }
			
		void set_iterations(int iterations) {
			this->iterations = iterations;
		}
		void set_number_of_particles(int number_of_particles) {
			this->number_of_particles = number_of_particles;
		}
		
		void set_c1(float c1) {
			this->c1 = c1;
		}
		void set_c2(float c2) {
			this->c2 = c2;
		}

		void init_swarm() {
            swarm.resize(number_of_particles);
            for (auto& p : swarm) {
                p.position.resize(number_of_operations);
                p.velocity.resize(number_of_operations);
                
                for (int i = 0; i < number_of_operations; ++i) {
                    p.position[i] = uniform_dist(random_engine);
                    p.velocity[i] = uniform_dist(random_engine) * 0.1f;
                }
                
                p.pbest_position = p.position;
                p.pbest_makespan = evaluate(p.position);
                
                if (p.pbest_makespan < gbest_makespan) {
                    gbest_position = p.pbest_position;
                    gbest_makespan = p.pbest_makespan;
                }
            }
        }

		int evaluate(const Particle::Position& position) {
            auto schedule = build_schedule(position);
            return jssp::makespan_schedule(schedule, jobs.size(), number_of_machines);
        }

		jssp::Schedule build_schedule(const Particle::Position& position) {
            auto priorities = decode_to_priorities(position);
            return build_active_schedule(priorities);
        }
		std::vector<int> decode_to_priorities(const Particle::Position& position) {
            std::vector<int> priorities(number_of_operations);
            auto indices = util::argsort(position);

            for (size_t i = 0; i < indices.size(); ++i) {
                priorities[indices[i]] = i;
            }
            
            return priorities;
        }
		jssp::Schedule build_active_schedule(const std::vector<int>& priorities) {
            jssp::Schedule schedule;
            
            std::vector<int> job_op_count(number_of_jobs, 0);
            std::vector<int> machine_available(number_of_machines, 0);
            std::vector<int> job_available(number_of_jobs, 0);
            
            for (int t = 0; t < number_of_operations; ++t) {
                // Find schedulable operations
                std::vector<std::pair<int, int>> schedulable_ops;
                for (int j = 0; j < number_of_jobs; ++j) {
                    if (job_op_count[j] < number_of_machines) {
                        int op_index = job_op_count[j];
                        schedulable_ops.emplace_back(j, op_index);
                    }
                }
                
                // Calculate sigma* and phi*
                int sigma_star = INT_MAX;
                int phi_star = INT_MAX;
                
                for (const auto& [j, op_idx] : schedulable_ops) {
                    const auto& task = jobs[j][op_idx];
                    int m = task.machine;
                    int start = std::max(job_available[j], machine_available[m]);
                    int end = start + task.time;
                    
                    sigma_star = std::min(sigma_star, start);
                    phi_star = std::min(phi_star, end);
                }
                
                // Select operation to schedule
                int best_j = -1, best_op = -1;
                int best_priority = INT_MAX;
                int best_end_time = INT_MAX;
                
                for (const auto& [j, op_idx] : schedulable_ops) {
                    const auto& task = jobs[j][op_idx];
                    int m = task.machine;
                    int start = std::max(job_available[j], machine_available[m]);
                    int end = start + task.time;
                    int global_op_idx = j * number_of_machines + op_idx;
                    int priority = priorities[global_op_idx];
                    
                    if (start <= sigma_star + current_delta * (phi_star - sigma_star)) {
                        if (priority < best_priority || 
                           (priority == best_priority && end < best_end_time)) {
                            best_priority = priority;
                            best_end_time = end;
                            best_j = j;
                            best_op = op_idx;
                        }
                    }
                }
                
                // Schedule the selected operation
                if (best_j != -1) {
                    const auto& task = jobs[best_j][best_op];
                    int m = task.machine;
                    int start = std::max(job_available[best_j], machine_available[m]);
                    int end = start + task.time;
                    
                    machine_available[m] = end;
                    job_available[best_j] = end;
                    job_op_count[best_j]++;
                    
                    schedule.push_back(jobs[best_j][best_op]);
                }
            }
            
            return schedule;
        }




		void run() {
            for (int iter = 0; iter < iterations; ++iter) {
                // Update parameters
                current_delta = delta_init - (delta_init - delta_end) * (iter / float(iterations));
                float w = w_init - (w_init - w_end) * (iter / float(iterations));
                
                for (auto& p : swarm) {
                    // Update velocity
                    float r1 = uniform_dist(random_engine);
                    float r2 = uniform_dist(random_engine);
                    float r3 = uniform_dist(random_engine);
                    
                    Particle::Position lbest = get_neighborhood_best(p);
                    
                    for (int i = 0; i < number_of_operations; ++i) {
                        float cognitive = c1 * r1 * (p.pbest_position[i] - p.position[i]);
                        float social = c2 * r2 * (gbest_position[i] - p.position[i]);
                        float local = cl * r3 * (lbest[i] - p.position[i]);
                        
                        p.velocity[i] = constriction * (w * p.velocity[i] + cognitive + social + local);
                        p.velocity[i] = std::clamp(p.velocity[i], -max_velocity, max_velocity);
                    }
                    
                    // Update position
                    for (int i = 0; i < number_of_operations; ++i) {
                        p.position[i] += p.velocity[i];
                        p.position[i] = std::clamp(p.position[i], 0.0f, 1.0f);
                    }
                    
                    // Evaluate
                    int makespan = evaluate(p.position);
                    
                    // Update personal best
                    if (makespan < p.pbest_makespan) {
                        p.pbest_position = p.position;
                        p.pbest_makespan = makespan;
                        p.stagnation_count = 0;
                        
                        // Update global best
                        if (makespan < gbest_makespan) {
                            gbest_position = p.position;
                            gbest_makespan = makespan;
                        }
                    } else {
                        p.stagnation_count++;
                    }
                    
                    // Apply mutation if stagnating
                    if (p.stagnation_count > 10) {
                        mutate_particle(p);
                        p.stagnation_count = 0;
                    }
                }
                
                // Adaptive parameter adjustment
                if (iter % 50 == 0) {
                    adapt_parameters();
                }
            }
        }

        Particle::Position get_neighborhood_best(const Particle& p) {
            // Ring topology implementation
            static std::vector<std::unordered_set<int>> neighborhoods;
            
            if (neighborhoods.empty()) {
                neighborhoods.resize(number_of_particles);
                for (int i = 0; i < number_of_particles; ++i) {
                    for (int j = 1; j <= 2; ++j) {  // Neighborhood size of 5
                        int left = (i - j + number_of_particles) % number_of_particles;
                        int right = (i + j) % number_of_particles;
                        neighborhoods[i].insert(left);
                        neighborhoods[i].insert(right);
                    }
                }
            }
            
            int current_index = &p - &swarm[0];
            int best_makespan = INT_MAX;
            Particle::Position best_position;
            
            for (int neighbor : neighborhoods[current_index]) {
                if (swarm[neighbor].pbest_makespan < best_makespan) {
                    best_makespan = swarm[neighbor].pbest_makespan;
                    best_position = swarm[neighbor].pbest_position;
                }
            }
            
            return best_position;
        }
		void mutate_particle(Particle& p) {
            int num_mutations = std::max(1, int(number_of_operations * 0.1f));
            
            for (int i = 0; i < num_mutations; ++i) {
                int idx = int_dist(random_engine);
                p.position[idx] = uniform_dist(random_engine);
                p.velocity[idx] = (uniform_dist(random_engine) - 0.5f) * 0.1f;
            }
        }

        void adapt_parameters() {
            // Reduce delta more aggressively if no improvement
            if (gbest_makespan == swarm[0].pbest_makespan) {
                current_delta = std::max(delta_end, current_delta * 0.95f);
            }
        }

        jssp::Schedule get_best_schedule() {
            return build_schedule(gbest_position);
        }
        
        
	};

}