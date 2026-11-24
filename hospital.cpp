// Hospital Appointment and Patient Management System (DSA OEL 3)
// Data structures: Linked List (patients), Array (appointment records), Queue (appointment order)
// Algorithms: Linear Search, Bubble Sort
// File handling: patients.txt and appointments.txt (saved after every change, loaded on start)
// The program automatically finds a folder where it is allowed to write its data files.

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <cctype>
using namespace std;

// ================= DATA STRUCTURES =================

// ---- 1. Linked list node: one patient ----
struct Patient {
    int id, age;
    string name, phone, disease;
    Patient* next;
};
Patient* head = NULL;            // start of the linked list

// ---- 2. Array: stores all appointments with their status ----
struct Appointment {
    int id, patientId;
    string date, doctor, status; // status = Scheduled / Completed / Cancelled
};
const int MAX = 100;             // maximum appointments
Appointment appts[MAX];
int apptCount = 0;

// ---- 3. Queue (circular array): keeps appointment order of arrival (FIFO) ----
class Queue {
    int arr[MAX];                // stores index of appointment in appts[]
    int front, rear, count;
public:
    Queue() { front = 0; rear = -1; count = 0; }
    bool isEmpty() { return count == 0; }
    void enqueue(int x) {        // add at rear
        rear = (rear + 1) % MAX;
        arr[rear] = x;
        count++;
    }
    int dequeue() {              // remove from front
        int x = arr[front];
        front = (front + 1) % MAX;
        count--;
        return x;
    }
};
Queue q;

// ================= FILE HANDLING =================

string dataDir = "";             // folder where the data files live (empty = current folder)
bool dataFolderOk = false;       // true if a writable folder was found

string patientsPath()     { return dataDir + "patients.txt"; }
string appointmentsPath() { return dataDir + "appointments.txt"; }

// Can we create/write both data files inside this folder? (append mode never erases existing data)
bool canWriteIn(string dir) {
    string a = dir + "patients.txt";
    string b = dir + "appointments.txt";
    ofstream f(a.c_str(), ios::app);
    if (f.fail()) return false;
    f.close();
    ofstream g(b.c_str(), ios::app);
    if (g.fail()) return false;
    g.close();
    return true;
}

// Try several folders and use the first one that works.
void chooseDataFolder(const char* exePath) {
    string candidates[6];
    int n = 0;
    candidates[n++] = "";                                    // current working folder

    string exe = exePath ? exePath : "";                     // folder of the .exe
    size_t pos = exe.find_last_of("\\/");
    if (pos != string::npos) candidates[n++] = exe.substr(0, pos + 1);

    const char* user = getenv("USERPROFILE");
    if (user) {
        candidates[n++] = string(user) + "\\Documents\\";
        candidates[n++] = string(user) + "\\";
    }
    const char* tmp = getenv("TEMP");
    if (tmp) candidates[n++] = string(tmp) + "\\";

    for (int i = 0; i < n; i++) {
        if (canWriteIn(candidates[i])) {
            dataDir = candidates[i];
            dataFolderOk = true;
            return;
        }
    }
    cout << "ERROR: could not create data files in any folder (" << strerror(errno) << ").\n";
    cout << "Data will NOT be saved. Try running the program as Administrator.\n";
}

void saveData() {
    if (!dataFolderOk) return;                               // already warned at start
    ofstream f(patientsPath().c_str());
    if (f.fail()) cout << "Warning: could not save " << patientsPath() << " (" << strerror(errno) << ")\n";
    else {
        for (Patient* p = head; p != NULL; p = p->next)
            f << p->id << "\n" << p->name << "\n" << p->age << "\n" << p->phone << "\n" << p->disease << "\n";
    }
    ofstream g(appointmentsPath().c_str());
    if (g.fail()) cout << "Warning: could not save " << appointmentsPath() << " (" << strerror(errno) << ")\n";
    else {
        for (int i = 0; i < apptCount; i++)
            g << appts[i].id << "\n" << appts[i].patientId << "\n" << appts[i].date << "\n"
              << appts[i].doctor << "\n" << appts[i].status << "\n";
    }
}

void appendPatient(Patient* p) {        // add node at end of linked list
    p->next = NULL;
    if (head == NULL) { head = p; return; }
    Patient* t = head;
    while (t->next != NULL) t = t->next;
    t->next = p;
}

void stripCR(string& s) {               // remove stray '\r' (Windows line endings)
    while (!s.empty() && (s[s.length() - 1] == '\r' || s[s.length() - 1] == '\n'))
        s.erase(s.length() - 1);
}

void loadData() {
    string a, b, c, d, e;
    ifstream f(patientsPath().c_str());  // if file does not exist, nothing is loaded
    while (f && getline(f, a) && getline(f, b) && getline(f, c) && getline(f, d) && getline(f, e)) {
        stripCR(a); stripCR(b); stripCR(c); stripCR(d); stripCR(e);
        Patient* p = new Patient;
        p->id = atoi(a.c_str()); p->name = b; p->age = atoi(c.c_str());
        p->phone = d; p->disease = e;
        appendPatient(p);
    }
    ifstream g(appointmentsPath().c_str());
    while (g && apptCount < MAX && getline(g, a) && getline(g, b) && getline(g, c) && getline(g, d) && getline(g, e)) {
        stripCR(a); stripCR(b); stripCR(c); stripCR(d); stripCR(e);
        appts[apptCount].id = atoi(a.c_str());
        appts[apptCount].patientId = atoi(b.c_str());
        appts[apptCount].date = c;
        appts[apptCount].doctor = d;
        appts[apptCount].status = e;
        if (e == "Scheduled") q.enqueue(apptCount);   // rebuild the queue
        apptCount++;
    }
}

// ================= INPUT VALIDATION =================

bool isBlank(const string& s) {
    for (int i = 0; i < (int)s.length(); i++)
        if (s[i] != ' ' && s[i] != '\t' && s[i] != '\r') return false;
    return true;
}

string readLine(string msg) {           // text that cannot be empty
    string s;
    while (true) {
        cout << msg;
        if (!getline(cin, s)) exit(0);
        if (!isBlank(s)) { stripCR(s); return s; }
        cout << "Field cannot be empty!\n";
    }
}

bool isDigits(string s) {
    if (s.length() == 0) return false;
    for (int i = 0; i < (int)s.length(); i++)
        if (s[i] < '0' || s[i] > '9') return false;
    return true;
}

int readInt(string msg, int lo, int hi) {   // whole number between lo and hi (no negatives)
    string s;
    while (true) {
        cout << msg;
        if (!getline(cin, s)) exit(0);
        stripCR(s);
        if (isBlank(s)) continue;               // just pressed Enter: ask again quietly
        if (isDigits(s) && s.length() <= 9) {
            int x = atoi(s.c_str());
            if (x >= lo && x <= hi) return x;
        }
        cout << "Invalid! Enter a whole number from " << lo << " to " << hi << ".\n";
    }
}

string toLower(string s) {
    for (int i = 0; i < (int)s.length(); i++) s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}

Patient* findById(int id) {                 // LINEAR SEARCH by ID
    for (Patient* p = head; p != NULL; p = p->next)
        if (p->id == id) return p;
    return NULL;
}

string readName() {                         // letters and spaces only
    while (true) {
        string s = readLine("Name: ");
        bool ok = true;
        for (int i = 0; i < (int)s.length(); i++)
            if (!isalpha((unsigned char)s[i]) && s[i] != ' ') ok = false;
        if (ok) return s;
        cout << "Name must contain letters only.\n";
    }
}

string readPhone(int skipId) {              // exactly 11 digits and not already used
    while (true) {
        string s = readLine("Phone (11 digits): ");
        if (!isDigits(s) || s.length() != 11) { cout << "Phone must be exactly 11 digits.\n"; continue; }
        bool used = false;
        for (Patient* p = head; p != NULL; p = p->next)
            if (p->phone == s && p->id != skipId) used = true;
        if (used) { cout << "Phone number already registered.\n"; continue; }
        return s;
    }
}

string readDate() {                         // DD-MM-YYYY
    int days[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    while (true) {
        string s = readLine("Date (DD-MM-YYYY): ");
        if (s.length() == 10 && s[2] == '-' && s[5] == '-' &&
            isDigits(s.substr(0, 2)) && isDigits(s.substr(3, 2)) && isDigits(s.substr(6, 4))) {
            int d = atoi(s.substr(0, 2).c_str());
            int m = atoi(s.substr(3, 2).c_str());
            int y = atoi(s.substr(6, 4).c_str());
            if (m >= 1 && m <= 12 && d >= 1 && d <= days[m - 1] && y >= 2024 && y <= 2100) return s;
        }
        cout << "Invalid date!\n";
    }
}

// ================= PATIENT MODULE =================

void showPatient(Patient* p) {
    cout << "ID: " << p->id << " | Name: " << p->name << " | Age: " << p->age
         << " | Phone: " << p->phone << " | Disease: " << p->disease << "\n";
}

void addPatient() {
    int id = readInt("Patient ID (1-999999): ", 1, 999999);
    if (findById(id) != NULL) { cout << "Duplicate ID! Patient already exists.\n"; return; }
    Patient* p = new Patient;
    p->id = id;
    p->name = readName();
    p->age = readInt("Age (1-120): ", 1, 120);
    p->phone = readPhone(-1);
    p->disease = readLine("Disease/Problem: ");
    appendPatient(p);
    saveData();
    cout << "Patient registered successfully.\n";
}

void updatePatient() {
    Patient* p = findById(readInt("Enter ID to update: ", 1, 999999));
    if (p == NULL) { cout << "Patient not found.\n"; return; }
    p->name = readName();
    p->age = readInt("Age (1-120): ", 1, 120);
    p->phone = readPhone(p->id);
    p->disease = readLine("Disease/Problem: ");
    saveData();
    cout << "Record updated.\n";
}

void deletePatient() {
    int id = readInt("Enter ID to delete: ", 1, 999999);
    Patient* p = findById(id);
    if (p == NULL) { cout << "Patient not found.\n"; return; }
    if (p == head) head = p->next;                      // delete first node
    else {
        Patient* prev = head;
        while (prev->next != p) prev = prev->next;
        prev->next = p->next;                           // skip the node
    }
    delete p;
    for (int i = 0; i < apptCount; i++)                 // cancel his scheduled appointments
        if (appts[i].patientId == id && appts[i].status == "Scheduled") appts[i].status = "Cancelled";
    saveData();
    cout << "Patient deleted.\n";
}

void searchPatient() {
    int ch = readInt("Search by 1) ID  2) Name: ", 1, 2);
    if (ch == 1) {
        Patient* p = findById(readInt("Enter ID: ", 1, 999999));
        if (p == NULL) cout << "Patient not found.\n"; else showPatient(p);
    } else {
        string key = toLower(readLine("Enter name: "));
        bool found = false;
        for (Patient* p = head; p != NULL; p = p->next)  // LINEAR SEARCH by name
            if (toLower(p->name).find(key) != string::npos) { showPatient(p); found = true; }
        if (!found) cout << "Patient not found.\n";
    }
}

void swapData(Patient* a, Patient* b) {
    Patient t = *a;
    a->id = b->id; a->age = b->age; a->name = b->name; a->phone = b->phone; a->disease = b->disease;
    b->id = t.id;  b->age = t.age;  b->name = t.name;  b->phone = t.phone;  b->disease = t.disease;
}

void sortAndShow() {                                    // BUBBLE SORT on linked list
    if (head == NULL) { cout << "No patients registered.\n"; return; }
    int by = readInt("Sort by 1) ID  2) Name  3) Age: ", 1, 3);
    bool swapped;
    do {
        swapped = false;
        for (Patient* p = head; p->next != NULL; p = p->next) {
            Patient* n = p->next;
            bool wrong;
            if (by == 1) wrong = p->id > n->id;
            else if (by == 2) wrong = toLower(p->name) > toLower(n->name);
            else wrong = p->age > n->age;
            if (wrong) { swapData(p, n); swapped = true; }
        }
    } while (swapped);
    saveData();                                         // keep file in the new sorted order
    for (Patient* p = head; p != NULL; p = p->next) showPatient(p);
}

// ================= APPOINTMENT MODULE =================

void bookAppointment() {
    if (apptCount == MAX) { cout << "Appointment list is full.\n"; return; }
    int pid = readInt("Patient ID: ", 1, 999999);
    if (findById(pid) == NULL) { cout << "Patient not registered. Register first.\n"; return; }
    string date = readDate();
    string doctor = readLine("Doctor name: ");
    for (int i = 0; i < apptCount; i++)
        if (appts[i].patientId == pid && appts[i].date == date && appts[i].status == "Scheduled") {
            cout << "Patient already has an appointment on this date.\n";
            return;
        }
    appts[apptCount].id = apptCount + 1;
    appts[apptCount].patientId = pid;
    appts[apptCount].date = date;
    appts[apptCount].doctor = doctor;
    appts[apptCount].status = "Scheduled";
    q.enqueue(apptCount);                               // join the queue
    apptCount++;
    saveData();
    cout << "Appointment booked. Appointment ID = " << apptCount << "\n";
}

void processNext() {                                    // serve the patient at the front
    while (!q.isEmpty()) {
        int i = q.dequeue();
        if (appts[i].status == "Scheduled") {           // skip cancelled ones
            appts[i].status = "Completed";
            cout << "Appointment " << appts[i].id << " (Patient ID " << appts[i].patientId << ") completed.\n";
            saveData();
            return;
        }
    }
    cout << "No appointments in the queue.\n";
}

void cancelAppointment() {
    int id = readInt("Enter Appointment ID to cancel: ", 1, 999999);
    if (id > apptCount || appts[id - 1].status != "Scheduled") {
        cout << "Appointment not available (not found, completed or already cancelled).\n";
        return;
    }
    appts[id - 1].status = "Cancelled";
    saveData();
    cout << "Appointment cancelled.\n";
}

void viewAppointments() {
    if (apptCount == 0) { cout << "No appointments.\n"; return; }
    for (int i = 0; i < apptCount; i++)
        cout << "Appointment " << appts[i].id << " | Patient ID " << appts[i].patientId << " | "
             << appts[i].date << " | Doctor: " << appts[i].doctor << " | " << appts[i].status << "\n";
}

// ================= MAIN =================

int main(int argc, char* argv[]) {
    chooseDataFolder(argc > 0 ? argv[0] : NULL);
    if (dataFolderOk)
        cout << "Data folder: " << (dataDir.empty() ? string("(current folder)") : dataDir) << "\n";
    loadData();
    int choice;
    do {
        cout << "\n===== HOSPITAL MANAGEMENT SYSTEM =====\n";
        cout << "1. Register patient\n2. Update patient\n3. Delete patient\n4. Search patient\n";
        cout << "5. Sort and show all patients\n6. Book appointment\n7. Process next appointment\n";
        cout << "8. Cancel appointment\n9. View all appointments\n0. Exit\n";
        choice = readInt("Enter choice: ", 0, 9);
        switch (choice) {
            case 1: addPatient(); break;
            case 2: updatePatient(); break;
            case 3: deletePatient(); break;
            case 4: searchPatient(); break;
            case 5: sortAndShow(); break;
            case 6: bookAppointment(); break;
            case 7: processNext(); break;
            case 8: cancelAppointment(); break;
            case 9: viewAppointments(); break;
        }
    } while (choice != 0);
    saveData();
    cout << "Data saved. Goodbye!\n";
    return 0;
}
