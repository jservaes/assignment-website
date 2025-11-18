# assignment-website

A C++ assignment tracker with a web interface that allows multiple users to manage their assignments, exams, quizzes, meetings, and presentations.

## Features

- **Multi-user Support**: Different users can log in with unique usernames and maintain their own private assignment lists
- **Category System**: Organize items by type (Assignment, Exam, Quiz, Meeting, Presentation)
- **Smart Notifications**:
  - Items due within 3 days are highlighted with a "Due Soon" badge
  - Overdue items are highlighted with an "OVERDUE!" badge
- **Auto-removal**: Items are automatically removed when marked as done
- **Modern Web Interface**: Clean, responsive design with gradient background and card-based layout
- **Color-coded Categories**: Each category has its own distinct color for easy identification

## Building the Project

### Prerequisites
- C++17 compatible compiler (g++, clang++)
- Make or CMake

### Using Make
```bash
make
```

### Using CMake
```bash
mkdir build
cd build
cmake ..
make
```

## Running the Application

```bash
./assignment_tracker
```

The server will start on http://localhost:8080

## Usage

1. Navigate to http://localhost:8080 in your web browser
2. Enter a username to login (creates a new account if it doesn't exist)
3. Add assignments with:
   - Title
   - Description (optional)
   - Category (Assignment, Exam, Quiz, Meeting, Presentation)
   - Due Date
4. View all your active items in the list below
5. Mark items as done by clicking the "✓ Mark Done" button
6. Logout when finished

## Multi-User Privacy

Each user has their own private workspace. Users cannot see other users' assignments. Simply login with a different username to access a different account.

## Architecture

- **assignment.h**: Core data structures for assignments and the assignment tracker
- **user_manager.h**: User authentication and session management
- **server.cpp**: HTTP server handling requests and serving the web interface
- **CMakeLists.txt** and **Makefile**: Build configuration files

## Technical Details

- Lightweight C++ HTTP server (no external web server required)
- Session-based authentication using HTTP cookies
- In-memory data storage (data is lost when server restarts)
- Supports multiple concurrent users
- RESTful-style POST endpoints for actions

## Future Enhancements

- Persistent storage (database or file-based)
- Password protection for users
- Edit existing assignments
- Search and filter capabilities
- Email notifications for approaching due dates
- Export assignments to calendar formats

## License

This is a demo/educational project.
