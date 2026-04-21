# TradingLedgerXChange


TradingLedgerXChange is a comprehensive, console-based trading simulation platform developed in C++. It provides a robust environment for users to engage in simulated asset trading, featuring secure authentication, real-time price updates, and advanced data management capabilities. The application leverages a SQL Server backend for data persistence and custom data structures for efficient in-memory operations.

## Features

*   **Secure User Authentication**: User registration and login system with password hashing (SHA-256) and salting for enhanced security.
*   **Real-time Price Simulation**: A background thread periodically updates asset prices by calling a database stored procedure, simulating a live market.
*   **Comprehensive Trading Operations**:
    *   Buy and sell assets with direct portfolio and wallet integration.
    *   Transfer funds and assets between users.
    *   Convert one asset type to another based on current market rates.
*   **Portfolio & Wallet Management**:
    *   View detailed portfolio summaries, including all held assets and quantities.
    *   Check wallet balance and transaction history.
*   **Market Analysis**:
    *   Identify the top 5 gaining and losing assets using a priority queue.
*   **Advanced Order Filtering**:
    *   Review complete order history.
    *   Filter orders dynamically by date range or price range.
*   **Data Export & Compression**:
    *   Export a user's complete data (profile, wallet, portfolio, orders) to a text file.
    *   Compress the exported data using a custom Huffman coding implementation for efficient storage.
*   **Custom Data Structures**:
    *   A **Priority Queue** for displaying top-performing assets.
    *   A **Stack** for viewing recent order history in a LIFO manner.
    *   A **Binary Search Tree (BST)** for organizing and filtering orders efficiently by price.

## Tech Stack & Dependencies

*   **Language**: C++
*   **Database**: Microsoft SQL Server
*   **Database API**: ODBC (Open Database Connectivity)
*   **Cryptography**: OpenSSL for SHA-256 hashing and salt generation.

## Setup and Installation

### Prerequisites

*   **Visual Studio**: The project is configured as a Visual Studio 2022 solution.
*   **Microsoft SQL Server**: A running instance of SQL Server.
*   **ODBC Driver 18 for SQL Server**: Required for the database connection.
*   **OpenSSL 3.x**: The project is linked against the OpenSSL library for cryptographic functions. The project files are pre-configured for a default installation path (`C:\Program Files\FireDaemon OpenSSL 3`).

### 1. Database Setup

1.  Create a new database in your SQL Server instance, for example, `DbTrading`.
2.  Update the connection string in `TradingLedgerXChange/DatabaseHandler.h` with your server details and credentials:
    ```cpp
    std::string connStr = "Driver={ODBC Driver 18 for SQL Server};Server=your_server_address;Database=DbTrading;Uid=your_username;Pwd=your_password;Encrypt=no;TrustServerCertificate=yes;";
    ```
3.  Set up the necessary tables (`Users`, `Assets`, `Wallet`, `Portfolio`, `Orders`, etc.) and stored procedures required by the application. The source code in `DatabaseHandler.cpp` references stored procedures such as `GenerateRandomPriceFluctuations`, `BuyAsset`, `SellAsset`, and various filtering procedures.

### 2. Library Configuration

1.  Install OpenSSL for Windows.
2.  If you installed OpenSSL to a different directory, update the project's properties in Visual Studio:
    *   **C/C++ -> General -> Additional Include Directories**: Add the path to your OpenSSL `include` folder.
    *   **Linker -> General -> Additional Library Directories**: Add the path to your OpenSSL `lib` folder.
    *   **Linker -> Input -> Additional Dependencies**: Ensure `libssl.lib` and `libcrypto.lib` are listed.

### 3. Build and Run

1.  Clone the repository.
2.  Open `TradingLedgerXChange.sln` in Visual Studio.
3.  Select the build configuration (e.g., `Debug | x64`).
4.  Build and run the project (F5).

## Usage

The application is entirely controlled via the command-line interface.

1.  **Launch**: On startup, an ASCII art animation is displayed.
2.  **Authentication**: You can choose to register a new account or log in. Passwords are masked with asterisks during input.
3.  **Main Menu**: After a successful login, you are presented with a menu of options, including:
    *   Buying/Selling Assets
    *   Viewing Your Portfolio
    *   Transferring Funds or Assets
    *   Viewing Market Movers (Top Gainers/Losers)
    *   Reviewing and Filtering Your Order History
    *   Exporting Your Data

## Code Overview

*   `main.cpp`: The main entry point of the application. Handles the primary application loop, UI animation, and directs users to registration or login flows.
*   `DatabaseHandler.h`/`.cpp`: A dedicated class that encapsulates all database interactions through the ODBC API. It handles connection management, query execution, and calls to stored procedures.
*   `Functions.h`/`.cpp`: Contains user-facing logic, including menu displays, input validation, and functions that orchestrate the user landing page experience.
*   `LinkedList.h`/`.cpp`: Implements a `PriorityQueue` class that serves dual purposes: a price-sorted priority queue for market analysis and a stack for displaying order history.
*   `OrderTree.h`/`.cpp`: Implements a Binary Search Tree (`OrderTree`) to store order data, allowing for efficient filtering based on price thresholds.
*   `HuffmanCompressor.h`/`.cpp`: Provides functionality for data compression using Huffman coding, used for the "Export Data" feature.
