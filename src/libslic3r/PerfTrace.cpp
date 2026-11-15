#include "PerfTrace.hpp"
#include "libslic3r/Log.hpp"

#include <cstdio>
#include <cstdlib>

namespace Slic3r {

namespace {
// Captured during static initialization, i.e. close to process start.
const std::chrono::steady_clock::time_point g_perf_epoch = std::chrono::steady_clock::now();

// Set BBS_PERF_STDERR=1 to also print perf traces to stderr, prefixed with the
// time since process start. Handy for profiling startup without decoding logs.
bool perf_to_stderr()
{
    static const bool enabled = [] {
        const char *v = std::getenv("BBS_PERF_STDERR");
        return v != nullptr && *v != '\0' && *v != '0';
    }();
    return enabled;
}

void perf_stderr(const std::string &msg)
{
    if (!perf_to_stderr())
        return;
    const auto t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - g_perf_epoch).count();
    std::fprintf(stderr, "[perf %6lld ms] %s\n", (long long) t, msg.c_str());
}
} // namespace

void perf_mark(const std::string& label)
{
    BOOST_LOG_TRIVIAL(info) << "[perf] " << label;
    perf_stderr(label);
}

PerfTimer::PerfTimer(const std::string &name, bool enabled) : m_name(name), m_start(std::chrono::steady_clock::now()), m_enabled(enabled)
{
    if (m_enabled) {
        BOOST_LOG_TRIVIAL(info) << "[perf] " << m_name << " begin";
        perf_stderr(m_name + " begin");
    }
}

PerfTimer::~PerfTimer()
{
    if (!m_enabled)
        return;
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start).count();
    BOOST_LOG_TRIVIAL(info) << "[perf] " << m_name << " cost " << ms << " ms";
    perf_stderr(m_name + " cost " + std::to_string(ms) + " ms");
    flush_logs();
}

} // namespace Slic3r
