# TO DO 
fix pso2:
1- resize the schedule at the beginin
2- store the start_time and end_time of the latest task of each job in a vector
3- remove the inecissary op_priority equality check
4- remove sort_schedule in generate_schedule_operations 
5- rename the variables sigma_star and phi_star 


# problem definition :
- we have n jobs and m machines
- each job j, machine m has a processing time p(j,m)

- goal : calculate the minimum time to finish all jobs on all machines known as makespan

# representation :
- we can represent the problem using m vectors of size n
- each vector represents a machine
- each element of the vector represents a job and its processing time
- the order of the jobs in the vector represents the order in which they are processed in that machine

# calculate makespan :
- the makespan is the maximum time taken by any machine to finish all its jobs
- to calculate the makespan, we need to keep track of the current time of each machine mt, and the current time of each job jt, the list of waiting tasks for each machine, and the list of waiting tasks for each job
- the makespan is the maximum time taken by any machine to finish all its jobs, or the maximum time taken by any job to finish all its machines

## algorithm :
```
create a queue of job_waiting_tasks for each job
create a list of job_times for each job
create a list of job_last_task_done for each job
create a queue of machine_waiting_tasks for each machine
create a list of machine_times for each machine

for task in schedule
	job = task.job
	machine = task.machine
	if task.order comes right after job_last_task_done[job]
		if machine_waiting_tasks[machine] is empty
			# update the machine and job times

			do
				completed_other_tasks = False
				for queue in job_waiting_tasks
					if queue is not empty
						task = queue.front()

						if machine_waiting_tasks[task.machine].font().job == task.job
							machine_waiting_tasks[machine].pop()
							job_waiting_tasks[job].pop()
							# update the machine and job times
							completed_other_tasks = True		

			while completed_other_tasks
			
		else
			machine_waiting_tasks[machine].append(task)
			job_waiting_tasks[job].append(task)
			job_waiting_tasks[job].sort()
	else
			machine_waiting_tasks[machine].append(task)
			job_waiting_tasks[job].append(task)
			job_waiting_tasks[job].sort()
	

for task in schedule
	job = task.job
	machine = task.machine
	job_waiting_tasks[job].append(task)
	job_waiting_tasks[job].sort()
	machine_waiting_tasks[machine].append(task)

do 
	completed_other_tasks = False
	queues_empty = True
	for queue in job_waiting_tasks
		if queue is not empty
			queues_empty = False
			task = queue.front()

			if machine_waiting_tasks[task.machine].font().job == task.job
				machine_waiting_tasks[machine].pop()
				job_waiting_tasks[job].pop()
				# update the machine and job times
				completed_other_tasks = True		

while completed_other_tasks

if queues_empty
	return max(job_times) or max(machine_times)
else
	return -1

```

1 : 1 2 3
2 : 2 3 1
3 : 3 1 2

# Problem Redifinition
## if a a task in the front of a machine waiting for previous tasks of the same job is invalid
- modify makespan to report 


# ALGO PSO2

## variables 
sigma_star: the earliest start time of all schedulable operations
phi_star: the earliest possible end time among all schedulable operations
delta: It defines the maximum allowed idle time for machines through the condition: start_time ≤ σ* + δ(φ* − σ*)

1- we pass by each job and we get the next scheduled operation of each one of them and we put them in schedulable_ops.
2- we pass by each operation in schedulable_ops and we calculate for each one the start time and the end time and we update simga_star and phi_star following that.
3- we pass by each operation in schedulable_ops and verify the conditon start_time ≤ σ* + δ(φ* - σ*)
4- if it's true we get the priority and we compare it with the highest priority. if it's less than we update highest_priority, selected_job, selected_op_index

at the end of the iteration we select the best operation out of those that are in schedulable_ops (op that will not delay the schedule and that has the highest priority), and we add it to the schedule. 


