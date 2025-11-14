#include <vector>
#include <random>
#include <functional>
#include <ranges>

#include "util.cpp"
#include "jssp.cpp"
#include "parse.cpp"
#include "pso2.cpp"

template<bool Log = false>
void grid_search(jssp::Jobs& jobs, int j, int m, std::function<void(float,float,float,int)> callback = nullptr) {
	using value_type = std::tuple<float, float, float, int>;
	value_type best = {0.0f, 0.0f, 0.0f, std::numeric_limits<int>::max()}; 
	for (float w = 0.1; w <= 1.0; w += 0.2) {
		for (float c1 = 0.1; c1 <= 2.0; c1 += 0.4) {
			for (float c2 = 0.1; c2 <= 2.0; c2 += 0.4) {
				pso::Pso pso(jobs, m);
				pso.set_iterations(500);
				pso.set_number_of_particles(100);
				pso.init_swarm();
				pso.set_w(w);
				pso.set_c1(c1);
				pso.set_c2(c2);
				pso.set_delta(0);
				pso.run_parallal();
				util::println("Best makespan: {}, w: {}, c1: {}, c2: {}", pso.gbest_makespan, w, c1, c2);
				if constexpr (Log) {
					callback(w, c1, c2, pso.gbest_makespan);
				}
				if (pso.gbest_makespan < std::get<3>(best)) {
					best = {w, c1, c2, pso.gbest_makespan};
				}
			}
		}
	}
	util::println("Best parameters: w: {}, c1: {}, c2: {}, makespan: {}", std::get<0>(best), std::get<1>(best), std::get<2>(best), std::get<3>(best));
}

void test_pso() {
	int j = 20;
	int m = 15;
	std::string benchmark = util::format("experiments/benchmarks/tai{}_{}.txt", j, m);
	jssp::Jobs jobs = load_jobs(benchmark)[1];

	pso::Pso pso(jobs, m);
	pso.set_iterations(500);
	pso.set_number_of_particles(100);
	pso.init_swarm();
	pso.set_w(0.3f);
	pso.set_c1(0.1f);
	pso.set_c2(0.9f);
	pso.set_delta(0);
	pso.run_parallal();
	jssp::Schedule schedule = pso.get_best_schedule();
	util::println("Best makespan: {}", pso.gbest_makespan);
}	


/*
// logs the makespan of the best solution found by pso2 for each set of parameters
void log_grid_search_pso2() {
	std::string filename = util::format("experiments/results/pso2-best-params.csv");
	// util::write(filename, "w,c1,c2,makespan\n", std::ios::trunc);

	std::function<void(float,float,float,int)> write = [&](float w, float c1, float c2, int makespan) {
		util::write(filename, util::format("{},{},{},{}\n", w, c1, c2, makespan), std::ios::app);
	};
	int j = 20;
	int m = 15;
	std::string benchmark = util::format("experiments/benchmarks/tai{}_{}.txt", j, m);
	jssp::Jobs jobs = load_jobs(benchmark)[0];

	grid_search<true>(jobs, j, m, write);
}

// logs the makespan of the best solution found by pso2 for each benchmark
void benchmark_pso2() {
	auto load_benchmark = [](int j, int m) {
		std::string benchmark = util::format("experiments/benchmarks/tai{}_{}.txt", j, m);
		return load_jobs(benchmark);
	};
	std::string filename = "experiments/results/pso2-parallel-benchmark.csv";
	util::stopwatch sw;
	
	auto time = [&](int j, int m, jssp::Jobs& jobs) {
		sw.init();
		pso::Pso pso(jobs, m);
		pso.set_iterations(500);
		pso.set_number_of_particles(100);
		pso.set_w(0.3f);
		pso.set_c1(0.1f);
		pso.set_c2(0.9f);
		pso.set_delta(0);
		pso.init_swarm();
		pso.run_parallal();
		int makespan = pso.gbest_makespan;
		auto elapsed = sw.elapsed<std::chrono::milliseconds>();
		return std::pair{makespan, elapsed};
	};

	using time_type = std::chrono::milliseconds;
	auto log = [&](int j, int m, int instance, int makespan, time_type time) {
		// std::string times = util::join(makespans_times.begin(), makespans_times.end(), ",", [](auto& p) { return util::format("{}", p.second.count()); });
		// std::string makespans = util::join(makespans_times.begin(), makespans_times.end(), ",", [](auto& p) { return util::format("{}", p.first); });

		std::string line = util::format("{} {},{},{},{}\n", j, m, instance, makespan, time.count());
		util::print(line);
		util::write(filename, line, std::ios::app);
	};

	auto benchmark = [&](int j, int m, std::vector<int> instance_indices) {
		auto instances = load_benchmark(j, m);
		std::vector<std::pair<int,time_type>> makespans_times;
		for (int i : instance_indices) {
			auto [makespan, elapsed] = time(j, m, instances[i]);
			// makespans_times.push_back({makespan, elapsed});
			log(j, m, i, makespan, elapsed);
		}
	};
	
	util::write(filename, "jobs machines,instance,makespan,time ms,\n", std::ios::out | std::ios::trunc);
	std::vector instance_indices = {0, 1, 2, 3};

	benchmark(20, 15, instance_indices);
	benchmark(30, 15, instance_indices);
	benchmark(50, 15, instance_indices);
	benchmark(100, 20, instance_indices);
}

*/


int main() {

	
	test_pso();

	
}



