// util.cpp
#pragma once

#include <string>
#include <format>
#include <chrono>
#include <iostream>
#include <fstream>
#include <semaphore>


namespace util {
    std::string format(const std::string& str, auto... args) {
        return std::vformat(str, std::make_format_args(args...));
    }
    void print(const std::string& str, auto... args) {
        std::cout << format(str, args...);
    }
    void println(const std::string& str, auto... args) {
        std::cout << format(str, args...) << '\n';
    }
	void println() {
		std::cout << '\n';
	}
	
	template <typename T>
	std::string join(T&& begin, T&& end, const std::string& delimiter, auto&& func = [](auto&& x) { return std::to_string(x); }) {
		std::string result;
		for (auto it = begin; it != end; ++it) {
			if (it != begin) {
				result += delimiter;
			}
			result += func(*it);
		}
		return result;
	};

	using nanoseconds = std::chrono::duration<double, std::nano>;
	using microseconds = std::chrono::duration<double, std::micro>;
	using milliseconds = std::chrono::duration<double, std::milli>;
	using seconds = std::chrono::duration<double>;
	using minutes = std::chrono::duration<double, std::ratio<60>>;
	using hours = std::chrono::duration<double, std::ratio<3600>>;
	
	struct stopwatch {
        using duration_type = nanoseconds;
        using time_point = std::chrono::steady_clock::time_point;

        time_point start;

        stopwatch() : start(std::chrono::steady_clock::now()) {}

		void init() {
			start = std::chrono::steady_clock::now();
		}

		template<typename DurationType = duration_type>
		DurationType elapsed() {
			auto end = std::chrono::steady_clock::now();
			return std::chrono::duration_cast<DurationType>(end - start);
		}	
	};
	
    struct timer : stopwatch {
        using stopwatch::duration_type;
		duration_type duration;

		// constructor that takes a duration in any duration type and doesn't initialize the starting of the timer
		timer(duration_type duration) : stopwatch(), duration(duration) {}
        
		// default constructor that takes a duration of 0 and doesn't initialize the starting of the timer
		timer() : timer(duration_type::zero()) {}
        
		// get the time left until the timer is done 
		template<typename DurationType = duration_type>
        DurationType left() {
			return std::chrono::duration_cast<DurationType>(duration) - elapsed<DurationType>();
		}

		// check if the timer is done
		bool is_done() {
			return left() <= duration_type::zero();
		}
		// get the duration of the timer
		duration_type get_duration() {
			return duration;
		}
		// set the duration of the timer in any duration type
		void set_duration(duration_type duration) {
			this->duration = duration;
		}
	};


	template<typename T>
	std::vector<int> argsort(const std::vector<T>& vec) {
		std::vector<int> indices(vec.size());
		for (int i = 0; i < vec.size(); ++i)
			indices[i] = i;
	
		std::sort(indices.begin(), indices.end(), [&vec](int i1, int i2) {
			return vec[i1] < vec[i2];
		});
	
		return indices;
	}

	void print_vector(auto&& vector) {
		for (const auto& elem : vector) 
			print("{} ", elem);
	}

	void write(std::string filename, const std::string& value, std::ios::openmode mode = std::ios::app) {
		std::fstream file(filename, mode);	
		file << value;
		file.close();
	};
	
	struct ThreadSleeper {
		std::counting_semaphore<0> sem{0};  

		void sleep_forever() {
			sem.acquire();  
		}		
		void wake_thread() {
			sem.release();  
		}
	};
	
}

