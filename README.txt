HOSPITAL APPOINTMENT AND PATIENT MANAGEMENT SYSTEM (DSA OEL 3)

HOW TO RUN (Embarcadero / Dev-C++ / any C++ compiler)
1. Open hospital.cpp and build/run it.
2. The first line printed shows "Data folder: ..." - that is where
   patients.txt and appointments.txt are saved. Your data is loaded from
   there automatically next time.

WHAT WAS FIXED
- "Warning: could not save patients file" : the program now tests several
  folders (current folder, the .exe folder, Documents, user folder, Temp)
  and uses the first one it can write to. If a save ever fails it prints
  the exact reason (e.g. Permission denied).
- Pressing Enter on an empty line at the menu now just asks again.
  ("Field cannot be empty!" still shows for text fields like name/doctor.)
- Files with Windows line endings (\r\n) are read correctly.
- Sorting (option 5) is now also saved to the file.
- No cin.ignore() is needed: the program only uses getline for input.
