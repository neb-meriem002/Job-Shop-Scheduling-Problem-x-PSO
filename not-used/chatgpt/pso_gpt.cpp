#include <vector>
#include <algorithm>
#include <random>

struct task {
    int job;
    int time;
    int index;
    int machine;
};

using schedule = std::vector<task>;

// Paramètres de PSO
const double w = 0.5;  // Facteur d'inertie
const double c1 = 1.5; // Coefficient cognitif
const double c2 = 1.5; // Coefficient social

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> dist(0.0, 1.0);

// Fonction pour calculer la "vitesse" entre deux positions
schedule calculate_velocity(const schedule& particle, const schedule& personal_best, const schedule& global_best, schedule& velocity) {
    schedule new_velocity;
    
    for (size_t i = 0; i < particle.size(); ++i) {
        double r1 = dist(gen);
        double r2 = dist(gen);
        
        // Mise à jour de la vitesse selon l'équation PSO classique
        if (particle[i].job != personal_best[i].job) {
            new_velocity.push_back({personal_best[i].job, personal_best[i].time, personal_best[i].index, personal_best[i].machine});
        }
        if (particle[i].job != global_best[i].job) {
            new_velocity.push_back({global_best[i].job, global_best[i].time, global_best[i].index, global_best[i].machine});
        }
    }
    
    return new_velocity;
}

// Fonction pour mettre à jour la position d'une particule
void update_position(schedule& position, schedule& velocity) {
    // Appliquer la mise à jour de la position
    for (const auto& v : velocity) {
        auto it = std::find_if(position.begin(), position.end(), [&](const task& t) {
            return t.job == v.job;
        });
        if (it != position.end()) {
            std::iter_swap(it, std::find(position.begin(), position.end(), v));
        }
    }
}
