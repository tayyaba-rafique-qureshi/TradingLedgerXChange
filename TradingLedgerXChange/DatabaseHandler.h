#pragma once
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <sstream>

#include <windows.h> // If you include Windows headers
#include <iostream>
#include <limits> // Include for numeric_limits

#undef max
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <stdexcept>
#include<conio.h>
#include<iomanip>

#include "LinkedList.h" // Assuming Asset is defined here
using namespace std;
#include <vector>
#include <utility>
#include <functional>



class DatabaseHandler {
public:
    SQLHANDLE sqlEnvHandle;
    SQLHANDLE sqlConnHandle;
    SQLHANDLE sqlStmtHandle;
    SQLRETURN retCode;
    std::string connStr = "Driver={ODBC Driver 18 for SQL Server};Server=localhost,1433;Database=DbTrading;Uid=SA;Pwd=YourPassword123;Encrypt=no;TrustServerCertificate=yes;Connection Timeout=30;";
    void checkSuccess(SQLRETURN retCode, const std::string& errorMessage, SQLHANDLE handle = nullptr, SQLSMALLINT handleType = SQL_HANDLE_STMT) {
        if (!SQL_SUCCEEDED(retCode)) {
            if (handle) logSQLError(handle, handleType);
            throw std::runtime_error(errorMessage);
        }
    }
    void logSQLError(SQLHANDLE handle, SQLSMALLINT type) {
        SQLCHAR sqlState[1024];
        SQLCHAR message[1024];
        SQLINTEGER nativeError;
        SQLSMALLINT textLength;

        SQLGetDiagRecA(type, handle, 1, sqlState, &nativeError, message, sizeof(message), &textLength);
        std::cerr << "SQL Error: " << message << " (SQL State: " << sqlState << ")" << std::endl;
    }
public:
    // Fetch user portfolio details
    std::string FetchPortfolioDetails(int userId);

    // Fetch user wallet details
    std::string FetchWalletDetails(int userId);

    // Export and compress user data
    void ExportAndCompressUserData(int userId);
    void DecompressAndLoadUserData(const std::string& compressedFileName, const std::string& decompressedFileName);

public:

    void  BuyAsset(int userId, const std::string& assetName, int quantity, double price);
    void ExecuteProcedure(const std::string& procedureName);
    //  void UpdatePricesPeriodically(DatabaseHandler& dbHandler);
    void PrintTopGainer();
    void DisplayOrderHistory(int userId);
    void PrintTopLosers();
    std::vector<Asset> GetAllAssets(); // Properly declare the function
    // Method to display filtered order results
    void ExecuteQuery(const std::string& query,
        std::function<void(SQLHANDLE)> bindParameters,
        std::function<void(SQLHANDLE)> processResults);
    void FilterOrderHistoryByDate(const std::string& userId, const std::string& startDate, const std::string& endDate);
    //void FilterOrderHistoryByType(const std::string& userId, const std::string& orderType);
    void DisplayOrderResults(SQLHANDLE stmt);
    void FilterOrderHistoryByPrice(const std::string& userId, double minPrice, double maxPrice);
public:
  //  void FilterOrderHistoryByType(const std::string& userId, const std::string& orderType);
    std::string FetchWallet(int userId);
    std::string FetchPortfolio(int userId);
    std::string FetchOrders(int userId);

    std::string FetchUserDetails(int userId);
    std::string FetchOrderHistory(int userId);
    std::string GetUserData(int userId);

    // void ExportAndCompressUserData(int userId);


    DatabaseHandler() : sqlEnvHandle(nullptr), sqlConnHandle(nullptr), sqlStmtHandle(nullptr) {
        // Allocate environment handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvHandle), "Failed to allocate environment handle.");

        // Set ODBC version
        checkSuccess(SQLSetEnvAttr(sqlEnvHandle, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0), "Failed to set ODBC version.");

        // Allocate connection handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvHandle, &sqlConnHandle), "Failed to allocate connection handle.");

        // Connect to the database
        retCode = SQLDriverConnectA(sqlConnHandle, NULL, (SQLCHAR*)connStr.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
        if (!SQL_SUCCEEDED(retCode)) {
            logSQLError(sqlConnHandle, SQL_HANDLE_DBC);
            throw std::runtime_error("Failed to connect to the database.");
        }


    }

    ~DatabaseHandler() {
        if (sqlStmtHandle) SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
        if (sqlConnHandle) {
            SQLDisconnect(sqlConnHandle);
            SQLFreeHandle(SQL_HANDLE_DBC, sqlConnHandle);
        }
        if (sqlEnvHandle) SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvHandle);
    }
    void DisplayPriceChanges() {
        // Allocate statement handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

        // Call the stored procedure
        std::string query = "{CALL GetAssetPriceChanges}";
        checkSuccess(SQLExecDirectA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to execute GetAssetPriceChanges procedure.");

        // Print column headers
        std::cout << "-----------------------------------------------" << std::endl;
        std::cout << "Asset ID | Asset Name      | Price      | % Change" << std::endl;
        std::cout << "-----------------------------------------------" << std::endl;

        // Variables to hold fetched data
        int assetId;
        char assetName[100];
        double assetPrice, percentageChange;

        // Fetch and display data
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &assetId, sizeof(assetId), NULL);
            SQLGetData(sqlStmtHandle, 2, SQL_C_CHAR, assetName, sizeof(assetName), NULL);
            SQLGetData(sqlStmtHandle, 3, SQL_C_DOUBLE, &assetPrice, sizeof(assetPrice), NULL);
            SQLGetData(sqlStmtHandle, 4, SQL_C_DOUBLE, &percentageChange, sizeof(percentageChange), NULL);

            std::cout << assetId << " | " << assetName << " | $" << assetPrice
                << " | " << (percentageChange > 0 ? "+" : "") << percentageChange << "%" << std::endl;
        }

        std::cout << "-----------------------------------------------" << std::endl;

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    }
    void AddOrUpdateAsset(const std::string& assetName, const std::string& assetParentPair, double assetPrice, int assetQuantity) {
        // Allocate statement handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

        std::string query = "{CALL AddOrUpdateAsset(?, ?, ?, ?)}";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare the AddOrUpdateAsset procedure.");

        // Bind parameters
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)assetName.c_str(), 0, NULL), "Failed to bind AssetName.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)assetParentPair.c_str(), 0, NULL), "Failed to bind AssetParentPair.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &assetPrice, 0, NULL), "Failed to bind AssetPrice.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 4, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &assetQuantity, 0, NULL), "Failed to bind AssetQuantity.");

        // Execute the procedure
        retCode = SQLExecute(sqlStmtHandle);
        if (!SQL_SUCCEEDED(retCode)) {
            logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
            throw std::runtime_error("Failed to execute AddOrUpdateAsset procedure.");
        }

        std::cout << "Asset added or updated successfully!" << std::endl;

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    }

    void SellAsset(int userId, int assetId, int quantity) {
        // Allocate statement handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

        // Query to verify the asset and check quantity in Portfolio
        std::string fetchQuery = "SELECT Quantity FROM Portfolio WHERE UserId = ? AND AssetId = ?";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)fetchQuery.c_str(), SQL_NTS), "Failed to prepare SELECT query for Portfolio.");

        // Bind parameters
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &assetId, 0, NULL), "Failed to bind AssetId.");

        // Execute the query
        retCode = SQLExecute(sqlStmtHandle);
        if (!SQL_SUCCEEDED(retCode)) {
            logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
            throw std::runtime_error("Failed to fetch Portfolio data for SellAsset.");
        }

        // Fetch the quantity from Portfolio
        int currentQuantity = 0;
        if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &currentQuantity, sizeof(currentQuantity), NULL);
        }
        else {
            throw std::runtime_error("Asset not found in Portfolio or insufficient quantity.");
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

        // Check if the user has enough quantity to sell
        if (currentQuantity < quantity) {
            throw std::runtime_error("Insufficient quantity to sell.");
        }

        // Execute the SellAsset stored procedure
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for SellAsset.");

        std::string query = "{CALL SellAsset(?, ?, ?)}";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare the SellAsset procedure.");

        // Bind parameters
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &assetId, 0, NULL), "Failed to bind AssetId.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &quantity, 0, NULL), "Failed to bind Quantity.");

        // Execute the procedure
        retCode = SQLExecute(sqlStmtHandle);
        if (!SQL_SUCCEEDED(retCode)) {
            logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
            throw std::runtime_error("Failed to execute SellAsset procedure.");
        }

        std::cout << "Asset sold successfully!" << std::endl;

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    }

    void DisplayAssets() {
        // Allocate statement handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

        std::string query = "SELECT AssetId, AssetName, AssetParentPair, AssetPrice, TotalAssets FROM Assets";
        checkSuccess(SQLExecDirectA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to execute SELECT query on Assets.");

        // Print column headers
        std::cout << "-----------------------------------------------" << std::endl;
        std::cout << "Asset ID | Asset Name | Parent Pair | Price | Total Assets" << std::endl;
        std::cout << "-----------------------------------------------" << std::endl;

        // Variables to hold fetched data
        int assetId, totalAssets;
        char assetName[100], assetParentPair[50];
        double assetPrice;

        // Fetch and print each row
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &assetId, sizeof(assetId), NULL);
            SQLGetData(sqlStmtHandle, 2, SQL_C_CHAR, assetName, sizeof(assetName), NULL);
            SQLGetData(sqlStmtHandle, 3, SQL_C_CHAR, assetParentPair, sizeof(assetParentPair), NULL);
            SQLGetData(sqlStmtHandle, 4, SQL_C_DOUBLE, &assetPrice, sizeof(assetPrice), NULL);
            SQLGetData(sqlStmtHandle, 5, SQL_C_SLONG, &totalAssets, sizeof(totalAssets), NULL);

            std::cout << assetId << " | " << assetName << " | " << assetParentPair
                << " | " << assetPrice << " | " << totalAssets << std::endl;
        }

        std::cout << "-----------------------------------------------" << std::endl;

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    }

    double FetchAssetPrice(int assetId) {
        double price = 0.0;

        // Allocate statement handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

        std::string query = "SELECT AssetPrice FROM Assets WHERE AssetId = ?";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SELECT query for AssetPrice.");

        // Bind the AssetId parameter
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &assetId, 0, NULL), "Failed to bind AssetId.");

        // Execute the query
        retCode = SQLExecute(sqlStmtHandle);
        if (!SQL_SUCCEEDED(retCode)) {
            logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
            throw std::runtime_error("Failed to execute query for AssetPrice.");
        }

        // Fetch the price
        if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_DOUBLE, &price, sizeof(price), NULL);
        }
        else {
            throw std::runtime_error("Asset not found or invalid AssetId.");
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

        return price;
    }

    void DisplayPortfolio(int userId) {
        // Allocate statement handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

        // Updated SQL query to include AssetName
        std::string query = "SELECT p.AssetId, a.AssetName, p.Quantity "
            "FROM Portfolio p "
            "JOIN Assets a ON p.AssetId = a.AssetId "
            "WHERE p.UserId = ?";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SELECT query for Portfolio.");

        // Bind the UserId parameter
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");

        // Execute the query
        checkSuccess(SQLExecute(sqlStmtHandle), "Failed to execute SELECT query for Portfolio.");

        // Define column widths
        const int colWidth1 = 10; // Asset ID width
        const int colWidth2 = 20; // Asset Name width
        const int colWidth3 = 15; // Quantity width

        // Print table header with proper alignment
        std::cout << "+" << std::string(colWidth1 + 2, '-') << "+"
            << std::string(colWidth2 + 2, '-') << "+"
            << std::string(colWidth3 + 2, '-') << "+" << std::endl;
        std::cout << "| " << std::left << std::setw(colWidth1) << "Asset ID"
            << "| " << std::setw(colWidth2) << "Asset Name"
            << "| " << std::setw(colWidth3) << "Quantity"
            << "|" << std::endl;
        std::cout << "+" << std::string(colWidth1 + 2, '-') << "+"
            << std::string(colWidth2 + 2, '-') << "+"
            << std::string(colWidth3 + 2, '-') << "+" << std::endl;

        // Fetch and print each row
        int assetId, quantity;
        char assetName[100];
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &assetId, sizeof(assetId), NULL);
            SQLGetData(sqlStmtHandle, 2, SQL_C_CHAR, assetName, sizeof(assetName), NULL);
            SQLGetData(sqlStmtHandle, 3, SQL_C_SLONG, &quantity, sizeof(quantity), NULL);

            // Print each row with proper alignment
            std::cout << "| " << std::left << std::setw(colWidth1) << assetId
                << "| " << std::setw(colWidth2) << assetName
                << "| " << std::setw(colWidth3) << quantity
                << "|" << std::endl;
        }

        // Print closing line
        std::cout << "+" << std::string(colWidth1 + 2, '-') << "+"
            << std::string(colWidth2 + 2, '-') << "+"
            << std::string(colWidth3 + 2, '-') << "+" << std::endl;

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    }
    std::string GetAssetNameById(int assetId) {
        std::string assetName;

        // Allocate statement handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

        // Query to fetch the Asset Name based on Asset ID
        std::string query = "SELECT AssetName FROM Assets WHERE AssetId = ?";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SELECT query for AssetName.");

        // Bind the AssetId parameter
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &assetId, 0, NULL), "Failed to bind AssetId.");

        // Execute the query
        retCode = SQLExecute(sqlStmtHandle);
        if (!SQL_SUCCEEDED(retCode)) {
            logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
            throw std::runtime_error("Failed to execute query for AssetName.");
        }

        // Fetch the Asset Name
        char nameBuffer[100];
        if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_CHAR, nameBuffer, sizeof(nameBuffer), NULL);
            assetName = std::string(nameBuffer);
        }
        else {
            throw std::runtime_error("Asset not found for the given AssetId.");
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

        return assetName;
    }
    double GetUserBalance(int userId) {
        double balance = 0.0;

        // Allocate statement handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

        std::string query = "SELECT Balance FROM Wallet WHERE UserId = ?";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SELECT query for Balance.");

        // Bind the UserId parameter
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");

        // Execute the query
        retCode = SQLExecute(sqlStmtHandle);
        if (!SQL_SUCCEEDED(retCode)) {
            logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
            throw std::runtime_error("Failed to fetch user balance.");
        }

        // Fetch the balance
        if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_DOUBLE, &balance, sizeof(balance), NULL);
        }
        else {
            throw std::runtime_error("User balance not found.");
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

        return balance;
    }
    double ValidateAmountInput() {
        double amount;
        while (true) {
            std::cout << "Enter the amount to transfer: ";
            std::cin >> amount;

            // Check if the input is valid
            if (std::cin.fail()) {
                std::cin.clear(); // Clear the error flag
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
                std::cerr << "Invalid input. Please enter a valid numeric value.\n";
            }
            else if (amount <= 0) {
                std::cerr << "Error: Amount must be greater than zero. Please try again.\n";
            }
            else {
                return amount; // Valid input
            }
        }
    }
    void TransferBalance(int senderId) {
        // Step 1: Get Sender's current balance
        double senderBalance = GetUserBalance(senderId);
        std::cout << "Your current balance: $" << senderBalance << std::endl;

        // Step 2: Ask user if they want to send money or return
        int choice;
        std::cout << "\nWhat would you like to do?\n";
        std::cout << "1. Send money to another user\n";
        std::cout << "2. Go back to the main menu\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;

        if (choice == 1) {
            // Proceed with sending money
            int receiverId;
            std::cout << "\nEnter the Receiver User ID: ";
            std::cin >> receiverId;

            // Validate the amount input
            double amount = ValidateAmountInput();

            // Check if the sender has enough balance
            if (amount > senderBalance) {
                std::cerr << "Error: Insufficient balance to complete the transfer.\n";
                return;
            }

            // Step 3: Get WalletIds for Sender and Receiver
            int senderWalletId = GetWalletId(senderId);
            int receiverWalletId = GetWalletId(receiverId);

            // Step 4: Insert Transaction Log for Sender
            checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for logging Sender transaction.");

            std::string logQuery = "INSERT INTO TransactionLogs (WalletId, TransactionType, Amount, TransactionDate) VALUES (?, 'Transfer', ?, GETDATE())";
            checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)logQuery.c_str(), SQL_NTS), "Failed to prepare INSERT query for Sender transaction.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &senderWalletId, 0, NULL), "Failed to bind Sender WalletId.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &amount, 0, NULL), "Failed to bind Amount for Sender.");

            retCode = SQLExecute(sqlStmtHandle);
            if (!SQL_SUCCEEDED(retCode)) {
                logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
                throw std::runtime_error("Failed to log transaction for Sender.");
            }

            // Free the statement handle
            SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

            // Step 5: Insert Transaction Log for Receiver
            checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for logging Receiver transaction.");

            checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)logQuery.c_str(), SQL_NTS), "Failed to prepare INSERT query for Receiver transaction.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &receiverWalletId, 0, NULL), "Failed to bind Receiver WalletId.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DECIMAL, 18, 2, &amount, 0, NULL), "Failed to bind Amount for Receiver.");

            retCode = SQLExecute(sqlStmtHandle);
            if (!SQL_SUCCEEDED(retCode)) {
                logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
                throw std::runtime_error("Failed to log transaction for Receiver.");
            }

            // Free the statement handle
            SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

            // Print only one success message
            std::cout << "Funds transferred successfully from User " << senderId << " to User " << receiverId << "!\n";
        }
        else if (choice == 2) {
            std::cout << "Returning to the main menu...\n";
        }
        else {
            std::cout << "Invalid choice. Returning to the main menu...\n";
        }
    }
    int GetWalletId(int userId) {
        int walletId = 0;

        // Allocate statement handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

        std::string query = "SELECT WalletId FROM Wallet WHERE UserId = ?";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SELECT query for WalletId.");

        // Bind the UserId parameter
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");

        // Execute the query
        retCode = SQLExecute(sqlStmtHandle);
        if (!SQL_SUCCEEDED(retCode)) {
            logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
            throw std::runtime_error("Failed to fetch WalletId.");
        }

        // Fetch the WalletId
        if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &walletId, sizeof(walletId), NULL);
        }
        else {
            throw std::runtime_error("Wallet not found for the given UserId.");
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

        return walletId;
    }
    void TransferAsset(int senderId, int receiverId, int assetId, int quantity) {
        // Step 1: Check sender's asset quantity
        int senderQuantity = 0;

        // Allocate statement handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

        std::string checkQuery = "SELECT Quantity FROM Portfolio WHERE UserId = ? AND AssetId = ?";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)checkQuery.c_str(), SQL_NTS), "Failed to prepare SELECT query for sender's asset quantity.");

        // Bind parameters for sender
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &senderId, 0, NULL), "Failed to bind SenderId.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &assetId, 0, NULL), "Failed to bind AssetId.");

        // Execute the query
        retCode = SQLExecute(sqlStmtHandle);
        if (!SQL_SUCCEEDED(retCode)) {
            logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
            throw std::runtime_error("Failed to fetch sender's asset quantity.");
        }

        if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &senderQuantity, sizeof(senderQuantity), NULL);
        }
        else {
            throw std::runtime_error("Sender does not have the specified asset or quantity.");
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

        // Ensure sender has sufficient quantity to transfer
        if (senderQuantity < quantity) {
            throw std::runtime_error("Sender does not have sufficient quantity to transfer.");
        }

        // Step 2: Deduct asset from sender's portfolio
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for updating sender's portfolio.");

        std::string deductQuery = "UPDATE Portfolio SET Quantity = Quantity - ? WHERE UserId = ? AND AssetId = ?";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)deductQuery.c_str(), SQL_NTS), "Failed to prepare UPDATE query for sender's portfolio.");

        // Bind parameters
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &quantity, 0, NULL), "Failed to bind Quantity.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &senderId, 0, NULL), "Failed to bind SenderId.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &assetId, 0, NULL), "Failed to bind AssetId.");

        retCode = SQLExecute(sqlStmtHandle);
        if (!SQL_SUCCEEDED(retCode)) {
            logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
            throw std::runtime_error("Failed to deduct asset from sender's portfolio.");
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

        // Step 3: Add asset to receiver's portfolio
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for updating receiver's portfolio.");

        // Check if the receiver already has the asset
        std::string checkReceiverQuery = "SELECT Quantity FROM Portfolio WHERE UserId = ? AND AssetId = ?";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)checkReceiverQuery.c_str(), SQL_NTS), "Failed to prepare SELECT query for receiver's portfolio.");

        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &receiverId, 0, NULL), "Failed to bind ReceiverId.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &assetId, 0, NULL), "Failed to bind AssetId.");

        retCode = SQLExecute(sqlStmtHandle);
        int receiverQuantity = 0;

        if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &receiverQuantity, sizeof(receiverQuantity), NULL);
            // Update receiver's portfolio if the asset already exists
            std::string updateQuery = "UPDATE Portfolio SET Quantity = Quantity + ? WHERE UserId = ? AND AssetId = ?";
            SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
            checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for updating receiver's portfolio.");
            checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)updateQuery.c_str(), SQL_NTS), "Failed to prepare UPDATE query for receiver's portfolio.");

            // Bind parameters
            checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &quantity, 0, NULL), "Failed to bind Quantity.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &receiverId, 0, NULL), "Failed to bind ReceiverId.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &assetId, 0, NULL), "Failed to bind AssetId.");

            retCode = SQLExecute(sqlStmtHandle);
            if (!SQL_SUCCEEDED(retCode)) {
                logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
                throw std::runtime_error("Failed to update receiver's portfolio.");
            }
        }
        else {
            // Insert new record if the asset does not exist in the receiver's portfolio
            std::string insertQuery = "INSERT INTO Portfolio (UserId, AssetId, Quantity, LastUpdated) VALUES (?, ?, ?, GETDATE())";
            SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
            checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for inserting receiver's portfolio.");
            checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)insertQuery.c_str(), SQL_NTS), "Failed to prepare INSERT query for receiver's portfolio.");

            // Bind parameters
            checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &receiverId, 0, NULL), "Failed to bind ReceiverId.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &assetId, 0, NULL), "Failed to bind AssetId.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &quantity, 0, NULL), "Failed to bind Quantity.");

            retCode = SQLExecute(sqlStmtHandle);
            if (!SQL_SUCCEEDED(retCode)) {
                logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
                throw std::runtime_error("Failed to insert asset into receiver's portfolio.");
            }
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

        std::cout << "Asset successfully transferred from User " << senderId << " to User " << receiverId << "!" << std::endl;
    }
    void ConvertAsset(int userId, int sourceAssetId, int targetAssetId, int sourceQuantity) {
        // Step 1: Fetch prices of source and target assets
        double sourcePrice = FetchAssetPrice(sourceAssetId);
        double targetPrice = FetchAssetPrice(targetAssetId);

        // Calculate the equivalent quantity of the target asset
        double conversionRate = sourcePrice / targetPrice;
        int targetQuantity = static_cast<int>(sourceQuantity * conversionRate);

        // Step 2: Check the user's portfolio for sufficient source asset quantity
        int currentSourceQuantity = 0;

        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

        std::string checkQuery = "SELECT Quantity FROM Portfolio WHERE UserId = ? AND AssetId = ?";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)checkQuery.c_str(), SQL_NTS), "Failed to prepare SELECT query for source asset quantity.");

        // Bind parameters for source asset check
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &sourceAssetId, 0, NULL), "Failed to bind SourceAssetId.");

        retCode = SQLExecute(sqlStmtHandle);
        if (!SQL_SUCCEEDED(retCode)) {
            logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
            throw std::runtime_error("Failed to fetch source asset quantity.");
        }

        if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &currentSourceQuantity, sizeof(currentSourceQuantity), NULL);
        }
        else {
            throw std::runtime_error("Source asset not found in user's portfolio.");
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

        // Check if user has enough quantity to convert
        if (currentSourceQuantity < sourceQuantity) {
            throw std::runtime_error("Insufficient quantity of source asset.");
        }

        // Step 3: Deduct source asset from portfolio
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for source asset deduction.");

        std::string deductQuery = "UPDATE Portfolio SET Quantity = Quantity - ? WHERE UserId = ? AND AssetId = ?";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)deductQuery.c_str(), SQL_NTS), "Failed to prepare UPDATE query for source asset deduction.");

        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &sourceQuantity, 0, NULL), "Failed to bind SourceQuantity.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &sourceAssetId, 0, NULL), "Failed to bind SourceAssetId.");

        retCode = SQLExecute(sqlStmtHandle);
        if (!SQL_SUCCEEDED(retCode)) {
            logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
            throw std::runtime_error("Failed to deduct source asset from portfolio.");
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

        // Step 4: Add target asset to portfolio
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for target asset update.");

        // Check if the target asset already exists in the portfolio
        std::string checkTargetQuery = "SELECT Quantity FROM Portfolio WHERE UserId = ? AND AssetId = ?";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)checkTargetQuery.c_str(), SQL_NTS), "Failed to prepare SELECT query for target asset.");

        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &targetAssetId, 0, NULL), "Failed to bind TargetAssetId.");

        retCode = SQLExecute(sqlStmtHandle);
        int targetAssetQuantity = 0;

        if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &targetAssetQuantity, sizeof(targetAssetQuantity), NULL);

            // Update target asset quantity if it exists
            std::string updateTargetQuery = "UPDATE Portfolio SET Quantity = Quantity + ? WHERE UserId = ? AND AssetId = ?";
            SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

            checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for target asset update.");
            checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)updateTargetQuery.c_str(), SQL_NTS), "Failed to prepare UPDATE query for target asset.");

            checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &targetQuantity, 0, NULL), "Failed to bind TargetQuantity.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &targetAssetId, 0, NULL), "Failed to bind TargetAssetId.");

            retCode = SQLExecute(sqlStmtHandle);
            if (!SQL_SUCCEEDED(retCode)) {
                logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
                throw std::runtime_error("Failed to update target asset quantity in portfolio.");
            }
        }
        else {
            // Insert target asset if it does not exist
            std::string insertTargetQuery = "INSERT INTO Portfolio (UserId, AssetId, Quantity, LastUpdated) VALUES (?, ?, ?, GETDATE())";
            SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

            checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for target asset insert.");
            checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)insertTargetQuery.c_str(), SQL_NTS), "Failed to prepare INSERT query for target asset.");

            checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &targetAssetId, 0, NULL), "Failed to bind TargetAssetId.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &targetQuantity, 0, NULL), "Failed to bind TargetQuantity.");

            retCode = SQLExecute(sqlStmtHandle);
            if (!SQL_SUCCEEDED(retCode)) {
                logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
                throw std::runtime_error("Failed to insert target asset into portfolio.");
            }
        }

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

        std::cout << "Successfully converted " << sourceQuantity << " units of Asset " << sourceAssetId
            << " to " << targetQuantity << " units of Asset " << targetAssetId << "!\n";
    }
    bool AuthenticateUser(const std::string& username, const std::string& hashedPassword) {
        try {
            // Allocate statement handle
            checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

            // SQL query to authenticate user
            std::string query = "SELECT UserId FROM Users WHERE UserName = ? AND HashedPassword = ?";
            checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SELECT query for authentication.");

            // Bind parameters
            checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)username.c_str(), 0, NULL), "Failed to bind UserName.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLCHAR*)hashedPassword.c_str(), 0, NULL), "Failed to bind HashedPassword.");

            // Execute the query
            retCode = SQLExecute(sqlStmtHandle);
            if (!SQL_SUCCEEDED(retCode)) {
                logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
                throw std::runtime_error("Failed to execute query for authentication.");
            }

            // Check if a row is fetched (valid credentials)
            if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
                std::cout << "Authentication successful!" << std::endl;
                SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
                return true;
            }
            else {
                std::cout << "Authentication failed. Invalid username or password." << std::endl;
            }

            // Free the statement handle
            SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
        }
        catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }

        return false;
    }
    bool RegisterUser(const std::string& username, const std::string& email, const std::string& password) {
        try {
            // Generate salt
            std::string salt = generateSalt();

            // Hash the password with the salt
            std::string hashedPassword = hashPassword(password, salt);

            // Allocate statement handle
            checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

            // SQL query to register a new user with salt and hashed password
            std::string query = "INSERT INTO Users (UserName, Email, HashedPassword, Salt, IsVerified, DateCreated) VALUES (?, ?, ?, ?, 0, GETDATE())";
            checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare INSERT query for user registration.");

            // Bind parameters
            checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)username.c_str(), 0, NULL), "Failed to bind UserName.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)email.c_str(), 0, NULL), "Failed to bind Email.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 255, 0, (SQLCHAR*)hashedPassword.c_str(), 0, NULL), "Failed to bind HashedPassword.");
            checkSuccess(SQLBindParameter(sqlStmtHandle, 4, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 64, 0, (SQLCHAR*)salt.c_str(), 0, NULL), "Failed to bind Salt.");

            // Execute the query
            retCode = SQLExecute(sqlStmtHandle);
            if (!SQL_SUCCEEDED(retCode)) {
                logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
                throw std::runtime_error("Failed to execute query for user registration.");
            }

            // Retrieve the UserId of the newly registered user
            int userId = GetUserIdByUsername(username);

            // Assign default balance to the user's wallet
            std::string walletQuery = "INSERT INTO Wallet (UserId, Balance) VALUES (?, 1000000)";
            SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
            checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for wallet.");
            checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)walletQuery.c_str(), SQL_NTS), "Failed to prepare INSERT query for wallet.");

            // Bind UserId parameter
            checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId for wallet.");

            // Execute the wallet query
            retCode = SQLExecute(sqlStmtHandle);
            if (!SQL_SUCCEEDED(retCode)) {
                logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
                throw std::runtime_error("Failed to assign default balance to wallet.");
            }

            // Free the statement handle
            SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

            return true;
        }
        catch (const std::exception& ex) {
            // Ensure handle cleanup before rethrowing
            SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
            std::cerr << "Error during registration: " << ex.what() << std::endl;
            return false;
        }
    }
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
    std::string GetSaltByUsername(const std::string& username) {
        std::string salt;
        SQLHSTMT stmt = nullptr;

        try {
            checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &stmt), "Failed to allocate statement handle.");
            std::string query = "SELECT Salt FROM Users WHERE UserName = ?";
            checkSuccess(SQLPrepareA(stmt, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SELECT query for Salt.");
            checkSuccess(SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)username.c_str(), 0, NULL), "Failed to bind username.");

            retCode = SQLExecute(stmt);
            if (SQL_SUCCEEDED(retCode) && SQLFetch(stmt) == SQL_SUCCESS) {
                char buffer[64];
                SQLGetData(stmt, 1, SQL_C_CHAR, buffer, sizeof(buffer), nullptr);
                salt = buffer;
            }
            SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        }
        catch (...) {
            if (stmt) SQLFreeHandle(SQL_HANDLE_STMT, stmt);
            throw;
        }
        return salt;
    }
    int GetUserIdByUsername(const std::string& username) {
        SQLHSTMT sqlStmtHandle = nullptr;  // Statement handle
        SQLRETURN retCode;
        int userId = 0;

        try {
            // Allocate statement handle
            checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

            // SQL query to fetch UserId
            std::string query = "SELECT UserId FROM Users WHERE UserName = ?";
            checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SELECT query for UserId.");

            // Bind parameters
            checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 50, 0, (SQLCHAR*)username.c_str(), 0, NULL), "Failed to bind UserName.");

            // Execute the query
            retCode = SQLExecute(sqlStmtHandle);
            if (!SQL_SUCCEEDED(retCode)) {
                logSQLError(sqlStmtHandle, SQL_HANDLE_STMT);
                throw std::runtime_error("Failed to execute query for UserId.");
            }

            // Fetch the result
            if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
                SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &userId, sizeof(userId), NULL);
            }
            else {
                throw std::runtime_error("User not found for the given username.");
            }

            // Free the statement handle
            SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

            return userId;
        }
        catch (const std::exception& ex) {
            if (sqlStmtHandle) {
                SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
            }
            std::cerr << "Error: " << ex.what() << std::endl;
            return -1;  // Return an invalid user ID if an error occurs
        }
    }
};