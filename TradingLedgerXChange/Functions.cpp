#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <conio.h>
#include <limits>
#include "DatabaseHandler.h"
/*#include "Trie.h" */ 
std::atomic<bool> isRunning{ true }; // Control flag for the thread
std::mutex consoleMutex; // Mutex for synchronized console access
std::string loggedInUserId;
bool isLoggedIn = false;
#include <vector>
#include <map>

#ifdef _WIN32
#include <windows.h>
#else
#include <cstdlib>
#endif

// Function to clear the console screen
void clearScreen() {
#ifdef _WIN32
    system("CLS");
#else
    system("clear");
#endif
}


void displayScreen(const std::vector<std::string>& screen) {
    for (const auto& row : screen) {
        std::cout << row << std::endl;
    }
}

// Function to create a letter map with larger letter templates
std::map<char, std::vector<std::string>> createLetterTemplates() {
    std::map<char, std::vector<std::string>> templates;

    templates['L'] = {
        "   *   ",
        "   *   ",
        "   *   ",
        "   *   ",
        "   *****"
    };

    templates['E'] = {
        "   *****",
        "   *    ",
        "   **** ",
        "   *    ",
        "   *****"
    };

    templates['D'] = {
        "   **** ",
        "   *   *",
        "   *   *",
        "   *   *",
        "   **** "
    };

    templates['G'] = {
        "   **** ",
        "   *    ",
        "   * ***",
        "   *   *",
        "   **** "
    };

    templates['R'] = {
        "   **** ",
        "   *   *",
        "   **** ",
        "   *  * ",
        "   *   *"
    };

    templates['A'] = {
        "    *   ",
        "   * *  ",
        "   *****",
        "   *   *",
        "   *   *"
    };

    templates['T'] = {
        "   *****",
        "     *  ",
        "     *  ",
        "     *  ",
        "     *  "
    };

    templates['X'] = {
        "  *     *  ",
        "   *   *   ",
        "    * *    ",
        "     *     ",
        "    * *    ",
        "   *   *   ",
        "  *     *  "
    };

    return templates;
}

// Function to reveal one letter at a time by removing one space at a time
void revealLetter(const std::string& name, std::vector<std::string>& screen, int startRow, int startCol, const std::map<char, std::vector<std::string>>& templates) {
    for (char letter : name) {
        if (templates.find(letter) == templates.end()) continue;  // Skip unsupported characters

        const auto& letterTemplate = templates.at(letter);
        int letterHeight = letterTemplate.size();
        int letterWidth = letterTemplate[0].size();

        for (int row = 0; row < letterHeight; ++row) {
            for (int col = 0; col < letterWidth; ++col) {
                if (letterTemplate[row][col] == '*') {
                    screen[startRow + row][startCol + col] = ' ';
                    clearScreen();
                    displayScreen(screen);
                    std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Delay for smooth animation
                }
            }
        }

        // Update the start column for the next letter
        startCol += letterWidth + 1;
    }
}
// Function to generate a random salt
std::string generateSalt() {
    unsigned char buffer[16]; // 16 bytes (128 bits)
    if (RAND_bytes(buffer, sizeof(buffer)) != 1) {
        throw std::runtime_error("Error generating random salt.");
    }

    std::ostringstream oss;
    for (int i = 0; i < sizeof(buffer); ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i];
    }
    return oss.str();
}
//
// Function to get a validated double input
double getValidatedDouble(const std::string& prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Clear input buffer
            return value;
        }
        else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a valid number.\n";
        }
    }
}

// Function to hash the password with salt
std::string hashPassword(const std::string& password, const std::string& salt) {
    std::string combined = salt + password; // Concatenate salt and password
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.size(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return oss.str();
}

// Function to get masked password input
std::string getPasswordInput() {
    std::string password;
    char ch;
    std::cout << "Enter Password: ";
    while (true) {
        ch = _getch();
        if (ch == '\r') { // Enter key
            break;
        }
        else if (ch == '\b' && !password.empty()) { // Backspace
            password.pop_back();
            std::cout << "\b \b"; // Erase asterisk
        }
        else if (ch != '\b') {
            password += ch;
            std::cout << '*'; // Display asterisk
        }
    }
    std::cout << std::endl;
    return password;
}

// Function to validate email format
bool validateEmail(const std::string& email) {
    size_t atPos = email.find('@');
    size_t dotPos = email.find('.', atPos);
    return atPos != std::string::npos && dotPos != std::string::npos && atPos < dotPos;
}

// Function to clear the console screen (Cross-platform)
void clearConsole() {
    std::cout << "\033[2J\033[1;1H"; // ANSI escape sequence to clear screen
}

// Helper function to get validated integer input
int getValidatedInt(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a valid number.\n";
        }
    }
}

// Function to handle user registration
void registerUser(DatabaseHandler& dbHandler) {
    system("CLS");
    std::cout << "\nWelcome to the Registration Page!\n";
    std::cout << "=================================\n";

    // Username input
    std::string username;
    while (true) {
        std::cout << "Enter Username: ";
        std::getline(std::cin, username);
        if (username.length() >= 3) break;
        std::cout << "Username must be at least 3 characters long.\n";
    }

    // Email input
    std::string email;
    while (true) {
        std::cout << "Enter Email: ";
        std::getline(std::cin, email);
        if (validateEmail(email)) break;
        std::cout << "Invalid email format. Please try again.\n";
    }

    // Password input
    std::string password;
    while (true) {
        password = getPasswordInput();
        if (password.length() >= 6) break;
        std::cout << "Password must be at least 6 characters long.\n";
    }

    try {
        if (dbHandler.RegisterUser(username, email, password)) {
            std::cout << "User registered successfully!\n";
        }
        else {
            std::cout << "Registration failed. Try again.\n";
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Error during registration: " << ex.what() << std::endl;
    }
}

// Function to handle user login
void authenticateUser(DatabaseHandler& dbHandler) {
    std::string username;
    std::string password;

    std::cout << "Enter Username: ";
    std::getline(std::cin, username);
    password = getPasswordInput();

    try {
        std::string salt = dbHandler.GetSaltByUsername(username);
        if (salt.empty()) {
            std::cout << "Invalid username or password.\n";
            return;
        }

        std::string hashedInputPassword = hashPassword(password, salt);

        if (dbHandler.AuthenticateUser(username, hashedInputPassword)) {
            std::cout << "Login successful! Welcome, " << username << "!\n";
            loggedInUserId = username;
            isLoggedIn = true;
        }
        else {
            std::cout << "Invalid username or password.\n";
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Error during authentication: " << ex.what() << std::endl;
    }
}

// Function to update prices periodically
void UpdatePricesPeriodically(DatabaseHandler& dbHandler) {
    while (isRunning) {
        try {
            dbHandler.ExecuteProcedure("GenerateRandomPriceFluctuations");
            std::this_thread::sleep_for(std::chrono::minutes(3));
        }
        catch (const std::exception& ex) {
            std::cerr << "Error in price update: " << ex.what() << std::endl;
        }
    }
}
std::string getValidatedString(const std::string& prompt) {
    std::string value;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, value);
        if (!value.empty()) {
            return value;
        }
        else {
            std::cout << "Invalid input. Please enter a non-empty value.\n";
        }
    }
}

void userLandingPage(DatabaseHandler& dbHandler) {
   // Trie trie;

    // Display a welcome message
    std::cout << "Welcome to your dashboard, " << loggedInUserId << "!\n";

    DatabaseHandler handler;
    handler.DisplayAssets();
    // Display user options on the landing page
    int choice;
    do {
        std::cout << "\nWelcome to the Trading Platform!\n";
        std::cout << "===============================================\n";
        std::cout << "Trading Menu:\n";
        std::cout << "1. Purchase an Asset (Buy)\n";
        std::cout << "2. Sell an Existing Asset\n";
        std::cout << "3. View Your Portfolio Summary\n";
        std::cout << "4. Transfer Money to Another User\n";
        std::cout << "5. Transfer Assets to Another User\n";
        std::cout << "6. Convert One Asset to Another\n";
        std::cout << "7. See Top Gaining Assets\n";
        std::cout << "8. See Top Losing Assets\n";
        std::cout << "9. Review Your Order History\n";
        std::cout << "10. Export and Compress User's Data\n";

        std::cout << "12. Exit Application\n";
        std::cout << "===============================================\n";

        choice = getValidatedInt("Enter your choice: ");

        try {
            switch (choice) {
            case 1: {
                // Buy an Asset
                int assetId = getValidatedInt("Enter Asset ID: ");
                int quantity = getValidatedInt("Enter Quantity: ");
                double price = dbHandler.FetchAssetPrice(assetId);
                dbHandler.BuyAsset(dbHandler.GetUserIdByUsername(loggedInUserId), dbHandler.GetAssetNameById(assetId), quantity, price);
                std::cout << "Asset purchased successfully!\n";
                break;
            }

            case 2: {
                // Sell an Asset
                int assetId = getValidatedInt("Enter Asset ID to sell: ");
                int quantity = getValidatedInt("Enter Quantity to sell: ");
                dbHandler.SellAsset(dbHandler.GetUserIdByUsername(loggedInUserId), assetId, quantity);
                std::cout << "Asset sold successfully!\n";
                break;
            }

            case 3: {
                // View Portfolio
                std::cout << "Your Portfolio:\n";
                dbHandler.DisplayPortfolio(dbHandler.GetUserIdByUsername(loggedInUserId));
                break;
            }

            case 4: {
                // Transfer Balance
                dbHandler.TransferBalance(dbHandler.GetUserIdByUsername(loggedInUserId));
                std::cout << "Balance transferred successfully!\n";
                break;
            }

            case 5: {
                // Transfer Assets
                int receiverId = getValidatedInt("Enter Receiver User ID: ");
                int assetId = getValidatedInt("Enter Asset ID to transfer: ");
                int quantity = getValidatedInt("Enter Quantity to transfer: ");
                dbHandler.TransferAsset(dbHandler.GetUserIdByUsername(loggedInUserId), receiverId, assetId, quantity);
                std::cout << "Assets transferred successfully!\n";
                break;
            }

            case 6: {
                // Convert Assets
                int sourceAssetId = getValidatedInt("Enter Source Asset ID: ");
                int targetAssetId = getValidatedInt("Enter Target Asset ID: ");
                int sourceQuantity = getValidatedInt("Enter Quantity of Source Asset to convert: ");
                dbHandler.ConvertAsset(dbHandler.GetUserIdByUsername(loggedInUserId), sourceAssetId, targetAssetId, sourceQuantity);
                std::cout << "Assets converted successfully!\n";
                break;
            }

            case 7: {
                // View Top Gainers
                dbHandler.PrintTopGainer();
                break;
            }

            case 8: {
                // View Top Losers
                dbHandler.PrintTopLosers();
                break;
            }

            case 9: {
                // View Order History
                std::cout << "Your Order History:\n";
                int userId = dbHandler.GetUserIdByUsername(loggedInUserId);
                dbHandler.DisplayOrderHistory(userId);

                // Filter menu options
                char filterChoice;
                std::cout << "\nWould you like to filter the order history? (y/n): ";
                std::cin >> filterChoice;

                if (filterChoice == 'y' || filterChoice == 'Y') {
                    bool exitFiltering = false;

                    while (!exitFiltering) {
                        std::cout << "\nFilter Options:\n";
                        std::cout << "1. Filter Orders by Date Range (e.g., 2025-01-01 to 2025-01-31)\n";
                        std::cout << "2. Filter Orders by Price Range (Specify Minimum and Maximum Prices)\n";


                        std::cout << "3. Exit Filtering\n";

                        int filterOption = getValidatedInt("Enter your choice: ");
                        try {
                            switch (filterOption) {
                            case 1: {
                                // Filter by Date Range
                                std::string startDate = getValidatedString("Enter start date (YYYY-MM-DD): ");
                                std::string endDate = getValidatedString("Enter end date (YYYY-MM-DD): ");
                                dbHandler.FilterOrderHistoryByDate(std::to_string(userId), startDate, endDate);
                                break;
                            }
                            case 2: {
                                // Filter by Price Range
                                double minPrice = getValidatedDouble("Enter minimum price: ");
                                double maxPrice = getValidatedDouble("Enter maximum price: ");
                                dbHandler.FilterOrderHistoryByPrice(std::to_string(userId), minPrice, maxPrice);
                                break;
                            }

                            case 3: {
                                // Exit Filtering
                                exitFiltering = true;
                                break;
                            }
                            default:
                                std::cout << "Invalid choice. Please try again.\n";
                                break;
                            }
                        }
                        catch (const std::exception& ex) {
                            std::cerr << "Error during filtering: " << ex.what() << std::endl;
                        }
                    }
                }
                break;
            }

            case 10: {
                // Export and compress user data
                int userId = dbHandler.GetUserIdByUsername(loggedInUserId);
                if (userId == -1) {
                    std::cerr << "Error: Invalid user ID.\n";
                    break;
                }

                std::cout << "Exporting and compressing user data...\n";
                dbHandler.ExportAndCompressUserData(userId);
                std::cout << "Data exported and compressed successfully!\n";
                break;
            }

            case 12: {
                // Exit
                std::cout << "Exiting the trading system. Goodbye!\n";
                return;
            }

            default:
                std::cout << "Invalid choice. Please select a valid option.\n";
                break;
            }

        }
        catch (const std::exception& ex) {
            std::cerr << "Error during operation: " << ex.what() << std::endl;
        }

        // Pause for user to see messages
        std::cout << "\nPress any key to return to the menu...";
        _getch();

    } while (true);  // Loop until the user chooses to exit
}
