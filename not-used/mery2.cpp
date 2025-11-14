#include <vector>
#include <algorithm>
#include <random>
#include <unordered_map>
#include <vector>
#include <list>
#include <ranges>
#include <chrono>
#include <iostream>
#include <format>

#include "parse.cpp"
#include "jssp.cpp"
#include "util.cpp"

const int NUM_PARTICLES = 100;
const int MAX_ITER = 1000;
const float w = 0.7f;   
const float c1 = 1.5f;  
const float c2 = 1.5f;  


std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> dist(0.0f, 1.0f);

using schedule = std::vector<jssp::Task>;

struct particle {

    schedule position;
    std::vector<float> velocity;
    schedule best_position;

    int best_makespan = std::numeric_limits<int>::max();
};


void sort_schedule(schedule& sched, const std::vector<float>& index_boost) {
    std::sort(sched.begin(), sched.end(), [&](const jssp::Task& a, const jssp::Task& b) {
        float boosted_a = a.priority + index_boost[a.index];
        float boosted_b = b.priority + index_boost[b.index];
        return boosted_a > boosted_b;
    });
}


void sort_schedule_by_jobs(schedule& schedule, int number_of_jobs, int number_of_machines) {
    for (int job = 0; job < number_of_jobs; job++) {
        std::vector<int> task_indices;
        
    }
}

schedule pso(schedule& base_schedule, int j, int m) {
    std::vector<float> index_boost(m);
    float max_boost = 0.09f;  // Réduit encore la valeur maximale du boost
    float step = max_boost / m;
    for (int i = 0; i < m; ++i)
        index_boost[i] = max_boost - i * step;  // Réduit l'influence du boost

    std::vector<particle> swarm(NUM_PARTICLES);
    schedule global_best;
    int global_best_makespan = std::numeric_limits<int>::max();

    // Initialisation
    for (auto& p : swarm) {
        p.position = base_schedule;

        for (auto& t : p.position)
            t.priority = dist(gen);

        p.velocity.resize(p.position.size());
        for (auto& v : p.velocity)
            v = dist(gen) * 0.2f; // Vitesse initiale plus forte

        sort_schedule(p.position, index_boost);  // Tri avec un boost faible

        jssp::sort_schedule(p.position, j, m);  // ============================

        int fit = jssp::makespan_schedule(p.position, j, m);
        p.best_position = p.position;
        p.best_makespan = fit;

        if (fit < global_best_makespan) {
            global_best = p.position;
            global_best_makespan = fit;
        }
    }
    // util::println("=pso= Done initializing particles");

    for (int iter = 0; iter < MAX_ITER; ++iter) {

        // Refroidissement plus doux du boost (ajuste légèrement)
        float cooling = 1.0f - 0.3f * (float)iter / MAX_ITER;  // Diminution encore plus douce
        std::vector<float> dynamic_boost = index_boost;
        for (auto& b : dynamic_boost) b *= cooling;  // Applique un refroidissement léger
        
        // util::println("=pso= Iteration {} | Done cooling", iter);

        for (auto& p : swarm) {
            for (size_t i = 0; i < p.position.size(); ++i) {
                // util::println("=pso= Particle {} | Position {} | Velocity {}", i, p.position[i].priority, p.velocity[i]);
                float r1 = dist(gen), r2 = dist(gen);
                // util::println("=pso= Generated random numbers: r1 = {}, r2 = {}", r1, r2);
                float personal = c1 * r1 * (p.best_position[i].priority - p.position[i].priority);
                // util::println("=pso= Personal best: {}", p.best_position[i].priority);

                // util::println("=pso= Global best size: {}", global_best.size());
                float global = c2 * r2 * (global_best[i].priority - p.position[i].priority);
                // util::println("=pso= Global best: {}", global_best[i].priority);
                p.velocity[i] = w * p.velocity[i] + personal + global;
                // util::println("=pso= Updated velocity: {}", p.velocity[i]);
                p.position[i].priority += p.velocity[i];
                // util::println("=pso= Updated position: {}", p.position[i].priority);
                p.position[i].priority = std::clamp(p.position[i].priority, 0.0f, 1.0f);
                // util::println("=pso= Clamped position: {}", p.position[i].priority);
            }

            // Tri avec un boost très faible et plus contrôlé
            sort_schedule(p.position, dynamic_boost);  

            jssp::sort_schedule(p.position, j, m);  // ============================

            int fit = jssp::makespan_schedule(p.position, j, m);
            if (fit < p.best_makespan) {
                p.best_position = p.position;
                p.best_makespan = fit;
            }
            if (fit < global_best_makespan) {
                global_best = p.position;
                global_best_makespan = fit;
            }
        }

        // Affichage de l'évolution du makespan
        // std::cout << std::format("Iteration {:>4} | Meilleur makespan : {}\n", iter, global_best_makespan);
    }

    return global_best;
}

void grid_search(schedule& init_schedule, int j, int m) {
    using value_type = std::tuple<float, float, float, int>;
    value_type best = {0.0f, 0.0f, 0.0f, std::numeric_limits<int>::max()}; 
    for (float w = 0.1; w <= 1.0; w += 0.2) {
        for (float c1 = 0.1; c1 <= 2.0; c1 += 0.4) {
            for (float c2 = 0.1; c2 <= 2.0; c2 += 0.4) {
                auto schedule = pso(init_schedule, j, m);
                int makespan = jssp::makespan(schedule, j, m);
                util::println("Best makespan: {}, w: {}, c1: {}, c2: {}", makespan, w, c1, c2);
                if (makespan < std::get<3>(best)) {
                    best = {w, c1, c2, makespan};
                }
            }
        }
    }
    util::println("Best parameters: w: {}, c1: {}, c2: {}, makespan: {}", std::get<0>(best), std::get<1>(best), std::get<2>(best), std::get<3>(best));
}

void test_pso() {
    schedule initial_schedule { 

         // Job 0 (5 operations)
         {0, 85, 1, 3, 0},  // Op1: 85 time units on Machine 4
         {0, 64, 2, 0, 0},  // Op2: 64 on M1
         {0, 31, 3, 2, 0},  // Op3: 31 on M3
         {0, 44, 4, 4, 0},  // Op4: 44 on M5
         {0, 66, 5, 1, 0},  // Op5: 66 on M2

       // Job 1
         {1,  7, 1, 0, 0},
         {1, 14, 2, 3, 0},
         {1, 69, 3, 1, 0},
         {1, 18, 4, 4, 0},
         {1, 68, 5, 2, 0},

       // Job 2
         {2,  1, 1, 3, 0},
         {2, 74, 2, 0, 0},
         {2, 70, 3, 1, 0},
         {2, 90, 4, 4, 0},
         {2, 60, 5, 2, 0},
         

       // Job 3
         {3, 45, 1, 1, 0},
         {3, 76, 2, 3, 0},
         {3, 13, 3, 4, 0},
         {3, 98, 4, 2, 0},
         {3, 54, 5, 0, 0},

       // Job 4
         {4, 80, 1, 0, 0},
         {4, 15, 2, 3, 0},
         {4, 45, 3, 1, 0},
         {4, 91, 4, 4, 0},
         {4, 10, 5, 2, 0}

    };

    for (auto& t : initial_schedule) t.priority = dist(gen);
    

    int j = 20;
    int m = 15;

    initial_schedule = load_schedules("experiments/tai20_15.txt")[0];

    auto start = std::chrono::high_resolution_clock::now(); 

    util::println("=main= Calling pso");
    schedule best = pso(initial_schedule, j, m);
    util::println("=main= Finished pso");

    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;
    std::cout << "Temps d'execution : " << duration.count() << " secondes" << std::endl;

    // Affiche le meilleur ordonnancement
    for (const auto& t : best) {
        std::cout << "Job " << t.job << ", Machine " << t.machine << ", Index " << t.index << ", Time " << t.time << "\n";
    }

    std::cout << "Le meilleur makespan " << makespan(best,j,m) << "\n";
}

int main () {

    int j = 20;
    int m = 15;
    std::string filename = util::format("experiments/tai{}_{}.txt", j, m);
    jssp::Schedule initial_schedule = load_schedules(filename)[0];
    
    grid_search(initial_schedule, j, m);

    return 0;
}