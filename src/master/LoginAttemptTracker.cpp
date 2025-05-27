#include "LoginAttemptTracker.hpp"

#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>

bool LoginAttemptTracker::allowLogin(const std::string& ip)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto now = std::chrono::steady_clock::now();
    auto& info = attempts[ip];

    if (info.failures >= maxFailures && now - info.lastFailure < std::chrono::seconds(lockoutSeconds))
        return false;

    return true;
}

void LoginAttemptTracker::recordFailure(const std::string& ip)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto& info = attempts[ip];
    info.failures++;
    info.lastFailure = std::chrono::steady_clock::now();
}

void LoginAttemptTracker::reset(const std::string& ip)
{
    std::lock_guard<std::mutex> lock(mutex);
    attempts.erase(ip);
}