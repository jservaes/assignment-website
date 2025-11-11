#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "assignment.h"
#include <string>
#include <map>
#include <random>
#include <sstream>

class UserManager {
private:
    struct UserData {
        std::string username;
        AssignmentTracker tracker;
    };

    std::map<std::string, UserData> users;  // username -> user data
    std::map<std::string, std::string> sessions;  // sessionId -> username

    std::string generateSessionId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        
        const char* hexChars = "0123456789abcdef";
        std::string sessionId;
        for (int i = 0; i < 32; ++i) {
            sessionId += hexChars[dis(gen)];
        }
        return sessionId;
    }

public:
    // Create or login a user and return a session ID
    std::string loginUser(const std::string& username) {
        if (username.empty()) return "";
        
        // Create user if doesn't exist
        if (users.find(username) == users.end()) {
            users[username] = UserData{username, AssignmentTracker()};
        }
        
        // Generate new session
        std::string sessionId = generateSessionId();
        sessions[sessionId] = username;
        
        return sessionId;
    }

    // Get username from session ID
    std::string getUserFromSession(const std::string& sessionId) {
        auto it = sessions.find(sessionId);
        if (it != sessions.end()) {
            return it->second;
        }
        return "";
    }

    // Get user's tracker
    AssignmentTracker* getTracker(const std::string& username) {
        auto it = users.find(username);
        if (it != users.end()) {
            return &(it->second.tracker);
        }
        return nullptr;
    }

    // Logout user
    void logout(const std::string& sessionId) {
        sessions.erase(sessionId);
    }
};

#endif // USER_MANAGER_H
