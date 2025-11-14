#include <vector>
#include <random>
#include <algorithm>
#include <thread>
#include <mutex>
#include "util.cpp"
#include "jssp.cpp"

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
		// using JSP-Decoding method to convert the vector of positions into a schedule

		Position position;
		Velocity velocity;
		Position pbest_position;
		int pbest_makespan;
	};

	
	struct Pso {

		jssp::Jobs& jobs;
		int number_of_machines;
		int number_of_tasks;
		int iterations = 500;
		int number_of_particles = 30;

		float w = 0.2f; // inertia weight
		float c1 = 1.f; // cognitive weight
		float c2 = 1.5f; // social weight
		float max_velocity = 1.0f; // maximum velocity

		float delta = 0.5f;

		std::vector<Particle> swarm;		
		Particle::Position gbest_position;
		int gbest_makespan = std::numeric_limits<int>::max();


		std::mt19937 random_engine;
		std::uniform_real_distribution<float> uniform_real_dist;	


		Pso(jssp::Jobs& jobs, int number_of_machines) : jobs(jobs), number_of_machines(number_of_machines), number_of_tasks(jobs.size() * number_of_machines), 
			random_engine(std::random_device()()), uniform_real_dist(0, 5) {
		}
			
		void set_iterations(int iterations) {
			this->iterations = iterations;
		}
		void set_number_of_particles(int number_of_particles) {
			this->number_of_particles = number_of_particles;
		}
		void set_w(float w) {
			this->w = w;
		}
		void set_c1(float c1) {
			this->c1 = c1;
		}
		void set_c2(float c2) {
			this->c2 = c2;
		}
		void set_delta(float d) {
			this->delta = std::clamp(d, 0.f, 1.f);
		}


		void init_swarm() {
			auto init_positions = [&](int size) {
				std::vector<float> positions(size);
				for (int i = 0; i < size; ++i) 
					positions[i] = uniform_real_dist(random_engine);
				return positions;
			};

			swarm.resize(number_of_particles);
			for (auto& p : swarm) {
				p.position = init_positions(number_of_tasks);
				p.velocity = init_positions(number_of_tasks);
				p.pbest_position = p.position;
				p.pbest_makespan = fitness(p.position);
				if (p.pbest_makespan < gbest_makespan) {
					gbest_position = p.position;
					gbest_makespan = p.pbest_makespan;
				}
			}

			// util::println("Best makespan: {}", gbest_makespan);
			// util::print("Best position: ");
		}

		int fitness(Particle::Position& position) {
			jssp::Schedule schedule = generate_schedule_from_positions(position);
			int makespan = jssp::makespan_schedule(schedule, jobs.size(), number_of_machines);
			return makespan;
		}

		jssp::Schedule generate_schedule_from_positions(const Particle::Position& position) {
            auto indices = util::argsort(position);
            std::vector<int> operation_priorities(position.size());
            
            // int job_count = jobs.size();
            for (size_t i = 0; i < indices.size(); ++i) {
                operation_priorities[indices[i]] = i; 
            }

            return build_parameterized_active_schedule(operation_priorities);
        }

		jssp::Schedule build_parameterized_active_schedule(const std::vector<int>& operation_priorities) {
            int job_count = jobs.size();
            int total_operations = job_count * number_of_machines;
            
            jssp::Schedule schedule;
            std::vector<int> scheduled_ops(job_count, 0); // Compteur d'opérations schedulées par job
            std::vector<int> machine_available_time(number_of_machines, 0);
            std::vector<int> job_available_time(job_count, 0);

            for (int t = 0; t < total_operations; ++t) {
                std::vector<std::tuple<int, int>> schedulable_ops; 
                for (int job_id = 0; job_id < job_count; ++job_id) {
					// if the job still has operations to schedule, add it to the list of schedulable operations
                    if (scheduled_ops[job_id] < number_of_machines) {
                        schedulable_ops.emplace_back(job_id, scheduled_ops[job_id]);
                    }
                }

                int sigma_star = std::numeric_limits<int>::max();
                int phi_star = std::numeric_limits<int>::max();
                
                for (const auto& [job_id, op_index] : schedulable_ops) {
                    const auto& task = jobs[job_id][op_index];
                    int machine = task.machine;
                    int start_time = std::max(job_available_time[job_id], machine_available_time[machine]);
                    int end_time = start_time + task.time;
                    
                    if (start_time < sigma_star) sigma_star = start_time;
                    if (end_time < phi_star) phi_star = end_time;
                }

                int selected_job = -1;
                int selected_op_index = -1;
                int highest_priority = std::numeric_limits<int>::max();
                int earliest_end_time = std::numeric_limits<int>::max();

                for (const auto& [job_id, op_index] : schedulable_ops) {
                    const auto& task = jobs[job_id][op_index];
                    int machine = task.machine;
                    int start_time = std::max(job_available_time[job_id], machine_available_time[machine]);
                    int end_time = start_time + task.time;
                    
                    if (start_time <= sigma_star + delta * (phi_star - sigma_star)) {
                        int op_priority = operation_priorities[job_id * number_of_machines + op_index];
                        
                        if (op_priority < highest_priority || 
                            (op_priority == highest_priority && end_time < earliest_end_time)) {
                            highest_priority = op_priority;
                            earliest_end_time = end_time;
                            selected_job = job_id;
                            selected_op_index = op_index;
                        }
                    }
                }

                if (selected_job != -1) {
                    const auto& task = jobs[selected_job][selected_op_index];
                    int machine = task.machine;
                    int start_time = std::max(job_available_time[selected_job], machine_available_time[machine]);
                    int end_time = start_time + task.time;
                    
                    machine_available_time[machine] = end_time;
                    job_available_time[selected_job] = end_time;
                    scheduled_ops[selected_job]++;
                    
                    schedule.push_back(jobs[selected_job][selected_op_index]);
                }
            }

            return schedule;
        }

		void run() {
			for (int iter = 0; iter < iterations; ++iter) {
				for (auto& p : swarm) {

					float r1 = uniform_real_dist(random_engine);
					float r2 = uniform_real_dist(random_engine);
					for (size_t i = 0; i < p.position.size(); ++i) {
						p.velocity[i] = w * p.velocity[i] + c1 * r1 * (p.pbest_position[i] - p.position[i]) + c2 * r2 * (gbest_position[i] - p.position[i]);
						p.velocity[i] = std::clamp(p.velocity[i], 0.f, max_velocity);
						p.position[i] += p.velocity[i];
					}
					int makespan = fitness(p.position);
					if (makespan < p.pbest_makespan) {
						p.pbest_position = p.position;
						p.pbest_makespan = makespan;
					}
					// mutex.lock();
					if (makespan < gbest_makespan) {
						gbest_position = p.position;
						gbest_makespan = makespan;
					}
				}
			}
		}

		void run_parallal() {
			std::vector<util::ThreadSleeper> threads(number_of_particles);
			std::vector<std::thread> thread_pool;
			util::ThreadSleeper main_thread;
			
			std::mutex gbest_mutex;
			int left_threads = number_of_particles;

			std::mutex stop_mutex;
			bool stop = false;	

			for (int i = 0; i < number_of_particles; i++) {
				thread_pool.emplace_back(([&, i] {
					while (true) {
						threads[i].sleep_forever();
						stop_mutex.lock();
						if (stop) {
							stop_mutex.unlock();
							return;
						}
						stop_mutex.unlock();

						Particle& p = swarm[i];
						float r1 = uniform_real_dist(random_engine);
						float r2 = uniform_real_dist(random_engine);
						for (size_t i = 0; i < p.position.size(); ++i) {
							p.velocity[i] = w * p.velocity[i] + c1 * r1 * (p.pbest_position[i] - p.position[i]) + c2 * r2 * (gbest_position[i] - p.position[i]);
							p.velocity[i] = std::clamp(p.velocity[i], 0.f, max_velocity);
							p.position[i] += p.velocity[i];
						}
						int makespan = fitness(p.position);
						if (makespan < p.pbest_makespan) {
							p.pbest_position = p.position;
							p.pbest_makespan = makespan;
						}
						gbest_mutex.lock();
						if (makespan < gbest_makespan) {
							gbest_position = p.position;
							gbest_makespan = makespan;
						}
						left_threads--;
						if (left_threads == 0) 
							main_thread.wake_thread();

						gbest_mutex.unlock();
					}						
				}));
			}

			for (int iter = 0; iter < iterations; ++iter) {
				for (auto& thread : threads)
					thread.wake_thread();
				main_thread.sleep_forever();
				left_threads = number_of_particles;
				// util::println("Iteration {}: Best makespan: {}", iter, gbest_makespan);
			}
			stop = true;
			for (auto [thread_sleeper, thread] : std::ranges::views::zip(threads, thread_pool)) {
				thread_sleeper.wake_thread(); 
				thread.join(); 
			}

		}

		jssp::Schedule get_best_schedule() {
			auto schedule = generate_schedule_from_positions(gbest_position);
			jssp::sort_schedule(schedule, jobs.size(), number_of_machines);
			return schedule;
		}

	};

}