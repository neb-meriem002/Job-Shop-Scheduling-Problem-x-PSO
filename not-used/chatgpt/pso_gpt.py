import random
import numpy as np

class Task:
    def __init__(self, job, time, index, machine):
        self.job = job
        self.time = time
        self.index = index
        self.machine = machine

    def __repr__(self):
        return f"Task(job={self.job}, time={self.time}, index={self.index}, machine={self.machine})"

class Particle:
    def __init__(self, schedule):
        self.position = schedule.copy()
        self.velocity = []
        self.personal_best = schedule.copy()
        self.personal_best_makespan = float('inf')

    def update_velocity(self, global_best, w, c1, c2):
        new_velocity = []
        for i in range(len(self.position)):
            r1, r2 = random.random(), random.random()
            if self.position[i].job != self.personal_best[i].job:
                new_velocity.append(self.personal_best[i])
            if self.position[i].job != global_best[i].job:
                new_velocity.append(global_best[i])
        self.velocity = new_velocity

    def update_position(self):
        for task in self.velocity:
            for i in range(len(self.position)):
                if self.position[i].job == task.job:
                    self.position[i], task = task, self.position[i]

# Fonction de calcul du makespan
def makespan(schedule, num_jobs, num_machines):
    machine_times = [0] * num_machines
    job_times = [0] * num_jobs
    for task in schedule:
        start_time = max(machine_times[task.machine], job_times[task.job])
        end_time = start_time + task.time
        machine_times[task.machine] = end_time
        job_times[task.job] = end_time
    return max(machine_times)

# Algorithme PSO
num_jobs = 2
num_machines = 2
population_size = 10
max_iterations = 100
w, c1, c2 = 0.5, 1.5, 1.5

base_schedule = [
    Task(0, 2, 1, 0), Task(1, 3, 1, 0), Task(1, 4, 2, 1), Task(0, 5, 2, 1)
]

particles = [Particle(random.sample(base_schedule, len(base_schedule))) for _ in range(population_size)]
global_best = min(particles, key=lambda p: makespan(p.position, num_jobs, num_machines))
global_best_makespan = makespan(global_best.position, num_jobs, num_machines)

for _ in range(max_iterations):
    for particle in particles:
        current_makespan = makespan(particle.position, num_jobs, num_machines)
        if current_makespan < particle.personal_best_makespan:
            particle.personal_best = particle.position.copy()
            particle.personal_best_makespan = current_makespan
        if current_makespan < global_best_makespan:
            global_best = particle
            global_best_makespan = current_makespan
        particle.update_velocity(global_best.position, w, c1, c2)
        particle.update_position()

print("Best Schedule Found:", global_best.position)
print("Best Makespan:", global_best_makespan)
