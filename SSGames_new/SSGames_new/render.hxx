#include <iostream>
#include<fstream>
#include<filesystem>
#include <sstream>
#include<string>
#include"render.h"
#include "Imgui/imgui_internal.h"
#include "font/IconsFontAwesome6.h"
#include <chrono>
#include <limits>
#include<thread>
#include <unordered_set>
#include <unordered_map>
#include <regex>

#define DEV 1
#define _CRT_SECURE_NO_WARNINGS

void Tab(ImFont*);
void Setting(ImFont*);
void Color_Menu_Edit();
void Info(ImFont*);
//void DrawLoadingSpinner();
bool IsGamePurchased(int , const std::string& );

struct Game {
    int id; // Новый идентификатор игры
    std::string name;
    std::string release_date;
    std::string genre;
    int sold_copies;
    float price;
};

// Структура пользователя
struct User {
    int id;
    std::string username;
    std::string password;
    std::string email;
    std::string role;
    float balance = 5.0f; // Начальный баланс в $5
};
std::vector<User> GetAllUsers();
void SaveAllUsers(const std::vector<User>&); // сохранять данные о пользователе
class Cart {
public:
    std::vector<Game> items; // Список игр в корзине
    bool viewingCart = false; // Флаг состояния: отображаем корзину или магазин

    // Метод для добавления игры в корзину
    void AddToCart(const Game& game) {
        items.push_back(game);
    }

    // Метод для удаления игры из корзины
    void RemoveFromCart(int gameId) {
        auto it = std::remove_if(items.begin(), items.end(), [gameId](const Game& game) {
            return game.id == gameId;
            });
        items.erase(it, items.end());
    }

    // Метод для того, чтобы установить состояние корзины (просмотр корзины или магазина)
    void SetViewingCart(bool state) {
        viewingCart = state;
    }

    // Метод для проверки, просматривает ли пользователь корзину
    bool IsViewingCart() const {
        return viewingCart;
    }

    // Метод для выполнения покупки
    void Purchase(const std::string& username) {
        // Реализуйте логику покупки, например:
        std::cout << "User " << username << " has purchased the following games:" << std::endl;
        for (const auto& game : items) {
            std::cout << "Game: " << game.name << " - $" << game.price << std::endl;
        }
        // После покупки очищаем корзину
        items.clear();
    }
};


// Покупка игры
void PurchaseGames(Cart& cart, const std::string& username) {
    auto users = GetAllUsers();
    auto userIt = std::find_if(users.begin(), users.end(), [&](const User& user) {
        return user.username == username;
    });

    if (userIt == users.end()) {
        return;
    }

    float totalPrice = 0.0f;
    for (const auto& game : cart.items) {
        if (IsGamePurchased(game.id, username)) {
            continue;
        }
        totalPrice += game.price;
    }

    if (totalPrice == 0.0f) {
        return;
    }

    if (userIt->balance < totalPrice) {
        return;
    }

    userIt->balance -= totalPrice;

    std::ofstream libraryFile(username + "_library.csv", std::ios::app);
    if (!libraryFile.is_open()) {
        return;
    }

    // Обновляем количество копий игр и записываем в библиотеку
    for (auto& game : cart.items) {
        if (!IsGamePurchased(game.id, username)) {
            // Уменьшаем количество оставшихся копий игры
            if (game.sold_copies == 0) {
                continue;
            }

            game.sold_copies--;

            libraryFile << game.id << "," << game.name << "," << game.release_date << ","
                        << game.genre << "," << game.sold_copies + 1 << "," << game.price << "\n";

            // Обновляем запись о количестве копий в файле games.csv
            std::ifstream gameFile("games.csv");
            std::vector<Game> allGames;
            std::string line;
            while (std::getline(gameFile, line)) {
                std::istringstream ss(line);
                Game g;
                if (ss >> g.id && ss.ignore() &&
                    std::getline(ss, g.name, ',') &&
                    std::getline(ss, g.release_date, ',') &&
                    std::getline(ss, g.genre, ',') &&
                    ss >> g.sold_copies && ss.ignore() &&
                    ss >> g.price) {
                    if (g.id == game.id) {
                        g.sold_copies = game.sold_copies; // Обновляем количество оставшихся копий
                    }
                    allGames.push_back(g);
                }
            }
            gameFile.close();

            // Записываем обновленные данные в файл
            std::ofstream outFile("games.csv", std::ios::trunc);
            if (outFile.is_open()) {
                outFile << "ID,Name,Release Date,Genre,Sold Copies,Price\n";
                for (const auto& g : allGames) {
                    outFile << g.id << "," << g.name << "," << g.release_date << ","
                            << g.genre << "," << g.sold_copies << "," << g.price << "\n";
                }
                outFile.close();
            }
        }
    }

    libraryFile.close();
    cart.items.clear();
    SaveAllUsers(users);
}

// Отображение списка купленных игр
void DisplayPurchasedGames(const std::string& username) {
    std::ifstream libraryFile(username + "_library.csv");
    if (!libraryFile.is_open()) {
        ImGui::SetNextWindowSize(ImVec2(400, 200));
        ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.0f,1.0f,1.0f,1.0f));
        ImGui::Begin(menufunctional::settings::language == 0 ? "Library" : U8(u8"Библиотека"));
        ImGui::PopStyleColor();
        ImGui::Text(menufunctional::settings::language == 0 ? "No games purchased yet.": U8(u8"Игры еще не куплены."));
        ImGui::End();
        return;
    }
   // ImGui::SetNextWindowSize(ImVec2(400, 300));
    ImGui::SetNextWindowSize(ImVec2(400, 200));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Begin(menufunctional::settings::language == 0 ? "Purchased Games":U8(u8"Приобретенные игры"));
    ImGui::PopStyleColor();
    ImGui::Text(menufunctional::settings::language == 0 ? "Purchased games:":U8(u8"Приобретенные игры:"));

    std::string line;
    std::string gamesList;
    while (std::getline(libraryFile, line)) {
        size_t firstCommaPos = line.find(',');
        if (firstCommaPos == std::string::npos) continue; // Нет запятых, пропускаем строку

        size_t secondCommaPos = line.find(',', firstCommaPos + 1);
        std::string gameName = (secondCommaPos != std::string::npos) ? line.substr(firstCommaPos + 1, secondCommaPos - firstCommaPos - 1) : line.substr(firstCommaPos + 1);

        if (!gamesList.empty()) {
            gamesList += ", ";
        }
        gamesList += gameName;
    }

    // Обрезаем строку, если она слишком длинная
    if (gamesList.length() > 100) {
        gamesList = gamesList.substr(0, 97) + "...";
    }

    ImGui::Text("%s", gamesList.c_str());
    ImGui::End();
}


// Отображение баланса пользователя
void DisplayUserBalance(const std::string& username) {
    auto users = GetAllUsers();
    auto userIt = std::find_if(users.begin(), users.end(), [&](const User& user) {
        return user.username == username;
        });

    if (userIt == users.end()) {
        ImGui::Text(menufunctional::settings::language == 0 ? "User not found!" :U8(u8"Пользователь не найден!"));
        return;
    }

    // Отображаем баланс пользователя через ImGui
    ImGui::Text(menufunctional::settings::language == 0 ? "Your current balance: $%.2f" :U8(u8"Ваш текущий баланс: $%.2f"), userIt->balance);
}


// Чтение данных из файла
std::vector<Game> ReadGames() {
    std::vector<Game> games;
    std::ifstream file("games.csv");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            int id;
            std::string name, release_date, genre;
            int sold_copies;
            float price;
            if (ss >> id && ss.ignore() &&
                std::getline(ss, name, ',') &&
                std::getline(ss, release_date, ',') &&
                std::getline(ss, genre, ',') &&
                ss >> sold_copies && ss.ignore() &&
                ss >> price) {
                games.push_back({ id, name, release_date, genre, sold_copies, price });
            }
        }
        file.close();
    }
    return games;
}
static bool buttonpress = false;
void DisplayGameStore(Cart& cart, ImFont* icons_font) {// показывает игровой магазин
    // Читаем данные из файла
    std::vector<Game> games = ReadGames();
    static bool library_on = false;
    //ImGui::SetNextWindowSize(ImVec2(1200,500));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Begin(menufunctional::settings::language == 0 ? "Game Store": U8(u8"Игровой магазин"));
    ImGui::PopStyleColor();
    if (menufunctional::role == "user")
    {
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_XMARK))
            menufunctional::menu_main_button::closemenu = true;
        ImGui::PopFont();

        if (ImGui::Button(menufunctional::settings::language == 0 ? "Log out account" : U8(u8"Выйти из аккаунта")))
            menufunctional::login::islogining = false;
        ImGui::PushFont(icons_font);
        if(ImGui::Button(ICON_FA_GEAR))
        menufunctional::settings::open = !menufunctional::settings::open;

        if(menufunctional::settings::open)
        Setting(icons_font);

        ImGui::PopFont();
    }
    if (menufunctional::role != "user")
    {
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_XMARK))
            buttonpress = false;
        ImGui::PopFont();
    }
    DisplayUserBalance(menufunctional::login::currentUser);
        // Отображаем данные в таблице
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        if (ImGui::BeginTable(menufunctional::settings::language == 0 ? "GameTable" : U8(u8"Игровой стол") , 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
            // Заголовки таблицы
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "ID": U8(u8"ИН"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Name": U8(u8"Имя"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Release Date": U8(u8"Дата выхода"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Genre":U8(u8"Жанр"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Sold Copies": U8(u8"Проданные экземпляры"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Price": U8(u8"Цена"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Action": U8(u8"Действие"));
            ImGui::TableHeadersRow();
            ImGui::PopStyleColor();
            // Заполняем строки таблицы
            for (const auto& game : games) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", game.id);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", game.name.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", game.release_date.c_str());

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%s", game.genre.c_str());

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%d", game.sold_copies);

                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%.2f", game.price);

                ImGui::TableSetColumnIndex(6);
                if (game.sold_copies > 0) { // Проверяем, что количество копий больше нуля
                    if (ImGui::Button((menufunctional::settings::language == 0 ? "Add to Cart##" : U8(u8"Добавить в корзину") + std::to_string(game.id)).c_str())) {
                        cart.AddToCart(game);
                    }
                }
                else {
                    ImGui::Text(menufunctional::settings::language == 0 ? "Out of stock" : U8(u8"Нет в наличии")); // Или другой текст для недоступных игр
                }

            }

            ImGui::EndTable();
        }
        if (ImGui::Button(menufunctional::settings::language == 0 ? "Library" : U8(u8"Библиотека")))
            library_on = !library_on;

        if (library_on)
            DisplayPurchasedGames(menufunctional::login::currentUser);

        ImGui::Separator();

        // Кнопка для перехода в корзину
        if (ImGui::Button(menufunctional::settings::language == 0 ? "View Cart": U8(u8"Просмотр корзины"))) {
            // Переход в интерфейс корзины
            cart.SetViewingCart(true);
        }

        ImGui::End();
    }



void DisplayCartInterface(Cart& cart, const std::string& username) {// показывает корзину покупок
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetNextWindowSize(ImVec2(700,400));
    ImGui::Begin(menufunctional::settings::language == 0 ? "Shopping Cart" : U8(u8"Корзина покупок"));
    ImGui::PopStyleColor();
    if (cart.items.empty()) {
        ImGui::Text(menufunctional::settings::language == 0 ? "Your cart is empty." :U8(u8"Ваша корзина пуста."));
    }
    else {
        ImGui::Text(menufunctional::settings::language == 0 ? "Games in your cart:" : U8(u8"Игры в вашей корзине:"));
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        if (ImGui::BeginTable("CartTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "ID" : U8(u8"ИН"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Name" : U8(u8"Имя"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Price" : U8(u8"Цена"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Action" : U8(u8"Действие"));
            ImGui::TableHeadersRow();
            ImGui::PopStyleColor();
            for (size_t i = 0; i < cart.items.size(); ++i) {
                const Game& game = cart.items[i];

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", game.id);

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", game.name.c_str());

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("$%.2f", game.price);

                ImGui::TableSetColumnIndex(3);
                if (ImGui::Button((menufunctional::settings::language == 0 ? "Remove##" : U8(u8"Удалить##") + std::to_string(i)).c_str())) {
                    cart.RemoveFromCart(game.id);
                }
            }

            ImGui::EndTable();
        }

        ImGui::Separator();
        ImGui::Text(menufunctional::settings::language == 0 ? "Total items: %d": U8(u8"Общее количество товаров: %d"), static_cast<int>(cart.items.size()));
        float totalPrice = 0.0f;
        for (const auto& game : cart.items) {
            totalPrice += game.price;
        }
        ImGui::Text(menufunctional::settings::language == 0 ? "Total price: $%.2f": U8(u8"Итоговая цена: $%.2f"), totalPrice);

        if (ImGui::Button(menufunctional::settings::language == 0 ? "Purchase": U8(u8"Покупка"))) {
            PurchaseGames(cart, username);
            cart.items.clear();
            ImGui::Text(menufunctional::settings::language == 0 ? "Purchase completed successfully!": U8(u8"Покупка успешно завершена!"));
        }
    }

    ImGui::Separator();

    if (ImGui::Button(menufunctional::settings::language == 0 ? "Back to Store": U8(u8"Вернуться в магазин"))) {
        cart.SetViewingCart(false);
    }

    ImGui::End();
}



std::string GetCurrentDateTime() { // показывает время создание игры
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::tm now_tm = {};
    // Используем localtime_s для безопасного получения времени
    if (localtime_s(&now_tm, &now_time) != 0) {
        return "";
    }

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

int GetNextGameID() { // записывает id игры в таблицу
    std::ifstream file("games.csv");
    int max_id = 0;
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            int id;
            std::string temp;
            if (ss >> id && std::getline(ss, temp, ',')) {
                if (id > max_id) {
                    max_id = id;
                }
            }
        }
        file.close();
    }
    return max_id + 1; // Следующий уникальный ID
}

void AddGame(const Game& game) { // добавляет игры в таблицу
    const std::string headers = "ID,Name,Release Date,Genre,Sold Copies,Price";
    std::ifstream infile("games.csv");
    bool headersExist = false;

    // Проверяем наличие заголовков
    if (infile.is_open()) {
        std::string firstLine;
        if (std::getline(infile, firstLine)) {
            headersExist = (firstLine == headers);
        }
        infile.close();
    }

    // Если заголовков нет, добавляем их
    if (!headersExist) {
        std::ofstream outfile("games.csv", std::ios::app);
        if (outfile.is_open()) {
            outfile << headers << "\n";
            outfile.close();
        }
        else {
            std::cerr << "Error opening file to write headers!" << std::endl;
            return;
        }
    }

    // Добавляем данные об игре
    std::ofstream file("games.csv", std::ios::app);
    if (file.is_open()) {
        file << game.id << "," << game.name << "," << game.release_date << ","
            << game.genre << "," << game.sold_copies << "," << game.price << "\n";
        file.close();
    }
    else {
        std::cerr << "Error opening file to add game!" << std::endl;
    }

    // Записываем дату создания в файл date.csv
    std::ofstream dateFile("date.csv", std::ios::app);
    if (dateFile.is_open()) {
        std::string currentDateTime = GetCurrentDateTime();
        dateFile << game.id << "," << currentDateTime << "\n";
        dateFile.close();
    }
    else {
        std::cerr << "Error opening date file to add creation time!" << std::endl;
    }
}


// Удаление игры
void DeleteGame(int game_id) {
    std::ifstream file("games.csv");
    if (!file.is_open()) {
        std::cerr << "Error opening file for reading!" << std::endl;
        return;
    }

    std::vector<std::string> lines; // Для хранения всех строк, кроме удаляемой
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        int id;
        ss >> id;
        if (id != game_id) { // Если ID не совпадает, сохраняем строку
            lines.push_back(line);
        }
    }

    file.close();

    // Перезаписываем файл с оставшимися строками
    std::ofstream outfile("games.csv", std::ios::trunc); // Открываем файл с очисткой содержимого
    if (!outfile.is_open()) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    for (const auto& saved_line : lines) {
        outfile << saved_line << "\n";
    }

    outfile.close();
}

// Функция для изменения определённых полей игры по ID
void RenameGameField(int game_id, int new_id = -1, const std::string& new_name = "", const std::string& new_release_date = "", const std::string& new_genre = "", int new_sold_copies = -1, float new_price = -1.0f) {
    std::ifstream file("games.csv");
    if (!file.is_open()) {
        std::cerr << "Error opening file for reading!" << std::endl;
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    bool updated = false;

    // Заголовок для файла
    const std::string headers = "ID,Name,Release Date,Genre,Sold Copies,Price";

    // Считываем первую строку и проверяем заголовок
    if (std::getline(file, line)) {
        if (line != headers) {
            lines.push_back(headers); // Добавляем заголовок, если он отсутствует
        }
        else {
            lines.push_back(line);
        }
    }

    std::unordered_set<int> existing_ids; // Для проверки уникальности ID

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        int id;
        std::string name, release_date, genre;
        int sold_copies;
        float price;

        if (ss >> id && ss.ignore() &&
            std::getline(ss, name, ',') &&
            std::getline(ss, release_date, ',') &&
            std::getline(ss, genre, ',') &&
            ss >> sold_copies && ss.ignore() &&
            ss >> price) {

            existing_ids.insert(id); // Запоминаем существующие ID

            if (id == game_id) {
                if (new_id >= 0 && new_id != game_id) {
                    // Проверяем, что новый ID уникален
                    if (existing_ids.count(new_id)) {
                        std::cerr << "Error: New ID " << new_id << " already exists!" << std::endl;
                        return;
                    }
                    id = new_id; // Изменяем ID
                    existing_ids.insert(new_id); // Добавляем новый ID в множество
                }

                // Обновляем другие поля
                if (!new_name.empty()) {
                    name = new_name;
                }
                if (!new_release_date.empty()) {
                    release_date = new_release_date;
                }
                if (!new_genre.empty()) {
                    genre = new_genre;
                }
                if (new_sold_copies >= 0) {
                    sold_copies = new_sold_copies;
                }
                if (new_price >= 0.0f) {
                    price = new_price;
                }
                updated = true;
            }

            std::ostringstream updated_line;
            updated_line << id << "," << name << "," << release_date << ","
                << genre << "," << sold_copies << "," << price;
            lines.push_back(updated_line.str());
        }
    }

    file.close();

    if (!updated) {
        std::cerr << "Game with ID " << game_id << " not found!" << std::endl;
        return;
    }

    std::ofstream outfile("games.csv", std::ios::trunc);
    if (!outfile.is_open()) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    for (const auto& saved_line : lines) {
        outfile << saved_line << "\n";
    }

    outfile.close();
    std::cout << "Game with ID " << game_id << " updated successfully!" << std::endl;
}


std::unordered_map<int, std::string> LoadCreationDates(const std::string& filename) {
    std::unordered_map<int, std::string> creation_dates;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return creation_dates;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        int id;
        std::string date;

        // Используем табуляцию как разделитель
        if (ss >> id && std::getline(ss, date, '\t')) {
            // Удаляем начальные и конечные пробелы и любые лишние символы, включая запятые
            date.erase(0, date.find_first_not_of(" \t,"));
            date.erase(date.find_last_not_of(" \t") + 1);
            creation_dates[id] = date;
        }
    }

    file.close();
    return creation_dates;
}

std::string GetCreationDate(int id, const std::unordered_map<int, std::string>& creation_dates) {
    auto it = creation_dates.find(id);
    if (it != creation_dates.end()) {
        return it->second;
    }
    return "Not Found"; // Возвращаем "Not Found", если ID не существует
}

//// Функция для получения даты создания по ID
//std::string GetCreationDate(int id, const std::unordered_map<int, std::string>& creation_dates) {
//    auto it = creation_dates.find(id);
//    if (it != creation_dates.end()) {
//        return it->second;
//    }
//    return "Date not found";
//}


// Вызов и отображение информации
void SysCheckDate(ImFont* icons_font) {
    static int id_to_check = 0;
    static std::string creation_date;
    static bool CloseCheckDate = false;
    ImGui::Checkbox(menufunctional::settings::language == 0 ? "Check Date" : U8(u8"Проверка даты"),&CloseCheckDate);
    if (CloseCheckDate) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::Begin(menufunctional::settings::language == 0 ? "Check Creation Date" : U8(u8"Проверка даты создания"));
        ImGui::PopStyleColor();
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_XMARK))
            CloseCheckDate = false;
        ImGui::PopFont();
        ImGui::InputInt(menufunctional::settings::language == 0 ? "Enter ID" : U8(u8"Введите ИН"), &id_to_check);

        if (ImGui::Button(menufunctional::settings::language == 0 ? "Check Date" : U8(u8"Дата проверки"))) {
            // Загружаем данные и ищем дату
            auto creation_dates = LoadCreationDates("date.csv");
            creation_date = GetCreationDate(id_to_check, creation_dates);
        }

        ImGui::Text(menufunctional::settings::language == 0 ? "Creation Date: %s" : U8(u8"Дата создания: %s"), creation_date.c_str());

        ImGui::End();
    }
}



// Проверка наличия заголовков в файле users.csv и добавление, если их нет
void EnsureUserFileHeaders() {
    const std::string headers = "id,user,password,email,role,balance";
    std::ifstream infile("users.csv");
    bool headersExist = false;

    if (infile.is_open()) {
        std::string firstLine;
        if (std::getline(infile, firstLine)) {
            headersExist = (firstLine == headers);
        }
        infile.close();
    }

    if (!headersExist) {
        std::ofstream outfile("users.csv", std::ios::app);
        if (outfile.is_open()) {
            outfile << headers << "\n";
            outfile.close();
        }
    }
}

// Функция для проверки валидности электронной почты
bool IsValidEmail(const std::string& email) {
    const std::regex pattern(R"((\w+)(\.\|_)?(\w*)@(\w+)(\.(\w+))+)");
    return std::regex_match(email, pattern);
}
// Функция для проверки пользователя
bool IsUserOrEmailExists(const std::string& username, const std::string& email) {
    std::ifstream infile("users.csv");
    std::string line;

    while (std::getline(infile, line)) {
        std::istringstream ss(line);
        std::string id, user, pass, mail, role;

        if (std::getline(ss, id, ',') &&
            std::getline(ss, user, ',') &&
            std::getline(ss, pass, ',') &&
            std::getline(ss, mail, ',') &&
            std::getline(ss, role, ',')) {

            if (user == username || mail == email) {
                return true; // Найдено совпадение
            }
        }
    }

    return false; // Совпадений нет
}
// Функция для сохранения информации о нём
void SaveUserData(const std::string& username, const std::string& password, const std::string& email) {
    EnsureUserFileHeaders();

    static int userId = 0;
    std::ifstream infile("users.csv");
    std::string line;
    while (std::getline(infile, line)) {
        userId++;
    }
    infile.close();

    std::ofstream file("users.csv", std::ios::app);
    if (file.is_open()) {
        file << userId << "," << username << "," << password << "," << email << ",user,5.0\n";
        file.close();
    }
}


// Функция для проверки данных при логине
bool ValidateLogin(const std::string& username, const std::string& password) {
    std::ifstream file("users.csv");
    std::string line;

    if (file.is_open()) {
        bool isFirstLine = true;
        while (std::getline(file, line)) {
            if (isFirstLine) {
                isFirstLine = false;
                continue; // Пропускаем заголовки
            }

            std::stringstream ss(line);
            std::string file_id, file_username, file_password, file_email, file_role;

            std::getline(ss, file_id, ',');
            std::getline(ss, file_username, ',');
            std::getline(ss, file_password, ',');
            std::getline(ss, file_email, ',');
            std::getline(ss, file_role);

            // Убираем возможные лишние пробелы
            file_username.erase(file_username.find_last_not_of(" \n\r\t") + 1);
            file_password.erase(file_password.find_last_not_of(" \n\r\t") + 1);

            if (username == file_username && password == file_password) {
                return true;
            }
        }
        file.close();
    }
    return false;
}

// Функция для получения роли пользователя
std::string GetUserRole(const std::string& username) {
    std::ifstream file("users.csv");
    std::string line;

    if (file.is_open()) {
        bool isFirstLine = true;
        while (std::getline(file, line)) {
            if (isFirstLine) {
                isFirstLine = false;
                continue; // Пропускаем заголовки
            }

            std::stringstream ss(line);
            std::string file_id, file_username, file_password, file_email, file_role, file_balance, file_price;

            // Чтение всех столбцов (с учётом нового столбца price)
            std::getline(ss, file_id, ',');
            std::getline(ss, file_username, ',');
            std::getline(ss, file_password, ',');
            std::getline(ss, file_email, ',');
            std::getline(ss, file_role, ',');
            std::getline(ss, file_balance, ',');  // Читаем баланс (который был до добавления price)
            std::getline(ss, file_price);         // Читаем новое поле price

            // Убираем лишние пробелы
            file_role.erase(0, file_role.find_first_not_of(" \t\n\r"));
            file_role.erase(file_role.find_last_not_of(" \t\n\r") + 1);

            if (username == file_username) {
                return file_role; // Возвращаем роль, если нашли пользователя
            }
        }
        file.close();
    }
    return "Role not found";
}

// Отрисовка логина
void RenderLoginUI(bool& showRegistration,ImFont* icons_font) {
    static char loginUsername[128] = "";
    static char loginPassword[128] = "";
    static std::string message;

  //  ImGui::Begin("Login");
    ImGui::SetCursorPos(ImVec2(370,25));
    ImGui::PushFont(icons_font);
    if(ImGui::Button(ICON_FA_XMARK))
        menufunctional::menu_main_button::closemenu = true;
    ImGui::PopFont();

    ImGui::Text(menufunctional::settings::language == 0 ? "Login:" : U8(u8"Авторизоваться:"));
    ImGui::InputText(menufunctional::settings::language == 0 ? "Username":U8(u8"Имя пользователя"), loginUsername, sizeof(loginUsername));
    ImGui::InputText(menufunctional::settings::language == 0 ? "Password":U8(u8"Пароль"), loginPassword, sizeof(loginPassword), ImGuiInputTextFlags_Password);

    if (ImGui::Button(menufunctional::settings::language == 0 ? "Login":U8(u8"Авторизоваться"))) {
        if (ValidateLogin(loginUsername, loginPassword)) {
            menufunctional::role = GetUserRole(loginUsername);
            message = menufunctional::settings::language == 0 ? "Login successful! Role: ":U8(u8"Вход в систему прошел успешно! Роль:") + menufunctional::role;
            menufunctional::login::currentUser = loginUsername;
            menufunctional::login::islogining = true;
        }
        else {
            message = menufunctional::settings::language == 0 ? "Invalid username or password.":U8(u8"Неверное имя пользователя или пароль");
        }
    }

    ImGui::Text("%s", message.c_str());

    ImGui::Separator();
    ImGui::Text(menufunctional::settings::language == 0 ? "Not registered yet?":U8(u8"Еще не зарегистрировались?"));
    if (ImGui::Button(menufunctional::settings::language == 0 ? "Register here":U8(u8"Зарегистрируйтесь здесь"))) {
        showRegistration = true;
    }

    //ImGui::End();
}

// Отрисовка регистрации
void RenderRegistrationUI(bool& showRegistration) {
    static char username[128] = "";
    static char password[128] = "";
    static char email[128] = "";
    static std::string message;

    //ImGui::Begin("Registration");

    ImGui::Text(menufunctional::settings::language == 0 ? "Register:" : U8(u8"Зарегистрироваться:"));
    ImGui::InputText(menufunctional::settings::language == 0 ? "Username" : U8(u8"Имя пользователя"), username, sizeof(username));
    ImGui::InputText(menufunctional::settings::language == 0 ? "Password" : U8(u8"Пароль"), password, sizeof(password), ImGuiInputTextFlags_Password);
    ImGui::InputText(menufunctional::settings::language == 0 ? "Email" : U8(u8"Электронная почта"), email, sizeof(email));

    if (ImGui::Button(menufunctional::settings::language == 0 ? "Register" : U8(u8"Зарегистрировать"))) {
        std::string user(username);
        std::string pass(password);
        std::string mail(email);

        if (user.empty() || pass.empty() || mail.empty()) {
            message = menufunctional::settings::language == 0 ? "All fields are required!" : U8(u8"Все поля обязательны для заполнения!");
        }
        else if (!IsValidEmail(mail)) {
            message = menufunctional::settings::language == 0 ? "Invalid email format!" : U8(u8"Недопустимый формат электронной почты!");
        }
        else if (IsUserOrEmailExists(user, mail)) {
            message = menufunctional::settings::language == 0 ? "Username or email already exists!" : U8(u8"Имя пользователя или адрес электронной почты уже существуют!");
        }
        else {
            SaveUserData(user, pass, mail);
            message = menufunctional::settings::language == 0 ? "Registration successful!": U8(u8"Регистрация прошла успешно!");
            showRegistration = false;
        }
    }

    ImGui::Text("%s", message.c_str());

    if (ImGui::Button(menufunctional::settings::language == 0 ? "Back to Login" : U8(u8"Вернуться к входу"))) {
        showRegistration = false;
    }

    //ImGui::End();
}

// Изменить роль пользователя
bool ChangeUserRole(const std::string& username, const std::string& newRole) {
    std::ifstream infile("users.csv");
    std::vector<std::string> lines;
    std::string line;
    bool userFound = false;

    // Чтение всех строк из файла
    while (std::getline(infile, line)) {
        std::istringstream ss(line);
        std::string id, user, pass, email, role, balance;

        if (std::getline(ss, id, ',') &&
            std::getline(ss, user, ',') &&
            std::getline(ss, pass, ',') &&
            std::getline(ss, email, ',') &&
            std::getline(ss, role, ',') &&
            std::getline(ss, balance)) {

            // Проверяем, совпадает ли имя пользователя
            if (user == username) {
                userFound = true;

                // Обновляем только роль, остальные поля остаются без изменений
                role = newRole;
            }

            // Добавляем строку обратно в вектор строк
            lines.push_back(id + "," + user + "," + pass + "," + email + "," + role + "," + balance);
        }
    }

    infile.close();

    if (userFound) {
        // Перезаписываем файл с обновленными данными
        std::ofstream outfile("users.csv", std::ios::trunc);
        if (outfile.is_open()) {
            for (const auto& saved_line : lines) {
                outfile << saved_line << "\n";
            }
            outfile.close();
            return true; // Успешно обновлено
        }
        else {
            std::cerr << "Error: Unable to open file for updating user data." << std::endl;
        }
    }
    else {
        std::cerr << "Error: User not found!" << std::endl;
    }

    return false; // Не удалось изменить роль
}

// Отрисовка для изменения роли
void RenderAdminUI(ImFont* icons_font) {
    static char username[128] = "";
    static std::string message;

    // Индекс роли по умолчанию (0 - "user", 1 - "developer")
    static int selectedRoleIndex = 0;
    static bool changerole = false;
    // Доступные роли
    const char* roles[] = { "user", "root" };
    ImGui::Checkbox(menufunctional::settings::language == 0 ? "Change Role" : U8(u8"Изменить роль"),&changerole);
    if (changerole) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::Begin(menufunctional::settings::language == 0 ? "Role" : U8(u8"Роль"));
        ImGui::PopStyleColor();
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_XMARK))
            changerole = false;
        ImGui::PopFont();
        ImGui::Text(menufunctional::settings::language == 0 ? "Change User Role" : U8(u8"Изменение роли пользователя"));

        // Ввод имени пользователя
        ImGui::InputText(menufunctional::settings::language == 0 ? "Username" : U8(u8"Имя пользователя"), username, sizeof(username));

        // Выпадающий список для выбора роли
        ImGui::Combo("Role", &selectedRoleIndex, roles, IM_ARRAYSIZE(roles));

        // Кнопка для изменения роли
        if (ImGui::Button(menufunctional::settings::language == 0 ? "Change Role" : U8(u8"Изменить роль"))) {
            std::string user(username);
            std::string role(roles[selectedRoleIndex]); // Получение роли по индексу

            if (user.empty()) {
                message = menufunctional::settings::language == 0 ? "Username is required!" : U8(u8"Требуется имя пользователя!");
            }
            else {
                // Изменение роли через ChangeUserRole
                bool success = ChangeUserRole(user, role);
                if (success) {
                    message = menufunctional::settings::language == 0 ? "Role updated successfully!" : U8(u8"Роль успешно обновлена!");
                }
                else {
                    message = menufunctional::settings::language == 0 ? "Failed to update role. User not found." : U8(u8"Не удалось обновить роль. Пользователь не найден.");
                }
            }
        }

        // Отображение сообщения
        ImGui::Text("%s", message.c_str());

        ImGui::End();
    }
}


// Отрисовка регистрации и логина
void SysPanelRegistrator(ImFont* icons_font)
{
    static bool showRegistration = false;
    ImGui::SetNextWindowSize(ImVec2(400,300));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Begin("SSGAMES");
    ImGui::PopStyleColor();
    if (showRegistration) {
        RenderRegistrationUI(showRegistration);
    }
    else {
        RenderLoginUI(showRegistration,icons_font);
    }

    ImGui::End();
}

//void WriteHeadersIfMissing(const std::string& filename, const std::string& headers) {
//    std::ifstream infile(filename);
//    bool headersExist = false;
//
//    if (infile.is_open()) {
//        std::string firstLine;
//        if (std::getline(infile, firstLine)) {
//            headersExist = (firstLine == headers);
//        }
//        infile.close();
//    }
//
//    if (!headersExist) {
//        std::ofstream outfile(filename, std::ios::app);
//        if (outfile.is_open()) {
//            outfile << headers << "\n";
//            outfile.close();
//        }
//    }
//}
//
//void AddRowToExcel(const std::string& filename, const std::vector<std::string>& row) {
//    std::ofstream outfile(filename, std::ios::app);
//    if (outfile.is_open()) {
//        for (size_t i = 0; i < row.size(); ++i) {
//            outfile << row[i];
//            if (i < row.size() - 1) {
//                outfile << ",";
//            }
//        }
//        outfile << "\n";
//        outfile.close();
//    }
//    else {
//        std::cerr << "Error opening file for writing!" << std::endl;
//    }
//}

//void AddGameMenu() {
//    static char name[128] = "";
//    static char release_date[64] = "";
//    static char genre[64] = "";
//    static int sold_copies = 0;
//    static float price = 0.0f;
//
//    ImGui::Text("Add New Game");
//    ImGui::InputText("Name", name, IM_ARRAYSIZE(name));
//    ImGui::InputText("Release Date", release_date, IM_ARRAYSIZE(release_date));
//    ImGui::InputText("Genre", genre, IM_ARRAYSIZE(genre));
//    ImGui::InputInt("Sold Copies", &sold_copies);
//    ImGui::InputFloat("Price", &price);
//
//    if (ImGui::Button("Add Game")) {
//        Game game = { name, release_date, genre, sold_copies, price };
//        AddGame(game);
//        ImGui::Text("Game Added Successfully!");
//    }
//}

//void DeleteGameMenu() {
//    static char name[128] = "";
//
//    ImGui::Text("Delete Game");
//    ImGui::InputText("Game Name", name, IM_ARRAYSIZE(name));
//
//    if (ImGui::Button("Delete Game")) {
//        DeleteGame(name);
//        ImGui::Text("Game Deleted Successfully!");
//    }
//}



// Функция для чтения всех пользователей
std::vector<User> GetAllUsers() {
    std::ifstream infile("users.csv");
    std::vector<User> users;
    std::string line;

    while (std::getline(infile, line)) {
        std::istringstream ss(line);
        User user;
        std::string idStr, balanceStr;

        if (std::getline(ss, idStr, ',') &&
            std::getline(ss, user.username, ',') &&
            std::getline(ss, user.password, ',') &&
            std::getline(ss, user.email, ',') &&
            std::getline(ss, user.role, ',') &&
            std::getline(ss, balanceStr, ',')) {

            try {
                user.id = std::stoi(idStr);
                user.balance = std::stof(balanceStr);
                users.push_back(user);
            }
            catch (...) {
                std::cerr << "Error parsing user data: " << line << std::endl;
            }
        }
    }

    return users;
}

// Функция для записи пользователей обратно в файл
void SaveAllUsers(const std::vector<User>& users) {
    std::ofstream outfile("users.csv");
    if (outfile.is_open()) {
        outfile << "id,user,password,email,role,balance\n";

        for (const auto& user : users) {
            outfile << user.id << "," << user.username << "," << user.password << "," << user.email << "," << user.role << "," << user.balance << "\n";
        }
        outfile.close();
    }
}

// Проверка, куплена ли игра
bool IsGamePurchased(int gameId, const std::string& username) {
    std::ifstream libraryFile(username + "_library.csv");
    if (!libraryFile.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(libraryFile, line)) {
        std::istringstream ss(line);
        int id;
        if (ss >> id && id == gameId) {
            return true;
        }
    }

    return false;
}

// Функция для удаления пользователя по ID
bool DeleteUser(int id) {
    auto users = GetAllUsers();
    bool found = false;

    // Удаляем пользователя с указанным ID
    users.erase(std::remove_if(users.begin(), users.end(),
        [&](const User& user) {
            if (user.id == id) {
                found = true;
                return true;
            }
            return false;
        }), users.end());

    if (found) {
        SaveAllUsers(users); // Сохраняем с заголовками
    }

    return found;
}


bool AddUser(const std::string& username, const std::string& password, const std::string& email, const std::string& role) {
    if (username.empty() || password.empty() || email.empty() || role.empty()) {
        std::cerr << "Error: All fields are required to add a user." << std::endl;
        return false;
    }

    auto users = GetAllUsers();

    // Генерация нового уникального ID
    int newId = 0;
    if (!users.empty()) {
        newId = users.back().id + 1; // ID будет на 1 больше, чем у последнего пользователя
    }

    // Проверяем, нет ли уже пользователя с таким же именем или email
    for (const auto& user : users) {
        if (user.username == username || user.email == email) {
            std::cerr << "Error: Username or email already exists." << std::endl;
            return false;
        }
    }

    // Добавляем нового пользователя
    User newUser = { newId, username, password, email, role };
    users.push_back(newUser);

    SaveAllUsers(users);
    return true;
}

void AddUserUI(ImFont* icons_font) {
    static char username[128] = "";
    static char password[128] = "";
    static char email[128] = "";
    static char role[64] = "";
    static std::string message;
    static bool CloseAddUserWindow = false;
    ImGui::Checkbox(menufunctional::settings::language == 0 ? "Add User" : U8(u8"Добавить пользователя"),&CloseAddUserWindow);
    if (CloseAddUserWindow) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::Begin(menufunctional::settings::language == 0 ? "Add New User" : U8(u8"Добавить нового пользователя"));
        ImGui::PopStyleColor();
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_XMARK))
            CloseAddUserWindow = false;
        ImGui::PopFont();
        ImGui::Text(menufunctional::settings::language == 0 ? "Enter new user details:" : U8(u8"Введите новые данные пользователя:"));
        ImGui::InputText(menufunctional::settings::language == 0 ? "Username" : U8(u8"Имя пользователя"), username, sizeof(username));
        ImGui::InputText(menufunctional::settings::language == 0 ? "Password" : U8(u8"Пароль"), password, sizeof(password));
        ImGui::InputText(menufunctional::settings::language == 0 ? "Email" : U8(u8"Электронная почта"), email, sizeof(email));
        ImGui::InputText(menufunctional::settings::language == 0 ? "Role" : U8(u8"Роль"), role, sizeof(role));

        if (ImGui::Button(menufunctional::settings::language == 0 ? "Add User" : U8(u8"Добавить пользователя"))) {
            // Проверяем, что все поля заполнены
            if (std::strlen(username) == 0 || std::strlen(password) == 0 || std::strlen(email) == 0 || std::strlen(role) == 0) {
                message = menufunctional::settings::language == 0 ? "Error: All fields are required!" : U8(u8"Ошибка: Все поля обязательны для заполнения!");
            }
            else {
                if (AddUser(username, password, email, role)) {
                    message = menufunctional::settings::language == 0 ? "User added successfully!" : U8(u8"Пользователь успешно добавлен!");
                    // Очищаем поля после успешного добавления
                    std::memset(username, 0, sizeof(username));
                    std::memset(password, 0, sizeof(password));
                    std::memset(email, 0, sizeof(email));
                    std::memset(role, 0, sizeof(role));
                }
                else {
                    message = menufunctional::settings::language == 0 ? "Error: Username or email already exists." : U8(u8"Ошибка: Имя пользователя или адрес электронной почты уже существуют.");
                }
            }
        }

        // Вывод сообщения
        if (!message.empty()) {
            ImGui::Text("%s", message.c_str());
        }

        ImGui::End();
    }
}


// Функция для обновления данных пользователя
bool UpdateUser(int id, const std::string& username, const std::string& password, const std::string& email, const std::string& role, float balance = -1.0f) {
    auto users = GetAllUsers();
    bool found = false;

    for (auto& user : users) {
        if (user.id == id) {
            found = true;

            // Обновляем только те данные, которые не пустые
            if (!username.empty()) user.username = username;
            if (!password.empty()) user.password = password;
            if (!email.empty()) user.email = email;
            if (!role.empty()) user.role = role;

            // Обновляем баланс, если он задан
            if (balance >= 0.0f) {
                user.balance = balance;
            }

            break;
        }
    }

    if (found) {
        SaveAllUsers(users); // Сохраняем с заголовками
    }

    return found;
}




// Функция для отображения управление пользователями в ImGui
void UserManagementUI(ImFont* icons_font) {
    static char inputId[10] = "";
    static char username[128] = "";
    static char password[128] = "";
    static char email[128] = "";
    static char role[64] = "";
    static float balance = 0;
    static std::string message; // Сообщение об ошибке или успехе
    static int action = 0; // 0 - ничего, 1 - удалить, 2 - обновить
    static bool UserManagement = false;
    ImGui::Checkbox(menufunctional::settings::language == 0 ? "User Management" : U8(u8"Управление пользователями"),&UserManagement);
    if (UserManagement) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::Begin(menufunctional::settings::language == 0 ? "User Management" : U8(u8"Управление пользователями"));
        ImGui::PopStyleColor();
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_XMARK))
            UserManagement = false;
        ImGui::PopFont();
        ImGui::Text(menufunctional::settings::language == 0 ? "Enter User ID:" : U8(u8"Введите идентификатор пользователя:"));
        ImGui::InputText(menufunctional::settings::language == 0 ? "ID" : U8(u8"ИН"), inputId, sizeof(inputId));

        ImGui::Separator();

        if (ImGui::RadioButton(menufunctional::settings::language == 0 ? "Delete User" : U8(u8"Удалить пользователя"), action == 1)) {
            action = 1;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(menufunctional::settings::language == 0 ? "Update User" : U8(u8"Обновить пользователя"), action == 2)) {
            action = 2;
        }

        if (action == 2) {
            // Поля для обновления пользователя
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Username" : U8(u8"Новое имя пользователя"), username, sizeof(username));
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Password" : U8(u8"Новый пароль"), password, sizeof(password));
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Email" : U8(u8"Новая электронная почта"), email, sizeof(email));
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Role" : U8(u8"Новая роль"), role, sizeof(role));
            ImGui::InputFloat(menufunctional::settings::language == 0 ? "Add money" : U8(u8"Добавить деньги"), &balance, sizeof(balance));
        }

        if (ImGui::Button(menufunctional::settings::language == 0 ? "Continue" : U8(u8"Продолжить"))) {
            int userId;
            try {
                userId = std::stoi(inputId);
            }
            catch (...) {
                message = menufunctional::settings::language == 0 ? "Invalid ID! Please enter a valid number." : U8(u8"Неверный идентификатор! Пожалуйста, введите действительный номер.");
                ImGui::Text("%s", message.c_str());
                ImGui::End();
                return;
            }

            if (action == 1) {
                // Удаление пользователя
                if (DeleteUser(userId)) {
                    message = menufunctional::settings::language == 0 ? "User deleted successfully!" : U8(u8"Пользователь успешно удален!");
                }
                else {
                    message = menufunctional::settings::language == 0 ? "Failed to delete user. User not found." : U8(u8"Не удалось удалить пользователя. Пользователь не найден.");
                }
            }
            else if (action == 2) {
                // Обновление пользователя
                if (UpdateUser(userId, username, password, email, role, balance)) {
                    message = menufunctional::settings::language == 0 ? "User updated successfully!" : U8(u8"Пользователь успешно обновился!");
                }
                else {
                    message = menufunctional::settings::language == 0 ? "Failed to update user. User not found." : U8(u8"Не удалось обновить пользователя. Пользователь не найден.");
                }
            }
            else {
                message = menufunctional::settings::language == 0 ? "Please select an action." : U8(u8"Пожалуйста, выберите действие.");
            }
        }

        // Отображение сообщения об ошибке или успехе
        if (!message.empty()) {
            ImGui::Text("%s", message.c_str());
        }

        ImGui::End();
    }
}

// Вывод таблицы пользователей
void DisplayUsersTable() {
    // Читаем данные пользователей из файла
    std::vector<User> users = GetAllUsers();

    // Отображаем данные в таблице
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (ImGui::BeginTable("UsersTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
       
        // Заголовки таблицы
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "ID": U8(u8"ИН"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Username":U8(u8"Имя пользователя"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Password": U8(u8"Пароль"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Email": U8(u8"Электронная почта"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Role": U8(u8"Роль"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Balance": U8(u8"Баланс"));
        ImGui::TableHeadersRow();
        ImGui::PopStyleColor();
        // Заполняем строки таблицы
        for (const auto& user : users) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", user.id);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", user.username.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", user.password.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", user.email.c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", user.role.c_str());

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("$%.2f", user.balance);
        }

        ImGui::EndTable();
    }
}


// Отображает в таблице игры
void DisplayTable() {
    // Читаем данные из файла
    std::vector<Game> games = ReadGames();

    if (menufunctional::settings::separateexcelwindow) {
        ImGui::SetNextWindowSize(ImVec2(500,300));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::Begin("GameTable");
        ImGui::PopStyleColor();
    }
    // Отображаем данные в таблице
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (ImGui::BeginTable("GameTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        // Заголовки таблицы
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "ID": U8(u8"ИН"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Name": U8(u8"Имя"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Release Date": U8(u8"Дата выхода"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Genre": U8(u8"Жанр"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Sold Copies": U8(u8"Проданные копии"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Price": U8(u8"Цена"));
        ImGui::TableHeadersRow();
        ImGui::PopStyleColor();

        // Заполняем строки таблицы
        for (const auto& game : games) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%d", game.id);

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", game.name.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", game.release_date.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", game.genre.c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%d", game.sold_copies);

            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%.2f", game.price);
        }

       
        ImGui::EndTable();
    }

    if (menufunctional::settings::separateexcelwindow)
    ImGui::End();
}

void SysAddGame()
{
        // сюда надо код
        static char name[128] = "";          // Поле для имени
        static char release_date[64] = "";  // Поле для даты релиза
        static char genre[64] = "";         // Поле для жанра
        static int sold_copies = 0;         // Поле для проданных копий
        static float price = 0.0f;          // Поле для цены

   

        // Поля ввода
        ImGui::InputText(menufunctional::settings::language == 0 ? "Name": U8(u8"Имя"), name, IM_ARRAYSIZE(name));
        ImGui::InputText(menufunctional::settings::language == 0 ? "Release Date": U8(u8"Дата выхода"), release_date, IM_ARRAYSIZE(release_date));
        ImGui::InputText(menufunctional::settings::language == 0 ? "Genre": U8(u8"Жанр"), genre, IM_ARRAYSIZE(genre));
        ImGui::InputInt(menufunctional::settings::language == 0 ? "Sold Copies": U8(u8"Проданные экземпляры"), &sold_copies);
        ImGui::InputFloat(menufunctional::settings::language == 0 ? "Price": U8(u8"Цена"), &price);

        if (ImGui::Button(menufunctional::settings::language == 0 ? "Add Game": U8(u8"Добавить игру"))) {
            // Создать объект Game и добавить его
            Game game = { GetNextGameID(), name, release_date, genre, sold_copies, price };
            AddGame(game);

            // Очистить поля после добавления
            memset(name, 0, sizeof(name));
            memset(release_date, 0, sizeof(release_date));
            memset(genre, 0, sizeof(genre));
            sold_copies = 0;
            price = 0.0f;

            ImGui::Text(menufunctional::settings::language == 0 ? "Game added successfully!": U8(u8"Игра успешно добавлена!"));
        }

      

}

Cart cart;
bool viewingCart = false;

void RenderUI(const std::string& username, ImFont* icons_font) {
    if (menufunctional::login::islogining) {
        if (!cart.IsViewingCart()) {
            DisplayGameStore(cart, icons_font); // Показываем магазин
        }
        else {
            DisplayCartInterface(cart, username); // Показываем корзину
        }
    }
}

void SysDeleteGame()
{
    static int game_id_to_delete = 0; // Поле для ввода ID игры

   // ImGui::Begin("Delete Game");

    // Поле для ввода ID игры
    ImGui::InputInt(menufunctional::settings::language == 0 ? "Game ID": U8(u8"Игровой ИН"), &game_id_to_delete);

    if (ImGui::Button(menufunctional::settings::language == 0 ? "Delete Game": U8(u8"Удалить игру"))) {
        // Вызов функции удаления
        DeleteGame(game_id_to_delete);

        ImGui::Text(menufunctional::settings::language == 0 ? "Game with ID %d deleted successfully (if it existed).": U8(u8"Игра с идентификатором %d успешно удалена (если она существовала)."), game_id_to_delete);

        // Сбрасываем поле ввода
        game_id_to_delete = 0;
    }

  //  ImGui::End();
}

// Экспорт пользователей
void ExportUsersToCSV(const std::vector<User>& users, const std::string& folder) {
    // Создаём папку, если её нет
    std::filesystem::create_directories(folder);

    std::string filepath = folder + "/exported_users.csv";
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing users!" << std::endl;
        return;
    }

    // Записываем заголовки
    file << "ID,Username,Password,Email,Role,Balance\n";

    // Записываем данные пользователей
    for (const auto& user : users) {
        file << user.id << "," << user.username << "," << user.password << ","
            << user.email << "," << user.role << "," << user.balance << "\n";
    }

    file.close();
    std::cout << "Users exported successfully to " << filepath << "\n";
}

// Экспорт игр
void ExportGamesToCSV(const std::vector<Game>& games, const std::string& folder) {
    // Создаём папку, если её нет
    std::filesystem::create_directories(folder);

    std::string filepath = folder + "/exported_games.csv";
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing games!" << std::endl;
        return;
    }

    // Записываем заголовки
    file << "ID,Name,Release Date,Genre,Sold Copies,Price\n";

    // Записываем данные игр
    for (const auto& game : games) {
        file << game.id << "," << game.name << "," << game.release_date << ","
            << game.genre << "," << game.sold_copies << "," << game.price << "\n";
    }

    file.close();
    std::cout << "Games exported successfully to " << filepath << "\n";
}


void SysRenameGame(ImFont* icons_font)
{
    static bool CloseRenameGame = false;
        static int game_id_to_rename = 0;         // Поле для ввода ID игры
        static int new_game_id = -1;             // Поле для ввода нового ID
        static char new_name[128] = "";          // Поле для нового имени
        static char new_release_date[128] = "";  // Поле для новой даты релиза
        static char new_genre[128] = "";         // Поле для нового жанра
        static int new_sold_copies = -1;         // Поле для новых проданных копий
        static float new_price = -1.0f;          // Поле для новой цены
        static bool Change_id = false;
        ImGui::Checkbox(menufunctional::settings::language == 0 ? "Rename Game" : U8(u8"Переименовать игру"), &CloseRenameGame);

        if (CloseRenameGame) {
            ImGui::SetNextWindowSize(ImVec2(650,300));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::Begin(menufunctional::settings::language == 0 ? "Rename Game" : U8(u8"Переименовать игру"));
            ImGui::PopStyleColor();
            ImGui::PushFont(icons_font);
            if (ImGui::Button(ICON_FA_XMARK))
                CloseRenameGame = false;
            ImGui::PopFont();
            ImGui::Text(menufunctional::settings::language == 0 ? "Edit Game Fields by ID" : U8(u8"Редактирование игровых полей по идентификатору"));
            ImGui::Separator();

            // Поле для ввода текущего ID игры
            ImGui::InputInt(menufunctional::settings::language == 0 ? "Current Game ID" : U8(u8"Текущий игровой ИН"), &game_id_to_rename);

            // Поле для ввода нового ID игры



            // Поля для редактирования других данных
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Name" : U8(u8"Новое имя"), new_name, IM_ARRAYSIZE(new_name));
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Release Date" : U8(u8"Новая дата выхода"), new_release_date, IM_ARRAYSIZE(new_release_date));
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Genre" : U8(u8"Новый жанр"), new_genre, IM_ARRAYSIZE(new_genre));
            ImGui::InputInt(menufunctional::settings::language == 0 ? "New Sold Copies" : U8(u8"Новые проданные экземпляры"), &new_sold_copies);
            ImGui::InputFloat(menufunctional::settings::language == 0 ? "New Price" : U8(u8"Новая цена"), &new_price);

            if (ImGui::Button(menufunctional::settings::language == 0 ? "Change id" : U8(u8"Изменить ИН")))
                Change_id = !Change_id;
            if (Change_id)
                ImGui::InputInt(menufunctional::settings::language == 0 ? "New Game ID" : U8(u8"Новый игровой ИН"), &new_game_id);

            // Кнопка для применения изменений
            if (ImGui::Button(menufunctional::settings::language == 0 ? "Update Game" : U8(u8"Обновить игру"))) {
                // Вызываем функцию RenameGameField с передачей нового ID
                RenameGameField(
                    game_id_to_rename,  // Текущий ID
                    new_game_id,        // Новый ID (или оставляем -1, если не изменяется)
                    std::string(new_name),
                    std::string(new_release_date),
                    std::string(new_genre),
                    new_sold_copies,
                    new_price
                );

                // Очистить поля после обновления
                game_id_to_rename = 0;
                new_game_id = -1;
                memset(new_name, 0, sizeof(new_name));
                memset(new_release_date, 0, sizeof(new_release_date));
                memset(new_genre, 0, sizeof(new_genre));
                new_sold_copies = -1;
                new_price = -1.0f;

                ImGui::Text(menufunctional::settings::language == 0 ? "Game updated successfully!" : U8(u8"Игра успешно обновлена!"));
            }

            ImGui::End();
        }
}
// основная функция для отрисовки
void render(ImFont* icons_font, ImFont* small_text_font, ImFont* text_font)
{
    
    ImGui::PushFont(text_font);
    MainStyle();
    if (menufunctional::login::islogining && menufunctional::role == "root") {
        

        ImGui::SetNextWindowSize(ImVec2(500, 300));
        ImGui::Begin("Add Game Main", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);
        if (ImGui::Button(menufunctional::settings::language == 0 ? "Log out account" : U8(u8"Выйти из аккаунта"))) {
            buttonpress = false;
            menufunctional::login::islogining = false;
        }
        ImGui::SetCursorPos(ImVec2(240,5));
        if(ImGui::Button(menufunctional::settings::language == 0 ? "Open window shop": U8(u8"Открыть окно магазина")))
            buttonpress = !buttonpress;
        ImGui::SetCursorPos(ImVec2(10, 30));
        Tab(icons_font);
        ImGui::SetCursorPos(ImVec2(100, 30));
        ImGui::BeginChild(" Main ", ImVec2(390, 250));

        if (menufunctional::menu_main_button::tab == 0) {
            SysAddGame();
           
        }
        if (menufunctional::menu_main_button::tab == 1)
        {
            DisplayTable();

            SysDeleteGame();

            SysRenameGame(icons_font);

            SysCheckDate(icons_font);
            if (ImGui::Button(menufunctional::settings::language == 0 ? "Export Games": U8(u8"Экспорт игр"))) {
                auto games = ReadGames();
                ExportGamesToCSV(games, "export");
            }
        }

        if (menufunctional::menu_main_button::tab == 2)
        {
            DisplayUsersTable();
            UserManagementUI(icons_font);
            RenderAdminUI(icons_font);
            AddUserUI(icons_font);

            if (ImGui::Button(menufunctional::settings::language == 0 ? "Export Users": U8(u8"Экспорт Пользователей"))) {
                auto users = GetAllUsers();
                ExportUsersToCSV(users, "export");
            }

        }

        //   DeleteGame();

        UpdateLoadingState();

       /* ImGui::SetCursorPos(ImVec2(130, 60));
        if (menufunctional::menu_main_button::loading)
            DrawLoadingSpinner();*/

        ImGui::EndChild();

        ImGui::SetCursorPos(ImVec2(410, 3));
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_CIRCLE_INFO))
            menufunctional::info::open = !menufunctional::info::open;
        ImGui::PopFont();

        ImGui::SetCursorPos(ImVec2(440, 3));
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_GEAR))
            menufunctional::settings::open = !menufunctional::settings::open;
        ImGui::PopFont();

        ImGui::SetCursorPos(ImVec2(470, 3));
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_XMARK))
            menufunctional::menu_main_button::closemenu = true;
        ImGui::PopFont();

        if (menufunctional::settings::open)
            Setting(icons_font);

        if (menufunctional::info::open)
            Info(icons_font);

        ImGui::PushFont(small_text_font);
        ImGui::PopFont();

        ImGui::End();



    }
    else if (menufunctional::login::islogining && menufunctional::role != "root")
    {
        RenderUI(menufunctional::login::currentUser, icons_font);
    }


    if (buttonpress)
    {
        RenderUI(menufunctional::login::currentUser, icons_font);
    }
    if (!menufunctional::login::islogining)
        SysPanelRegistrator(icons_font);
    ImGui::PopFont();
}

void Tab(ImFont* icons_font)
{

    /*ImGui::SetNextWindowSize(ImVec2(80, 130));
    ImGui::Begin("Add Game Tab", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);*/

    ImGui::BeginChild("Tab", ImVec2(85, 250));

    ImGui::SetCursorPosX(10);
    ImGui::Text("SSGames");
    ImGui::Separator();

    ImGui::SetCursorPos(ImVec2(menufunctional::animationtab::pos_buttonX, menufunctional::animationtab::pos_buttonY));
    ImGui::PushFont(icons_font);
    if (ImGui::Button(ICON_FA_HOUSE, ImVec2(60, 50))) {
        menufunctional::menu_main_button::tab = 0;
        menufunctional::menu_main_button::loading = true;
    }
    ImGui::PopFont();

    ImGui::PushFont(icons_font);
    ImGui::SetCursorPos(ImVec2(menufunctional::animationtab::pos_buttonX, menufunctional::animationtab::pos_buttonY + 60));
    if (ImGui::Button(ICON_FA_GAMEPAD, ImVec2(60, 50))) {
        menufunctional::menu_main_button::tab = 1;
        menufunctional::menu_main_button::loading = true;
    }
    ImGui::PopFont();

    ImGui::PushFont(icons_font);
    ImGui::SetCursorPos(ImVec2(menufunctional::animationtab::pos_buttonX, menufunctional::animationtab::pos_buttonY + 120));
    if (ImGui::Button(ICON_FA_USER, ImVec2(60, 50))) {
        menufunctional::menu_main_button::tab = 2;
        menufunctional::menu_main_button::loading = true;
    }
    ImGui::PopFont();

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.25f, 0.71f, 0.57f, 1.0f));
    ImGui::SetCursorPos(ImVec2(2, menufunctional::animationtab::animpos));
    ImGui::BeginChild("AnimTab", ImVec2(8, 20));
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::EndChild();
    // ImGui::End();

    if (menufunctional::menu_main_button::tab == 0 && menufunctional::animationtab::animpos > 55)
        menufunctional::animationtab::animpos -= menufunctional::animationtab::speedanim;
    if (menufunctional::menu_main_button::tab == 1 && menufunctional::animationtab::animpos < 115)
        menufunctional::animationtab::animpos += menufunctional::animationtab::speedanim;
    if (menufunctional::menu_main_button::tab == 1 && menufunctional::animationtab::animpos > 115)
        menufunctional::animationtab::animpos -= menufunctional::animationtab::speedanim;
    if (menufunctional::menu_main_button::tab == 2 && menufunctional::animationtab::animpos < 175)
        menufunctional::animationtab::animpos += menufunctional::animationtab::speedanim;
}

void Setting(ImFont* icons_font)
{
    ImGui::SetNextWindowSize(ImVec2(400, 300));
    ImGui::Begin("Settings", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking);
    ImGui::SetCursorPos(ImVec2(150, 0));
    ImGui::Text(menufunctional::settings::language == 0 ? "Settings" : U8(u8"Настройки"));
    ImGui::Separator();

    ImGui::SetCursorPos(ImVec2(10, 30));
    ImGui::BeginChild("InfoChild", ImVec2(380, 260));

    //ImGui::Checkbox(menufunctional::settings::language == 0 ? "Pin the side menu" : U8(u8"Закрепить боковое меню"), &menufunctional::settings::pintab);
    ImGui::SetCursorPos(ImVec2(10, 5));
    ImGui::Checkbox(menufunctional::settings::language == 0 ? "Edit color and menu" : U8(u8"Изменить цвет и меню"), &menufunctional::editcolormenu::open);

    if (menufunctional::role == "root") {
        ImGui::SetCursorPos(ImVec2(10, 40));
        ImGui::Checkbox(menufunctional::settings::language == 0 ? "separate excel window" : U8(u8"Отдельное окно вывода"), &menufunctional::settings::separateexcelwindow);
    }
    if (menufunctional::editcolormenu::open)
        Color_Menu_Edit();

    ImGui::SetCursorPos(ImVec2(10, 75));
    //Language();
    if (menufunctional::role == "root") {
        ImGui::SetCursorPos(ImVec2(10, 110));
        ImGui::Text(menufunctional::settings::language == 0 ? "Speed animation: " : U8(u8"Скорость анимации: "));
        ImGui::SliderFloat("", &menufunctional::animationtab::speedanim, 0.0f, 20.0f, "%.0f", 2.0f);
    }
    ImGui::EndChild();
    ImGui::End();
}


void Color_Menu_Edit()
{
    ImGui::SetNextWindowSize(ImVec2(400, 300));
    ImGui::Begin("ColorMenuEdit", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar);
    ImGui::SetCursorPos(ImVec2(150, 0));
    ImGui::Text(menufunctional::settings::language == 0 ? "Edit color and menu" : U8(u8"Изменить цвет и меню"));
    ImGui::Separator();
    ImGui::SetCursorPos(ImVec2(5, 30));
    ImGui::BeginChild("InfoChild", ImVec2(390, 260));

    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button" : U8(u8"Кнопка"), (float*)&menufunctional::editcolormenu::Button, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Hover button" : U8(u8"Наведение на кнопку"), (float*)&menufunctional::editcolormenu::ButtonHovered, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Active button" : U8(u8"Активная кнопка"), (float*)&menufunctional::editcolormenu::ButtonActive, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Frame backround" : U8(u8"Фон рамок"), (float*)&menufunctional::editcolormenu::FrameBg, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Slider Grab" : U8(u8"Захват ползунка"), (float*)&menufunctional::editcolormenu::SliderGrab, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Slider Active" : U8(u8"Активный ползунок"), (float*)&menufunctional::editcolormenu::SliderActive, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Slider Hovered" : U8(u8"Наведение ползунка"), (float*)&menufunctional::editcolormenu::SliderHovered, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Text" : U8(u8"Текст"), (float*)&menufunctional::editcolormenu::Text, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Text error" : U8(u8"Ошибка текста"), (float*)&menufunctional::editcolormenu::TextError, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Window Backround" : U8(u8"Фон окна"), (float*)&menufunctional::editcolormenu::WindowBg, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Delete" : U8(u8"Кнопка: Удалить"), (float*)&menufunctional::editcolormenu::Button_delete, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Delete - Hovered" : U8(u8"Кнопка: Удалить - Наведение"), (float*)&menufunctional::editcolormenu::Button_delete_Hovered, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Delete - Active" : U8(u8"Кнопка:Удалить - Активная"), (float*)&menufunctional::editcolormenu::Button_delete_Active, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Confirm Delete" : U8(u8"Кнопка: Подтвердить удаление"), (float*)&menufunctional::editcolormenu::Button_confirmdelete, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Confirm Delete - Hovered" : U8(u8"Кнопка: Подтвердить удаление - Наведение"), (float*)&menufunctional::editcolormenu::Button_confirmdelete_Hovered, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Confirm Delete - Active" : U8(u8"Кнопка: Подтвердить удаление - Активная"), (float*)&menufunctional::editcolormenu::Button_confirmdelete_Active, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Cancle" : U8(u8"Кнопка: Отмена "), (float*)&menufunctional::editcolormenu::Button_cancle, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Cancle - Hovered" : U8(u8"Кнопка: Отмена - Наведение "), (float*)&menufunctional::editcolormenu::Button_cancle_Hovered, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Cancle - Active" : U8(u8"Кнопка: Отмена - Активная "), (float*)&menufunctional::editcolormenu::Button_cancle_Active, ImGuiColorEditFlags_NoInputs);

    ImGui::PushItemWidth(150);
    ImGui::SliderFloat(menufunctional::settings::language == 0 ? "Menu rounding" : U8(u8"Закруглить меню"), &menufunctional::editcolormenu::FrameRounding, 0.0f, 10.0f);
    ImGui::SliderFloat(menufunctional::settings::language == 0 ? "Grab slider rounding" : U8(u8"Закруглить захват ползунка"), &menufunctional::editcolormenu::GrabRounding, 0.0f, 10.0f);
    ImGui::PopItemWidth();

    ImGui::EndChild();
    ImGui::End();
}

void Info(ImFont* icons_font)
{
    ImGui::SetNextWindowSize(ImVec2(300, 300));
    ImGui::Begin("Info", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking);
    ImGui::SetCursorPos(ImVec2(105, 0));
    ImGui::Text(menufunctional::settings::language == 0 ? "Information" : U8(u8"Информация"));
    ImGui::Separator();
    ImGui::SetCursorPos(ImVec2(10, 30));
    ImGui::BeginChild("InfoChild", ImVec2(280, 260));



    ImGui::EndChild();
    ImGui::End();

}

ImVec2 SumVectors(const ImVec2& vec1, const ImVec2& vec2) {
    return ImVec2(vec1.x + vec2.x, vec1.y + vec2.y);
}
//void DrawLoadingSpinner() {
//    static float angle = 0.0f;
//    static const float speed = 1.0f;
//
//    angle += ImGui::GetIO().DeltaTime * speed;
//
//    ImDrawList* draw_list = ImGui::GetWindowDrawList();
//    ImVec2 result = SumVectors(ImGui::GetCursorScreenPos(), ImVec2(50, 50));
//    ImVec2 center = result;
//    float radius = 20.0f;
//
//    float alphaSA = 0.5f + 0.5f * sin(ImGui::GetTime() * 2);
//
//    ImVec4 text_colorD = ImVec4(1.0f, alphaSA, 1.0f, 1.0f);
//    ImGui::PushStyleColor(ImGuiCol_Text, text_colorD);
//
//    //colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.15f, 0.15 + alphaSA * 0.020, 0.2f);
//
//    for (int i = 0; i < 12; i++) {
//        float theta = angle + (i * (IM_PI / 6));
//        float x = center.x + cos(theta) * radius;
//        float y = center.y + sin(theta) * radius;
//
//
//        ImColor color(0.65f + sin(ImGui::GetTime() * 1), 0.75f + sin(ImGui::GetTime() * 2), 1.0f, 1.0f - (i / 12.0f));
//        draw_list->AddCircleFilled(ImVec2(x, y), 5.0f, color);
//        ImGui::PopStyleColor();
//    }
//}