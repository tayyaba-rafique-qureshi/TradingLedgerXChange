

#ifdef _WIN32

#else

#endif
#include"Functions.h"

// Function to clear the console screen


// Function to display the entire screen


// Main function
int main() {
    // Start the animation before the main menu
    std::string name = "LEDGERXTRADE";  // Trading system name
    int width = 120;  // Width of the screen
    int height = 15;  // Height of the screen

    // Create a screen filled with asterisks
    std::vector<std::string> screen(height, std::string(width, '*'));

    // Create templates only once
    auto templates = createLetterTemplates();

    // Center the name vertically and calculate starting column for centering
    int startRow = 5;
    int totalWidth = 0;
    for (char letter : name) {
        if (templates.find(letter) != templates.end()) {
            totalWidth += templates.at(letter)[0].size() + 1;
        }
    }
    int startCol = (width - totalWidth) / 2;

    // Display the name with progressively revealed letters
    revealLetter(name, screen, startRow, startCol, templates);

    // Database handler
    DatabaseHandler dbHandler;

    try {
        // Start the thread for periodic price updates
        std::thread priceUpdater(UpdatePricesPeriodically, std::ref(dbHandler));

        // Main application loop
        while (true) {
            std::cout << "\n";
            std::cout << "************************************************************************************************************************\n";
            std::cout << "*                                                                                                                      *\n";
            std::cout << "*                                          Welcome to LedgerXTrade                                                     *\n";
            std::cout << "*                                                                                                                      *\n";
            std::cout << "************************************************************************************************************************\n";
            std::cout << "\n";
            std::cout << "Please choose an option from the menu below:\n";
            std::cout << "--------------------------------------------\n";
            std::cout << "1. Create a New Account (Register)\n";
            std::cout << "2. Log In to Your Account\n";
            std::cout << "3. Exit the Application\n";
            std::cout << "--------------------------------------------\n";
            

            int choice = getValidatedInt("Enter your choice: ");
            if (choice == 1) {
                registerUser(dbHandler);
            }
            else if (choice == 2) {
                authenticateUser(dbHandler);
                if (isLoggedIn) {
                    userLandingPage(dbHandler);
                    
                }
            }
            else if (choice == 3) {
                std::cout << "Goodbye! :D\n";
                break;
            }
            else {
                std::cout << "Invalid choice. Please try again.\n";
            }
        }

        // Stop the thread safely
        isRunning = false;
        if (priceUpdater.joinable()) {
            priceUpdater.join();
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Critical error: " << ex.what() << std::endl;
        return 1; // Exit with error code
    }

    // Display the animation again before the program exits
    clearScreen();  // Clear the screen to display animation properly
    revealLetter(name, screen, startRow, startCol, templates);  // Show animation on exit

    std::cout << "Exiting the application. Goodbye!" << std::endl;
    return 0;
}
