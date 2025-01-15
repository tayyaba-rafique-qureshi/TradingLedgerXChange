
#include "DatabaseHandler.h"
#include <exception>
#include<vector>
#include"LinkedList.h"
#include <stdexcept>
#include <iostream>
#include "HuffmanCompressor.h" 
#include <functional>
#include <sstream>
#include <sql.h>

#include <sqlext.h>

void DatabaseHandler::ExecuteProcedure(const std::string& procedureName) {
    SQLHANDLE stmtHandle;
    checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &stmtHandle), "Failed to allocate statement handle.");

    try {
        std::string query = "{CALL " + procedureName + "}";
        checkSuccess(SQLExecDirectA(stmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to execute procedure: " + procedureName);

        //  std::cout << "Procedure " << procedureName << " executed successfully!" << std::endl;
    }
    catch (const std::exception& ex) {
        SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
        throw;
    }


    SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
}


void DatabaseHandler::BuyAsset(int userId, const std::string& assetName, int quantity, double price) {
    SQLHANDLE sqlStmtHandle = nullptr;

    try {
        // Allocate statement handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for BuyAsset.");

        // Prepare the BuyAsset procedure call
        std::string query = "{CALL BuyAsset(?, ?, ?, ?)}";
        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare the BuyAsset procedure.");

        // Bind parameters
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, nullptr), "Failed to bind UserId.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 100, 0, (SQLCHAR*)assetName.c_str(), 0, nullptr), "Failed to bind AssetName.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &quantity, 0, nullptr), "Failed to bind Quantity.");
        checkSuccess(SQLBindParameter(sqlStmtHandle, 4, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, &price, 0, nullptr), "Failed to bind Price.");

        // Execute the stored procedure
        checkSuccess(SQLExecute(sqlStmtHandle), "Failed to execute BuyAsset procedure.");

        std::cout << "BuyAsset procedure executed successfully." << std::endl;
    }
    catch (const std::exception& ex) {
        // Handle exceptions and log errors
        std::cerr << "Error in BuyAsset: " << ex.what() << std::endl;
        throw; // Rethrow to allow the caller to handle the error
    }

    // Ensure the statement handle is freed in both success and failure cases
    if (sqlStmtHandle != SQL_NULL_HANDLE) {
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    }
}


// Helper function for SQL Error Logging (improved)
void logSQLError(SQLHANDLE handle, SQLSMALLINT type, const std::string& message) {
    SQLCHAR sqlState[6], errorMessage[SQL_MAX_MESSAGE_LENGTH];
    SQLINTEGER nativeError;
    SQLSMALLINT textLength;

    SQLRETURN ret = SQLGetDiagRecA(type, handle, 1, sqlState, &nativeError, errorMessage, SQL_MAX_MESSAGE_LENGTH, &textLength);
    if (ret == SQL_SUCCESS) {
        std::cerr << message << " | SQL State: " << sqlState << " | Native Error: " << nativeError << " | Message: " << errorMessage << std::endl;
    }
    else if (ret == SQL_NO_DATA) {
        std::cerr << message << " | No Data Error" << std::endl;
    }
    else {
        std::cerr << message << " | Unable to fetch detailed error message." << std::endl;
    }
}


// Improved checkSuccess function (now takes handle type)
void checkSuccess(SQLRETURN ret, const std::string& message, SQLHANDLE handle, SQLSMALLINT handleType) {
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) { // Check for both SUCCESS and SUCCESS_WITH_INFO
        logSQLError(handle, handleType, message);
        std::ostringstream errorStream;
        errorStream << message << " SQL Return Code: " << ret;
        throw std::runtime_error(errorStream.str());
    }
}

#include <string>
#include <algorithm> // for std::min

#include <iomanip> // For std::put_time

void DatabaseHandler::FilterOrderHistoryByDate(const std::string& userId, const std::string& startDate, const std::string& endDate) {
    ExecuteQuery(
        "{CALL FilterOrdersByDate(?, ?, ?)}",
        [&](SQLHANDLE stmt) {
            SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, userId.size(), 0, (SQLPOINTER)userId.c_str(), 0, nullptr);
            SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, startDate.size(), 0, (SQLPOINTER)startDate.c_str(), 0, nullptr);
            SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, endDate.size(), 0, (SQLPOINTER)endDate.c_str(), 0, nullptr);
        },
        [&](SQLHANDLE stmt) { DisplayOrderResults(stmt); });
}

void DatabaseHandler::FilterOrderHistoryByPrice(const std::string& userId, double minPrice, double maxPrice) {
    ExecuteQuery(
        "{CALL FilterOrdersByPrice(?, ?, ?)}",
        [&](SQLHANDLE stmt) {
            SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, userId.size(), 0, (SQLPOINTER)userId.c_str(), 0, nullptr);
            SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, (SQLPOINTER)&minPrice, 0, nullptr);
            SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, (SQLPOINTER)&maxPrice, 0, nullptr);
        },
        [&](SQLHANDLE stmt) { DisplayOrderResults(stmt); });
}

void DatabaseHandler::DisplayOrderResults(SQLHANDLE stmt) {
    try {
        std::cout << "Displaying results...\n";

        // Declare variables to hold data
        SQLINTEGER orderId = 0;
        SQLINTEGER assetId = 0;
        SQLDOUBLE price = 0.0;
        SQL_TIMESTAMP_STRUCT transactionDate;
        SQLLEN transactionDateLen = 0, priceLen = 0;

        // Fetch and display rows
        while (SQLFetch(stmt) == SQL_SUCCESS) {
            // Fetch data for each column
            SQLGetData(stmt, 1, SQL_C_SLONG, &orderId, sizeof(orderId), nullptr);       // Order ID
            SQLGetData(stmt, 2, SQL_C_SLONG, &assetId, sizeof(assetId), nullptr);      // Asset ID
            SQLGetData(stmt, 3, SQL_C_DOUBLE, &price, sizeof(price), &priceLen);       // Price
            SQLGetData(stmt, 4, SQL_C_TYPE_TIMESTAMP, &transactionDate, sizeof(transactionDate), &transactionDateLen); // Transaction Date

            // Handle null price values
            std::string priceStr = (priceLen == SQL_NULL_DATA) ? "N/A" : std::to_string(price);

            // Format the transaction date
            std::string formattedDate = (transactionDateLen == SQL_NULL_DATA)
                ? "N/A"
                : [&]() {
                std::tm tm{};
                tm.tm_year = transactionDate.year - 1900;
                tm.tm_mon = transactionDate.month - 1;
                tm.tm_mday = transactionDate.day;
                tm.tm_hour = transactionDate.hour;
                tm.tm_min = transactionDate.minute;
                tm.tm_sec = transactionDate.second;
                std::ostringstream dateStream;
                dateStream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
                return dateStream.str();
                }();

            // Print the result row
            std::cout << "Order ID: " << orderId
                << " | Asset ID: " << assetId
                << " | Price: $" << priceStr
                << " | Date: " << formattedDate << "\n";
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Error during DisplayOrderResults: " << ex.what() << std::endl;
    }

    std::cout << "Results displayed.\n";
}


std::string DatabaseHandler::FetchOrderHistory(int userId) {
    std::ostringstream orderHistory;
    SQLHANDLE stmtHandle;

    // Allocate statement handle
    checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &stmtHandle),
        "Failed to allocate statement handle for order history.", sqlConnHandle, SQL_HANDLE_DBC);

    try {
        // SQL query
        const std::string query = "SELECT O.OrderId, A.AssetName, O.Quantity, O.Price, O.TransactionDate, O.OrderType "
            "FROM Orders O INNER JOIN Assets A ON O.AssetId = A.AssetId WHERE O.UserId = ?";

        // Prepare the statement
        checkSuccess(SQLPrepareA(stmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS),
            "Failed to prepare SQL statement.", stmtHandle, SQL_HANDLE_STMT);

        // Bind the userId parameter
        checkSuccess(SQLBindParameter(stmtHandle, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userId, 0, nullptr),
            "Failed to bind parameter.", stmtHandle, SQL_HANDLE_STMT);

        // Execute the query
        checkSuccess(SQLExecute(stmtHandle), "Failed to execute SQL statement.", stmtHandle, SQL_HANDLE_STMT);

        // Define buffers for result columns
        SQLINTEGER orderId, quantity;
        SQLDOUBLE price;
        SQL_TIMESTAMP_STRUCT transactionDate;
        SQLCHAR assetName[256] = { 0 }, orderType[256] = { 0 };
        SQLLEN assetNameLen = 0, orderTypeLen = 0;

        // Fetch rows and format the results
        orderHistory << "Order History:\n";
        while (SQLFetch(stmtHandle) == SQL_SUCCESS) {
            // Fetch data for each column
            SQLGetData(stmtHandle, 1, SQL_C_LONG, &orderId, 0, nullptr);
            SQLGetData(stmtHandle, 2, SQL_C_CHAR, assetName, sizeof(assetName), &assetNameLen);
            SQLGetData(stmtHandle, 3, SQL_C_LONG, &quantity, 0, nullptr);
            SQLGetData(stmtHandle, 4, SQL_C_DOUBLE, &price, 0, nullptr);
            SQLGetData(stmtHandle, 5, SQL_C_TYPE_TIMESTAMP, &transactionDate, 0, nullptr);
            SQLGetData(stmtHandle, 6, SQL_C_CHAR, orderType, sizeof(orderType), &orderTypeLen);

            // Format the transaction date
            std::string formattedDate = (transactionDate.year == 0) ? "N/A" : [&]() {
                std::tm tm{};
                tm.tm_year = transactionDate.year - 1900;
                tm.tm_mon = transactionDate.month - 1;
                tm.tm_mday = transactionDate.day;
                tm.tm_hour = transactionDate.hour;
                tm.tm_min = transactionDate.minute;
                tm.tm_sec = transactionDate.second;
                std::ostringstream dateStream;
                dateStream << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
                return dateStream.str();
                }();

            // Format order type as a string
            std::string typeStr = (orderTypeLen == SQL_NULL_DATA) ? "N/A" : reinterpret_cast<char*>(orderType);

            // Append the fetched data to the output stream
            orderHistory << "Order ID: " << orderId
                << ", Asset: " << ((assetNameLen == SQL_NULL_DATA) ? "N/A" : reinterpret_cast<char*>(assetName))
                << ", Quantity: " << quantity
                << ", Price: $" << price
                << ", Date: " << formattedDate
                << ", Type: " << typeStr
                << "\n";
        }
    }
    catch (const std::exception& ex) {
        // Ensure statement handle is freed in case of an error
        SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
        throw std::runtime_error("Error in FetchOrderHistory: " + std::string(ex.what()));
    }

    // Free the statement handle after use
    SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
    return orderHistory.str();
}


//// Example usage in a DatabaseHandler method (FetchOrderHistory)
//std::string DatabaseHandler::FetchOrderHistory(int userId) {
//    std::ostringstream orderHistory;
//    SQLHANDLE stmtHandle;
//
//    checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &stmtHandle), "Failed to allocate statement handle for order history.", sqlConnHandle, SQL_HANDLE_DBC);
//
//    try {
//        const std::string query = "SELECT O.OrderId, A.AssetName, O.Quantity, O.Price, O.TransactionDate, O.OrderType "
//            "FROM Orders O INNER JOIN Assets A ON O.AssetId = A.AssetId WHERE O.UserId = ?";
//
//        checkSuccess(SQLPrepareA(stmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SQL statement.", stmtHandle, SQL_HANDLE_STMT);
//        checkSuccess(SQLBindParameter(stmtHandle, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userId, 0, nullptr), "Failed to bind parameter.", stmtHandle, SQL_HANDLE_STMT);
//        checkSuccess(SQLExecute(stmtHandle), "Failed to execute SQL statement.", stmtHandle, SQL_HANDLE_STMT);
//
//        // ... (rest of the data retrieval and formatting code as in my previous corrected answer)
//
//    }
//    catch (...) { // Catch any exceptions
//        SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle); //Crucially free the handle here in the catch block
//        throw; // Re-throw the exception after cleanup
//    }
//
//    SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle); // Free if no exception
//
//    return orderHistory.str();
//}


void DatabaseHandler::PrintTopGainer() {
    PriorityQueue pq;

    // Allocate statement handle
    checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

    // SQL query to fetch asset details
    std::string query = "SELECT AssetId, AssetName, AssetPrice FROM Assets";
    checkSuccess(SQLExecDirectA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to execute query for Top Gainer.");

    // Fetch asset data and insert into the priority queue
    int assetId;
    char assetName[100];
    double price;

    while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
        SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &assetId, sizeof(assetId), NULL);
        SQLGetData(sqlStmtHandle, 2, SQL_C_CHAR, assetName, sizeof(assetName), NULL);
        SQLGetData(sqlStmtHandle, 3, SQL_C_DOUBLE, &price, sizeof(price), NULL);

        // Placeholder for date (you may update this logic)
        std::string currentDate = "2024-12-27"; // Example date

        pq.insert(Asset(assetId, std::string(assetName), price, currentDate));
    }

    // Display the priority queue
    pq.display();

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
}

void DatabaseHandler::PrintTopLosers() {
    PriorityQueue pq;

    // Allocate statement handle
    checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

    // SQL query to fetch asset details
    std::string query = "SELECT AssetId, AssetName, AssetPrice FROM Assets";
    checkSuccess(SQLExecDirectA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to execute query for Top Losers.");

    // Fetch asset data and insert into the priority queue
    int assetId;
    char assetName[100];
    double price;

    while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
        SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &assetId, sizeof(assetId), NULL);
        SQLGetData(sqlStmtHandle, 2, SQL_C_CHAR, assetName, sizeof(assetName), NULL);
        SQLGetData(sqlStmtHandle, 3, SQL_C_DOUBLE, &price, sizeof(price), NULL);

        // Placeholder for date
        std::string currentDate = "2024-12-27"; // Example date

        pq.insert(Asset(assetId, std::string(assetName), price, currentDate));
    }

    // Display the priority queue in reverse order
    pq.displayReverse();

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
}

void DatabaseHandler::DisplayOrderHistory(int userId) {
    try {
        // Allocate statement handle
        checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle.");

        // Query to fetch order history for the user
        std::string query = "SELECT T.AssetId, A.AssetName, T.Quantity, T.TradePrice, T.TradeDate "
            "FROM TradeHistory T "
            "INNER JOIN Assets A ON T.AssetId = A.AssetId "
            "WHERE T.UserId = ? ORDER BY T.TradeDate DESC";

        checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SELECT query for order history.");

        // Bind userId parameter
        checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");

        // Execute the query
        checkSuccess(SQLExecute(sqlStmtHandle), "Failed to execute query for order history.");

        // Stack to store the order history
        PriorityQueue orderStack;

        // Variables to hold fetched data
        int assetId, quantity;
        char assetName[100];
        double tradePrice;
        char tradeDate[20];

        // Fetch and push data onto the stack
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &assetId, sizeof(assetId), NULL);
            SQLGetData(sqlStmtHandle, 2, SQL_C_CHAR, assetName, sizeof(assetName), NULL);
            SQLGetData(sqlStmtHandle, 3, SQL_C_SLONG, &quantity, sizeof(quantity), NULL);
            SQLGetData(sqlStmtHandle, 4, SQL_C_DOUBLE, &tradePrice, sizeof(tradePrice), NULL);
            SQLGetData(sqlStmtHandle, 5, SQL_C_CHAR, tradeDate, sizeof(tradeDate), NULL);

            // Push data into the stack
            orderStack.push(Asset(assetId, std::string(assetName), tradePrice, std::string(tradeDate)));
        }

        // Display the order history stack
        std::cout << "Order History for User ID: " << userId << "\n";
        orderStack.displayStack();

        // Free the statement handle
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}
std::vector<Asset> DatabaseHandler::GetAllAssets() {
    std::vector<Asset> assets;

    // Retrieve assets from the database
    // Ensure `sqlConnHandle` and `sqlStmtHandle` are properly defined and initialized
    checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle),
        "Failed to allocate statement handle.");

    std::string query = "SELECT AssetId, AssetName, AssetPrice FROM Assets";
    checkSuccess(SQLExecDirectA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS),
        "Failed to execute query for assets.");

    int assetId = 0;
    char assetName[100] = { 0 };
    double price = 0;

    while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
        SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &assetId, sizeof(assetId), NULL);
        SQLGetData(sqlStmtHandle, 2, SQL_C_CHAR, assetName, sizeof(assetName), NULL);
        SQLGetData(sqlStmtHandle, 3, SQL_C_DOUBLE, &price, sizeof(price), NULL);

        assets.emplace_back(assetId, std::string(assetName), price, "2024-12-28"); // Adjust date field
    }

    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    return assets;
}
//
//
//// Filter by Order Type
//void DatabaseHandler::FilterOrderHistoryByType(const std::string& userId, const std::string& orderType) {
//    ExecuteQuery(
//        "{CALL FilterOrdersByType(?, ?)}",
//        [&](SQLHANDLE stmt) {
//            SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, userId.size(), 0, (SQLPOINTER)userId.c_str(), 0, nullptr);
//            SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, orderType.size(), 0, (SQLPOINTER)orderType.c_str(), 0, nullptr);
//            std::cout << "About to call DisplayOrderResults\n";
//
//        },
//        [&](SQLHANDLE stmt) { DisplayOrderResults(stmt); });
//}
//
//// Filter by Date Range
//void DatabaseHandler::FilterOrderHistoryByDate(const std::string& userId, const std::string& startDate, const std::string& endDate) {
//    ExecuteQuery(
//        "{CALL FilterOrdersByDate(?, ?, ?)}",
//        [&](SQLHANDLE stmt) {
//            SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, userId.size(), 0, (SQLPOINTER)userId.c_str(), 0, nullptr);
//            SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, startDate.size(), 0, (SQLPOINTER)startDate.c_str(), 0, nullptr);
//            SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, endDate.size(), 0, (SQLPOINTER)endDate.c_str(), 0, nullptr);
//        },
//        [&](SQLHANDLE stmt) { DisplayOrderResults(stmt); });
//}
//
//// Filter by Price Range
//void DatabaseHandler::FilterOrderHistoryByPrice(const std::string& userId, double minPrice, double maxPrice) {
//    ExecuteQuery(
//        "{CALL FilterOrdersByPrice(?, ?, ?)}",
//        [&](SQLHANDLE stmt) {
//            SQLBindParameter(stmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, userId.size(), 0, (SQLPOINTER)userId.c_str(), 0, nullptr);
//            SQLBindParameter(stmt, 2, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, (SQLPOINTER)&minPrice, 0, nullptr);
//            SQLBindParameter(stmt, 3, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, (SQLPOINTER)&maxPrice, 0, nullptr);
//
//
//
//        },
//        [&](SQLHANDLE stmt) { DisplayOrderResults(stmt); });
//}
//void DatabaseHandler::DisplayOrderResults(SQLHANDLE stmt) {
//    try {
//        std::cout << "Displaying results...\n";
//
//        int orderId = 0;
//        double price = 0.0;
//        char date[100] = { 0 };
//        char type[50] = { 0 };
//        SQLLEN dateIndicator = 0, typeIndicator = 0;
//
//        while (SQLFetch(stmt) == SQL_SUCCESS) {
//            // Ensure column order matches query output
//            SQLGetData(stmt, 1, SQL_C_SLONG, &orderId, sizeof(orderId), nullptr); // OrderID
//            SQLGetData(stmt, 2, SQL_C_DOUBLE, &price, sizeof(price), nullptr);   // Price
//            SQLGetData(stmt, 3, SQL_C_CHAR, date, sizeof(date) - 1, &dateIndicator); // Date
//            SQLGetData(stmt, 4, SQL_C_CHAR, type, sizeof(type) - 1, &typeIndicator); // Type
//
//            // Null-terminate strings
//            date[sizeof(date) - 1] = '\0';
//            type[sizeof(type) - 1] = '\0';
//
//            // Convert nulls to "N/A"
//            std::string dateStr = (dateIndicator == SQL_NULL_DATA) ? "N/A" : date;
//            std::string typeStr = (typeIndicator == SQL_NULL_DATA) ? "N/A" : type;
//
//            // Display the result
//            std::cout << "Order ID: " << orderId
//                << " | Price: $" << price
//                << " | Date: " << dateStr
//                << " | Type: " << typeStr << "\n";
//        }
//    }
//    catch (const std::exception& ex) {
//        std::cerr << "Error during DisplayOrderResults: " << ex.what() << std::endl;
//    }
//    std::cout << "Results displayed.\n";
//}
//


void DatabaseHandler::ExecuteQuery(const std::string& query, std::function<void(SQLHANDLE)> bindParameters, std::function<void(SQLHANDLE)> processResults)
{
    SQLHANDLE stmtHandle;

    // Allocate statement handle
    if (SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &stmtHandle) != SQL_SUCCESS) {
        throw std::runtime_error("Failed to allocate statement handle.");
    }

    try {
        // Prepare the query
        if (SQLPrepareA(stmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS) != SQL_SUCCESS) {
            throw std::runtime_error("Failed to prepare SQL query.");
        }

        // Bind parameters using the provided lambda
        if (bindParameters) {
            bindParameters(stmtHandle);
        }

        // Execute the query
        if (SQLExecute(stmtHandle) != SQL_SUCCESS) {
            throw std::runtime_error("Failed to execute SQL query.");
        }

        // Process results using the provided lambda
        if (processResults) {
            processResults(stmtHandle);
        }
    }
    catch (const std::exception& ex) {
        // Log error and cleanup
        std::cerr << "Error in ExecuteQuery: " << ex.what() << std::endl;
        SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
        throw;
    }

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
}


std::string DatabaseHandler::GetUserData(int userId) {
    try {
        std::ostringstream userData;

        // Fetch user details
        userData << "User Data for User ID: " << userId << "\n";
        userData << "----------------------------------------\n";

        // Fetch wallet details
        userData << FetchWallet(userId);

        // Fetch portfolio details
        userData << FetchPortfolio(userId);

        // Fetch order history
        userData << FetchOrders(userId);

        return userData.str();
    }
    catch (const std::exception& ex) {
        throw std::runtime_error("Failed to fetch user data: " + std::string(ex.what()));
    }
}

std::string DatabaseHandler::FetchWallet(int userId) {
    std::ostringstream walletData;

    SQLHANDLE stmtHandle;
    std::string query = "SELECT WalletId, Balance FROM Wallet WHERE UserId = ?";
    checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &stmtHandle), "Failed to allocate statement handle for wallet.");

    try {
        checkSuccess(SQLPrepareA(stmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare wallet query.");
        checkSuccess(SQLBindParameter(stmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId for wallet.");

        checkSuccess(SQLExecute(stmtHandle), "Failed to execute wallet query.");

        int walletId;
        double balance;
        while (SQLFetch(stmtHandle) == SQL_SUCCESS) {
            SQLGetData(stmtHandle, 1, SQL_C_SLONG, &walletId, sizeof(walletId), NULL);
            SQLGetData(stmtHandle, 2, SQL_C_DOUBLE, &balance, sizeof(balance), NULL);

            walletData << "Wallet ID: " << walletId << ", Balance: $" << balance << "\n";
        }
    }
    catch (const std::exception& ex) {
        SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
        throw;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
    return walletData.str();
}

std::string DatabaseHandler::FetchPortfolio(int userId) {
    std::ostringstream portfolioData;

    SQLHANDLE stmtHandle;
    std::string query = "SELECT A.AssetName, P.Quantity FROM Portfolio P INNER JOIN Assets A ON P.AssetId = A.AssetId WHERE P.UserId = ?";
    checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &stmtHandle), "Failed to allocate statement handle for portfolio.");

    try {
        checkSuccess(SQLPrepareA(stmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare portfolio query.");
        checkSuccess(SQLBindParameter(stmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId for portfolio.");

        checkSuccess(SQLExecute(stmtHandle), "Failed to execute portfolio query.");

        char assetName[100];
        int quantity;
        while (SQLFetch(stmtHandle) == SQL_SUCCESS) {
            SQLGetData(stmtHandle, 1, SQL_C_CHAR, assetName, sizeof(assetName), NULL);
            SQLGetData(stmtHandle, 2, SQL_C_SLONG, &quantity, sizeof(quantity), NULL);

            portfolioData << "Asset: " << assetName << ", Quantity: " << quantity << "\n";
        }
    }
    catch (const std::exception& ex) {
        SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
        throw;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmtHandle);
    return portfolioData.str();
}


#include <fstream>
#include <sstream>
#include "HuffmanCompressor.h"
std::string DatabaseHandler::FetchUserDetails(int userId) {
    std::ostringstream userDetails;

    // Allocate statement handle
    checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for user details.");

    // SQL query to fetch user details
    std::string query = "SELECT UserName, Email FROM Users WHERE UserId = ?";
    checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SELECT query for user details.");

    // Bind parameters
    checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");

    // Execute the query
    checkSuccess(SQLExecute(sqlStmtHandle), "Failed to execute query for user details.");

    // Variables to hold fetched data
    char username[100];
    char email[100];

    // Fetch the results
    if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
        SQLGetData(sqlStmtHandle, 1, SQL_C_CHAR, username, sizeof(username), NULL);
        SQLGetData(sqlStmtHandle, 2, SQL_C_CHAR, email, sizeof(email), NULL);

        userDetails << "Username: " << username << "\n"
            << "Email: " << email << "\n";
    }
    else {
        userDetails << "No user found for UserId: " << userId << "\n";
    }

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

    return userDetails.str();
}




std::string DatabaseHandler::FetchOrders(int userId) {
    std::ostringstream orders;
    SQLRETURN retCode;

    // SQL query to fetch orders
    const std::string query = "SELECT OrderID, AssetName, Quantity, Price, OrderDate "
        "FROM Orders WHERE UserID = ?";

    // Allocate statement handle
    SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle);

    try {
        // Prepare the SQL statement
     // Convert std::string query to std::wstring
        std::wstring wideQuery(query.begin(), query.end());

        // Pass the converted wide string to SQLPrepareW
        retCode = SQLPrepareW(sqlStmtHandle, (SQLWCHAR*)wideQuery.c_str(), SQL_NTS);

        checkSuccess(retCode, "Failed to prepare SQL statement.", sqlStmtHandle, SQL_HANDLE_STMT);

        // Bind the userId parameter
        retCode = SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &userId, 0, nullptr);
        checkSuccess(retCode, "Failed to bind parameter.", sqlStmtHandle, SQL_HANDLE_STMT);

        // Execute the query
        retCode = SQLExecute(sqlStmtHandle);
        checkSuccess(retCode, "Failed to execute SQL statement.", sqlStmtHandle, SQL_HANDLE_STMT);

        // Bind the result columns
        SQLCHAR orderID[50], assetName[100], orderDate[50];
        SQLINTEGER quantity;
        SQLDOUBLE price;

        SQLBindCol(sqlStmtHandle, 1, SQL_C_CHAR, orderID, sizeof(orderID), nullptr);
        SQLBindCol(sqlStmtHandle, 2, SQL_C_CHAR, assetName, sizeof(assetName), nullptr);
        SQLBindCol(sqlStmtHandle, 3, SQL_C_LONG, &quantity, 0, nullptr);
        SQLBindCol(sqlStmtHandle, 4, SQL_C_DOUBLE, &price, 0, nullptr);
        SQLBindCol(sqlStmtHandle, 5, SQL_C_CHAR, orderDate, sizeof(orderDate), nullptr);

        // Fetch the rows
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            orders << "OrderID: " << orderID << ", Asset: " << assetName
                << ", Quantity: " << quantity << ", Price: " << price
                << ", Date: " << orderDate << "\n";
        }
    }
    catch (const std::exception& ex) {
        SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle); // Ensure resources are freed
        throw;
    }

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

    return orders.str();
}


std::string DatabaseHandler::FetchPortfolioDetails(int userId) {
    std::ostringstream portfolioDetails;

    // Allocate statement handle
    checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for portfolio details.");

    // SQL query to fetch portfolio details
    std::string query = "SELECT A.AssetName, P.Quantity FROM Portfolio P INNER JOIN Assets A ON P.AssetId = A.AssetId WHERE P.UserId = ?";
    checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SELECT query for portfolio details.");

    // Bind parameters
    checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");

    // Execute the query
    checkSuccess(SQLExecute(sqlStmtHandle), "Failed to execute query for portfolio details.");

    // Variables to hold fetched data
    char assetName[100];
    int quantity;

    // Fetch and append each result to the string
    portfolioDetails << "Portfolio Details:\n";
    while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
        SQLGetData(sqlStmtHandle, 1, SQL_C_CHAR, assetName, sizeof(assetName), NULL);
        SQLGetData(sqlStmtHandle, 2, SQL_C_SLONG, &quantity, sizeof(quantity), NULL);

        portfolioDetails << "Asset: " << assetName << ", Quantity: " << quantity << "\n";
    }

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

    return portfolioDetails.str();
}
std::string DatabaseHandler::FetchWalletDetails(int userId) {
    std::ostringstream walletDetails;

    // Allocate statement handle
    checkSuccess(SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle), "Failed to allocate statement handle for wallet details.");

    // SQL query to fetch wallet details
    std::string query = "SELECT WalletId, Balance FROM Wallet WHERE UserId = ?";
    checkSuccess(SQLPrepareA(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS), "Failed to prepare SELECT query for wallet details.");

    // Bind parameters
    checkSuccess(SQLBindParameter(sqlStmtHandle, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &userId, 0, NULL), "Failed to bind UserId.");

    // Execute the query
    checkSuccess(SQLExecute(sqlStmtHandle), "Failed to execute query for wallet details.");

    // Variables to hold fetched data
    int walletId;
    double balance;

    // Fetch the results
    if (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
        SQLGetData(sqlStmtHandle, 1, SQL_C_SLONG, &walletId, sizeof(walletId), NULL);
        SQLGetData(sqlStmtHandle, 2, SQL_C_DOUBLE, &balance, sizeof(balance), NULL);

        walletDetails << "Wallet ID: " << walletId << "\n"
            << "Balance: $" << balance << "\n";
    }
    else {
        walletDetails << "No wallet found for UserId: " << userId << "\n";
    }

    // Free the statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);

    return walletDetails.str();
}
void DatabaseHandler::ExportAndCompressUserData(int userId) {
    try {
        // Fetch user data
        std::ostringstream userDataStream;
        userDataStream << "User Information:\n";
        userDataStream << FetchUserDetails(userId) << "\n\n";

        userDataStream << "Wallet Information:\n";
        userDataStream << FetchWalletDetails(userId) << "\n\n";

        userDataStream << "Portfolio Information:\n";
        userDataStream << FetchPortfolioDetails(userId) << "\n\n";

        userDataStream << "Order History:\n";
        userDataStream << FetchOrderHistory(userId) << "\n\n";

        // Save the data to a temporary file
        std::string tempFileName = "UserData.txt";
        std::ofstream tempFile(tempFileName);
        if (!tempFile) {
            throw std::runtime_error("Failed to create temporary file for user data.");
        }
        tempFile << userDataStream.str();
        tempFile.close();

        // Compress the file using Huffman encoding
        HuffmanCompressor compressor;
        std::string compressedFileName = "UserData_compressed.huff";
        compressor.compressFile(tempFileName, compressedFileName);

        std::cout << "User data exported and compressed successfully to " << compressedFileName << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error during export and compression: " << ex.what() << std::endl;
    }
}
void DatabaseHandler::DecompressAndLoadUserData(const std::string& compressedFileName, const std::string& decompressedFileName) {
    try {
        HuffmanCompressor compressor;

        // Load compressed file and original Huffman tree
        std::string compressedData = compressor.loadCompressedFromFile(compressedFileName);

        // Rebuild Huffman tree from the data (you need the original data or the frequency map for this step)
        std::string originalData = "your_original_data"; // Replace this with the original data or logic to reconstruct the tree
        HuffmanNode* root = compressor.buildHuffmanTree(originalData);

        // Decompress the file

        std::string decompressedData = compressor.decompressData(compressedData, root);

        // Save decompressed data to file
        std::ofstream outputFile(decompressedFileName);
        if (!outputFile.is_open()) {
            throw std::runtime_error("Failed to create decompressed file.");
        }

        outputFile << decompressedData;
        outputFile.close();

        std::cout << "Decompressed data written to " << decompressedFileName << std::endl;

        // Free the Huffman tree memory
        compressor.freeTree(root);
    }
    catch (const std::exception& ex) {
        std::cerr << "Error during decompression: " << ex.what() << std::endl;
    }
}
