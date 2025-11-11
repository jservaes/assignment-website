#include "assignment.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>

class WebServer {
private:
    AssignmentTracker tracker;
    int port;

    std::string urlDecode(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '%' && i + 2 < str.length()) {
                int value;
                std::istringstream is(str.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    result += static_cast<char>(value);
                    i += 2;
                } else {
                    result += str[i];
                }
            } else if (str[i] == '+') {
                result += ' ';
            } else {
                result += str[i];
            }
        }
        return result;
    }

    std::string parseFormData(const std::string& data, const std::string& field) {
        size_t pos = data.find(field + "=");
        if (pos == std::string::npos) return "";
        
        pos += field.length() + 1;
        size_t end = data.find('&', pos);
        if (end == std::string::npos) end = data.length();
        
        return urlDecode(data.substr(pos, end - pos));
    }

    time_t parseDate(const std::string& dateStr) {
        struct tm tm = {};
        // Parse YYYY-MM-DD format manually for better compatibility
        int year, month, day;
        if (sscanf(dateStr.c_str(), "%d-%d-%d", &year, &month, &day) == 3) {
            tm.tm_year = year - 1900;
            tm.tm_mon = month - 1;
            tm.tm_mday = day;
            tm.tm_hour = 12; // Set to noon to avoid DST issues
            tm.tm_min = 0;
            tm.tm_sec = 0;
            tm.tm_isdst = -1; // Let mktime determine DST
            return mktime(&tm);
        }
        return time(nullptr); // Return current time if parsing fails
    }

    std::string formatDate(time_t t) {
        char buffer[20];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", localtime(&t));
        return std::string(buffer);
    }

    std::string generateHTML() {
        std::ostringstream html;
        html << R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Assignment Tracker</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            padding: 30px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
        }
        h1 {
            color: #333;
            margin-bottom: 30px;
            text-align: center;
            font-size: 2.5em;
        }
        .form-container {
            background: #f8f9fa;
            padding: 25px;
            border-radius: 10px;
            margin-bottom: 30px;
        }
        .form-group {
            margin-bottom: 15px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            color: #555;
            font-weight: 600;
        }
        input, select, textarea {
            width: 100%;
            padding: 10px;
            border: 2px solid #ddd;
            border-radius: 5px;
            font-size: 14px;
            transition: border-color 0.3s;
        }
        input:focus, select:focus, textarea:focus {
            outline: none;
            border-color: #667eea;
        }
        textarea {
            resize: vertical;
            min-height: 80px;
        }
        button {
            background: #667eea;
            color: white;
            padding: 12px 30px;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            cursor: pointer;
            transition: background 0.3s;
        }
        button:hover {
            background: #5568d3;
        }
        .assignments-list {
            display: grid;
            gap: 15px;
        }
        .assignment-card {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
            padding: 20px;
            transition: all 0.3s;
        }
        .assignment-card:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0,0,0,0.1);
        }
        .assignment-card.approaching {
            background: #fff3cd;
            border-color: #ffc107;
        }
        .assignment-card.overdue {
            background: #f8d7da;
            border-color: #dc3545;
        }
        .assignment-header {
            display: flex;
            justify-content: space-between;
            align-items: start;
            margin-bottom: 10px;
        }
        .assignment-title {
            font-size: 1.3em;
            font-weight: 600;
            color: #333;
        }
        .category-badge {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 0.85em;
            font-weight: 600;
            color: white;
        }
        .category-ASSIGNMENT { background: #667eea; }
        .category-EXAM { background: #dc3545; }
        .category-QUIZ { background: #28a745; }
        .category-MEETING { background: #17a2b8; }
        .category-PRESENTATION { background: #fd7e14; }
        .assignment-description {
            color: #666;
            margin: 10px 0;
        }
        .assignment-footer {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-top: 15px;
            padding-top: 15px;
            border-top: 1px solid #e0e0e0;
        }
        .due-date {
            color: #555;
            font-weight: 600;
        }
        .actions form {
            display: inline;
            margin-left: 10px;
        }
        .btn-done {
            background: #28a745;
            padding: 8px 20px;
            font-size: 14px;
        }
        .btn-done:hover {
            background: #218838;
        }
        .notification {
            display: inline-block;
            margin-left: 10px;
            padding: 5px 10px;
            background: #dc3545;
            color: white;
            border-radius: 5px;
            font-size: 0.85em;
            font-weight: 600;
        }
        .notification.warning {
            background: #ffc107;
            color: #333;
        }
        .empty-state {
            text-align: center;
            padding: 60px 20px;
            color: #999;
        }
        .empty-state svg {
            width: 100px;
            height: 100px;
            margin-bottom: 20px;
            opacity: 0.3;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>📚 Assignment Tracker</h1>
        
        <div class="form-container">
            <h2 style="margin-bottom: 20px; color: #333;">Add New Item</h2>
            <form method="POST" action="/add">
                <div class="form-group">
                    <label for="title">Title:</label>
                    <input type="text" id="title" name="title" required>
                </div>
                <div class="form-group">
                    <label for="description">Description:</label>
                    <textarea id="description" name="description"></textarea>
                </div>
                <div class="form-group">
                    <label for="category">Category:</label>
                    <select id="category" name="category" required>
                        <option value="Assignment">Assignment</option>
                        <option value="Exam">Exam</option>
                        <option value="Quiz">Quiz</option>
                        <option value="Meeting">Meeting</option>
                        <option value="Presentation">Presentation</option>
                    </select>
                </div>
                <div class="form-group">
                    <label for="dueDate">Due Date:</label>
                    <input type="date" id="dueDate" name="dueDate" required>
                </div>
                <button type="submit">Add Item</button>
            </form>
        </div>

        <h2 style="margin-bottom: 20px; color: #333;">Active Items</h2>
        <div class="assignments-list">
)";

        auto assignments = tracker.getActiveAssignments();
        if (assignments.empty()) {
            html << R"(
            <div class="empty-state">
                <svg viewBox="0 0 24 24" fill="currentColor">
                    <path d="M19 3h-4.18C14.4 1.84 13.3 1 12 1c-1.3 0-2.4.84-2.82 2H5c-1.1 0-2 .9-2 2v14c0 1.1.9 2 2 2h14c1.1 0 2-.9 2-2V5c0-1.1-.9-2-2-2zm-7 0c.55 0 1 .45 1 1s-.45 1-1 1-1-.45-1-1 .45-1 1-1zm2 14H7v-2h7v2zm3-4H7v-2h10v2zm0-4H7V7h10v2z"/>
                </svg>
                <p style="font-size: 1.2em;">No active items yet!</p>
                <p>Add your first assignment or event above.</p>
            </div>
)";
        } else {
            for (const auto& assignment : assignments) {
                std::string cardClass = "assignment-card";
                std::string notification = "";
                
                if (assignment.isOverdue()) {
                    cardClass += " overdue";
                    notification = R"(<span class="notification">OVERDUE!</span>)";
                } else if (assignment.isApproaching()) {
                    cardClass += " approaching";
                    notification = R"(<span class="notification warning">Due Soon</span>)";
                }

                html << "<div class=\"" << cardClass << "\">\n";
                html << "  <div class=\"assignment-header\">\n";
                html << "    <div class=\"assignment-title\">" << assignment.title << notification << "</div>\n";
                html << "    <span class=\"category-badge category-" << assignment.categoryToString() << "\">" 
                     << assignment.categoryToString() << "</span>\n";
                html << "  </div>\n";
                
                if (!assignment.description.empty()) {
                    html << "  <div class=\"assignment-description\">" << assignment.description << "</div>\n";
                }
                
                html << "  <div class=\"assignment-footer\">\n";
                html << "    <div class=\"due-date\">📅 Due: " << formatDate(assignment.dueDate) << "</div>\n";
                html << "    <div class=\"actions\">\n";
                html << "      <form method=\"POST\" action=\"/done\">\n";
                html << "        <input type=\"hidden\" name=\"id\" value=\"" << assignment.id << "\">\n";
                html << "        <button type=\"submit\" class=\"btn-done\">✓ Mark Done</button>\n";
                html << "      </form>\n";
                html << "    </div>\n";
                html << "  </div>\n";
                html << "</div>\n";
            }
        }

        html << R"(
        </div>
    </div>
</body>
</html>
)";
        return html.str();
    }

    std::string handleRequest(const std::string& request) {
        std::istringstream stream(request);
        std::string method, path, version;
        stream >> method >> path >> version;

        // Read headers to find Content-Length
        std::string line;
        int contentLength = 0;
        while (std::getline(stream, line) && line != "\r") {
            if (line.find("Content-Length:") == 0) {
                contentLength = std::stoi(line.substr(15));
            }
        }

        // Read body if present
        std::string body;
        if (contentLength > 0) {
            body.resize(contentLength);
            stream.read(&body[0], contentLength);
        }

        std::string response;
        if (method == "GET" && path == "/") {
            std::string html = generateHTML();
            response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text/html; charset=UTF-8\r\n";
            response += "Content-Length: " + std::to_string(html.length()) + "\r\n";
            response += "\r\n";
            response += html;
        } else if (method == "POST" && path == "/add") {
            std::string title = parseFormData(body, "title");
            std::string description = parseFormData(body, "description");
            std::string categoryStr = parseFormData(body, "category");
            std::string dueDateStr = parseFormData(body, "dueDate");

            Category category = Assignment::stringToCategory(categoryStr);
            time_t dueDate = parseDate(dueDateStr);

            tracker.addAssignment(title, description, category, dueDate);

            // Redirect to home
            response = "HTTP/1.1 303 See Other\r\n";
            response += "Location: /\r\n";
            response += "\r\n";
        } else if (method == "POST" && path == "/done") {
            std::string idStr = parseFormData(body, "id");
            int id = std::stoi(idStr);
            tracker.markDone(id);

            // Redirect to home
            response = "HTTP/1.1 303 See Other\r\n";
            response += "Location: /\r\n";
            response += "\r\n";
        } else {
            response = "HTTP/1.1 404 Not Found\r\n\r\n";
        }

        return response;
    }

public:
    WebServer(int port = 8080) : port(port) {}

    void run() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "Failed to create socket\n";
            return;
        }

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed\n";
            close(server_fd);
            return;
        }

        if (listen(server_fd, 10) < 0) {
            std::cerr << "Listen failed\n";
            close(server_fd);
            return;
        }

        std::cout << "Assignment Tracker Server running on http://localhost:" << port << "\n";
        std::cout << "Press Ctrl+C to stop the server.\n\n";

        while (true) {
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) continue;

            char buffer[4096] = {0};
            ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
            if (bytes_read <= 0) {
                close(client_fd);
                continue;
            }

            std::string response = handleRequest(std::string(buffer));
            ssize_t bytes_written = write(client_fd, response.c_str(), response.length());
            (void)bytes_written; // Suppress unused variable warning
            close(client_fd);
        }

        close(server_fd);
    }
};

int main() {
    WebServer server(8080);
    server.run();
    return 0;
}
