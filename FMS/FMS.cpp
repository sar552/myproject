#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <algorithm>
#include <sys/stat.h>  // For mkdir on Linux/macOS
#include <errno.h>     // For error number checking
using namespace std;

// ---------- Global Variables ----------
map<string, vector<string>> messageInbox; // Stores messages for users (username -> messages)
map<string, pair<string, string>> userCredentials;
map<string, string> studentFiles;        // Stores student files (studentID -> filename)

// ---------- Abstract User ----------
class User {
protected:
    string username;
public:
    virtual void password() = 0;
    virtual void greet() = 0;
    virtual void fileOperations() = 0;
    virtual ~User() {}
    string getUsername() const { return username; }
};

// ---------- File Manager ----------
class FileManager {
public:
    void run(User* currentUser) {
        while (true) {
            displayOptions(currentUser);
            int choice = getChoice();
            
            if (choice == 5) {
                cout << "Returning to main menu...\n";
                return;  // Simply return from the function to go back to the main menu
            }

            if (choice == 6) {
                cout << "Logging out...\n";
                throw runtime_error("Returning to login");
            }

            if (choice == 7) {
                exitProgram();
            }

            string filename;
            if (choice >= 1 && choice <= 4) {
                filename = getFilename();
            }

            try {
                switch (choice) {
                    case 1: writeToFile(filename); break;
                    case 2: readFromFile(filename); break;
                    case 3: deleteFile(filename); break;
                    case 4: sendFileToUser(filename, currentUser); break;
                    default: cout << "Invalid choice.\n";
                }
            } catch (const exception& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
    }

private:
    void displayOptions(User* user) {
        cout << "\nFile Menu:\n";
        cout << "1. Write to file\n";
        cout << "2. Read from file\n";
        cout << "3. Delete file\n";
        cout << "4. Send file to user\n";
        cout << "5. Return to Main Menu\n";  // Changed from "Log out"
        cout << "6. Log out\n";             // Moved logout to option 6
        cout << "7. Exit program\n";        // Moved exit to option 7
        
        if (messageInbox.count(user->getUsername()) && !messageInbox[user->getUsername()].empty()) {
            cout << "\nYou have " << messageInbox[user->getUsername()].size() << " unread message(s)\n";
        }
    }

    int getChoice() {
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

    string getFilename() {
        string name;
        cout << "Enter filename (with .txt): ";
        getline(cin, name);
        if (name.find(".txt") == string::npos) {
            name += ".txt";
        }
        return name;
    }

    void writeToFile(const string& filename) {
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

    void readFromFile(const string& filename) {
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

    void deleteFile(const string& filename) {
        if (remove(filename.c_str()) != 0) {
            throw runtime_error("Could not delete file (not found or permission issue).");
        } else {
            cout << "File deleted successfully.\n";
        }
    }
    
    void sendFileToUser(const string& filename, User* sender) {
        string recipient;
        cout << "Enter recipient username (or 'all' to send to all users): ";
        getline(cin, recipient);
        
        ifstream sourceFile(filename);
        if (!sourceFile) {
            throw runtime_error("Could not open source file.");
        }
    
        // Convert input to lowercase for case-insensitive comparison
        string lowercaseInput = recipient;
        transform(lowercaseInput.begin(), lowercaseInput.end(), lowercaseInput.begin(), ::tolower);
    
        if (lowercaseInput == "all") {
            // Send to all users
            int sentCount = 0;
            for (const auto& user : userCredentials) {
                string userFolder = user.first + "_received_files";
                
                #ifdef _WIN32
                    if (_mkdir(userFolder.c_str()) != 0 && errno != EEXIST) {
                        cerr << "Warning: Could not create directory for " << user.first << endl;
                        continue;
                    }
                #else
                    if (mkdir(userFolder.c_str(), 0755) != 0 && errno != EEXIST) {
                        cerr << "Warning: Could not create directory for " << user.first << endl;
                        continue;
                    }
                #endif
                
                string newFilename = user.first + "_from_" + sender->getUsername() + "_" + filename;
                string fullPath = userFolder + "/" + newFilename;
                
                ofstream destFile(fullPath);
                if (destFile) {
                    sourceFile.clear();
                    sourceFile.seekg(0);
                    destFile << sourceFile.rdbuf();
                    destFile.close();
                    
                    string message = "You received a file from " + sender->getUsername() + ": " + filename;
                    messageInbox[user.first].push_back(message);
                    sentCount++;
                } else {
                    cerr << "Warning: Could not send file to " << user.first << endl;
                }
            }
            sourceFile.close();
            cout << "File sent successfully to " << sentCount << " users.\n";
        } else {
            // Original single-user sending logic
            if (!userCredentials.count(recipient)) {
                throw runtime_error("Recipient not found.");
            }
            
            string recipientFolder = recipient + "_received_files";
            
            #ifdef _WIN32
                if (_mkdir(recipientFolder.c_str()) != 0 && errno != EEXIST) {
                    throw runtime_error("Could not create recipient directory.");
                }
            #else
                if (mkdir(recipientFolder.c_str(), 0755) != 0 && errno != EEXIST) {
                    throw runtime_error("Could not create recipient directory.");
                }
            #endif
            
            string newFilename = recipient + "_from_" + sender->getUsername() + "_" + filename;
            string fullPath = recipientFolder + "/" + newFilename;
            
            ofstream destFile(fullPath);
            if (!destFile) {
                throw runtime_error("Could not create file for recipient.");
            }
            
            destFile << sourceFile.rdbuf();
            sourceFile.close();
            destFile.close();
            
            string message = "You received a file from " + sender->getUsername() + ": " + filename;
            messageInbox[recipient].push_back(message);
            
            cout << "File sent successfully to " << recipient << endl;
            cout << "File saved in: " << fullPath << endl;
        }
    }

    void exitProgram() {
        cout << "Exiting...\n";
        exit(0);
    }
};

// ---------- Control Admin ----------
class ControlAdmin : public User {
public:
    void password() override {
        string id, password;
        cout << "\n----------CONTROL ADMIN AUTHENTICATION----------\n";
        cout << "User ID: ";
        cin >> id;
        cout << "Password: ";
        cin >> password;
        
        username = id;

        if (id == "Admin" && password == "admin123") {
            cout << "Access granted for Control Admin.\n";
        } else {
            throw runtime_error("Invalid credentials.");
        }
    }

    void greet() override {
        cout << "\nWelcome, Control Admin!\n";
    }

    void fileOperations() override {
        while (true) {
            cout << "\nControl Admin Menu:\n";
            cout << "1. Create Professor Account\n";
            cout << "2. Create Student Account\n";
            cout << "3. View All Users\n";
            cout << "4. File Operations\n";
            cout << "5. Log out\n";
            cout << "6. Exit\n";
            
            int choice;
            cout << "Your choice: ";
            cin >> choice;
            cin.ignore();
            
            try {
                switch (choice) {
                    case 1: createProfessor(); break;
                    case 2: createStudent(); break;
                    case 3: viewAllUsers(); break;
                    case 4: {
                        FileManager manager;
                        manager.run(this);
                        break;  // Will return here when choosing "Return to Main Menu"
                    }
                    case 5: throw runtime_error("Returning to login");
                    case 6: cout << "Exiting...\n"; exit(0);
                    default: cout << "Invalid choice.\n";
                }
            } catch (const exception& e) {
                if (string(e.what()) == "Returning to login") {
                    throw;
                }
                cout << "Error: " << e.what() << "\n";
            }
        }
    }
private:

    void createProfessor() {
        string username, password;
        cout << "Enter professor username: ";
        getline(cin, username);
        cout << "Enter password: ";
        getline(cin, password);
        
        userCredentials[username] = {password, "Professor"};
        cout << "Professor account created successfully.\n";
    }
    void createStudent() {
        string username, password;
        cout << "Enter student username: ";
        getline(cin, username);
        cout << "Enter password: ";
        getline(cin, password);
        
        userCredentials[username] = {password, "Student"};
        cout << "Student account created successfully.\n";
    }
    
    void viewAllUsers() {
        cout << "\n--- Registered Users ---\n";
        for (const auto& user : userCredentials) {
            cout << "Username: " << user.first << " [" << user.second.second << "]\n";
        }
    }

};

// ---------- Admin ----------
class Admin : public User {
public:
    void password() override {
        string id, password;
        cout << "\n----------AUTHENTICATION PAGE----------\n";
        cout << "User ID: ";
        cin >> id;
        cout << "Password: ";
        cin >> password;
        
        username = id;

        if ((id.substr(0, 2) == "AA" || id.substr(0, 2) == "PR" || 
            id.substr(0, 2) == "AC" || id.substr(0, 2) == "HR" || 
            id.substr(0, 2) == "SA") && password == "admin123") {
            departmentPrefix = id.substr(0, 2);
            cout << "Access granted for " << departmentName(departmentPrefix) << " Department.\n";
        } else {
            throw runtime_error("Invalid credentials.");
        }
    }

    void greet() override {
        cout << "\nWelcome, Admin of " << departmentName(departmentPrefix) << " Department!\n";
    }

    void fileOperations() override {
        FileManager manager;
        manager.run(this);
    }

private:
    string departmentPrefix;

    string departmentName(const string& prefix) {
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
    void password() override {
        string id, password;
        cout << "\n----------PROFESSOR AUTHENTICATION----------\n";
        cout << "User ID: ";
        cin >> id;
        cout << "Password: ";
        cin >> password;
        
        username = id;

        if (!userCredentials.count(id) || userCredentials[id].first != password || userCredentials[id].second != "Professor") {
            throw runtime_error("Invalid credentials.");
        }
        cout << "Access granted.\n";
    }
    void greet() override {
        cout << "\nWelcome, Professor " << username << "!\n";
        cout << "Your sections:\n";
        for (int i = 1; i <= 5; ++i) {
            cout << "OOP2 -- Section[00" << i << "]\n";
        }
        
        // Just show count, not full messages
        if (messageInbox.count(username) && !messageInbox[username].empty()) {
            cout << "\nYou have " << messageInbox[username].size() 
                 << " unread message(s)\n";
        }
    }

    void fileOperations() override {
        while (true) {
            cout << "\nProfessor Menu:\n";
            cout << "1. File Operations\n";
            cout << "2. Send File to Student\n";
            cout << "3. View Messages\n";
            cout << "4. Log out\n";
            cout << "5. Exit\n";
            
            int choice;
            cout << "Your choice: ";
            cin >> choice;
            cin.ignore();
            
            try {
                switch (choice) {
                    case 1: {
                        FileManager manager;
                        manager.run(this);
                        break;  // Returns here when choosing "Return to Main Menu"
                    }
                    case 2: sendFileToStudent(); break;
                    case 3: checkMessages(); break;
                    case 4: throw runtime_error("Returning to login");
                    case 5: cout << "Exiting...\n"; exit(0);
                    default: cout << "Invalid choice.\n";
                }
            } catch (const exception& e) {
                if (string(e.what()) == "Returning to login") {
                    throw;
                }
                cout << "Error: " << e.what() << "\n";
            }
        }
    } 
private:
void sendFileToStudent() {
    string studentId, filename;
    cout << "Enter student ID (or 'all' to send to all students): ";
    getline(cin, studentId);
    
    cout << "Enter filename to send: ";
    getline(cin, filename);
    
    ifstream sourceFile(filename);
    if (!sourceFile) {
        throw runtime_error("Could not open source file.");
    }

    // Convert input to lowercase for case-insensitive comparison
    string lowercaseInput = studentId;
    transform(lowercaseInput.begin(), lowercaseInput.end(), lowercaseInput.begin(), ::tolower);

    if (lowercaseInput == "all") {
        // Send to all students
        int sentCount = 0;
        for (const auto& user : userCredentials) {
            if (user.second.second == "Student") {  // Check if user is a student
                string studentFolder = user.first;
                
                #ifdef _WIN32
                    if (_mkdir(studentFolder.c_str()) != 0 && errno != EEXIST) {
                        cerr << "Warning: Could not create directory for " << user.first << endl;
                        continue;
                    }
                #else
                    if (mkdir(studentFolder.c_str(), 0755) != 0 && errno != EEXIST) {
                        cerr << "Warning: Could not create directory for " << user.first << endl;
                        continue;
                    }
                #endif
                
                string newFilename = "_" + filename;
                string fullPath = studentFolder + "/" + newFilename;
                
                ofstream destFile(fullPath);
                if (destFile) {
                    sourceFile.clear();
                    sourceFile.seekg(0);
                    destFile << sourceFile.rdbuf();
                    destFile.close();
                    
                    string message = "You received a file from professor " + username + ": " + filename;
                    messageInbox[user.first].push_back(message);
                    sentCount++;
                } else {
                    cerr << "Warning: Could not send file to " << user.first << endl;
                }
            }
        }
        sourceFile.close();
        cout << "File sent successfully to " << sentCount << " students.\n";
    } else {
        // Original single-student sending logic
        if (!userCredentials.count(studentId)) {
            throw runtime_error("Student not found.");
        }
        
        if (userCredentials[studentId].second != "Student") {
            throw runtime_error("The specified user is not a student.");
        }

        string studentFolder = studentId + "_received_files";
        
        #ifdef _WIN32
            if (_mkdir(studentFolder.c_str()) != 0 && errno != EEXIST) {
                throw runtime_error("Could not create student directory.");
            }
        #else
            if (mkdir(studentFolder.c_str(), 0755) != 0 && errno != EEXIST) {
                throw runtime_error("Could not create student directory.");
            }
        #endif
        
        string newFilename = studentId + "_from_" + username + "_" + filename;
        string fullPath = studentFolder + "/" + newFilename;
        
        ofstream destFile(fullPath);
        if (!destFile) {
            throw runtime_error("Could not create file for student.");
        }
        
        destFile << sourceFile.rdbuf();
        sourceFile.close();
        destFile.close();
        
        string message = "You received a file from professor " + username + ": " + filename;
        messageInbox[studentId].push_back(message);
        
        cout << "File sent successfully to student " << studentId << endl;
        cout << "File saved in: " << fullPath << endl;
    }
}    
void checkMessages() {
    // Check if user has any messages
    if (!messageInbox.count(username) || messageInbox[username].empty()) {
        cout << "\n--- No new messages ---\n";
        return;
    }

    cout << "\n--- Your Messages (" << messageInbox[username].size() << ") ---\n";
    
    // Display messages with numbering and better formatting
    int messageNumber = 1;
    for (const auto& msg : messageInbox[username]) {
        cout << messageNumber++ << ". " << msg << "\n";
    }

    // Optional: Add message management menu
    cout << "\nMessage Options:\n";
    cout << "1. Keep messages\n";
    cout << "2. Delete all messages\n";
    cout << "3. Return to menu\n";
    cout << "Your choice: ";

    int choice;
    cin >> choice;
    cin.ignore();  // Clear input buffer

    switch (choice) {
        case 1:
            cout << "Messages kept in inbox.\n";
            break;
        case 2:
            messageInbox[username].clear();
            cout << "All messages deleted.\n";
            break;
        case 3:
            // Just return without changing messages
            break;
        default:
            cout << "Invalid choice. Messages remain unchanged.\n";
    }
}
};

// ---------- Student ----------
class Student : public User {
public:
    void password() override {
        string id, password;
        cout << "\n----------STUDENT AUTHENTICATION----------\n";
        cout << "User ID: ";
        cin >> id;
        cout << "Password: ";
        cin >> password;
        
        username = id;

        if (!userCredentials.count(id) || userCredentials[id].first != password || userCredentials[id].second != "Student") {
            throw runtime_error("Invalid credentials.");
        }
        cout << "Access granted.\n";
    }

    void greet() override {
        cout << "\nWelcome, Student " << username << "!\n";
        cout << "Welcome to IUT's File Management System\n";
        
        checkMessages();
    }

    void fileOperations() override {
        while (true) {
            cout << "\nStudent Menu:\n";
            cout << "1. Department Files\n";
            cout << "2. File Operations\n";
            cout << "3. Send File to Another Student\n";
            cout << "4. View Messages\n";
            cout << "5. Log out\n";
            cout << "6. Exit\n";
            
            int choice;
            cout << "Your choice: ";
            cin >> choice;
            cin.ignore();
            
            try {
                switch (choice) {
                    case 1: showDepartments(); break;
                    case 2: {
                        FileManager manager;
                        manager.run(this);
                        break;  // Returns here when choosing "Return to Main Menu"
                    }
                    case 3: sendFileToStudent(); break;
                    case 4: checkMessages(); break;
                    case 5: throw runtime_error("Returning to login");
                    case 6: cout << "Exiting...\n"; exit(0);
                    default: cout << "Invalid choice.\n";
                }
            } catch (const exception& e) {
                if (string(e.what()) == "Returning to login") {
                    throw;
                }
                cout << "Error: " << e.what() << "\n";
            }
        }
    }
    
private:
    void showDepartments() {
        while (true) {
            cout << "\nDepartments:\n";
            cout << "1. Academic Affairs\n";
            cout << "2. Student Union\n";
            cout << "3. PR Department\n";
            cout << "4. Accounting\n";
            cout << "5. Return to Menu\n";
            
            int choice;
            cout << "Your choice: ";
            cin >> choice;
            cin.ignore();
            
            try {
                switch (choice) {
                    case 1: readDepartment("Academic_Affairs.txt"); break;
                    case 2: readDepartment("Student_Union.txt"); break;
                    case 3: readDepartment("PR_department.txt"); break;
                    case 4: readDepartment("Accounting.txt"); break;
                    case 5: return;
                    default: cout << "Invalid option.\n";
                }
            } catch (const exception& e) {
                cout << "Error: " << e.what() << "\n";
            }
        }
    }

    void readDepartment(const string& filename) {
        try {
            ifstream file(filename);
            if (!file.is_open()) {
                throw runtime_error("File not found or cannot be read.");
            }

            string line;
            cout << "\n--- Department Info ---\n";
            while (getline(file, line)) {
                cout << line << '\n';
            }
        } catch (const ifstream::failure&) {
            throw runtime_error("Error reading file. Make sure the file exists and is readable.");
        }
    }
    
    void sendFileToStudent() {
        string studentId, filename;
        cout << "Enter student ID: ";
        getline(cin, studentId);
        
        if (!userCredentials.count(studentId)) {
            throw runtime_error("Student not found.");
        }
        
        cout << "Enter filename to send: ";
        getline(cin, filename);
        
        ifstream sourceFile(filename);
        if (!sourceFile) {
            throw runtime_error("Could not open source file.");
        }
        
        string studentFolder = studentId;
        
        #ifdef _WIN32
            if (_mkdir(studentFolder.c_str()) != 0 && errno != EEXIST) {
                throw runtime_error("Could not create student directory.");
            }
        #else
            if (mkdir(studentFolder.c_str(), 0755) != 0 && errno != EEXIST) {
                throw runtime_error("Could not create student directory.");
            }
        #endif
        
        string newFilename = "_" + filename;
        string fullPath = studentFolder + "/" + newFilename;
        
        ofstream destFile(fullPath);
        if (!destFile) {
            throw runtime_error("Could not create file for student.");
        }
        
        destFile << sourceFile.rdbuf();
        sourceFile.close();
        destFile.close();
        
        string message = "You received a file from student " + username + ": " + filename;
        messageInbox[studentId].push_back(message);
        
        cout << "File sent successfully to student " << studentId << endl;
        cout << "File saved in: " << fullPath << endl;
    }
    
    void checkMessages() {
        // Check if user has any messages
        if (!messageInbox.count(username) || messageInbox[username].empty()) {
            cout << "\n--- No new messages ---\n";
            return;
        }
    
        cout << "\n--- Your Messages (" << messageInbox[username].size() << ") ---\n";
        
        // Display messages with numbering and better formatting
        int messageNumber = 1;
        for (const auto& msg : messageInbox[username]) {
            cout << messageNumber++ << ". " << msg << "\n";
        }
    
        // Optional: Add message management menu
        cout << "\nMessage Options:\n";
        cout << "1. Keep messages\n";
        cout << "2. Delete all messages\n";
        cout << "3. Return to menu\n";
        cout << "Your choice: ";
    
        int choice;
        cin >> choice;
        cin.ignore();  // Clear input buffer
    
        switch (choice) {
            case 1:
                cout << "Messages kept in inbox.\n";
                break;
            case 2:
                messageInbox[username].clear();
                cout << "All messages deleted.\n";
                break;
            case 3:
                // Just return without changing messages
                break;
            default:
                cout << "Invalid choice. Messages remain unchanged.\n";
        }
    }
};

// ---------- Main Application ----------
class FMSApp {
public:
void run() {
    
    userCredentials["Sarvar"] = {"sarvar123", "Professor"};
    userCredentials["Oybek"] = {"oybek123", "Professor"};
    userCredentials["Laziz"] = {"laziz123", "Professor"};
    userCredentials["Laylo"] = {"laylo123", "Professor"};
    userCredentials["Ahmad"] = {"ahmad123", "Student"};
    userCredentials["Umid"] = {"umid123", "Student"};
    userCredentials["Aziz"] = {"aziz123", "Student"};
    userCredentials["Lola"] = {"lola123", "Student"};   
    userCredentials["Sanjar"] = {"sanjar123", "Student"};
    userCredentials["Munisa"] = {"munisa123", "Student"};
    userCredentials["Mubina"] = {"mubina123", "Student"};
    userCredentials["Muhammad"] = {"muhammad123", "Student"};
    userCredentials["Usman"] = {"usman123", "Student"};
    userCredentials["Omar"] = {"omar123", "Student"};
    userCredentials["Kamron"] = {"kamron123", "Student"};




    
   

        while (true) {
            try {
                displayLogin();
                int role = getChoice();
                User* user = nullptr;

                switch (role) {
                    case 1: user = new ControlAdmin(); break;
                    case 2: user = new Admin(); break;
                    case 3: user = new Professor(); break;
                    case 4: user = new Student(); break;
                    default: throw invalid_argument("Invalid role selection.");
                }

                try {
                    user->password();
                    user->greet();
                    user->fileOperations();
                } catch (const runtime_error& e) {
                    if (string(e.what()) == "Returning to login") {
                        cout << "Logging out...\n";
                        delete user;
                        continue;
                    }
                    throw;
                }

                delete user;
            } catch (const exception& e) {
                cout << "Error: " << e.what() << "\nTry again.\n";
            }
        }
    }

private:
    void displayLogin() {
        cout << "\n----------LOGIN PAGE----------\n";
        cout << "1. Control Admin\n";
        cout << "2. Department Admin\n";
        cout << "3. Professor\n";
        cout << "4. Student\n";
    }

    int getChoice() {
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