#include <iostream>
#include <chrono>

class ServerTimer {
private:
    std::chrono::time_point<std::chrono::steady_clock> created; // creation
    std::chrono::time_point<std::chrono::steady_clock> timer;   // last refresh
    float expiry;  // expiry time in seconds

public:
    ServerTimer(float expiry = -1.f)
        : expiry(expiry)
    {
        created = timer = std::chrono::steady_clock::now();
    }

    // Changes the expiry for the timer in seconds
    void SetExpiry(float i) {
        if (i > 0) {
            expiry = i;
        }
    }

    // Refreshes the Timer
    void Refresh(float i = -1) {
        timer = std::chrono::steady_clock::now();
        if (i > 0) {
            expiry = i; // If the new expiry is specified in seconds, it's stored as is
        }
    }

    // Returns true if the Timer expired
    bool Expired(float i = -1) {
        float time_expiry = (i > 0) ? i : expiry;
        float elapsed = std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - timer).count() / 1000; // Convert to seconds
        return (elapsed > time_expiry);
    }

    // Returns true if the Timer expired & refreshes it
    bool Expired_Refresh(float i = -1) {
        float time_expiry = (i > 0) ? i : expiry;
        float time_diff = Diff();
        if (time_diff >= time_expiry) {
            Refresh();
            return true;
        }
        return false;
    }

    // Returns the remaining time until the timer expires
    float GetExpiry() const {
        float diff = std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - timer).count() / 1000; // Convert to seconds
        return (expiry - diff);
    }

    // Returns the time difference between the creation of the timer
    float Diff_C() const {
        return std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - created).count() / 1000; // Convert to seconds
    }

    // Returns the time difference since the last refresh of the timer
    float Diff() const {
        return std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - timer).count() / 1000; // Convert to seconds
    }

    // Returns the time difference since the last refresh of the timer & refreshes the timer
    float Diff_Refresh() {
        float diff = std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - timer).count() / 1000; // Convert to seconds
        Refresh(); // Resets the timer
        return diff;
    }
};
