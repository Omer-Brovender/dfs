#ifndef LOGINATTEMPTTRACKER_HPP
#define LOGINATTEMPTTRACKER_HPP

#include <string>
#include <unordered_map>
#include <chrono>
#include <mutex>

#undef min

struct Info {
    int failures = 0;
    std::chrono::steady_clock::time_point lastFailure = std::chrono::steady_clock::time_point::min();
};

class LoginAttemptTracker {
private:

    std::unordered_map<std::string, Info> attempts;
    const int maxFailures = 5;
    const int lockoutSeconds = 60;
    std::mutex mutex;

public:
    bool allowLogin(const std::string& ip);
    void recordFailure(const std::string& ip);
    void reset(const std::string& ip);
};

#endif