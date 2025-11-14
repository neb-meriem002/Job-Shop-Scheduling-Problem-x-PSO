#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "jssp.cpp"
#include "util.cpp"




std::vector<jssp::Schedule> load_schedules(const std::string& filename) {
    std::ifstream file(filename);
    jssp::Jobs all_instances;

    if (!file.is_open()) {
        std::cerr << "Erreur lors de l'ouverture du fichier\n";
        return {};
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("Nb of jobs") != std::string::npos) {
            int nb_jobs, nb_machines;
            file >> nb_jobs >> nb_machines;

            // Lire Times
            while (std::getline(file, line) && line.find("Times") == std::string::npos);
            std::vector<std::vector<int>> times(nb_jobs, std::vector<int>(nb_machines));
            for (int i = 0; i < nb_jobs; ++i)
                for (int j = 0; j < nb_machines; ++j)
                    file >> times[i][j];

            // Lire Machines
            while (std::getline(file, line) && line.find("Machines") == std::string::npos);
            std::vector<std::vector<int>> machines(nb_jobs, std::vector<int>(nb_machines));
            for (int i = 0; i < nb_jobs; ++i)
                for (int j = 0; j < nb_machines; ++j)
                    file >> machines[i][j];

            // Créer les tâches
            std::vector<jssp::Task> instance;
            for (int i = 0; i < nb_jobs; ++i) {
                for (int j = 0; j < nb_machines; ++j) {
                    jssp::Task t;
                    t.job = i ;
                    t.index = j;
                    t.machine = machines[i][j] - 1; // -1 pour l'indexation à partir de 0
                    t.time = times[i][j];
                    instance.push_back(t);
                }
            }
            all_instances.push_back(instance);
        }
    }

    return all_instances;
}



std::vector<jssp::Jobs> load_jobs(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<jssp::Jobs> all_instances;

    if (!file.is_open()) {
        std::cerr << "Erreur lors de l'ouverture du fichier\n";
        return {};
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("Nb of jobs") != std::string::npos) {
            int nb_jobs, nb_machines;
            file >> nb_jobs >> nb_machines;

            // Lire Times
            while (std::getline(file, line) && line.find("Times") == std::string::npos);
            std::vector<std::vector<int>> times(nb_jobs, std::vector<int>(nb_machines));
            for (int i = 0; i < nb_jobs; ++i)
                for (int j = 0; j < nb_machines; ++j)
                    file >> times[i][j];

            // Lire Machines
            while (std::getline(file, line) && line.find("Machines") == std::string::npos);
            std::vector<std::vector<int>> machines(nb_jobs, std::vector<int>(nb_machines));
            for (int i = 0; i < nb_jobs; ++i)
                for (int j = 0; j < nb_machines; ++j)
                    file >> machines[i][j];

            // Créer les tâches
            jssp::Jobs instance(nb_jobs);
            for (int i = 0; i < nb_jobs; ++i) {
                for (int j = 0; j < nb_machines; ++j) {
                    jssp::Task t;
                    t.job = i;
                    t.index = j;
                    t.machine = machines[i][j] - 1; // -1 pour l'indexation à partir de 0
                    t.time = times[i][j];
                    instance[i].push_back(t);
                }
            }
            all_instances.push_back(instance);
        }
    }

    return all_instances;
}
