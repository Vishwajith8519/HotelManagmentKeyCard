#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>

using namespace std;

string getCurrentTime() {
    time_t now = time(0);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y", localtime(&now));
    return string(buffer);
}

string hashKey(string key) {
    unsigned int hash = 0;
    for (char c : key) {
        hash = (hash * 31) + c;
    }
    return to_string(hash);
}

class Keycard {
public:
    string keyID;
    string guestName;
    bool isValid;
    string issueTime;

    Keycard(string name) {
        guestName = name;
        keyID = generateKeyID();
        isValid = true;
        issueTime = getCurrentTime();
    }

    string generateKeyID() {
        srand(time(0) + rand());
        string key = "";
        for (int i = 0; i < 6; i++) {
            key += to_string(rand() % 10);
        }
        return hashKey(key);
    }

    void displayKey() {
        cout << "Keycard for " << guestName << ": " << keyID << endl;
    }
};

void saveKeycardToFile(const Keycard& keycard) {
    ofstream file("keycards.txt", ios::app);
    if (!file) {
        cout << "Error: Unable to save keycard data!" << endl;
        return;
    }
    file << keycard.guestName << " " << keycard.keyID << " " << keycard.isValid << " " << keycard.issueTime << endl;
    file.close();
    cout << "Keycard saved successfully!" << endl;
}

bool validateKeycard(string enteredKey) {
    ifstream file("keycards.txt");
    if (!file) {
        cout << "Error: Unable to open keycard database!" << endl;
        return false;
    }

    string name, key, timeIssued;
    bool valid;
    while (file >> name >> key >> valid) {
        getline(file, timeIssued);
        if (key == enteredKey && valid) {
            cout << "Access Granted! Welcome, " << name << "!" << endl;
            return true;
        }
    }
    cout << "Access Denied! Invalid or expired key." << endl;
    return false;
}

void invalidateKeycard(string enteredKey) {
    ifstream file("keycards.txt");
    ofstream temp("temp.txt");
    if (!file || !temp) {
        cout << "Error: Unable to access keycard database!" << endl;
        return;
    }

    string name, key, timeIssued;
    bool valid;
    bool found = false;
    while (file >> name >> key >> valid) {
        getline(file, timeIssued);
        if (key == enteredKey) {
            found = true;
            temp << name << " " << key << " " << 0 << " " << timeIssued << endl;
        } else {
            temp << name << " " << key << " " << valid << " " << timeIssued << endl;
        }
    }
    file.close();
    temp.close();
    remove("keycards.txt");
    rename("temp.txt", "keycards.txt");

    if (found)
        cout << "Keycard invalidated successfully!" << endl;
    else
        cout << "Key not found!" << endl;
}

void adminOverride() {
    string enteredKey, newGuest;
    cout << "Enter Key ID to Reset: ";
    cin >> enteredKey;
    cout << "Enter New Guest Name: ";
    cin >> newGuest;

    ifstream file("keycards.txt");
    ofstream temp("temp.txt");
    if (!file || !temp) {
        cout << "Error: Unable to access keycard database!" << endl;
        return;
    }

    string name, key, timeIssued;
    bool valid;
    bool found = false;
    while (file >> name >> key >> valid) {
        getline(file, timeIssued);
        if (key == enteredKey) {
            found = true;
            Keycard newKey(newGuest);
            temp << newGuest << " " << newKey.keyID << " " << 1 << " " << getCurrentTime() << endl;
            cout << "Keycard reset successfully for " << newGuest << "!" << endl;
        } else {
            temp << name << " " << key << " " << valid << " " << timeIssued << endl;
        }
    }
    file.close();
    temp.close();
    remove("keycards.txt");
    rename("temp.txt", "keycards.txt");

    if (!found)
        cout << "Key not found!" << endl;
}

void expireOldKeycards() {
    ifstream file("keycards.txt");
    ofstream temp("temp.txt");
    if (!file || !temp) {
        cout << "Error: Unable to access keycard database!" << endl;
        return;
    }
    string name, key, timeIssued;
    bool valid;
    bool updated = false;
    time_t now = time(0);

    while (file >> name >> key >> valid) {
        getline(file, timeIssued);
        tm tmIssueTime = {};
        istringstream ss(timeIssued);

        ss >> get_time(&tmIssueTime, "%a %b %d %H:%M:%S %Y");
        if (ss.fail()) {
            cout << "Error: Unable to parse time for key " << key << endl;
            continue;
        }

        time_t issueTime = mktime(&tmIssueTime);

        if (valid && difftime(now, issueTime) > 86400) {
            valid = false;
            updated = true;
        }
        temp << name << " " << key << " " << valid << " " << timeIssued << endl;
    }
    file.close();
    temp.close();
    remove("keycards.txt");
    rename("temp.txt", "keycards.txt");

    if (updated)
        cout << "Expired old keycards automatically!" << endl;
}

int main() {
    int choice;
    string guestName, enteredKey;
    while (true) {
        cout << "\nHotel Keycard System\n";
        cout << "1. Generate Keycard\n";
        cout << "2. Validate Keycard\n";
        cout << "3. Invalidate Keycard\n";
        cout << "4. Admin Override\n";
        cout << "5. Auto Expire Old Keycards\n";
        cout << "6. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "Enter Guest Name: ";
                cin >> guestName;
                {
                    Keycard newKey(guestName);
                    newKey.displayKey();
                    saveKeycardToFile(newKey);
                }
                break;
            case 2:
                cout << "Enter Key ID: ";
                cin >> enteredKey;
                validateKeycard(enteredKey);
                break;
            case 3:
                cout << "Enter Key ID to Invalidate: ";
                cin >> enteredKey;
                invalidateKeycard(enteredKey);
                break;
            case 4:
                adminOverride();
                break;
            case 5:
                expireOldKeycards();
                break;
            case 6:
                return 0;
            default:
                cout << "Invalid choice! Try again." << endl;
        }
    }
}