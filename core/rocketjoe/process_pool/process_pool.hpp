#pragma once

#include <boost/process.hpp>
#include <rocketjoe/log/log.hpp>

namespace rocketjoe {

class process_pool_t final {
public:
  process_pool_t(const process_pool_t &) = delete;
  process_pool_t &operator=(const process_pool_t &) = delete;
  process_pool_t(log_t log);

  ~process_pool_t();

  void add_worker_process(std::size_t);

  void add_worker_process();

private:
  std::uint64_t worker_counter_;
  boost::process::group g_;
  log_t log_t_;
};

} // namespace rocketjoe