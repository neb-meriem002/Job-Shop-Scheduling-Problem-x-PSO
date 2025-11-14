import numpy as np
import random

# Class for solving the Job Shop Scheduling Problem (JSSP) using Particle Swarm Optimization (PSO)
class JSSP_PSO:
	def __init__(self, num_jobs, num_machines, processing_times, max_iter=100, num_particles=30):
		"""
		Initialize the PSO solver for JSSP.
		:param num_jobs: Number of jobs
		:param num_machines: Number of machines
		:param processing_times: Matrix of processing times (jobs x machines)
		:param max_iter: Maximum number of iterations for optimization
		:param num_particles: Number of particles in the swarm
		"""
		self.num_jobs = num_jobs
		self.num_machines = num_machines
		self.processing_times = processing_times  # Matrix: jobs x machines
		self.max_iter = max_iter
		self.num_particles = num_particles
		self.particles = []  # List of particles (solutions)
		self.velocities = []  # List of velocities for each particle
		self.p_best = []  # Personal best solutions for each particle
		self.g_best = None  # Global best solution
		self.g_best_fitness = float('inf')  # Fitness of the global best solution
		self.init_particles()  # Initialize particles and their velocities
	
	def init_particles(self):
		"""
		Initialize particles with random solutions and calculate their fitness.
		"""
		for _ in range(self.num_particles):
			# Generate a random solution (particle)
			particle = self.generate_random_solution()
			self.particles.append(particle)
			# Initialize velocity as a zero vector
			self.velocities.append(np.zeros(len(particle)))
			# Calculate the fitness (makespan) of the particle
			fitness = self.calculate_makespan(particle)
			# Store the particle as its own personal best initially
			self.p_best.append((particle.copy(), fitness))
			# Update the global best if this particle is better
			if fitness < self.g_best_fitness:
				self.g_best = particle.copy()
				self.g_best_fitness = fitness
	
	def generate_random_solution(self):
		"""
		Generate a random solution (job sequence) for the JSSP.
		:return: A random sequence of jobs
		"""
		sequence = []
		# Each job appears `num_machines` times in the sequence
		for job in range(self.num_jobs):
			sequence.extend([job] * self.num_machines)
		# Shuffle the sequence randomly
		random.shuffle(sequence)
		return sequence
	
	def calculate_makespan(self, sequence):
		"""
		Calculate the makespan (total time to complete all jobs) for a given sequence.
		:param sequence: Job sequence
		:return: Makespan value
		"""
		# Track the completion times of jobs and machines
		job_times = {job: 0 for job in range(self.num_jobs)}
		machine_times = {machine: 0 for machine in range(self.num_machines)}
		# Track the number of operations completed for each job
		job_machine_counter = {job: 0 for job in range(self.num_jobs)}
		
		# Process each job in the sequence
		for job in sequence:
			if job >= self.num_jobs:
				continue  # Skip invalid job indices
			
			# Determine the machine for the current operation of the job
			machine = job_machine_counter[job]
			if machine >= self.num_machines:
				continue  # Ensure machine index is valid
			
			# Calculate the start and end times for the operation
			start_time = max(job_times[job], machine_times[machine])
			processing_time = self.processing_times[job][machine]
			end_time = start_time + processing_time
			
			# Update the completion times for the job and machine
			job_times[job] = end_time
			machine_times[machine] = end_time
			# Increment the operation counter for the job
			job_machine_counter[job] += 1
		
		# The makespan is the maximum completion time across all machines
		return max(machine_times.values())
	
	def update_particles(self, inertia=0.7, c1=1.5, c2=1.5):
		"""
		Update the particles' positions and velocities based on PSO equations.
		:param inertia: Inertia weight
		:param c1: Cognitive coefficient (personal best influence)
		:param c2: Social coefficient (global best influence)
		"""
		for i in range(self.num_particles):
			# Convert personal best, current particle, and global best to arrays
			p_best_array = np.array(self.p_best[i][0])
			particle_array = np.array(self.particles[i])
			g_best_array = np.array(self.g_best)
			
			# Ensure consistent shape for operations
			min_length = min(len(p_best_array), len(particle_array), len(g_best_array))
			p_best_array = p_best_array[:min_length]
			particle_array = particle_array[:min_length]
			g_best_array = g_best_array[:min_length]
			
			# Update velocity using the PSO velocity update equation
			new_velocity = (inertia * np.array(self.velocities[i][:min_length]) +
							c1 * random.random() * (p_best_array - particle_array) +
							c2 * random.random() * (g_best_array - particle_array))
			
			# Update the particle's velocity and position
			self.velocities[i] = new_velocity.tolist()
			new_particle = np.clip(particle_array + new_velocity, 0, self.num_jobs - 1).astype(int).tolist()
			self.particles[i] = new_particle
			
			# Calculate the fitness of the new particle
			fitness = self.calculate_makespan(new_particle)
			# Update personal best if the new fitness is better
			if fitness < self.p_best[i][1]:
				self.p_best[i] = (new_particle.copy(), fitness)
			# Update global best if the new fitness is better
			if fitness < self.g_best_fitness:
				self.g_best = new_particle.copy()
				self.g_best_fitness = fitness
	
	def optimize(self):
		"""
		Perform the optimization process using PSO.
		:return: Best job sequence and its makespan
		"""
		for _ in range(self.max_iter):
			# Update particles in each iteration
			self.update_particles()
		return self.g_best, self.g_best_fitness

# Example Usage
num_jobs = 3  # Number of jobs
num_machines = 3  # Number of machines
# Processing times matrix (jobs x machines)
processing_times = np.array([[3, 2, 2], [4, 3, 1], [2, 3, 3]])

# Create the PSO solver
pso_solver = JSSP_PSO(num_jobs, num_machines, processing_times)
# Run the optimization
best_sequence, best_makespan = pso_solver.optimize()
# Print the results
print("Best Job Sequence:", best_sequence)
print("Best Makespan:", best_makespan)
