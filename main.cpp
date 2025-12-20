#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>
#include <iomanip> // Для форматирования колонок (setw)

using namespace std;

// Функция очистки экрана
void clearScreen() {
#ifdef _WIN32
    system("cls");   // Для Windows
#else
    system("clear"); // Для Linux/Mac
#endif
}

void executeSQL(sqlite3* db, const string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "Ошибка SQL: " << errMsg << endl;
        sqlite3_free(errMsg);
    }
}

// Красивый вывод списка продуктов
void showProducts(sqlite3* db) {
    clearScreen();
    sqlite3_stmt* stmt;
    string sql = "SELECT id, name, price, stock FROM products;";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Ошибка подготовки запроса" << endl;
        return;
    }

    cout << "==================== СПИСОК ТОВАРОВ ====================" << endl;
    // Устанавливаем ширину колонок: ID(4), Название(20), Цена(10), Склад(8)
    cout << left << setw(4) << "ID"
         << setw(20) << "Название"
         << setw(10) << "Цена (руб)"
         << setw(8) << "Склад" << endl;
    cout << "--------------------------------------------------------" << endl;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        cout << left << setw(4) << sqlite3_column_int(stmt, 0)
             << setw(20) << (const char*)sqlite3_column_text(stmt, 1)
             << setw(12) << fixed << setprecision(2) << sqlite3_column_double(stmt, 2)
             << setw(8) << sqlite3_column_int(stmt, 3) << endl;
    }
    sqlite3_finalize(stmt);
    cout << "========================================================" << endl;
    cout << "\nНажмите Enter, чтобы вернуться в меню...";
    cin.ignore();
    cin.get();
}

void addProduct(sqlite3* db) {
    clearScreen();
    string name;
    double price;
    int stock;
    cout << "--- Добавление нового товара ---" << endl;
    cout << "Название: "; cin.ignore(); getline(cin, name);
    cout << "Цена: "; cin >> price;
    cout << "Количество: "; cin >> stock;

    string sql = "INSERT INTO products (name, price, stock) VALUES ('"
                 + name + "', " + to_string(price) + ", " + to_string(stock) + ");";
    executeSQL(db, sql);
    cout << "\nГотово! Нажмите Enter...";
    cin.ignore();
    cin.get();
}

void deleteProduct(sqlite3* db) {
    clearScreen();
    int id;
    sqlite3_stmt* stmt;
    string sql_l = "SELECT id, name, price, stock FROM products;";

    if (sqlite3_prepare_v2(db, sql_l.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Ошибка подготовки запроса" << endl;
        return;
    }

    cout << "==================== СПИСОК ТОВАРОВ ====================" << endl;
    // Устанавливаем ширину колонок: ID(4), Название(20), Цена(10), Склад(8)
    cout << left << setw(4) << "ID"
         << setw(20) << "Название"
         << setw(10) << "Цена (руб)"
         << setw(8) << "Склад" << endl;
    cout << "--------------------------------------------------------" << endl;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        cout << left << setw(4) << sqlite3_column_int(stmt, 0)
             << setw(20) << (const char*)sqlite3_column_text(stmt, 1)
             << setw(10) << fixed << setprecision(2) << sqlite3_column_double(stmt, 2)
             << setw(8) << sqlite3_column_int(stmt, 3) << endl;
    }
    sqlite3_finalize(stmt);
    cout << "========================================================" << endl;
    cout << "Введите ID продукта: "; cin >> id;
    string sql = "DELETE FROM products WHERE id = " + to_string(id) + ";";
    executeSQL(db, sql);
    cout << "\nУдалено! Нажмите Enter...";
    cin.ignore();
    cin.get();
}

int main() {
    sqlite3* db;
    if (sqlite3_open("products.db", &db)) return 1;

    string setupSql = "CREATE TABLE IF NOT EXISTS products ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "name TEXT NOT NULL,"
                      "price REAL NOT NULL,"
                      "stock INTEGER DEFAULT 0);";
    executeSQL(db, setupSql);

    int choice = 0;
    while (choice != 4) {
        clearScreen();
        cout << "==== УПРАВЛЕНИЕ СКЛАДОМ ====" << endl;
        cout << "1. Показать товары" << endl;
        cout << "2. Добавить товар" << endl;
        cout << "3. Удалить товар" << endl;
        cout << "4. Выход" << endl;
        cout << "---------------------------------" << endl;
        cout << "Ваш выбор: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        switch (choice) {
            case 1: showProducts(db); break;
            case 2: addProduct(db); break;
            case 3: deleteProduct(db); break;
        }
    }

    sqlite3_close(db);
    return 0;
}
