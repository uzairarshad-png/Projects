📚 Library Management System (C++)
A console-based Library Management System developed in C++ using file handling.
This project allows library administrators to manage books, users, issuing/returning of books, misplaced records, and system statistics efficiently.

🚀 Features Overview
📖 Book Management


Add new books with ISBN validation


Display all available books in tabular format


Search books by:


ISBN


Title


Author


Category




Add or update book stock


Automatic quantity updates on issue/return


👤 User Management


Add new users (with CNIC & contact)


Block users (due to fines or misplaced books)


Unblock users


Display all registered users


Prevent blocked users from issuing books


📤 Book Issuing


Issue books using:


ISBN


Title


Author


Category




Date validation (DD-MM-YYYY)


Prevent issuing if no stock is available


Prevent issuing to blocked users


📥 Book Returning


Return books using:


ISBN


User ID


Title


Author


Category




Late fine calculation:


Allowed period: 7 days


Fine: Rs. 20 per extra day




Automatic stock restoration


❌ Misplaced Book Handling


Report misplaced books


Record reason and date


Reduce stock automatically


Block responsible user (if applicable)


View all misplaced records


📊 Statistics


Total number of book titles


Total available book copies


Total issued records


Total misplaced records


🔐 Authentication


Admin login system


Username: admin


Password: admin


System locks after 3 failed attempts



🗂️ File Structure
File Name	Description
books.txt	Stores all book records
users.txt	Stores user details
issued.txt	Stores issued book records
misplaced.txt	Stores misplaced book records
temp.txt	Temporary file for safe updates

📁 Data Formats
📘 Books (books.txt)
ISBN Title|Author|Category|Quantity

👤 Users (users.txt)
ID|Name|CNIC|Contact|Status

📤 Issued Books (issued.txt)
ISBN Title|Author|Category|UserID|IssueDate|IssuedBy

❌ Misplaced Books (misplaced.txt)
ISBN Title|Author|Category|Reason|ReportDate|ReportedBy


🛠️ Technologies Used


Language: C++


Concepts:


File Handling (fstream)


Structures


Input Validation


Date Handling (<ctime>)


Vectors & Streams


Modular Programming




Platform: Console Application



▶️ How to Run


Open the project in Visual Studio / Dev-C++ / Code::Blocks


Compile the code using:
g++ main.cpp -o Code



Run the program:
./Code



Login using:


Username: admin


Password: admin





⚠️ Important Notes


Dates must be entered in DD-MM-YYYY format


ISBN must be 10–13 digits


User IDs must be 4-digit numbers


All data is persistent using .txt files


System safely rolls back on file write failure



📌 Future Enhancements (Optional)


GUI version


Role-based access (Admin/User)


Database integration


Password encryption


Book reservation system



👨‍💻 Author
Uzair Arshad
Library Management System – C++ Project

