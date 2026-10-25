#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
using namespace std;

// Stores the information for one course.
struct Course {
    string number;
    string title;
    vector<string> prerequisites;
};

// Removes extra spaces from input.
string trim(const string& text) {
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

// Makes course numbers consistent for searching.
string normalize(string value) {
    value = trim(value);
    transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(toupper(c));
    });
    return value;
}

// Binary Search Tree for storing and sorting courses.
class CourseTree {
private:
    struct Node {
        Course course;
        Node* left;
        Node* right;

        explicit Node(const Course& value)
            : course(value), left(nullptr), right(nullptr) {}
    };

    Node* root = nullptr;

    // Frees the memory used by the tree.
    static void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    // In-order traversal prints courses alphabetically.
    static void printInOrder(const Node* node) {
        if (!node) return;
        printInOrder(node->left);
        cout << node->course.number << ", "
             << node->course.title << '\n';
        printInOrder(node->right);
    }

public:
    CourseTree() = default;
    CourseTree(const CourseTree&) = delete;
    CourseTree& operator=(const CourseTree&) = delete;

    ~CourseTree() {
        destroy(root);
    }

    // Inserts a course into the binary search tree.
    void insert(const Course& course) {
        Node** current = &root;

        while (*current) {
            if (course.number < (*current)->course.number) {
                current = &((*current)->left);
            } else if (course.number > (*current)->course.number) {
                current = &((*current)->right);
            } else {
                (*current)->course = course;
                return;
            }
        }

        *current = new Node(course);
    }

    // Searches for a course by its course number.
    const Course* find(const string& number) const {
        const Node* current = root;

        while (current) {
            if (number == current->course.number) {
                return &current->course;
            }
            current = number < current->course.number
                    ? current->left : current->right;
        }
        return nullptr;
    }

    void printAll() const {
        printInOrder(root);
    }

    void swap(CourseTree& other) {
        std::swap(root, other.root);
    }
};

// Reads and validates the CSV file before loading the tree.
bool loadCourses(const string& filename, CourseTree& tree) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Could not open file '"
             << filename << "'.\n";
        return false;
    }

    vector<Course> courses;
    unordered_set<string> numbers;
    string line;
    size_t lineNumber = 0;

    while (getline(file, line)) {
        ++lineNumber;
        if (trim(line).empty()) continue;

        stringstream stream(line);
        string field;
        vector<string> fields;

        while (getline(stream, field, ',')) {
            fields.push_back(trim(field));
        }

        if (fields.size() < 2 ||
            fields[0].empty() || fields[1].empty()) {
            cout << "Error: Invalid course data on line "
                 << lineNumber << ".\n";
            return false;
        }

        Course course;
        course.number = normalize(fields[0]);
        course.title = fields[1];

        if (!numbers.insert(course.number).second) {
            cout << "Error: Duplicate course number on line "
                 << lineNumber << ".\n";
            return false;
        }


        for (size_t i = 2; i < fields.size(); ++i) {
            if (!fields[i].empty()) {
                course.prerequisites.push_back(normalize(fields[i]));
            }
        }


        courses.push_back(course);
    }

    if (courses.empty()) {
        cout << "Error: The file contains no courses.\n";
        return false;
    }

    // Verify that every prerequisite exists in the file.
    for (const Course& course : courses) {
        for (const string& prerequisite : course.prerequisites) {
            if (numbers.count(prerequisite) == 0) {
                cout << "Error: Unknown prerequisite "
                     << prerequisite << " for "
                     << course.number << ".\n";
                return false;
            }
        }
    }

    CourseTree newTree;

    for (const Course& course : courses) {
        newTree.insert(course);
    }

    // Replace existing data only after a successful load.
    tree.swap(newTree);

    cout << courses.size()
         << " courses loaded successfully.\n";
    return true;
}

// Prints one course and its prerequisite titles.
void printCourseDetails(const CourseTree& tree,
                        const string& courseNumber) {
    const Course* course = tree.find(normalize(courseNumber));

    if (!course) {
        cout << "Error: Course not found.\n";
        return;
    }

    cout << course->number << ", "
         << course->title << '\n';

    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None\n";
        return;
    }

    cout << "Prerequisites: ";

    for (size_t i = 0; i < course->prerequisites.size(); ++i) {
        if (i > 0) cout << ", ";

        const string& number = course->prerequisites[i];
        const Course* prerequisite = tree.find(number);

        cout << number << " ("
             << (prerequisite ? prerequisite->title : "Unknown")
             << ")";
    }

    cout << '\n';
}

// Main menu for the advising assistance program.
int main() {
    CourseTree courses;
    bool loaded = false;
    string choice;

    cout << "Welcome to the ABCU Advising Assistance Program.\n";

    while (true) {
        cout << "\n1. Load Data Structure\n"
             << "2. Print Course List\n"
             << "3. Print Course\n"
             << "9. Exit\n"
             << "What would you like to do? ";

        if (!getline(cin, choice)) break;
        choice = trim(choice);

        if (choice == "1") {
            string filename;
            cout << "Enter the course data filename: ";

            if (!getline(cin, filename)) break;

            if (loadCourses(trim(filename), courses)) {
                loaded = true;
            }

        } else if (choice == "2") {
            if (!loaded) {
                cout << "Please load the course data first (option 1).\n";
            } else {
                cout << "\nHere is a sample schedule:\n";
                courses.printAll();
            }

        } else if (choice == "3") {
            if (!loaded) {
                cout << "Please load the course data first (option 1).\n";
            } else {
                string number;
                cout << "What course do you want to know about? ";

                if (!getline(cin, number)) break;
                printCourseDetails(courses, number);
            }

        } else if (choice == "9") {
            cout << "Thank you for using the course planner!\n";
            break;

        } else {
            cout << "Error: Invalid option. "
                 << "Please enter 1, 2, 3, or 9.\n";
        }
    }

    return 0;
}
