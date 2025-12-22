#include <iostream>
#include <vector>
#include <string>
#include "sqlite3.h"
#include <iomanip>

using namespace std;

// Функция очистки экрана
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
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

// Вывод товаров
void showProducts(sqlite3* db) {
    clearScreen();
    sqlite3_stmt* stmt;
    string sql = "SELECT id, name, price, stock FROM products;";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Ошибка подготовки запроса" << endl;
        return;
    }

    cout << "==================== СПИСОК ТОВАРОВ ====================" << endl;
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

// Добавление товара в корзину
void addToCart(sqlite3* db) {
    clearScreen();
    int product_id, quantity;
    
    // Показываем доступные товары
    sqlite3_stmt* stmt;
    string sql = "SELECT id, name, price, stock FROM products WHERE stock > 0;";
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Ошибка подготовки запроса" << endl;
        return;
    }
    
    cout << "=========== ДОСТУПНЫЕ ТОВАРЫ (есть на складе) ===========" << endl;
    cout << left << setw(4) << "ID" << setw(20) << "Название" 
         << setw(10) << "Цена" << setw(8) << "Склад" << endl;
    cout << "--------------------------------------------------------" << endl;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        cout << left << setw(4) << sqlite3_column_int(stmt, 0)
             << setw(20) << (const char*)sqlite3_column_text(stmt, 1)
             << setw(10) << fixed << setprecision(2) << sqlite3_column_double(stmt, 2)
             << setw(8) << sqlite3_column_int(stmt, 3) << endl;
    }
    sqlite3_finalize(stmt);
    cout << "========================================================" << endl;
    
    // Ввод данных
    cout << "\nВведите ID товара для добавления в корзину: ";
    cin >> product_id;
    cout << "Введите количество: ";
    cin >> quantity;
    
    // Проверка наличия товара на складе
    sql = "SELECT stock FROM products WHERE id = " + to_string(product_id) + ";";
    sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int available = sqlite3_column_int(stmt, 0);
        if (available < quantity) {
            cout << "\nОшибка: недостаточно товара на складе. Доступно: " << available << endl;
        } else {
            // Добавляем в корзину
            sql = "INSERT INTO cart_items (product_id, quantity) VALUES (" 
                  + to_string(product_id) + ", " + to_string(quantity) + ");";
            executeSQL(db, sql);
            cout << "\nТовар добавлен в корзину!" << endl;
        }
    } else {
        cout << "\nОшибка: товар с таким ID не найден!" << endl;
    }
    sqlite3_finalize(stmt);
    
    cout << "\nНажмите Enter, чтобы вернуться...";
    cin.ignore();
    cin.get();
}

// Просмотр корзины
void viewCart(sqlite3* db) {
    clearScreen();
    sqlite3_stmt* stmt;
    string sql = "SELECT c.id, p.name, p.price, c.quantity, "
                 "(p.price * c.quantity) as total "
                 "FROM cart_items c "
                 "JOIN products p ON c.product_id = p.id;";
    
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        cerr << "Ошибка подготовки запроса" << endl;
        return;
    }
    
    cout << "==================== КОРЗИНА ====================" << endl;
    cout << left << setw(4) << "ID" << setw(20) << "Товар" 
         << setw(10) << "Цена" << setw(8) << "Кол-во" << setw(10) << "Сумма" << endl;
    cout << "------------------------------------------------" << endl;
    
    double grandTotal = 0.0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        double total = sqlite3_column_double(stmt, 4);
        grandTotal += total;
        
        cout << left << setw(4) << sqlite3_column_int(stmt, 0)
             << setw(20) << (const char*)sqlite3_column_text(stmt, 1)
             << setw(10) << fixed << setprecision(2) << sqlite3_column_double(stmt, 2)
             << setw(8) << sqlite3_column_int(stmt, 3)
             << setw(10) << total << endl;
    }
    sqlite3_finalize(stmt);
    
    cout << "------------------------------------------------" << endl;
    cout << right << setw(42) << "ИТОГО: " << fixed << setprecision(2) << grandTotal << " руб." << endl;
    cout << "================================================" << endl;
    
    cout << "\nНажмите Enter, чтобы вернуться...";
    cin.ignore();
    cin.get();
}

// Удаление товара из корзины
void removeFromCart(sqlite3* db) {
    clearScreen();
    int cart_item_id;
    
    // Показываем корзину
    viewCart(db);
    clearScreen();
    
    cout << "Введите ID позиции в корзине для удаления: ";
    cin >> cart_item_id;
    
    string sql = "DELETE FROM cart_items WHERE id = " + to_string(cart_item_id) + ";";
    executeSQL(db, sql);
    
    cout << "\nПозиция удалена из корзины!" << endl;
    cout << "\nНажмите Enter, чтобы вернуться...";
    cin.ignore();
    cin.get();
}

// Оформление заказа (Задание 4 - базово)
void checkoutOrder(sqlite3* db) {
    clearScreen();
    cout << "========== ОФОРМЛЕНИЕ ЗАКАЗА ==========" << endl;
    
    // Показываем корзину
    viewCart(db);
    clearScreen();
    
    char confirm;
    cout << "Вы уверены, что хотите оформить заказ? (y/n): ";
    cin >> confirm;
    
    if (confirm == 'y' || confirm == 'Y') {
        // Здесь должна быть логика оформления заказа
        // Пока просто очищаем корзину
        executeSQL(db, "DELETE FROM cart_items;");
        cout << "\nЗаказ оформлен! Корзина очищена." << endl;
    } else {
        cout << "\nОформление отменено." << endl;
    }
    
    cout << "\nНажмите Enter, чтобы вернуться...";
    cin.ignore();
    cin.get();
}

// Очистка всей корзины
void clearCart(sqlite3* db) {
    clearScreen();
    char confirm;
    cout << "Вы уверены, что хотите очистить всю корзину? (y/n): ";
    cin >> confirm;
    
    if (confirm == 'y' || confirm == 'Y') {
        executeSQL(db, "DELETE FROM cart_items;");
        cout << "\nКорзина очищена!" << endl;
    } else {
        cout << "\nОтменено." << endl;
    }
    
    cout << "\nНажмите Enter, чтобы вернуться...";
    cin.ignore();
    cin.get();
}

// Существующие функции (оставьте без изменений)
void addProduct(sqlite3* db) { /* ваш код */ }
void deleteProduct(sqlite3* db) { /* ваш код */ }

int main() {
    sqlite3* db;
    if (sqlite3_open("products.db", &db)) return 1;

    // Создаем таблицы если их нет
    string setupSql = 
        "CREATE TABLE IF NOT EXISTS products ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "price REAL NOT NULL,"
        "stock INTEGER DEFAULT 0);"
        
        "CREATE TABLE IF NOT EXISTS cart_items ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "product_id INTEGER NOT NULL,"
        "quantity INTEGER NOT NULL DEFAULT 1,"
        "added_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (product_id) REFERENCES products(id) ON DELETE CASCADE);";
    
    executeSQL(db, setupSql);

    int choice = 0;
    while (choice != 9) {
        clearScreen();
        cout << "==== ИНТЕРНЕТ-МАГАЗИН ====" << endl;
        cout << "1. Показать товары" << endl;
        cout << "2. Добавить товар в корзину" << endl;
        cout << "3. Просмотреть корзину" << endl;
        cout << "4. Удалить из корзины" << endl;
        cout << "5. Очистить корзину" << endl;
        cout << "6. Оформить заказ" << endl;
        cout << "7. Добавить товар (админ)" << endl;
        cout << "8. Удалить товар (админ)" << endl;
        cout << "9. Выход" << endl;
        cout << "--------------------------" << endl;
        cout << "Ваш выбор: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        switch (choice) {
            case 1: showProducts(db); break;
            case 2: addToCart(db); break;
            case 3: viewCart(db); break;
            case 4: removeFromCart(db); break;
            case 5: clearCart(db); break;
            case 6: checkoutOrder(db); break;
            case 7: addProduct(db); break;
            case 8: deleteProduct(db); break;
        }
    }

    sqlite3_close(db);
    return 0;
}