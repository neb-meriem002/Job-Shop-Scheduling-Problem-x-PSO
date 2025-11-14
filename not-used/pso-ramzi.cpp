
#include <vector>
#include <random>
#include <algorithm>
#include "../util.cpp"
#include "../jssp.cpp"


namespace pso {
	auto print_positions = [](const std::vector<float>& positions) {
		util::print("Positions: ");
		for (const auto& pos : positions)
			util::print("{}, ", pos);
		util::println();
	};


	struct Particle {
		using Position = std::vector<float>;
		using Velocity = Position;
		// using JSP-Decoding method to convert the vector of positions into a schedule

		Position position;
		Velocity velocity;
		Position pbest_position;
		int pbest_makespan;
	};

	struct PsoBase {
		int number_of_machines;
		int number_of_tasks;
		int iterations = 500;
		int number_of_particles = 30;
		float w = 0.2f; // inertia weight
		float c1 = 1.f; // cognitive weight
		float c2 = 1.5f; // social weight
		float max_velocity = 1.0f; // maximum velocity
	
		PsoBase(int number_of_machines, int number_of_tasks) : number_of_machines(number_of_machines), number_of_tasks(number_of_tasks) {}

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
		void set_max_velocity(float max_velocity) {
			this->max_velocity = max_velocity;
		}


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

		void init_swarm() {
			auto init_positions = [&](int size) {
				std::vector<float> positions(size);
				for (int i = 0; i < size; ++i) 
					positions[i] = uniform_real_dist(random_engine);
				return positions;
			};
			// util::println("=inti_swarm= Done initializing positions");

			swarm.resize(number_of_particles);
			for (auto& p : swarm) {
				// util::println("=inti_swarm= Initializing particle");
				p.position = init_positions(number_of_tasks);
				// util::println("=inti_swarm= Done initializing particle position");
				p.velocity = init_positions(number_of_tasks);
				// util::println("=inti_swarm= Done initializing particle velocity");
				p.pbest_position = p.position;
				// util::println("=inti_swarm= Calculating fitness");
				p.pbest_makespan = fitness(p.position);
				// util::println("=inti_swarm= Done calculating fitness");
				if (p.pbest_makespan < gbest_makespan) {
					gbest_position = p.position;
					gbest_makespan = p.pbest_makespan;
				}
			}

			// util::println("Best makespan: {}", gbest_makespan);
			// util::print("Best position: ");
		}

		int fitness(Particle::Position& position) {
			// util::println("=fitness= Generating schedule from positions");
			jssp::Schedule schedule = generate_schedule_from_positions(position);
			
			jssp::sort_schedule(schedule, jobs.size(), number_of_machines);
			int makespan = jssp::makespan_schedule(schedule, jobs.size(), number_of_machines);
			// util::println("=fitness= Calculating makespan");
			// util::println("Makespan: {}", makespan);
			return makespan;
		}

		jssp::Schedule generate_schedule_from_positions(const Particle::Position& position) {
			// util::println("=generate_schedule_from_positions= Initializing schedule");
			jssp::Schedule schedule(position.size());
			// util::println("=generate_schedule_from_positions= Sorting positions");
			auto indices = util::argsort(position);
			// util::println("=generate_schedule_from_positions= Sorting schedule");
			int i = 0;
			for (auto& job : jobs) {
				for (auto& task : job) {
					schedule[indices[i]] = task;
					i++;
				}
			}
			// util::println("=generate_schedule_from_positions= Done generating schedule");
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
					if (makespan < gbest_makespan) {
						gbest_position = p.position;
						gbest_makespan = makespan;
						// util::println("Iteration {:>4} | Best makespan : {}", iter, gbest_makespan);
						// util::print("Best position: ");
						// print_positions(gbest_position);
					}
				}
				// util::println("Best schedule: ");
				// jssp::Schedule schedule = generate_schedule_from_positions(gbest_position);
				// jssp::sort_schedule(schedule, jobs.size(), number_of_machines);
				// jssp::print_schedule(schedule);
			}
		}

		jssp::Schedule get_best_schedule() {
			auto schedule = generate_schedule_from_positions(gbest_position);
			jssp::sort_schedule(schedule, jobs.size(), number_of_machines);
			return schedule;
		}

	};

	// struct PsoSwapBased : PsoBase {
		
		
	// 	PsoSwapBased(jssp::Jobs& jobs
	// };
}