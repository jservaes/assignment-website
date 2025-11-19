#include <iostream>
#include <mutex>
#include <vector>
#include <chrono>
#include <ctime>

#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

enum class Category {
    Assignment,
    Exam,
    Quiz,
    Meeting,
    Presentation
};

enum class ItemStatus {
    Pending,
    Done
};

struct Item {
    int id;
    int userId;
    std::string title;
    std::string description;
    Category category;
    std::string dueDateIso; // ISO string
    ItemStatus status;
};

std::vector<Item> items;
std::mutex itemsMutex;
int nextId = 1;

// === Helpers ===

Category categoryFromString(const std::string &s) {
    if (s == "Exam") return Category::Exam;
    if (s == "Quiz") return Category::Quiz;
    if (s == "Meeting") return Category::Meeting;
    if (s == "Presentation") return Category::Presentation;
    return Category::Assignment;
}

std::string categoryToString(Category c) {
    switch (c) {
        case Category::Exam: return "Exam";
        case Category::Quiz: return "Quiz";
        case Category::Meeting: return "Meeting";
        case Category::Presentation: return "Presentation";
        default: return "Assignment";
    }
}

std::string statusToString(ItemStatus s) {
    return s == ItemStatus::Done ? "Done" : "Pending";
}

// Rough diff in days between now and dueDateIso (YYYY-MM-DDTHH:MM:SS)
int daysUntil(const std::string &dueIso) {
    if (dueIso.size() < 10) return 9999;
    int y = std::stoi(dueIso.substr(0, 4));
    int m = std::stoi(dueIso.substr(5, 2));
    int d = std::stoi(dueIso.substr(8, 2));

    std::tm due_tm{};
    due_tm.tm_year = y - 1900;
    due_tm.tm_mon  = m - 1;
    due_tm.tm_mday = d;
    auto due_time = std::mktime(&due_tm);

    auto now_time_t = std::time(nullptr);
    double diff_sec = std::difftime(due_time, now_time_t);
    int days = static_cast<int>(diff_sec / (60 * 60 * 24));
    return days;
}

// Choose badge based on due date and status
std::string computeBadge(const Item &item) {
    if (item.status == ItemStatus::Done) return "";
    int days = daysUntil(item.dueDateIso);
    if (days < 0)  return "OVERDUE";
    if (days <= 3) return "DUE_SOON";
    return "";
}

json itemToJson(const Item &it) {
    json j;
    j["id"] = it.id;
    j["userId"] = it.userId;
    j["title"] = it.title;
    j["description"] = it.description;
    j["category"] = categoryToString(it.category);
    j["dueDate"] = it.dueDateIso;
    j["status"] = statusToString(it.status);
    j["badge"] = computeBadge(it);
    return j;
}

// === main ===

int main() {
    httplib::Server svr;

    // GET /api/items?userId=3
    svr.Get("/api/items", [](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("userId")) {
            res.status = 400;
            res.set_content(R"({"error":"userId required"})", "application/json");
            return;
        }
        int userId = std::stoi(req.get_param_value("userId"));

        json out = json::array();
        {
            std::lock_guard<std::mutex> lock(itemsMutex);
            for (const auto &it : items) {
                if (it.userId == userId) {
                    out.push_back(itemToJson(it));
                }
            }
        }

        res.set_content(out.dump(), "application/json");
    });

    // POST /api/items  (JSON body)
    // { "userId": 3, "title": "...", "description": "...", "category":"Assignment", "dueDate":"2025-11-22T23:59:00" }
    svr.Post("/api/items", [](const httplib::Request &req, httplib::Response &res) {
        try {
            auto body = json::parse(req.body);

            Item it;
            it.id = nextId++;
            it.userId = body.at("userId").get<int>();
            it.title = body.at("title").get<std::string>();
            it.description = body.value("description", "");
            it.category = categoryFromString(body.value("category", "Assignment"));
            it.dueDateIso = body.at("dueDate").get<std::string>();
            it.status = ItemStatus::Pending;

            {
                std::lock_guard<std::mutex> lock(itemsMutex);
                items.push_back(it);
            }

            res.status = 201;
            res.set_content(itemToJson(it).dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error":"invalid JSON"})", "application/json");
        }
    });

    // PATCH /api/items/{id}  body: { "status": "Done" }
    svr.Patch(R"(/api/items/(\d+))", [](const httplib::Request &req, httplib::Response &res) {
        int id = std::stoi(req.matches[1]);
        try {
            auto body = json::parse(req.body);
            std::string newStatus = body.value("status", "");

            std::lock_guard<std::mutex> lock(itemsMutex);
            for (auto it = items.begin(); it != items.end(); ++it) {
                if (it->id == id) {
                    if (newStatus == "Done") {
                        // Auto-removal when marked as done
                        items.erase(it);
                        res.status = 204; // No Content
                        return;
                    } else {
                        it->status = ItemStatus::Pending;
                        res.set_content(itemToJson(*it).dump(), "application/json");
                        return;
                    }
                }
            }

            res.status = 404;
            res.set_content(R"({"error":"not found"})", "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error":"invalid JSON"})", "application/json");
        }
    });

    // Optional DELETE /api/items/{id}
    svr.Delete(R"(/api/items/(\d+))", [](const httplib::Request &req, httplib::Response &res) {
        int id = std::stoi(req.matches[1]);
        std::lock_guard<std::mutex> lock(itemsMutex);
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (it->id == id) {
                items.erase(it);
                res.status = 204;
                return;
            }
        }
        res.status = 404;
        res.set_content(R"({"error":"not found"})", "application/json");
    });

    std::cout << "Server running on http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}
