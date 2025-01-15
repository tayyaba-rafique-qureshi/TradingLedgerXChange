#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <vector>
#include <map>
#include <conio.h>
#include <limits>
#include <sstream>
#include <openssl/sha.h>
#include <openssl/rand.h>

#include "DatabaseHandler.h"


// Global Variables
extern std::atomic<bool> isRunning; // Control flag for the thread
extern std::mutex consoleMutex;     // Mutex for synchronized console access
extern std::string loggedInUserId;
extern bool isLoggedIn;

// Utility Functions
void clearScreen();
void clearConsole();
void displayScreen(const std::vector<std::string>& screen);
std::map<char, std::vector<std::string>> createLetterTemplates();
void revealLetter(const std::string& name, std::vector<std::string>& screen, int startRow, int startCol, const std::map<char, std::vector<std::string>>& templates);
std::string generateSalt();
std::string hashPassword(const std::string& password, const std::string& salt);
std::string getPasswordInput();
bool validateEmail(const std::string& email);
double getValidatedDouble(const std::string& prompt);
int getValidatedInt(const std::string& prompt);
std::string getValidatedString(const std::string& prompt);

// Core Functions
void registerUser(DatabaseHandler& dbHandler);
void authenticateUser(DatabaseHandler& dbHandler);
void UpdatePricesPeriodically(DatabaseHandler& dbHandler);
void userLandingPage(DatabaseHandler& dbHandler);
#pragma once
