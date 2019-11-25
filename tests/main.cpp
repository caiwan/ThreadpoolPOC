#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>

using ::testing::EmptyTestEventListener;
using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestCase;
using ::testing::TestEventListeners;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;

constexpr size_t loggerPoolBuggerSize = 16 * 8192;

int main(int argc, char ** argv)
{
  spdlog::init_thread_pool(loggerPoolBuggerSize, 1);
  spdlog::set_pattern("[%E%e%f] [%t] %v");
  spdlog::set_level(spdlog::level::trace);
  InitGoogleTest(&argc, argv);
  const int res = RUN_ALL_TESTS();
  return res;
}
