#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <iomanip>
#include <limits>
#include <vector>
using namespace std;

// ================= STRUCTURES =================
struct Book {
    long long isbn;
    string title;
    string author;
    string category;
    int quantity;
};

struct IssuedBook {
    long long isbn;
    string title;
    string author;
    string category;
    string issueDate;
    string issuedby;
};
struct MisplacedBook {
    long long isbn;
    string title;
    string author;
    string category;
    string reason;
    string reportDate;
    string misplacedby;
};
struct user {
    int id;
    long long cnic;
    string name;
    string contact;
    string status;
};

// ================= HELPERS =================
bool validateISBN(long long isbn) {
    return (isbn >= 1000000000LL && isbn <= 9999999999999LL);
}

bool validateDateFormat(const string& d) {
    if (d.size() != 10) return false;
    if (d[2] != '-' || d[5] != '-') return false;
    for (size_t i = 0; i < d.size(); ++i) {
        if (i == 2 || i == 5) continue;
        if (!isdigit(static_cast<unsigned char>(d[i]))) return false;
    }
    // basic ranges
    int dd = stoi(d.substr(0, 2));
    int mm = stoi(d.substr(3, 2));
    int yyyy = stoi(d.substr(6, 4));
    if (mm < 1 || mm > 12) return false;
    if (dd < 1 || dd > 31) return false;
    // simple month-day validation
    static int mdays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    bool leap = ((yyyy % 4 == 0 && yyyy % 100 != 0) || (yyyy % 400 == 0));
    int maxd = mdays[mm - 1];
    if (mm == 2 && leap) maxd = 29;
    if (dd > maxd) return false;
    return true;
}

static bool parseUserLine(const string& line, user& out) {
    istringstream ss(line);
    string token;

    if (!getline(ss, token, '|')) return false;
    try { out.id = stoi(token); }
    catch (...) { return false; }

    if (!getline(ss, out.name, '|')) return false;
    if (!getline(ss, token, '|')) return false;
    try { out.cnic = stoll(token); }
    catch (...) { return false; }
    if (!getline(ss, out.contact, '|')) return false;
    if (!getline(ss, out.status)) return false;

    return true;
}

static string formatUserLine(const user& u) {
    ostringstream os;
    os << u.id << '|' << u.name << '|' << u.cnic << '|' << u.contact << '|' << u.status;
    return os.str();
}


bool findBook(long long isbn, Book& out) {
    ifstream in("books.txt");
    if (!in.is_open()) return false;
    Book b;
    while (in >> b.isbn) {
        in.ignore(); // skip single delimiter char (now expected to be '|')
        getline(in, b.title, '|');
        getline(in, b.author, '|');
        getline(in, b.category, '|');
        in >> b.quantity;
        if (b.isbn == isbn) {
            out = b;
            in.close();
            return true;
        }
    }
    in.close();
    return false;
}


static string fitField(const string& s, size_t width) {
    if (width == 0) return string();
    if (s.size() <= width) return s;
    if (width <= 3) return s.substr(0, width);
    return s.substr(0, width - 3) + "...";
}

bool parseDate(const string& d, tm& out) {
    if (!validateDateFormat(d)) return false;
    int dd = stoi(d.substr(0, 2));
    int mm = stoi(d.substr(3, 2));
    int yyyy = stoi(d.substr(6, 4));
    memset(&out, 0, sizeof(tm));
    out.tm_mday = dd;
    out.tm_mon = mm - 1;
    out.tm_year = yyyy - 1900;
    out.tm_hour = 0;
    out.tm_min = 0;
    out.tm_sec = 0;
    // normalize
    time_t t = mktime(&out);
    if (t == -1) return false;
    // mktime may modify tm fields; that's fine
    return true;
}

// returns days from a to b (b - a). If parse fails returns INT_MIN
int daysBetween(const string& a, const string& b) {
    tm ta, tb;
    if (!parseDate(a, ta) || !parseDate(b, tb)) return numeric_limits<int>::min();
    time_t tA = mktime(&ta);
    time_t tB = mktime(&tb);
    if (tA == -1 || tB == -1) return numeric_limits<int>::min();
    double diff = difftime(tB, tA);
    int days = static_cast<int>(diff / 86400.0 + 0.5); // round
    return days;
}


// ================= FILE HELPERS =================
bool bookExists(long long isbn) {
    ifstream file("books.txt");
    Book b;
    while (file >> b.isbn) {
        file.ignore();
        getline(file, b.title, '|');
        getline(file, b.author, '|');
        getline(file, b.category, '|');
        file >> b.quantity;
        if (b.isbn == isbn) return true;
    }
    return false;
}

bool updateBookQuantity(long long isbn, int delta) {
    ifstream in("books.txt");
    ofstream out("temp.txt");
    if (!in.is_open() || !out.is_open()) {
        if (in.is_open()) in.close();
        if (out.is_open()) out.close();
        return false;
    }

    Book b;
    bool found = false;
    while (in >> b.isbn) {
        in.ignore();
        getline(in, b.title, '|');
        getline(in, b.author, '|');
        getline(in, b.category, '|');
        in >> b.quantity;
        if (b.isbn == isbn) {
            b.quantity += delta;
            if (b.quantity < 0) b.quantity = 0;
            found = true;
        }
        out << b.isbn << "|" << b.title << "|" << b.author << "|" << b.category << "|" << b.quantity << endl;
    }
    in.close(); out.close();

    if (found) {
        remove("books.txt");
        rename("temp.txt", "books.txt");
    }
    else {
        remove("temp.txt");
    }
    return found;
}

bool userExists(int id, user& u) {
    ifstream file("users.txt");
    if (!file.is_open()) return false;

    string line;
    while (getline(file, line)) {
        user tmp;
        if (!parseUserLine(line, tmp)) continue;
        if (tmp.id == id) {
            u = tmp;
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

bool setBookQuantity(long long isbn, int newQuantity) {
    ifstream in("books.txt");
    ofstream out("temp.txt");
    if (!in.is_open() || !out.is_open()) {
        if (in.is_open()) in.close();
        if (out.is_open()) out.close();
        return false;
    }

    Book b;
    bool found = false;
    while (in >> b.isbn) {
        in.ignore(); // skip delimiter
        getline(in, b.title, '|');
        getline(in, b.author, '|');
        getline(in, b.category, '|');
        in >> b.quantity;
        if (b.isbn == isbn) {
            b.quantity = b.quantity + newQuantity;
            if (b.quantity < 0) b.quantity = 0;
            found = true;
        }
        out << b.isbn << "|" << b.title << "|" << b.author << "|" << b.category << "|" << b.quantity << endl;
    }
    in.close(); out.close();

    if (found) {
        remove("books.txt");
        rename("temp.txt", "books.txt");
    }
    else {
        remove("temp.txt");
    }
    return found;
}

// ================= FUNCTIONS =================
void addBook() {
    Book b;
IN: {
    cout << "Enter ISBN: ";
    cin >> b.isbn;
    }
if (cin.fail()) {  // Check if input failed (e.g., user entered text)
    cin.clear();  // Clear error flags
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    cout << "Invalid input! Please enter a number.\n";
    goto IN;
}

// Check if extra characters exist
if (cin.peek() != '\n') {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input! No extra characters allowed.\n";
    goto IN;
}

if (!validateISBN(b.isbn)) {
    cout << "Invalid ISBN!\n";
    goto IN;
}

if (bookExists(b.isbn)) {
    cout << "Book already exists!\n";
    goto IN;
}

cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
cout << "Enter Title: ";
getline(cin, b.title);
cout << "Enter Author: ";
getline(cin, b.author);
cout << "Enter Category(Adventure, Informative, Encyclopedia, Story, Poetry): ";
getline(cin, b.category);
Qual: {
cout << "Enter Quantity: ";
cin >> b.quantity;
}
if (cin.fail()) {  // Check if input failed (e.g., user entered text)
    cin.clear();  // Clear error flags
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    cout << "Invalid input! Please enter a number.\n";
    goto Qual;
}

// Check if extra characters exist
if (cin.peek() != '\n') {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input! No extra characters allowed.\n";
    goto Qual;
}

if (b.quantity <= 0) {
    cout << "Quantity Must be Greater than 0.\n";
    goto Qual;
}

ofstream file("books.txt", ios::app);
file << b.isbn << "|" << b.title << "|" << b.author << "|" << b.category << "|" << b.quantity << endl;
cout << "Book Added Successfully!\n";
}

void showAllBooks() {
    ifstream file("books.txt");
    if (!file.is_open()) {
        cout << "No books available (books.txt missing).\n";
        return;
    }

    // column widths
    const int wIsbn = 13;
    const int wTitle = 40;
    const int wAuthor = 30;
    const int wCategory = 15;
    const int wQty = 5;

    cout << "\n" << left
        << setw(wIsbn) << "ISBN"
        << setw(wTitle) << "Title"
        << setw(wAuthor) << "Author"
        << setw(wCategory) << "Category"
        << setw(wQty) << "Qty" << "\n";
    cout << string(wIsbn + wTitle + wAuthor + wCategory + wQty, '-') << "\n";

    Book b;
    while (file >> b.isbn) {
        file.ignore();
        if (!getline(file, b.title, '|')) break;
        if (!getline(file, b.author, '|')) break;
        if (!getline(file, b.category, '|')) break;
        if (!(file >> b.quantity)) break;

        cout << left
            << setw(wIsbn) << to_string(b.isbn)
            << setw(wTitle) << fitField(b.title, wTitle)
            << setw(wAuthor) << fitField(b.author, wAuthor)
            << setw(wCategory) << fitField(b.category, wCategory)
            << setw(wQty) << b.quantity << "\n";
    }
    file.close();
}

void addUser() {
    user u;
id: {
    cout << "Enter User ID [4-digits]: ";
    cin >> u.id;
    }
if (u.id < 1000 || u.id>9999) {
    cout << "Invalid Format of User ID\n ";
    goto id;
}
if (cin.fail()) {  // Check if input failed (e.g., user entered text)
    cin.clear();  // Clear error flags
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    cout << "Invalid input! Please enter a number.\n";
    goto id;
}
// Check if extra characters exist
if (cin.peek() != '\n') {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input! No extra characters allowed.\n";
    goto id;
}

user temp;
if (userExists(u.id, temp)) {
    cout << "User already exists!\n";
    goto id;
}

cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
cout << "Enter Name: ";
getline(cin, u.name);
NIC:
{
    cout << "Enter CNIC: ";
    cin >> u.cnic;
}
if (cin.fail()) {
    cin.clear();
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    cout << "Invalid input! Please enter a valid CNIC.\n";
    goto NIC;
}

// Check if extra characters exist
if (cin.peek() != '\n') {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input! No extra characters allowed.\n";
    goto NIC;
}
cin.ignore();
cout << "Enter Contact (Email/Number): ";
getline(cin, u.contact);

u.status = "ACTIVE";

ofstream file("users.txt", ios::app);
file << u.id << "|" << u.name << "|" << u.cnic << "|"
<< u.contact << "|" << u.status << endl;

cout << "User Added Successfully!\n";
}

void blockUser() {
    int id;
ID_INPUT:
    {
        cout << "Enter User ID to block: ";
        cin >> id;
    }
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number.\n";
        goto ID_INPUT;
    }
    if (id < 1000 || id > 9999) {
        cout << "Invalid Format of User ID\n";
        goto ID_INPUT;
    }

    ifstream in("users.txt");
    ofstream tmp("temp.txt");
    if (!in.is_open() || !tmp.is_open()) {
        if (in.is_open()) in.close();
        if (tmp.is_open()) tmp.close();
        cout << "Unable to open users file.\n";
        return;
    }

    string line;
    bool found = false;
    while (getline(in, line)) {
        user uu;
        if (parseUserLine(line, uu)) {
            if (uu.id == id) {
                uu.status = "BLOCKED";
                found = true;
            }
            tmp << formatUserLine(uu) << '\n';
        }
        else {
            // preserve malformed lines as-is
            tmp << line << '\n';
        }
    }
    in.close();
    tmp.close();

    if (!found) {
        remove("temp.txt");
        cout << "User with ID " << id << " not found.\n";
        return;
    }

    if (remove("users.txt") != 0) {
        cout << "Warning: could not remove original users file. Operation may be inconsistent.\n";
    }
    if (rename("temp.txt", "users.txt") != 0) {
        cout << "Failed to update users file. Please try again.\n";
        remove("temp.txt");
        return;
    }

    cout << "User with ID " << id << " has been BLOCKED successfully.\n";
}

void addNewStock() {

    long long isbn;
IN: {
    cout << "Enter ISBN: ";
    cin >> isbn;
    }
if (cin.fail()) {  // Check if input failed (e.g., user entered text)
    cin.clear();  // Clear error flags
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    cout << "Invalid input! Please enter a number.\n";
    goto IN;
}

// Check if extra characters exist
if (cin.peek() != '\n') {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input! No extra characters allowed.\n";
    goto IN;
}

if (!validateISBN(isbn)) {
    cout << "Invalid ISBN!\n";
    goto IN;
}

Book b;
if (!findBook(isbn, b)) {
    cout << "Book with ISBN " << isbn << " not found.\n";
    return;
}

cout << "Found: \"" << b.title << "\" by " << b.author << " | Current Qty: " << b.quantity << "\n";

// get new quantity
int newQty;
GET_QTY:
{
    cout << "Enter new stock quantity (0 or greater): ";
    cin >> newQty;
}
if (cin.fail()) {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input! Please enter a whole number.\n";
    goto GET_QTY;
}
// check for stray characters after the number
if (cin.peek() != '\n') {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input! No extra characters allowed.\n";
    goto GET_QTY;
}
if (newQty < 0) {
    cout << "Quantity must be 0 or greater.\n";
    goto GET_QTY;
}

if (!setBookQuantity(isbn, newQty)) {
    cout << "Failed to update book stock. (I/O error)\n";
    return;
}

cout << "Stock updated successfully." << endl;
}

void searchBook() {
    auto toLower = [](const string& s) {
        string r = s;
        for (char& c : r) {
            if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
        }
        return r;
        };

    int choice;
SEARCH_MENU: {
    cout << "Search by:\n";
    cout << "1. ISBN\n";
    cout << "2. Book Title\n";
    cout << "3. Author\n";
    cout << "4. Category\n";
    cout << "Enter choice (1-4): ";
    cin >> choice;
    }

if (cin.fail()) {
    cin.clear();
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    cout << "Invalid input! Please enter a number between 1 and 4.\n";
    goto SEARCH_MENU;
}
if (choice < 1 || choice > 4) {
    cout << "Choice must be between 1 and 4.\n";
    goto SEARCH_MENU;
}

if (choice == 1) {
ISBN_SEARCH: {
    cout << "Enter ISBN to search: ";
    long long isbn;
    cin >> isbn;
    if (cin.fail()) {
        cin.clear();
        cout << "Invalid input! Please enter a numeric ISBN.\n";
        goto ISBN_SEARCH;
    }
    // Check if extra characters exist
    if (cin.peek() != '\n') {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! No extra characters allowed.\n";
        goto ISBN_SEARCH;
    }

    if (!validateISBN(isbn)) {
        cout << "Invalid ISBN (must be 10-13 digits in range).\n";
        return;
    }
    Book b;
    if (findBook(isbn, b)) {
        cout << "Book Found:\n";
        cout << "ISBN: " << b.isbn << "\nTitle: " << b.title
            << "\nAuthor: " << b.author << "\nCategory: " << b.category
            << "\nQuantity: " << b.quantity << endl;
    }
    else {
        cout << "Book Not Found!\n";
    }
    return;
    }
}

string term;
search: {
cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
cout << "Enter search term: ";
getline(cin, term);
}
if (term.empty()) {
    cout << "Search term cannot be empty.\n";
    goto search;
}
string termLower = toLower(term);

ifstream file("books.txt");
if (!file.is_open()) {
    cout << "No books available (books.txt missing).\n";
    return;
}

Book b;
bool foundAny = false;
cout << "\nMatches:\n";
cout << "ISBN\tTitle\tAuthor\tCategory\tQty\n";
cout << "---------------------------------------------------------------\n";

while (file >> b.isbn) {
    file.ignore();
    if (!getline(file, b.title, '|')) break;
    if (!getline(file, b.author, '|')) break;
    if (!getline(file, b.category, '|')) break;
    if (!(file >> b.quantity)) break;

    string fieldToCheck;
    switch (choice) {
    case 2: fieldToCheck = b.title; break;
    case 3: fieldToCheck = b.author; break;
    case 4: fieldToCheck = b.category; break;
    default: fieldToCheck = ""; break;
    }

    string fieldLower = toLower(fieldToCheck);
    if (fieldLower.find(termLower) != string::npos) {
        foundAny = true;
        cout << b.isbn << "\t" << b.title << "\t" << b.author << "\t" << b.category << "\t" << b.quantity << endl;
    }
}

if (!foundAny) {
    cout << "No matching books found.\n";
}
file.close();
}

void issueBook() {
    auto toLower = [](const string& s) {
        string r = s;
        for (char& c : r) if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
        return r;
        };

    int choice;
ISSUE_SEARCH_MENU:
    {
        cout << "Select book search method for issuing:\n";
        cout << "1. ISBN\n";
        cout << "2. Title\n";
        cout << "3. Author\n";
        cout << "4. Category\n";
        cout << "Enter choice (1-4): ";
        cin >> choice;
    }
    if (cin.fail()) {
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number between 1 and 4.\n";
        goto ISSUE_SEARCH_MENU;
    }

    // Check if extra characters exist
    if (cin.peek() != '\n') {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! No extra characters allowed.\n";
        goto ISSUE_SEARCH_MENU;
    }
    if (choice < 1 || choice > 4) {
        cout << "Choice must be between 1 and 4.\n";
        goto ISSUE_SEARCH_MENU;
    }

    long long selectedIsbn = 0;
    Book selectedBook;

    if (choice == 1) {
    ISBN_ASK:
        {
            cout << "Enter ISBN: ";
            cin >> selectedIsbn;
        }
        if (cin.fail()) {
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a numeric ISBN.\n";
            goto ISBN_ASK;
        }

        // Check if extra characters exist
        if (cin.peek() != '\n') {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! No extra characters allowed.\n";
            goto ISBN_ASK;
        }
        if (!validateISBN(selectedIsbn)) {
            cout << "Invalid ISBN!\n";
            goto ISBN_ASK;
        }
        if (!findBook(selectedIsbn, selectedBook)) {
            cout << "Book not found!\n";
            goto ISBN_ASK;
        }
    }
    else {
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    SEARCH_TERM:
        {
            cout << "Enter search term: ";
            string term;
            getline(cin, term);
            if (term.empty()) {
                cout << "Search term cannot be empty.\n";
                goto SEARCH_TERM;
            }
            string termLower = toLower(term);

            ifstream file("books.txt");
            if (!file.is_open()) {
                cout << "No books available (books.txt missing).\n";
                return;
            }

            struct Hit { long long isbn; Book b; };
            vector<Hit> hits;
            Book b;
            while (file >> b.isbn) {
                file.ignore();
                if (!getline(file, b.title, '|')) break;
                if (!getline(file, b.author, '|')) break;
                if (!getline(file, b.category, '|')) break;
                if (!(file >> b.quantity)) break;

                string field;
                if (choice == 2) field = b.title;
                else if (choice == 3) field = b.author;
                else field = b.category;

                if (toLower(field).find(termLower) != string::npos) {
                    hits.push_back({ b.isbn, b });
                }
            }
            file.close();

            if (hits.empty()) {
                cout << "No matching books found.\n";
                return;
            }

            cout << "\nMatches:\n";
            for (int i = 0; i < (int)hits.size(); ++i) {
                cout << i + 1 << ". ISBN: " << hits[i].isbn
                    << " | Title: " << hits[i].b.title
                    << " | Author: " << hits[i].b.author
                    << " | Category: " << hits[i].b.category
                    << " | Qty: " << hits[i].b.quantity << '\n';
            }

            int sel;
        SELECT_INDEX:
            {
                cout << "Enter result number to select book: ";
                cin >> sel;
            }
            if (cin.fail()) {
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                cout << "Invalid input! Please enter a number.\n";
                goto SELECT_INDEX;
            }

            // Check if extra characters exist
            if (cin.peek() != '\n') {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input! No extra characters allowed.\n";
                goto SELECT_INDEX;
            }
            if (sel < 1 || sel >(int)hits.size()) {
                cout << "Selection out of range.\n";
                goto SELECT_INDEX;
            }
            selectedIsbn = hits[sel - 1].isbn;
            selectedBook = hits[sel - 1].b;
        }
    }

    if (selectedBook.quantity <= 0) {
        cout << "No copies available to issue!\n";
        return;
    }

    int userID;
USER_ID_INPUT:
    {
        cout << "Enter User ID: ";
        cin >> userID;
    }
    if (cin.fail()) {
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number.\n";
        goto USER_ID_INPUT;
    }
    // Check if extra characters exist
    if (cin.peek() != '\n') {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! No extra characters allowed.\n";
        goto USER_ID_INPUT;
    }

    user u;
    if (!userExists(userID, u)) {
        cout << "User not found!\n";
        goto USER_ID_INPUT;
    }
    if (u.status == "BLOCKED") {
        cout << "User is BLOCKED due to fine or missing book!\n";
        return;
    }

    // get issue date from user (DD-MM-YYYY)
    string issueDate;
ISSUE_DATE_INPUT:
    {
        cout << "Enter Issue Date (DD-MM-YYYY): ";
        cin >> issueDate;
    }
    if (!validateDateFormat(issueDate)) {
        cout << "Invalid date format. Use DD-MM-YYYY.\n";
        goto ISSUE_DATE_INPUT;
    }

    // decrement quantity (attempt)
    if (!updateBookQuantity(selectedIsbn, -1)) {
        cout << "Failed to update book quantity. Issue aborted.\n";
        return;
    }

    // prepare issued record and write
    IssuedBook ib;
    ib.isbn = selectedIsbn;
    ib.title = selectedBook.title;
    ib.author = selectedBook.author;
    ib.category = selectedBook.category;
    ib.issueDate = issueDate;
    ib.issuedby = "Library Staff";

    ofstream outf("issued.txt", ios::app);
    if (!outf.is_open()) {
        updateBookQuantity(selectedIsbn, +1);
        cout << "Failed to record issued book. Please try again.\n";
        return;
    }
    outf << ib.isbn << "|" << ib.title << "|" << ib.author << "|" << ib.category << "|" << userID << "|" << ib.issueDate << "|" << ib.issuedby << endl;
    if (!outf) {
        updateBookQuantity(selectedIsbn, +1);
        cout << "Failed to write issued record. Operation rolled back.\n";
        return;
    }
    outf.close();

    cout << "Book Issued Successfully to " << u.name << " (ISBN: " << selectedIsbn << ", Date: " << ib.issueDate << ")\n";
}

void returnBook() {
    auto toLower = [](const string& s) {
        string r = s;
        for (char& c : r) if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
        return r;
        };

    struct IssuedRec {
        long long isbn;
        string title;
        string author;
        string category;
        string userStr;
        string issueDate;
        string issuedby;
    };

    cout << "Return by:\n";
    cout << "1. ISBN\n";
    cout << "2. User ID\n";
    cout << "3. Title\n";
    cout << "4. Author\n";
    cout << "5. Category\n";
    int choice;
MENU_CHOICE:
    {
        cout << "Enter choice (1-5): ";
        cin >> choice;
    }
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a number 1-5.\n";
        goto MENU_CHOICE;
    }

    // Check if extra characters exist
    if (cin.peek() != '\n') {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! No extra characters allowed.\n";
        goto MENU_CHOICE;
    }
    if (choice < 1 || choice > 5) {
        cout << "Choice must be between 1 and 5.\n";
        goto MENU_CHOICE;
    }

    // load issued records
    vector<IssuedRec> records;
    ifstream in("issued.txt");
    if (!in.is_open()) {
        cout << "No issued records found.\n";
        return;
    }
    while (true) {
        IssuedRec r;
        if (!(in >> r.isbn)) break;
        in.ignore();
        if (!getline(in, r.title, '|')) break;
        if (!getline(in, r.author, '|')) break;
        if (!getline(in, r.category, '|')) break;
        if (!getline(in, r.userStr, '|')) break;
        if (!getline(in, r.issueDate, '|')) break;
        if (!getline(in, r.issuedby)) break;
        records.push_back(r);
    }
    in.close();

    if (records.empty()) {
        cout << "No issued records available.\n";
        return;
    }

    vector<int> candidates;

    if (choice == 1) {
    ISBN_IN:
        {
            cout << "Enter ISBN: ";
            long long isbn;
            cin >> isbn;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
                cout << "Invalid input. Enter numeric ISBN.\n";
                goto ISBN_IN;
            }
            // Check if extra characters exist
            if (cin.peek() != '\n') {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input! No extra characters allowed.\n";
                goto ISBN_IN;
            }
            if (!validateISBN(isbn)) {
                cout << "Invalid ISBN.\n";
                goto ISBN_IN;
            }
            for (int i = 0; i < (int)records.size(); ++i)
                if (records[i].isbn == isbn) candidates.push_back(i);

            if (candidates.empty()) {
                cout << "No issued record found for that ISBN.\n";
                return;
            }
        }
    }
    else if (choice == 2) {
    USER_IN:
        {
            cout << "Enter User ID: ";
            int uid;
            cin >> uid;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
                cout << "Invalid input. Enter numeric User ID.\n";
                goto USER_IN;
            }
            // Check if extra characters exist
            if (cin.peek() != '\n') {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input! No extra characters allowed.\n";
                goto USER_IN;
            }
            string us = to_string(uid);
            for (int i = 0; i < (int)records.size(); ++i) {
                if (records[i].userStr == us) candidates.push_back(i);
            }
            if (candidates.empty()) {
                cout << "No issued records found for that User ID.\n";
                return;
            }
        }
    }
    else {
        cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
    SEARCH_TERM:
        {
            cout << "Enter search term: ";
            string term;
            getline(cin, term);
            if (term.empty()) {
                cout << "Search term cannot be empty.\n";
                goto SEARCH_TERM;
            }
            string t = toLower(term);
            for (int i = 0; i < (int)records.size(); ++i) {
                string field;
                if (choice == 3) field = records[i].title;
                else if (choice == 4) field = records[i].author;
                else field = records[i].category;
                if (toLower(field).find(t) != string::npos) candidates.push_back(i);
            }
            if (candidates.empty()) {
                cout << "No matching issued records found.\n";
                return;
            }
        }
    }

    int targetIndex = -1;
    if (candidates.size() == 1) {
        targetIndex = candidates[0];
        cout << "Found 1 matching issued record:\n";
        cout << "ISBN: " << records[targetIndex].isbn
            << " | Title: " << records[targetIndex].title
            << " | UserID: " << records[targetIndex].userStr
            << " | Issued: " << records[targetIndex].issueDate << "\n";
    }
    else {
        cout << "\nMatches:\n";
        for (int idx = 0; idx < (int)candidates.size(); ++idx) {
            int i = candidates[idx];
            cout << idx + 1 << ". ISBN: " << records[i].isbn
                << " | Title: " << records[i].title
                << " | Author: " << records[i].author
                << " | UserID: " << records[i].userStr
                << " | Issued: " << records[i].issueDate << '\n';
        }
    SELECT_ONE:
        {
            cout << "Enter number of the record to return: ";
            int sel;
            cin >> sel;
            if (cin.fail()) {
                cin.clear();
                cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
                cout << "Invalid input. Enter a number.\n";
                goto SELECT_ONE;
            }
            if (sel < 1 || sel >(int)candidates.size()) {
                cout << "Selection out of range.\n";
                goto SELECT_ONE;
            }
            targetIndex = candidates[sel - 1];
        }
    }

    // require return date input
    string returnDate;
RETURN_DATE_INPUT:
    {
        cout << "Enter Return Date (DD-MM-YYYY): ";
        cin >> returnDate;
    }
    if (!validateDateFormat(returnDate)) {
        cout << "Invalid date format. Use DD-MM-YYYY.\n";
        goto RETURN_DATE_INPUT;
    }

    // compute late days and fine
    int totalDays = daysBetween(records[targetIndex].issueDate, returnDate);
    if (totalDays == numeric_limits<int>::min()) {
        cout << "Unable to compute days difference (invalid stored issue date?). Proceeding without fine calculation.\n";
    }
    else if (totalDays < 0) {
        cout << "Return date is before issue date. Abort.\n";
        return;
    }
    else {
        int late = totalDays - 7;
        if (late > 0) {
            int fine = late * 20;
            cout << "Book is returned " << late << " day(s) late. Fine = Rs." << fine << "\n";
            // We do not automatically block the user here; just inform.
        }
        else {
            cout << "Book returned within allowed period (<=7 days). No fine.\n";
        }
    }

    // For safety, confirm User ID matches record
    int confirmUid;
CONFIRM_UID:
    {
        cout << "Enter User ID to confirm return: ";
        cin >> confirmUid;
    }
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input. Enter numeric User ID.\n";
        goto CONFIRM_UID;
    }

    // Check if extra characters exist
    if (cin.peek() != '\n') {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! No extra characters allowed.\n";
        goto CONFIRM_UID;
    }
    try {
        int recUid = stoi(records[targetIndex].userStr);
        if (recUid != confirmUid) {
            cout << "Provided User ID does not match the selected record. Abort.\n";
            return;
        }
    }
    catch (...) {
        cout << "Stored user id invalid in record; cannot verify. Abort.\n";
        return;
    }

    // Remove the selected record and rewrite issued.txt
    ofstream out("temp.txt");
    if (!out.is_open()) {
        cout << "Failed to open temporary file.\n";
        return;
    }
    for (int i = 0; i < (int)records.size(); ++i) {
        if (i == targetIndex) continue;
        out << records[i].isbn << "|" << records[i].title << "|" << records[i].author << "|" << records[i].category << "|" << records[i].userStr << "|" << records[i].issueDate << "|" << records[i].issuedby << endl;
    }
    out.close();

    if (remove("issued.txt") != 0) {
        // ignore if file removal fails, try rename anyway
    }
    if (rename("temp.txt", "issued.txt") != 0) {
        cout << "Failed to update issued records file. Return aborted.\n";
        remove("temp.txt");
        return;
    }

    // increment book quantity back; warn on failure
    if (!updateBookQuantity(records[targetIndex].isbn, +1)) {
        cout << "Warning: could not increment book quantity (books.txt update failed or book missing).\n";
    }

    cout << "Book returned successfully (ISBN: " << records[targetIndex].isbn << ", User ID: " << records[targetIndex].userStr << ").\n";
}

void reportMisplacedBook() {
    auto toLower = [](const string& s) {
        string r = s;
        for (char& c : r) if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
        return r;
        };

    int choice;
MISPLACED_SEARCH_MENU:
    {
        cout << "Select book search method for misplaced report:\n";
        cout << "1. ISBN\n";
        cout << "2. Title\n";
        cout << "3. Author\n";
        cout << "4. Category\n";
        cout << "Enter choice (1-4): ";
        cin >> choice;
    }
    if (cin.fail()) {
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number between 1 and 4.\n";
        goto MISPLACED_SEARCH_MENU;
    }

    // Check if extra characters exist
    if (cin.peek() != '\n') {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! No extra characters allowed.\n";
        goto MISPLACED_SEARCH_MENU;
    }
    if (choice < 1 || choice > 4) {
        cout << "Choice must be between 1 and 4.\n";
        goto MISPLACED_SEARCH_MENU;
    }

    long long selectedIsbn = 0;
    Book selectedBook;

    if (choice == 1) {
    MIS_ISBN:
        {
            cout << "Enter ISBN: ";
            cin >> selectedIsbn;
        }
        if (cin.fail()) {
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a numeric ISBN.\n";
            goto MIS_ISBN;
        }

        // Check if extra characters exist
        if (cin.peek() != '\n') {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! No extra characters allowed.\n";
            goto MIS_ISBN;
        }
        if (!validateISBN(selectedIsbn)) {
            cout << "Invalid ISBN!\n";
            goto MIS_ISBN;
        }
        if (!findBook(selectedIsbn, selectedBook)) {
            cout << "Book not found!\n";
            return;
        }
    }
    else {
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    MIS_TERM:
        {
            cout << "Enter search term: ";
            string term;
            getline(cin, term);
            if (term.empty()) {
                cout << "Search term cannot be empty.\n";
                goto MIS_TERM;
            }
            string termLower = toLower(term);

            ifstream file("books.txt");
            if (!file.is_open()) {
                cout << "No books available (books.txt missing).\n";
                return;
            }

            struct Hit {
                long long isbn;
                Book b;
            };
            vector<Hit> hits;
            Book b;
            while (file >> b.isbn) {
                file.ignore();
                if (!getline(file, b.title, '|')) break;
                if (!getline(file, b.author, '|')) break;
                if (!getline(file, b.category, '|')) break;
                if (!(file >> b.quantity)) break;

                string field;
                if (choice == 2) field = b.title;
                else if (choice == 3) field = b.author;
                else field = b.category;

                if (toLower(field).find(termLower) != string::npos) {
                    hits.push_back({ b.isbn, b });
                }
            }
            file.close();

            if (hits.empty()) {
                cout << "No matching books found.\n";
                return;
            }

            cout << "\nMatches:\n";
            for (size_t i = 0; i < hits.size(); ++i) {
                cout << i + 1 << ". ISBN: " << hits[i].isbn
                    << " | Title: " << hits[i].b.title
                    << " | Author: " << hits[i].b.author
                    << " | Category: " << hits[i].b.category
                    << " | Qty: " << hits[i].b.quantity << '\n';
            }

            int sel;
        MIS_SELECT:
            {
                cout << "Enter result number to select book: ";
                cin >> sel;
            }
            if (cin.fail()) {
                cin.clear();
                cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                cout << "Invalid input! Please enter a number.\n";
                goto MIS_SELECT;
            }

            // Check if extra characters exist
            if (cin.peek() != '\n') {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input! No extra characters allowed.\n";
                goto MIS_SELECT;
            }
            if (sel < 1 || sel >(int)hits.size()) {
                cout << "Selection out of range.\n";
                goto MIS_SELECT;
            }
            selectedIsbn = hits[sel - 1].isbn;
            selectedBook = hits[sel - 1].b;
        }
    }

    int userID;
MIS_USER:
    {
        cout << "Misplaced by User ID (0 if staff): ";
        cin >> userID;
    }
    if (cin.fail()) {
        cin.clear();
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number.\n";
        goto MIS_USER;
    }

    // Check if extra characters exist
    if (cin.peek() != '\n') {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! No extra characters allowed.\n";
        goto MIS_USER;
    }
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    string reportDate;
REPORT_DATE:
    {
        cout << "Enter Report Date (DD-MM-YYYY): ";
        getline(cin, reportDate);
    }
    if (!validateDateFormat(reportDate)) {
        cout << "Invalid date format. Use DD-MM-YYYY.\n";
        goto REPORT_DATE;
    }

    // get reason
MIS_REASON:
    {
        cout << "Reason: ";
        string reason;
        getline(cin, reason);
        if (reason.empty()) {
            cout << "Reason cannot be empty.\n";
            goto MIS_REASON;
        }

        // prepare misplaced record
        MisplacedBook mb;
        mb.isbn = selectedIsbn;
        Book fresh;
        if (findBook(selectedIsbn, fresh)) {
            mb.title = fresh.title;
            mb.author = fresh.author;
            mb.category = fresh.category;
        }
        else {
            mb.title = "N/A";
            mb.author = "N/A";
            mb.category = "N/A";
        }
        mb.reason = reason;
        mb.reportDate = reportDate;
        mb.misplacedby = (userID == 0) ? "Staff" : "User";

        // decrement quantity (attempt)
        if (!updateBookQuantity(mb.isbn, -1)) {
            cout << "Warning: couldn't decrement book quantity (books.txt update failed or book missing).\n";
        }

        ofstream outf("misplaced.txt", ios::app);
        if (!outf.is_open()) {
            updateBookQuantity(mb.isbn, +1);
            cout << "Failed to record misplaced book. Please try again.\n";
            return;
        }
        outf << mb.isbn << "|" << mb.title << "|" << mb.author << "|" << mb.category << "|" << mb.reason << "|" << mb.reportDate << "|" << mb.misplacedby << endl;
        if (!outf) {
            updateBookQuantity(mb.isbn, +1);
            cout << "Failed to write misplaced record. Operation rolled back.\n";
            return;
        }
        outf.close();

        if (userID != 0) {
            // Block the user who misplaced the book
            cout << "Blocking the user who misplaced the book (User ID: " << userID << ").\n";
            // blockUser() prompts for ID; since we already have userID, update users file directly here to avoid double prompt
            ifstream inUsers("users.txt");
            ofstream tmp("temp.txt");
            if (inUsers.is_open() && tmp.is_open()) {
                string uline;
                user uu;
                bool blocked = false;
                while (getline(inUsers, uline)) {
                    if (parseUserLine(uline, uu)) {
                        if (uu.id == userID) {
                            uu.status = "BLOCKED";
                            blocked = true;
                        }
                        tmp << formatUserLine(uu) << '\n';
                    }
                    else {
                        tmp << uline << '\n';
                    }
                }
                inUsers.close();
                tmp.close();
                if (blocked) {
                    remove("users.txt");
                    rename("temp.txt", "users.txt");
                    cout << "User " << userID << " has been BLOCKED due to misplacing a book.\n";
                }
                else {
                    remove("temp.txt");
                    cout << "User ID not found; could not block.\n";
                }
            }
            else {
                cout << "Could not update users file to block user.\n";
            }
        }

        cout << "Misplaced Book Reported! (ISBN: " << mb.isbn << ", Report Date: " << mb.reportDate << ")\n";
    }
}

void showMisplacedBooks() {
    ifstream file("misplaced.txt");
    if (!file.is_open()) {
        cout << "No misplaced records found.\n";
        return;
    }

    // column widths
    const int wIsbn = 13;
    const int wTitle = 40;
    const int wAuthor = 30;
    const int wCategory = 15;
    const int wReason = 30;
    const int wDate = 12;
    const int wBy = 10;

    cout << "\n" << left
        << setw(wIsbn) << "ISBN"
        << setw(wTitle) << "Title"
        << setw(wAuthor) << "Author"
        << setw(wCategory) << "Category"
        << setw(wReason) << "Reason"
        << setw(wDate) << "Date"
        << setw(wBy) << "By" << "\n";
    cout << string(wIsbn + wTitle + wAuthor + wCategory + wReason + wDate + wBy, '-') << "\n";

    while (true) {
        long long isbn;
        if (!(file >> isbn)) break;
        file.ignore();
        string title, author, category, reason, reportDate, misplacedby;
        if (!getline(file, title, '|')) break;
        if (!getline(file, author, '|')) break;
        if (!getline(file, category, '|')) break;
        if (!getline(file, reason, '|')) break;
        if (!getline(file, reportDate, '|')) break;
        if (!getline(file, misplacedby)) break;

        cout << left
            << setw(wIsbn) << to_string(isbn)
            << setw(wTitle) << fitField(title, wTitle)
            << setw(wAuthor) << fitField(author, wAuthor)
            << setw(wCategory) << fitField(category, wCategory)
            << setw(wReason) << fitField(reason, wReason)
            << setw(wDate) << fitField(reportDate, wDate)
            << setw(wBy) << fitField(misplacedby, wBy) << "\n";
    }

    file.close();
}

void showIssuedBooks() {
    ifstream file("issued.txt");
    if (!file.is_open()) {
        cout << "No issued records found.\n";
        return;
    }

    // column widths
    const int wIsbn = 13;
    const int wTitle = 40;
    const int wAuthor = 30;
    const int wCategory = 15;
    const int wUser = 8;
    const int wDate = 12;
    const int wBy = 12;

    cout << "\n" << left
        << setw(wIsbn) << "ISBN"
        << setw(wTitle) << "Title"
        << setw(wAuthor) << "Author"
        << setw(wCategory) << "Category"
        << setw(wUser) << "UserID"
        << setw(wDate) << "IssueDate"
        << setw(wBy) << "IssuedBy" << "\n";
    cout << string(wIsbn + wTitle + wAuthor + wCategory + wUser + wDate + wBy, '-') << "\n";

    while (true) {
        long long isbn;
        if (!(file >> isbn)) break;
        file.ignore(); // skip the single delimiter after ISBN

        string title, author, category, userStr, issueDate, issuedby;
        if (!getline(file, title, '|')) break;
        if (!getline(file, author, '|')) break;
        if (!getline(file, category, '|')) break;
        if (!getline(file, userStr, '|')) break;
        if (!getline(file, issueDate, '|')) break;
        if (!getline(file, issuedby)) break;

        cout << left
            << setw(wIsbn) << to_string(isbn)
            << setw(wTitle) << fitField(title, wTitle)
            << setw(wAuthor) << fitField(author, wAuthor)
            << setw(wCategory) << fitField(category, wCategory)
            << setw(wUser) << fitField(userStr, wUser)
            << setw(wDate) << fitField(issueDate, wDate)
            << setw(wBy) << fitField(issuedby, wBy) << "\n";
    }

    file.close();
}

void showAllUsers() {
    ifstream file("users.txt");
    if (!file.is_open()) {
        cout << "No users available (users.txt missing).\n";
        return;
    }

    cout << "\n" << left << setw(6) << "ID"
        << setw(30) << "Name"
        << setw(20) << "CNIC"
        << setw(20) << "Contact"
        << setw(10) << "Status" << "\n";
    cout << string(86, '-') << "\n";

    string line;
    while (getline(file, line)) {
        user u;
        if (!parseUserLine(line, u)) {
            // skip or optionally report malformed line
            continue;
        }
        cout << left << setw(6) << u.id
            << setw(30) << fitField(u.name, 30)
            << setw(20) << to_string(u.cnic)
            << setw(20) << fitField(u.contact, 20)
            << setw(10) << fitField(u.status, 10) << "\n";
    }

    file.close();
}

void unblockUser() {
    int id;
UNBLOCK_INPUT:
    {
        cout << "Enter User ID [4-digits] to unblock: ";
        cin >> id;
    }

    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number.\n";
        goto UNBLOCK_INPUT;
    }
    // Check if extra characters exist
    if (cin.peek() != '\n') {
        cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input! No extra characters allowed.\n";
        goto UNBLOCK_INPUT;
    }
    if (id < 1000 || id > 9999) {
        cout << "Invalid Format of User ID (must be 4 digits)\n";
        goto UNBLOCK_INPUT;
    }

    ifstream in("users.txt");
    ofstream tmp("temp.txt");
    if (!in.is_open() || !tmp.is_open()) {
        if (in.is_open()) in.close();
        if (tmp.is_open()) tmp.close();
        cout << "Unable to open users file.\n";
        return;
    }

    string line;
    bool found = false;
    while (getline(in, line)) {
        user uu;
        if (parseUserLine(line, uu)) {
            if (uu.id == id) {
                if (uu.status == "ACTIVE") {
                    // already active; still rewrite the file unchanged
                    cout << "User " << id << " is already ACTIVE.\n";
                }
                else {
                    uu.status = "ACTIVE";
                    cout << "User " << id << " will be UNBLOCKED.\n";
                }
                found = true;
            }
            tmp << formatUserLine(uu) << '\n';
        }
        else {
            // preserve malformed lines as-is
            tmp << line << '\n';
        }
    }
    in.close();
    tmp.close();

    if (!found) {
        remove("temp.txt");
        cout << "User with ID " << id << " not found.\n";
        return;
    }

    if (remove("users.txt") != 0) {
        cout << "Warning: could not remove original users file. Operation may be inconsistent.\n";
    }
    if (rename("temp.txt", "users.txt") != 0) {
        cout << "Failed to update users file. Please try again.\n";
        remove("temp.txt");
        return;
    }

    cout << "User with ID " << id << " has been UNBLOCKED successfully.\n";
}

void showStatistics() {
    int totalTitles = 0;
    int totalCopies = 0;
    int issued = 0;
    int misplaced = 0;
    string line;

    ifstream f1("books.txt");
    Book b2;
    while (f1 >> b2.isbn) {
        f1.ignore();
        getline(f1, b2.title, '|');
        getline(f1, b2.author, '|');
        getline(f1, b2.category, '|');
        f1 >> b2.quantity;
        totalTitles++;
        totalCopies += b2.quantity;
    }
    f1.close();

    ifstream f2("issued.txt");
    while (getline(f2, line)) issued++;
    f2.close();

    ifstream f3("misplaced.txt");
    while (getline(f3, line)) misplaced++;
    f3.close();

    cout << "\n===== STATISTICS =====\n";
    cout << "Total Book Titles: " << totalTitles << endl;
    cout << "Total Book Copies: " << totalCopies << endl;
    cout << "Issued Records: " << issued << endl;
    cout << "Misplaced Records: " << misplaced << endl;
}

// ================= MAIN =================
int main() {
    string user, pass;
    int attempts = 0;
    cout << "Welcome to the Library Management System!" << endl;
    cout << "Please Enter the Following Credentials to continue..." << endl;
LOGIN:
    {
        cout << "Username: ";
        cin >> user;
        cout << "Password: ";
        cin >> pass;
    }
    if (user != "admin" || pass != "admin") {
        attempts++;
        if (attempts == 3) {
            cout << "System Locked!\n";
            return 0;
        }
        cout << "Wrong credentials!\n";
        goto LOGIN;
    }
    if (user == "admin" && pass == "admin") cout << "Login Successful!" << endl;

    int choice;
Prog: {
MENU:
    {
        cout << "1. Add Book\n";
        cout << "2. Search Book\n";
        cout << "3. Issue Book\n";
        cout << "4. Return Book\n";
        cout << "5. Add User\n";
        cout << "6. Show All Books\n";
        cout << "7. Show Issued Books\n";
        cout << "8. Report Misplaced Book\n";
        cout << "9. Show Misplaced Books\n";
        cout << "10. Block User\n";
        cout << "11. Unblock User\n";
        cout << "12. Show All Users\n";
        cout << "13. Add New Stock.\n";
        cout << "14. Show Statistics.\n";
        cout << "0. Exit\n";
        cout << "Enter Your Choice(0-14): ";
        cin >> choice;
        if (choice < 0 || choice > 14) {
            cout << "Invalid Choice! Try Again..." << endl;
            goto MENU;
        }
    }
    if (cin.fail()) {  // Check if input failed (e.g., user entered text)
        cin.clear();  // Clear error flags
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        cout << "Invalid input! Please enter a number.\n";
        goto MENU;
    }

    // Check if extra characters exist
    if (cin.peek() != '\n') {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input! No extra characters allowed.\n";
        goto MENU;
    }
    /*else if (choice < 0 || choice > 14) {
        cout << "Choice must be between 0 and 14.\n";
        goto MENU;
    }*/
    switch (choice) {
    case 1: addBook(); break;
    case 2: searchBook(); break;
    case 3: issueBook(); break;
    case 4: returnBook(); break;
    case 5: addUser(); break;
    case 6: showAllBooks(); break;
    case 7: showIssuedBooks(); break;
    case 8: reportMisplacedBook(); break;
    case 9:showMisplacedBooks(); break;
    case 10:blockUser(); break;
    case 11:unblockUser(); break;
    case 12:showAllUsers(); break;
    case 13:addNewStock(); break;
    case 14:showStatistics(); break;
    case 0: cout << "Goodbye!\n"; break;
    default: cout << "Invalid choice!\n";
    }
    }
if (choice != 0) goto Prog;
system("pause");
return 0;
}
