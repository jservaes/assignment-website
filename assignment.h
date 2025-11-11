#ifndef ASSIGNMENT_H
#define ASSIGNMENT_H

#include <string>
#include <ctime>
#include <vector>
#include <algorithm>

enum class Category {
    ASSIGNMENT,
    EXAM,
    QUIZ,
    MEETING,
    PRESENTATION
};

struct Assignment {
    int id;
    std::string title;
    std::string description;
    Category category;
    time_t dueDate;
    bool isDone;

    Assignment(int id, const std::string& title, const std::string& desc, 
               Category cat, time_t due)
        : id(id), title(title), description(desc), category(cat), 
          dueDate(due), isDone(false) {}

    // Check if due date is approaching (within 3 days)
    bool isApproaching() const {
        if (isDone) return false;
        time_t now = time(nullptr);
        double diffSeconds = difftime(dueDate, now);
        double diffDays = diffSeconds / (60 * 60 * 24);
        return diffDays >= 0 && diffDays <= 3;
    }

    // Check if overdue
    bool isOverdue() const {
        if (isDone) return false;
        time_t now = time(nullptr);
        return difftime(dueDate, now) < 0;
    }

    std::string categoryToString() const {
        switch (category) {
            case Category::ASSIGNMENT: return "Assignment";
            case Category::EXAM: return "Exam";
            case Category::QUIZ: return "Quiz";
            case Category::MEETING: return "Meeting";
            case Category::PRESENTATION: return "Presentation";
            default: return "Unknown";
        }
    }

    static Category stringToCategory(const std::string& str) {
        if (str == "Assignment") return Category::ASSIGNMENT;
        if (str == "Exam") return Category::EXAM;
        if (str == "Quiz") return Category::QUIZ;
        if (str == "Meeting") return Category::MEETING;
        if (str == "Presentation") return Category::PRESENTATION;
        return Category::ASSIGNMENT;
    }
};

class AssignmentTracker {
private:
    std::vector<Assignment> assignments;
    int nextId;

public:
    AssignmentTracker() : nextId(1) {}

    int addAssignment(const std::string& title, const std::string& desc,
                     Category category, time_t dueDate) {
        Assignment newAssignment(nextId++, title, desc, category, dueDate);
        assignments.push_back(newAssignment);
        return newAssignment.id;
    }

    bool removeAssignment(int id) {
        auto it = std::find_if(assignments.begin(), assignments.end(),
                              [id](const Assignment& a) { return a.id == id; });
        if (it != assignments.end()) {
            assignments.erase(it);
            return true;
        }
        return false;
    }

    bool markDone(int id) {
        auto it = std::find_if(assignments.begin(), assignments.end(),
                              [id](const Assignment& a) { return a.id == id; });
        if (it != assignments.end()) {
            it->isDone = true;
            // Auto-remove when marked done
            assignments.erase(it);
            return true;
        }
        return false;
    }

    const std::vector<Assignment>& getAssignments() const {
        return assignments;
    }

    std::vector<Assignment> getActiveAssignments() const {
        std::vector<Assignment> active;
        for (const auto& a : assignments) {
            if (!a.isDone) {
                active.push_back(a);
            }
        }
        return active;
    }

    Assignment* findById(int id) {
        auto it = std::find_if(assignments.begin(), assignments.end(),
                              [id](const Assignment& a) { return a.id == id; });
        if (it != assignments.end()) {
            return &(*it);
        }
        return nullptr;
    }
};

#endif // ASSIGNMENT_H
