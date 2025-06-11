#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
// #include <stdexcept>
// #include <vector>
// #include <memory>

using namespace std;

// ---------- Abstract User ----------
class User {
public:
    virtual void password() const = 0;
    virtual void greet() const = 0;
    virtual void fileOperations() const = 0;
    virtual ~User() {}
};

// ---------- File Manager ----------
class FileManager {
public:
void run() const {
    while (true) {
        displayOptions();
        int choice = getChoice();
        
        if (choice == 4) {
            cout << "Returning to main login page...\n";
            throw runtime_error("Returning to login"); // Signal to return to login
        }

        if (choice == 5) {
            exitProgram();
        }

        string filename;
        if (choice >= 1 && choice <= 3) {
            filename = getFilename();
        }

        try {
            switch (choice) {
                case 1: writeToFile(filename); break;
                case 2: readFromFile(filename); break;
                case 3: deleteFile(filename); break;
                default: cout << "Invalid choice.\n";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
}

private:
    void displayOptions() const {
        cout << "\nFile Menu:\n";
        cout << "1. Write to file\n";
        cout << "2. Read from file\n";
        cout << "3. Delete file\n";
        cout << "4. Exit file menu\n";
        cout << "5. Exit form program\n";
    }

    int getChoice() const {
        int choice;
        while (true) {
            cout << "Your choice: ";
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Invalid input. Enter a number.\n";
            } else {
                cin.ignore(1000, '\n');
                return choice;
            }
        }
    }

    string getFilename() const {
        string name;
        cout << "Enter filename (with .txt): ";
        getline(cin, name);
        if (name.find(".txt") == string::npos) {
            name += ".txt";
        }
        return name;
    }

    void writeToFile(const string& filename) const {
        ofstream file;
        file.exceptions(ofstream::failbit | ofstream::badbit);
        try {
            file.open(filename, ios::app);
            cout << "Enter text: ";
            string text;
            getline(cin, text);
            file << text << '\n';
            file.close();
            cout << "Text written successfully.\n";
        } catch (const ofstream::failure&) {
            throw runtime_error("Failed to open file for writing.");
        }
    }

    void readFromFile(const string& filename) const {
        ifstream file;
        try {
            file.open(filename); 
            if (!file.is_open()) {
                throw runtime_error("File not found or cannot be read.");
            }

            string line;
            cout << "\n--- File Content ---\n";
            while (getline(file, line)) {
                cout << line << '\n';
            }
        } catch (const ifstream::failure&) {
            throw runtime_error("Error reading file. Make sure the file exists and is readable.");
        }
    }

    void deleteFile(const string& filename) const {
        if (remove(filename.c_str()) == 0) {
            cout << "File deleted successfully.\n";
        } else {
            throw runtime_error("Could not delete file (not found or permission issue).");
        }
    }
      
    void exitProgram() const {
        cout << "Exiting...\n";
        exit(0);
    }
};

// ---------- Admin ----------
class Admin : public User {
public:
    void password() const override {
        string id, password;
        cout << "\n----------AUTHENTICATION PAGE----------\n";
        cout << "User ID: ";
        cin >> id;
        cout << "Password: ";
        cin >> password;

        if ((id.substr(0, 2) == "AA" || id.substr(0, 2) == "PR" || 
             id.substr(0, 2) == "AC" || id.substr(0, 2) == "HR" || 
             id.substr(0, 2) == "SA") && password == "admin123") {
            departmentPrefix = id.substr(0, 2);
            cout << "Access granted for " << departmentName(departmentPrefix) << " Department.\n";
        } else {
            throw runtime_error("Invalid credentials.");
        }
    }

    void greet() const override {
        cout << "\nWelcome, Admin of " << departmentName(departmentPrefix) << " Department!\n";
    }

    void fileOperations() const override {
        FileManager manager;
        manager.run();
    }

    static Admin* createAdmin(const string& prefix) {
        return new Admin();
    }

private:
    mutable string departmentPrefix;

    string departmentName(const string& prefix) const {
        if (prefix == "AA") return "Academic Affairs";
        if (prefix == "PR") return "PR Department";
        if (prefix == "AC") return "Accounting";
        if (prefix == "HR") return "HR Department";
        if (prefix == "SA") return "Student Affairs";
        return "Unknown";
    }
};

// ---------- Professor ----------
class Professor : public User {
    public:
        void password() const override {
            string id, password;
            cout << "\n----------AUTHANTICATION PAGE----------\n";
            cout << "\nUser ID: ";
            cin >> id;
            cout << "Password: ";
            cin >> password;
            
            // Simulated validation
            if (id == "prof" && password == "1234") {
                cout << "Access granted.\n";
            } else {
                throw runtime_error("Invalid credentials.");
            }
        }
    
        void greet() const override {
            cout << "\nAssalomu alaykum, Professor!\n";
            for (int i = 1; i <= 5; ++i) {
                cout << "OOP2 -- Section[00" << i << "]\n";
            }
        }
    
        void fileOperations() const override {
            FileManager manager;
            manager.run();
        }
    };
    
    // ---------- Student ----------
    class Student : public User {
    public:
        void greet() const override {
            cout << "\nAssalomu alaykum, Student!\nWelcome to IUT's File Management System\n";
        }
    
        void fileOperations() const override {
            showDepartments();
            FileManager manager;
            manager.run();
            
        }
    
    private:
        
        void showDepartments() const {
            while (true) {
                displayDepartments();
                int choice = getChoice();
                try {
                    switch (choice) {
                        case 1: readDepartment("Academic_Affairs.txt"); break;
                        case 2: readDepartment("Student_Union.txt"); break;
                        case 3: readDepartment("PR_department.txt"); break;
                        case 4: readDepartment("Accounting.txt"); break;
                        case 5: return;
                        case 6: exitProgram(); break;
                        default: cout << "Invalid option.\n";
                    }
                } catch (const exception& e) {
                    cout << "Error: " << e.what() << "\n";
                }
    
                
            }
        }
    
        void displayDepartments() const {
            cout << "\nDepartments:\n";
            cout << "1. Academic Affairs\n";
            cout << "2. Student Union\n";
            cout << "3. PR Department\n";
            cout << "4. Accounting\n";
            cout << "5. Return to FMS\n";
            cout << "6. Exit\n";
           
        }
        void readDepartment(const string& filename) const {
            try {
                ifstream file(filename); // File is opened here
                if (!file.is_open()) {
                    throw runtime_error("File not found or cannot be read.");
                }
        
                string line;
                cout << "\n--- Department Info ---\n";
                while (getline(file, line)) {
                    cout << line << '\n';
                }
                // File is automatically closed when file goes out of scope
            } catch (const ifstream::failure&) {
                throw runtime_error("Error reading file. Make sure the file exists and is readable.");
            } catch (const runtime_error& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
        
        
    
        int getChoice() const {
            int choice;
            while (true) {
                cout << "Your choice: ";
                if (!(cin >> choice)) {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    cout << "Invalid input.\n";
                } else {
                    cin.ignore(1000, '\n');
                    return choice;
                }
            }
        }
    
        void password() const override {
            string id, password;
            cout << "\n----------AUTHANTICATION PAGE----------\n";
            cout << "\nUser ID: ";
            cin >> id;
            cout << "Password: ";
            cin >> password;
            
            // Simulated validation
            if (id == "student" && password == "1234") {
                cout << "Access granted.\n";
            } else {
                throw runtime_error("Invalid credentials.");
            }
        }
        
        void exitProgram() const {
            cout << "Exiting...\n";
            exit(0);
        }
    };
    
    
// ---------- Main Application ----------
class FMSApp {
public:
void run() {
    while (true) {
        try {
            displayLogin();
            int role = getChoice();
            User* user = nullptr;

            if (role == 1) {
                user = new Admin();
            } else if (role == 2) {
                user = new Professor();
            } else if (role == 3) {
                user = new Student();
            } else {
                throw invalid_argument("Invalid role selection.");
            }

            try {
                user->password();
                user->greet();
                user->fileOperations();
            } catch (const runtime_error& e) {
                if (string(e.what()) == "Returning to login") {
                    cout << "Restarting login...\n";
                    delete user; // Free allocated memory before restarting
                    continue;   // Restart the login loop
                }
                throw; // Rethrow other runtime errors
            }

            delete user; // Clean up after successful execution
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\nTry again.\n";
        }
    }
}

private:
    void displayLogin() const {
        cout << "\n----------LOGIN PAGE----------\n";
        cout << "1. Admin\n";
        cout << "2. Professor\n";
        cout << "3. Student\n";
    }

    int getChoice() const {
        int choice;
        while (true) {
            cout << "Your choice: ";
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Invalid input.\n";
            } else {
                cin.ignore(1000, '\n');
                return choice;
            }
        }
    }
};

// ---------- Main Function ----------
int main() {
    FMSApp app;
    app.run();
    return 0;
}
