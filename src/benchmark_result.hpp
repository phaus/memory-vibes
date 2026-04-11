#ifndef BENCHMARK_RESULT_HPP
#define BENCHMARK_RESULT_HPP

#include <string>

struct BenchmarkResult {
  unsigned long long timestamp;
  std::string system_id;
  std::string kernel;
  unsigned long long size_mib;
  std::string data_type;
  unsigned long long iterations;
  double bandwidth_gb_s;
  double time_seconds;
  unsigned long long bytes_per_iter;
  std::string cpu_model;
  std::string os_name;
  std::string os_version;
};

#endif // BENCHMARK_RESULT_HPP
