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
    int id; // ����� ������������� ����
    std::string name;
    std::string release_date;
    std::string genre;
    int sold_copies;
    float price;
};

// ��������� ������������
struct User {
    int id;
    std::string username;
    std::string password;
    std::string email;
    std::string role;
    float balance = 5.0f; // ��������� ������ � $5
};
std::vector<User> GetAllUsers();
void SaveAllUsers(const std::vector<User>&); // ��������� ������ � ������������
class Cart {
public:
    std::vector<Game> items; // ������ ��� � �������
    bool viewingCart = false; // ���� ���������: ���������� ������� ��� �������

    // ����� ��� ���������� ���� � �������
    void AddToCart(const Game& game) {
        items.push_back(game);
    }

    // ����� ��� �������� ���� �� �������
    void RemoveFromCart(int gameId) {
        auto it = std::remove_if(items.begin(), items.end(), [gameId](const Game& game) {
            return game.id == gameId;
            });
        items.erase(it, items.end());
    }

    // ����� ��� ����, ����� ���������� ��������� ������� (�������� ������� ��� ��������)
    void SetViewingCart(bool state) {
        viewingCart = state;
    }

    // ����� ��� ��������, ������������� �� ������������ �������
    bool IsViewingCart() const {
        return viewingCart;
    }

    // ����� ��� ���������� �������
    void Purchase(const std::string& username) {
        // ���������� ������ �������, ��������:
        std::cout << "User " << username << " has purchased the following games:" << std::endl;
        for (const auto& game : items) {
            std::cout << "Game: " << game.name << " - $" << game.price << std::endl;
        }
        // ����� ������� ������� �������
        items.clear();
    }
};


// ������� ����
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

    // ��������� ���������� ����� ��� � ���������� � ����������
    for (auto& game : cart.items) {
        if (!IsGamePurchased(game.id, username)) {
            // ��������� ���������� ���������� ����� ����
            if (game.sold_copies == 0) {
                continue;
            }

            game.sold_copies--;

            libraryFile << game.id << "," << game.name << "," << game.release_date << ","
                        << game.genre << "," << game.sold_copies + 1 << "," << game.price << "\n";

            // ��������� ������ � ���������� ����� � ����� games.csv
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
                        g.sold_copies = game.sold_copies; // ��������� ���������� ���������� �����
                    }
                    allGames.push_back(g);
                }
            }
            gameFile.close();

            // ���������� ����������� ������ � ����
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

// ����������� ������ ��������� ���
void DisplayPurchasedGames(const std::string& username) {
    std::ifstream libraryFile(username + "_library.csv");
    if (!libraryFile.is_open()) {
        ImGui::SetNextWindowSize(ImVec2(400, 200));
        ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.0f,1.0f,1.0f,1.0f));
        ImGui::Begin(menufunctional::settings::language == 0 ? "Library" : U8(u8"����������"));
        ImGui::PopStyleColor();
        ImGui::Text(menufunctional::settings::language == 0 ? "No games purchased yet.": U8(u8"���� ��� �� �������."));
        ImGui::End();
        return;
    }
   // ImGui::SetNextWindowSize(ImVec2(400, 300));
    ImGui::SetNextWindowSize(ImVec2(400, 200));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Begin(menufunctional::settings::language == 0 ? "Purchased Games":U8(u8"������������� ����"));
    ImGui::PopStyleColor();
    ImGui::Text(menufunctional::settings::language == 0 ? "Purchased games:":U8(u8"������������� ����:"));

    std::string line;
    std::string gamesList;
    while (std::getline(libraryFile, line)) {
        size_t firstCommaPos = line.find(',');
        if (firstCommaPos == std::string::npos) continue; // ��� �������, ���������� ������

        size_t secondCommaPos = line.find(',', firstCommaPos + 1);
        std::string gameName = (secondCommaPos != std::string::npos) ? line.substr(firstCommaPos + 1, secondCommaPos - firstCommaPos - 1) : line.substr(firstCommaPos + 1);

        if (!gamesList.empty()) {
            gamesList += ", ";
        }
        gamesList += gameName;
    }

    // �������� ������, ���� ��� ������� �������
    if (gamesList.length() > 100) {
        gamesList = gamesList.substr(0, 97) + "...";
    }

    ImGui::Text("%s", gamesList.c_str());
    ImGui::End();
}


// ����������� ������� ������������
void DisplayUserBalance(const std::string& username) {
    auto users = GetAllUsers();
    auto userIt = std::find_if(users.begin(), users.end(), [&](const User& user) {
        return user.username == username;
        });

    if (userIt == users.end()) {
        ImGui::Text(menufunctional::settings::language == 0 ? "User not found!" :U8(u8"������������ �� ������!"));
        return;
    }

    // ���������� ������ ������������ ����� ImGui
    ImGui::Text(menufunctional::settings::language == 0 ? "Your current balance: $%.2f" :U8(u8"��� ������� ������: $%.2f"), userIt->balance);
}


// ������ ������ �� �����
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
void DisplayGameStore(Cart& cart, ImFont* icons_font) {// ���������� ������� �������
    // ������ ������ �� �����
    std::vector<Game> games = ReadGames();
    static bool library_on = false;
    //ImGui::SetNextWindowSize(ImVec2(1200,500));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::Begin(menufunctional::settings::language == 0 ? "Game Store": U8(u8"������� �������"));
    ImGui::PopStyleColor();
    if (menufunctional::role == "user")
    {
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_XMARK))
            menufunctional::menu_main_button::closemenu = true;
        ImGui::PopFont();

        if (ImGui::Button(menufunctional::settings::language == 0 ? "Log out account" : U8(u8"����� �� ��������")))
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
        // ���������� ������ � �������
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        if (ImGui::BeginTable(menufunctional::settings::language == 0 ? "GameTable" : U8(u8"������� ����") , 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
            // ��������� �������
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "ID": U8(u8"��"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Name": U8(u8"���"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Release Date": U8(u8"���� ������"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Genre":U8(u8"����"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Sold Copies": U8(u8"��������� ����������"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Price": U8(u8"����"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Action": U8(u8"��������"));
            ImGui::TableHeadersRow();
            ImGui::PopStyleColor();
            // ��������� ������ �������
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
                if (game.sold_copies > 0) { // ���������, ��� ���������� ����� ������ ����
                    if (ImGui::Button((menufunctional::settings::language == 0 ? "Add to Cart##" : U8(u8"�������� � �������") + std::to_string(game.id)).c_str())) {
                        cart.AddToCart(game);
                    }
                }
                else {
                    ImGui::Text(menufunctional::settings::language == 0 ? "Out of stock" : U8(u8"��� � �������")); // ��� ������ ����� ��� ����������� ���
                }

            }

            ImGui::EndTable();
        }
        if (ImGui::Button(menufunctional::settings::language == 0 ? "Library" : U8(u8"����������")))
            library_on = !library_on;

        if (library_on)
            DisplayPurchasedGames(menufunctional::login::currentUser);

        ImGui::Separator();

        // ������ ��� �������� � �������
        if (ImGui::Button(menufunctional::settings::language == 0 ? "View Cart": U8(u8"�������� �������"))) {
            // ������� � ��������� �������
            cart.SetViewingCart(true);
        }

        ImGui::End();
    }



void DisplayCartInterface(Cart& cart, const std::string& username) {// ���������� ������� �������
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    ImGui::SetNextWindowSize(ImVec2(700,400));
    ImGui::Begin(menufunctional::settings::language == 0 ? "Shopping Cart" : U8(u8"������� �������"));
    ImGui::PopStyleColor();
    if (cart.items.empty()) {
        ImGui::Text(menufunctional::settings::language == 0 ? "Your cart is empty." :U8(u8"���� ������� �����."));
    }
    else {
        ImGui::Text(menufunctional::settings::language == 0 ? "Games in your cart:" : U8(u8"���� � ����� �������:"));
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        if (ImGui::BeginTable("CartTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "ID" : U8(u8"��"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Name" : U8(u8"���"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Price" : U8(u8"����"));
            ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Action" : U8(u8"��������"));
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
                if (ImGui::Button((menufunctional::settings::language == 0 ? "Remove##" : U8(u8"�������##") + std::to_string(i)).c_str())) {
                    cart.RemoveFromCart(game.id);
                }
            }

            ImGui::EndTable();
        }

        ImGui::Separator();
        ImGui::Text(menufunctional::settings::language == 0 ? "Total items: %d": U8(u8"����� ���������� �������: %d"), static_cast<int>(cart.items.size()));
        float totalPrice = 0.0f;
        for (const auto& game : cart.items) {
            totalPrice += game.price;
        }
        ImGui::Text(menufunctional::settings::language == 0 ? "Total price: $%.2f": U8(u8"�������� ����: $%.2f"), totalPrice);

        if (ImGui::Button(menufunctional::settings::language == 0 ? "Purchase": U8(u8"�������"))) {
            PurchaseGames(cart, username);
            cart.items.clear();
            ImGui::Text(menufunctional::settings::language == 0 ? "Purchase completed successfully!": U8(u8"������� ������� ���������!"));
        }
    }

    ImGui::Separator();

    if (ImGui::Button(menufunctional::settings::language == 0 ? "Back to Store": U8(u8"��������� � �������"))) {
        cart.SetViewingCart(false);
    }

    ImGui::End();
}



std::string GetCurrentDateTime() { // ���������� ����� �������� ����
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::tm now_tm = {};
    // ���������� localtime_s ��� ����������� ��������� �������
    if (localtime_s(&now_tm, &now_time) != 0) {
        return "";
    }

    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

int GetNextGameID() { // ���������� id ���� � �������
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
    return max_id + 1; // ��������� ���������� ID
}

void AddGame(const Game& game) { // ��������� ���� � �������
    const std::string headers = "ID,Name,Release Date,Genre,Sold Copies,Price";
    std::ifstream infile("games.csv");
    bool headersExist = false;

    // ��������� ������� ����������
    if (infile.is_open()) {
        std::string firstLine;
        if (std::getline(infile, firstLine)) {
            headersExist = (firstLine == headers);
        }
        infile.close();
    }

    // ���� ���������� ���, ��������� ��
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

    // ��������� ������ �� ����
    std::ofstream file("games.csv", std::ios::app);
    if (file.is_open()) {
        file << game.id << "," << game.name << "," << game.release_date << ","
            << game.genre << "," << game.sold_copies << "," << game.price << "\n";
        file.close();
    }
    else {
        std::cerr << "Error opening file to add game!" << std::endl;
    }

    // ���������� ���� �������� � ���� date.csv
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


// �������� ����
void DeleteGame(int game_id) {
    std::ifstream file("games.csv");
    if (!file.is_open()) {
        std::cerr << "Error opening file for reading!" << std::endl;
        return;
    }

    std::vector<std::string> lines; // ��� �������� ���� �����, ����� ���������
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        int id;
        ss >> id;
        if (id != game_id) { // ���� ID �� ���������, ��������� ������
            lines.push_back(line);
        }
    }

    file.close();

    // �������������� ���� � ����������� ��������
    std::ofstream outfile("games.csv", std::ios::trunc); // ��������� ���� � �������� �����������
    if (!outfile.is_open()) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    for (const auto& saved_line : lines) {
        outfile << saved_line << "\n";
    }

    outfile.close();
}

// ������� ��� ��������� ����������� ����� ���� �� ID
void RenameGameField(int game_id, int new_id = -1, const std::string& new_name = "", const std::string& new_release_date = "", const std::string& new_genre = "", int new_sold_copies = -1, float new_price = -1.0f) {
    std::ifstream file("games.csv");
    if (!file.is_open()) {
        std::cerr << "Error opening file for reading!" << std::endl;
        return;
    }

    std::vector<std::string> lines;
    std::string line;
    bool updated = false;

    // ��������� ��� �����
    const std::string headers = "ID,Name,Release Date,Genre,Sold Copies,Price";

    // ��������� ������ ������ � ��������� ���������
    if (std::getline(file, line)) {
        if (line != headers) {
            lines.push_back(headers); // ��������� ���������, ���� �� �����������
        }
        else {
            lines.push_back(line);
        }
    }

    std::unordered_set<int> existing_ids; // ��� �������� ������������ ID

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

            existing_ids.insert(id); // ���������� ������������ ID

            if (id == game_id) {
                if (new_id >= 0 && new_id != game_id) {
                    // ���������, ��� ����� ID ��������
                    if (existing_ids.count(new_id)) {
                        std::cerr << "Error: New ID " << new_id << " already exists!" << std::endl;
                        return;
                    }
                    id = new_id; // �������� ID
                    existing_ids.insert(new_id); // ��������� ����� ID � ���������
                }

                // ��������� ������ ����
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

        // ���������� ��������� ��� �����������
        if (ss >> id && std::getline(ss, date, '\t')) {
            // ������� ��������� � �������� ������� � ����� ������ �������, ������� �������
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
    return "Not Found"; // ���������� "Not Found", ���� ID �� ����������
}

//// ������� ��� ��������� ���� �������� �� ID
//std::string GetCreationDate(int id, const std::unordered_map<int, std::string>& creation_dates) {
//    auto it = creation_dates.find(id);
//    if (it != creation_dates.end()) {
//        return it->second;
//    }
//    return "Date not found";
//}


// ����� � ����������� ����������
void SysCheckDate(ImFont* icons_font) {
    static int id_to_check = 0;
    static std::string creation_date;
    static bool CloseCheckDate = false;
    ImGui::Checkbox(menufunctional::settings::language == 0 ? "Check Date" : U8(u8"�������� ����"),&CloseCheckDate);
    if (CloseCheckDate) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::Begin(menufunctional::settings::language == 0 ? "Check Creation Date" : U8(u8"�������� ���� ��������"));
        ImGui::PopStyleColor();
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_XMARK))
            CloseCheckDate = false;
        ImGui::PopFont();
        ImGui::InputInt(menufunctional::settings::language == 0 ? "Enter ID" : U8(u8"������� ��"), &id_to_check);

        if (ImGui::Button(menufunctional::settings::language == 0 ? "Check Date" : U8(u8"���� ��������"))) {
            // ��������� ������ � ���� ����
            auto creation_dates = LoadCreationDates("date.csv");
            creation_date = GetCreationDate(id_to_check, creation_dates);
        }

        ImGui::Text(menufunctional::settings::language == 0 ? "Creation Date: %s" : U8(u8"���� ��������: %s"), creation_date.c_str());

        ImGui::End();
    }
}



// �������� ������� ���������� � ����� users.csv � ����������, ���� �� ���
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

// ������� ��� �������� ���������� ����������� �����
bool IsValidEmail(const std::string& email) {
    const std::regex pattern(R"((\w+)(\.\|_)?(\w*)@(\w+)(\.(\w+))+)");
    return std::regex_match(email, pattern);
}
// ������� ��� �������� ������������
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
                return true; // ������� ����������
            }
        }
    }

    return false; // ���������� ���
}
// ������� ��� ���������� ���������� � ��
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


// ������� ��� �������� ������ ��� ������
bool ValidateLogin(const std::string& username, const std::string& password) {
    std::ifstream file("users.csv");
    std::string line;

    if (file.is_open()) {
        bool isFirstLine = true;
        while (std::getline(file, line)) {
            if (isFirstLine) {
                isFirstLine = false;
                continue; // ���������� ���������
            }

            std::stringstream ss(line);
            std::string file_id, file_username, file_password, file_email, file_role;

            std::getline(ss, file_id, ',');
            std::getline(ss, file_username, ',');
            std::getline(ss, file_password, ',');
            std::getline(ss, file_email, ',');
            std::getline(ss, file_role);

            // ������� ��������� ������ �������
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

// ������� ��� ��������� ���� ������������
std::string GetUserRole(const std::string& username) {
    std::ifstream file("users.csv");
    std::string line;

    if (file.is_open()) {
        bool isFirstLine = true;
        while (std::getline(file, line)) {
            if (isFirstLine) {
                isFirstLine = false;
                continue; // ���������� ���������
            }

            std::stringstream ss(line);
            std::string file_id, file_username, file_password, file_email, file_role, file_balance, file_price;

            // ������ ���� �������� (� ������ ������ ������� price)
            std::getline(ss, file_id, ',');
            std::getline(ss, file_username, ',');
            std::getline(ss, file_password, ',');
            std::getline(ss, file_email, ',');
            std::getline(ss, file_role, ',');
            std::getline(ss, file_balance, ',');  // ������ ������ (������� ��� �� ���������� price)
            std::getline(ss, file_price);         // ������ ����� ���� price

            // ������� ������ �������
            file_role.erase(0, file_role.find_first_not_of(" \t\n\r"));
            file_role.erase(file_role.find_last_not_of(" \t\n\r") + 1);

            if (username == file_username) {
                return file_role; // ���������� ����, ���� ����� ������������
            }
        }
        file.close();
    }
    return "Role not found";
}

// ��������� ������
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

    ImGui::Text(menufunctional::settings::language == 0 ? "Login:" : U8(u8"��������������:"));
    ImGui::InputText(menufunctional::settings::language == 0 ? "Username":U8(u8"��� ������������"), loginUsername, sizeof(loginUsername));
    ImGui::InputText(menufunctional::settings::language == 0 ? "Password":U8(u8"������"), loginPassword, sizeof(loginPassword), ImGuiInputTextFlags_Password);

    if (ImGui::Button(menufunctional::settings::language == 0 ? "Login":U8(u8"��������������"))) {
        if (ValidateLogin(loginUsername, loginPassword)) {
            menufunctional::role = GetUserRole(loginUsername);
            message = menufunctional::settings::language == 0 ? "Login successful! Role: ":U8(u8"���� � ������� ������ �������! ����:") + menufunctional::role;
            menufunctional::login::currentUser = loginUsername;
            menufunctional::login::islogining = true;
        }
        else {
            message = menufunctional::settings::language == 0 ? "Invalid username or password.":U8(u8"�������� ��� ������������ ��� ������");
        }
    }

    ImGui::Text("%s", message.c_str());

    ImGui::Separator();
    ImGui::Text(menufunctional::settings::language == 0 ? "Not registered yet?":U8(u8"��� �� ������������������?"));
    if (ImGui::Button(menufunctional::settings::language == 0 ? "Register here":U8(u8"����������������� �����"))) {
        showRegistration = true;
    }

    //ImGui::End();
}

// ��������� �����������
void RenderRegistrationUI(bool& showRegistration) {
    static char username[128] = "";
    static char password[128] = "";
    static char email[128] = "";
    static std::string message;

    //ImGui::Begin("Registration");

    ImGui::Text(menufunctional::settings::language == 0 ? "Register:" : U8(u8"������������������:"));
    ImGui::InputText(menufunctional::settings::language == 0 ? "Username" : U8(u8"��� ������������"), username, sizeof(username));
    ImGui::InputText(menufunctional::settings::language == 0 ? "Password" : U8(u8"������"), password, sizeof(password), ImGuiInputTextFlags_Password);
    ImGui::InputText(menufunctional::settings::language == 0 ? "Email" : U8(u8"����������� �����"), email, sizeof(email));

    if (ImGui::Button(menufunctional::settings::language == 0 ? "Register" : U8(u8"����������������"))) {
        std::string user(username);
        std::string pass(password);
        std::string mail(email);

        if (user.empty() || pass.empty() || mail.empty()) {
            message = menufunctional::settings::language == 0 ? "All fields are required!" : U8(u8"��� ���� ����������� ��� ����������!");
        }
        else if (!IsValidEmail(mail)) {
            message = menufunctional::settings::language == 0 ? "Invalid email format!" : U8(u8"������������ ������ ����������� �����!");
        }
        else if (IsUserOrEmailExists(user, mail)) {
            message = menufunctional::settings::language == 0 ? "Username or email already exists!" : U8(u8"��� ������������ ��� ����� ����������� ����� ��� ����������!");
        }
        else {
            SaveUserData(user, pass, mail);
            message = menufunctional::settings::language == 0 ? "Registration successful!": U8(u8"����������� ������ �������!");
            showRegistration = false;
        }
    }

    ImGui::Text("%s", message.c_str());

    if (ImGui::Button(menufunctional::settings::language == 0 ? "Back to Login" : U8(u8"��������� � �����"))) {
        showRegistration = false;
    }

    //ImGui::End();
}

// �������� ���� ������������
bool ChangeUserRole(const std::string& username, const std::string& newRole) {
    std::ifstream infile("users.csv");
    std::vector<std::string> lines;
    std::string line;
    bool userFound = false;

    // ������ ���� ����� �� �����
    while (std::getline(infile, line)) {
        std::istringstream ss(line);
        std::string id, user, pass, email, role, balance;

        if (std::getline(ss, id, ',') &&
            std::getline(ss, user, ',') &&
            std::getline(ss, pass, ',') &&
            std::getline(ss, email, ',') &&
            std::getline(ss, role, ',') &&
            std::getline(ss, balance)) {

            // ���������, ��������� �� ��� ������������
            if (user == username) {
                userFound = true;

                // ��������� ������ ����, ��������� ���� �������� ��� ���������
                role = newRole;
            }

            // ��������� ������ ������� � ������ �����
            lines.push_back(id + "," + user + "," + pass + "," + email + "," + role + "," + balance);
        }
    }

    infile.close();

    if (userFound) {
        // �������������� ���� � ������������ �������
        std::ofstream outfile("users.csv", std::ios::trunc);
        if (outfile.is_open()) {
            for (const auto& saved_line : lines) {
                outfile << saved_line << "\n";
            }
            outfile.close();
            return true; // ������� ���������
        }
        else {
            std::cerr << "Error: Unable to open file for updating user data." << std::endl;
        }
    }
    else {
        std::cerr << "Error: User not found!" << std::endl;
    }

    return false; // �� ������� �������� ����
}

// ��������� ��� ��������� ����
void RenderAdminUI(ImFont* icons_font) {
    static char username[128] = "";
    static std::string message;

    // ������ ���� �� ��������� (0 - "user", 1 - "developer")
    static int selectedRoleIndex = 0;
    static bool changerole = false;
    // ��������� ����
    const char* roles[] = { "user", "root" };
    ImGui::Checkbox(menufunctional::settings::language == 0 ? "Change Role" : U8(u8"�������� ����"),&changerole);
    if (changerole) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::Begin(menufunctional::settings::language == 0 ? "Role" : U8(u8"����"));
        ImGui::PopStyleColor();
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_XMARK))
            changerole = false;
        ImGui::PopFont();
        ImGui::Text(menufunctional::settings::language == 0 ? "Change User Role" : U8(u8"��������� ���� ������������"));

        // ���� ����� ������������
        ImGui::InputText(menufunctional::settings::language == 0 ? "Username" : U8(u8"��� ������������"), username, sizeof(username));

        // ���������� ������ ��� ������ ����
        ImGui::Combo("Role", &selectedRoleIndex, roles, IM_ARRAYSIZE(roles));

        // ������ ��� ��������� ����
        if (ImGui::Button(menufunctional::settings::language == 0 ? "Change Role" : U8(u8"�������� ����"))) {
            std::string user(username);
            std::string role(roles[selectedRoleIndex]); // ��������� ���� �� �������

            if (user.empty()) {
                message = menufunctional::settings::language == 0 ? "Username is required!" : U8(u8"��������� ��� ������������!");
            }
            else {
                // ��������� ���� ����� ChangeUserRole
                bool success = ChangeUserRole(user, role);
                if (success) {
                    message = menufunctional::settings::language == 0 ? "Role updated successfully!" : U8(u8"���� ������� ���������!");
                }
                else {
                    message = menufunctional::settings::language == 0 ? "Failed to update role. User not found." : U8(u8"�� ������� �������� ����. ������������ �� ������.");
                }
            }
        }

        // ����������� ���������
        ImGui::Text("%s", message.c_str());

        ImGui::End();
    }
}


// ��������� ����������� � ������
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



// ������� ��� ������ ���� �������������
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

// ������� ��� ������ ������������� ������� � ����
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

// ��������, ������� �� ����
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

// ������� ��� �������� ������������ �� ID
bool DeleteUser(int id) {
    auto users = GetAllUsers();
    bool found = false;

    // ������� ������������ � ��������� ID
    users.erase(std::remove_if(users.begin(), users.end(),
        [&](const User& user) {
            if (user.id == id) {
                found = true;
                return true;
            }
            return false;
        }), users.end());

    if (found) {
        SaveAllUsers(users); // ��������� � �����������
    }

    return found;
}


bool AddUser(const std::string& username, const std::string& password, const std::string& email, const std::string& role) {
    if (username.empty() || password.empty() || email.empty() || role.empty()) {
        std::cerr << "Error: All fields are required to add a user." << std::endl;
        return false;
    }

    auto users = GetAllUsers();

    // ��������� ������ ����������� ID
    int newId = 0;
    if (!users.empty()) {
        newId = users.back().id + 1; // ID ����� �� 1 ������, ��� � ���������� ������������
    }

    // ���������, ��� �� ��� ������������ � ����� �� ������ ��� email
    for (const auto& user : users) {
        if (user.username == username || user.email == email) {
            std::cerr << "Error: Username or email already exists." << std::endl;
            return false;
        }
    }

    // ��������� ������ ������������
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
    ImGui::Checkbox(menufunctional::settings::language == 0 ? "Add User" : U8(u8"�������� ������������"),&CloseAddUserWindow);
    if (CloseAddUserWindow) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::Begin(menufunctional::settings::language == 0 ? "Add New User" : U8(u8"�������� ������ ������������"));
        ImGui::PopStyleColor();
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_XMARK))
            CloseAddUserWindow = false;
        ImGui::PopFont();
        ImGui::Text(menufunctional::settings::language == 0 ? "Enter new user details:" : U8(u8"������� ����� ������ ������������:"));
        ImGui::InputText(menufunctional::settings::language == 0 ? "Username" : U8(u8"��� ������������"), username, sizeof(username));
        ImGui::InputText(menufunctional::settings::language == 0 ? "Password" : U8(u8"������"), password, sizeof(password));
        ImGui::InputText(menufunctional::settings::language == 0 ? "Email" : U8(u8"����������� �����"), email, sizeof(email));
        ImGui::InputText(menufunctional::settings::language == 0 ? "Role" : U8(u8"����"), role, sizeof(role));

        if (ImGui::Button(menufunctional::settings::language == 0 ? "Add User" : U8(u8"�������� ������������"))) {
            // ���������, ��� ��� ���� ���������
            if (std::strlen(username) == 0 || std::strlen(password) == 0 || std::strlen(email) == 0 || std::strlen(role) == 0) {
                message = menufunctional::settings::language == 0 ? "Error: All fields are required!" : U8(u8"������: ��� ���� ����������� ��� ����������!");
            }
            else {
                if (AddUser(username, password, email, role)) {
                    message = menufunctional::settings::language == 0 ? "User added successfully!" : U8(u8"������������ ������� ��������!");
                    // ������� ���� ����� ��������� ����������
                    std::memset(username, 0, sizeof(username));
                    std::memset(password, 0, sizeof(password));
                    std::memset(email, 0, sizeof(email));
                    std::memset(role, 0, sizeof(role));
                }
                else {
                    message = menufunctional::settings::language == 0 ? "Error: Username or email already exists." : U8(u8"������: ��� ������������ ��� ����� ����������� ����� ��� ����������.");
                }
            }
        }

        // ����� ���������
        if (!message.empty()) {
            ImGui::Text("%s", message.c_str());
        }

        ImGui::End();
    }
}


// ������� ��� ���������� ������ ������������
bool UpdateUser(int id, const std::string& username, const std::string& password, const std::string& email, const std::string& role, float balance = -1.0f) {
    auto users = GetAllUsers();
    bool found = false;

    for (auto& user : users) {
        if (user.id == id) {
            found = true;

            // ��������� ������ �� ������, ������� �� ������
            if (!username.empty()) user.username = username;
            if (!password.empty()) user.password = password;
            if (!email.empty()) user.email = email;
            if (!role.empty()) user.role = role;

            // ��������� ������, ���� �� �����
            if (balance >= 0.0f) {
                user.balance = balance;
            }

            break;
        }
    }

    if (found) {
        SaveAllUsers(users); // ��������� � �����������
    }

    return found;
}




// ������� ��� ����������� ���������� �������������� � ImGui
void UserManagementUI(ImFont* icons_font) {
    static char inputId[10] = "";
    static char username[128] = "";
    static char password[128] = "";
    static char email[128] = "";
    static char role[64] = "";
    static float balance = 0;
    static std::string message; // ��������� �� ������ ��� ������
    static int action = 0; // 0 - ������, 1 - �������, 2 - ��������
    static bool UserManagement = false;
    ImGui::Checkbox(menufunctional::settings::language == 0 ? "User Management" : U8(u8"���������� ��������������"),&UserManagement);
    if (UserManagement) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::Begin(menufunctional::settings::language == 0 ? "User Management" : U8(u8"���������� ��������������"));
        ImGui::PopStyleColor();
        ImGui::PushFont(icons_font);
        if (ImGui::Button(ICON_FA_XMARK))
            UserManagement = false;
        ImGui::PopFont();
        ImGui::Text(menufunctional::settings::language == 0 ? "Enter User ID:" : U8(u8"������� ������������� ������������:"));
        ImGui::InputText(menufunctional::settings::language == 0 ? "ID" : U8(u8"��"), inputId, sizeof(inputId));

        ImGui::Separator();

        if (ImGui::RadioButton(menufunctional::settings::language == 0 ? "Delete User" : U8(u8"������� ������������"), action == 1)) {
            action = 1;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton(menufunctional::settings::language == 0 ? "Update User" : U8(u8"�������� ������������"), action == 2)) {
            action = 2;
        }

        if (action == 2) {
            // ���� ��� ���������� ������������
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Username" : U8(u8"����� ��� ������������"), username, sizeof(username));
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Password" : U8(u8"����� ������"), password, sizeof(password));
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Email" : U8(u8"����� ����������� �����"), email, sizeof(email));
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Role" : U8(u8"����� ����"), role, sizeof(role));
            ImGui::InputFloat(menufunctional::settings::language == 0 ? "Add money" : U8(u8"�������� ������"), &balance, sizeof(balance));
        }

        if (ImGui::Button(menufunctional::settings::language == 0 ? "Continue" : U8(u8"����������"))) {
            int userId;
            try {
                userId = std::stoi(inputId);
            }
            catch (...) {
                message = menufunctional::settings::language == 0 ? "Invalid ID! Please enter a valid number." : U8(u8"�������� �������������! ����������, ������� �������������� �����.");
                ImGui::Text("%s", message.c_str());
                ImGui::End();
                return;
            }

            if (action == 1) {
                // �������� ������������
                if (DeleteUser(userId)) {
                    message = menufunctional::settings::language == 0 ? "User deleted successfully!" : U8(u8"������������ ������� ������!");
                }
                else {
                    message = menufunctional::settings::language == 0 ? "Failed to delete user. User not found." : U8(u8"�� ������� ������� ������������. ������������ �� ������.");
                }
            }
            else if (action == 2) {
                // ���������� ������������
                if (UpdateUser(userId, username, password, email, role, balance)) {
                    message = menufunctional::settings::language == 0 ? "User updated successfully!" : U8(u8"������������ ������� ���������!");
                }
                else {
                    message = menufunctional::settings::language == 0 ? "Failed to update user. User not found." : U8(u8"�� ������� �������� ������������. ������������ �� ������.");
                }
            }
            else {
                message = menufunctional::settings::language == 0 ? "Please select an action." : U8(u8"����������, �������� ��������.");
            }
        }

        // ����������� ��������� �� ������ ��� ������
        if (!message.empty()) {
            ImGui::Text("%s", message.c_str());
        }

        ImGui::End();
    }
}

// ����� ������� �������������
void DisplayUsersTable() {
    // ������ ������ ������������� �� �����
    std::vector<User> users = GetAllUsers();

    // ���������� ������ � �������
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (ImGui::BeginTable("UsersTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
       
        // ��������� �������
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "ID": U8(u8"��"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Username":U8(u8"��� ������������"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Password": U8(u8"������"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Email": U8(u8"����������� �����"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Role": U8(u8"����"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Balance": U8(u8"������"));
        ImGui::TableHeadersRow();
        ImGui::PopStyleColor();
        // ��������� ������ �������
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


// ���������� � ������� ����
void DisplayTable() {
    // ������ ������ �� �����
    std::vector<Game> games = ReadGames();

    if (menufunctional::settings::separateexcelwindow) {
        ImGui::SetNextWindowSize(ImVec2(500,300));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::Begin("GameTable");
        ImGui::PopStyleColor();
    }
    // ���������� ������ � �������
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
    if (ImGui::BeginTable("GameTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        // ��������� �������
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "ID": U8(u8"��"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Name": U8(u8"���"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Release Date": U8(u8"���� ������"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Genre": U8(u8"����"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Sold Copies": U8(u8"��������� �����"));
        ImGui::TableSetupColumn(menufunctional::settings::language == 0 ? "Price": U8(u8"����"));
        ImGui::TableHeadersRow();
        ImGui::PopStyleColor();

        // ��������� ������ �������
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
        // ���� ���� ���
        static char name[128] = "";          // ���� ��� �����
        static char release_date[64] = "";  // ���� ��� ���� ������
        static char genre[64] = "";         // ���� ��� �����
        static int sold_copies = 0;         // ���� ��� ��������� �����
        static float price = 0.0f;          // ���� ��� ����

   

        // ���� �����
        ImGui::InputText(menufunctional::settings::language == 0 ? "Name": U8(u8"���"), name, IM_ARRAYSIZE(name));
        ImGui::InputText(menufunctional::settings::language == 0 ? "Release Date": U8(u8"���� ������"), release_date, IM_ARRAYSIZE(release_date));
        ImGui::InputText(menufunctional::settings::language == 0 ? "Genre": U8(u8"����"), genre, IM_ARRAYSIZE(genre));
        ImGui::InputInt(menufunctional::settings::language == 0 ? "Sold Copies": U8(u8"��������� ����������"), &sold_copies);
        ImGui::InputFloat(menufunctional::settings::language == 0 ? "Price": U8(u8"����"), &price);

        if (ImGui::Button(menufunctional::settings::language == 0 ? "Add Game": U8(u8"�������� ����"))) {
            // ������� ������ Game � �������� ���
            Game game = { GetNextGameID(), name, release_date, genre, sold_copies, price };
            AddGame(game);

            // �������� ���� ����� ����������
            memset(name, 0, sizeof(name));
            memset(release_date, 0, sizeof(release_date));
            memset(genre, 0, sizeof(genre));
            sold_copies = 0;
            price = 0.0f;

            ImGui::Text(menufunctional::settings::language == 0 ? "Game added successfully!": U8(u8"���� ������� ���������!"));
        }

      

}

Cart cart;
bool viewingCart = false;

void RenderUI(const std::string& username, ImFont* icons_font) {
    if (menufunctional::login::islogining) {
        if (!cart.IsViewingCart()) {
            DisplayGameStore(cart, icons_font); // ���������� �������
        }
        else {
            DisplayCartInterface(cart, username); // ���������� �������
        }
    }
}

void SysDeleteGame()
{
    static int game_id_to_delete = 0; // ���� ��� ����� ID ����

   // ImGui::Begin("Delete Game");

    // ���� ��� ����� ID ����
    ImGui::InputInt(menufunctional::settings::language == 0 ? "Game ID": U8(u8"������� ��"), &game_id_to_delete);

    if (ImGui::Button(menufunctional::settings::language == 0 ? "Delete Game": U8(u8"������� ����"))) {
        // ����� ������� ��������
        DeleteGame(game_id_to_delete);

        ImGui::Text(menufunctional::settings::language == 0 ? "Game with ID %d deleted successfully (if it existed).": U8(u8"���� � ��������������� %d ������� ������� (���� ��� ������������)."), game_id_to_delete);

        // ���������� ���� �����
        game_id_to_delete = 0;
    }

  //  ImGui::End();
}

// ������� �������������
void ExportUsersToCSV(const std::vector<User>& users, const std::string& folder) {
    // ������ �����, ���� � ���
    std::filesystem::create_directories(folder);

    std::string filepath = folder + "/exported_users.csv";
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing users!" << std::endl;
        return;
    }

    // ���������� ���������
    file << "ID,Username,Password,Email,Role,Balance\n";

    // ���������� ������ �������������
    for (const auto& user : users) {
        file << user.id << "," << user.username << "," << user.password << ","
            << user.email << "," << user.role << "," << user.balance << "\n";
    }

    file.close();
    std::cout << "Users exported successfully to " << filepath << "\n";
}

// ������� ���
void ExportGamesToCSV(const std::vector<Game>& games, const std::string& folder) {
    // ������ �����, ���� � ���
    std::filesystem::create_directories(folder);

    std::string filepath = folder + "/exported_games.csv";
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing games!" << std::endl;
        return;
    }

    // ���������� ���������
    file << "ID,Name,Release Date,Genre,Sold Copies,Price\n";

    // ���������� ������ ���
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
        static int game_id_to_rename = 0;         // ���� ��� ����� ID ����
        static int new_game_id = -1;             // ���� ��� ����� ������ ID
        static char new_name[128] = "";          // ���� ��� ������ �����
        static char new_release_date[128] = "";  // ���� ��� ����� ���� ������
        static char new_genre[128] = "";         // ���� ��� ������ �����
        static int new_sold_copies = -1;         // ���� ��� ����� ��������� �����
        static float new_price = -1.0f;          // ���� ��� ����� ����
        static bool Change_id = false;
        ImGui::Checkbox(menufunctional::settings::language == 0 ? "Rename Game" : U8(u8"������������� ����"), &CloseRenameGame);

        if (CloseRenameGame) {
            ImGui::SetNextWindowSize(ImVec2(650,300));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::Begin(menufunctional::settings::language == 0 ? "Rename Game" : U8(u8"������������� ����"));
            ImGui::PopStyleColor();
            ImGui::PushFont(icons_font);
            if (ImGui::Button(ICON_FA_XMARK))
                CloseRenameGame = false;
            ImGui::PopFont();
            ImGui::Text(menufunctional::settings::language == 0 ? "Edit Game Fields by ID" : U8(u8"�������������� ������� ����� �� ��������������"));
            ImGui::Separator();

            // ���� ��� ����� �������� ID ����
            ImGui::InputInt(menufunctional::settings::language == 0 ? "Current Game ID" : U8(u8"������� ������� ��"), &game_id_to_rename);

            // ���� ��� ����� ������ ID ����



            // ���� ��� �������������� ������ ������
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Name" : U8(u8"����� ���"), new_name, IM_ARRAYSIZE(new_name));
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Release Date" : U8(u8"����� ���� ������"), new_release_date, IM_ARRAYSIZE(new_release_date));
            ImGui::InputText(menufunctional::settings::language == 0 ? "New Genre" : U8(u8"����� ����"), new_genre, IM_ARRAYSIZE(new_genre));
            ImGui::InputInt(menufunctional::settings::language == 0 ? "New Sold Copies" : U8(u8"����� ��������� ����������"), &new_sold_copies);
            ImGui::InputFloat(menufunctional::settings::language == 0 ? "New Price" : U8(u8"����� ����"), &new_price);

            if (ImGui::Button(menufunctional::settings::language == 0 ? "Change id" : U8(u8"�������� ��")))
                Change_id = !Change_id;
            if (Change_id)
                ImGui::InputInt(menufunctional::settings::language == 0 ? "New Game ID" : U8(u8"����� ������� ��"), &new_game_id);

            // ������ ��� ���������� ���������
            if (ImGui::Button(menufunctional::settings::language == 0 ? "Update Game" : U8(u8"�������� ����"))) {
                // �������� ������� RenameGameField � ��������� ������ ID
                RenameGameField(
                    game_id_to_rename,  // ������� ID
                    new_game_id,        // ����� ID (��� ��������� -1, ���� �� ����������)
                    std::string(new_name),
                    std::string(new_release_date),
                    std::string(new_genre),
                    new_sold_copies,
                    new_price
                );

                // �������� ���� ����� ����������
                game_id_to_rename = 0;
                new_game_id = -1;
                memset(new_name, 0, sizeof(new_name));
                memset(new_release_date, 0, sizeof(new_release_date));
                memset(new_genre, 0, sizeof(new_genre));
                new_sold_copies = -1;
                new_price = -1.0f;

                ImGui::Text(menufunctional::settings::language == 0 ? "Game updated successfully!" : U8(u8"���� ������� ���������!"));
            }

            ImGui::End();
        }
}
// �������� ������� ��� ���������
void render(ImFont* icons_font, ImFont* small_text_font, ImFont* text_font)
{
    
    ImGui::PushFont(text_font);
    MainStyle();
    if (menufunctional::login::islogining && menufunctional::role == "root") {
        

        ImGui::SetNextWindowSize(ImVec2(500, 300));
        ImGui::Begin("Add Game Main", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);
        if (ImGui::Button(menufunctional::settings::language == 0 ? "Log out account" : U8(u8"����� �� ��������"))) {
            buttonpress = false;
            menufunctional::login::islogining = false;
        }
        ImGui::SetCursorPos(ImVec2(240,5));
        if(ImGui::Button(menufunctional::settings::language == 0 ? "Open window shop": U8(u8"������� ���� ��������")))
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
            if (ImGui::Button(menufunctional::settings::language == 0 ? "Export Games": U8(u8"������� ���"))) {
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

            if (ImGui::Button(menufunctional::settings::language == 0 ? "Export Users": U8(u8"������� �������������"))) {
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
    ImGui::Text(menufunctional::settings::language == 0 ? "Settings" : U8(u8"���������"));
    ImGui::Separator();

    ImGui::SetCursorPos(ImVec2(10, 30));
    ImGui::BeginChild("InfoChild", ImVec2(380, 260));

    //ImGui::Checkbox(menufunctional::settings::language == 0 ? "Pin the side menu" : U8(u8"��������� ������� ����"), &menufunctional::settings::pintab);
    ImGui::SetCursorPos(ImVec2(10, 5));
    ImGui::Checkbox(menufunctional::settings::language == 0 ? "Edit color and menu" : U8(u8"�������� ���� � ����"), &menufunctional::editcolormenu::open);

    if (menufunctional::role == "root") {
        ImGui::SetCursorPos(ImVec2(10, 40));
        ImGui::Checkbox(menufunctional::settings::language == 0 ? "separate excel window" : U8(u8"��������� ���� ������"), &menufunctional::settings::separateexcelwindow);
    }
    if (menufunctional::editcolormenu::open)
        Color_Menu_Edit();

    ImGui::SetCursorPos(ImVec2(10, 75));
    //Language();
    if (menufunctional::role == "root") {
        ImGui::SetCursorPos(ImVec2(10, 110));
        ImGui::Text(menufunctional::settings::language == 0 ? "Speed animation: " : U8(u8"�������� ��������: "));
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
    ImGui::Text(menufunctional::settings::language == 0 ? "Edit color and menu" : U8(u8"�������� ���� � ����"));
    ImGui::Separator();
    ImGui::SetCursorPos(ImVec2(5, 30));
    ImGui::BeginChild("InfoChild", ImVec2(390, 260));

    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button" : U8(u8"������"), (float*)&menufunctional::editcolormenu::Button, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Hover button" : U8(u8"��������� �� ������"), (float*)&menufunctional::editcolormenu::ButtonHovered, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Active button" : U8(u8"�������� ������"), (float*)&menufunctional::editcolormenu::ButtonActive, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Frame backround" : U8(u8"��� �����"), (float*)&menufunctional::editcolormenu::FrameBg, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Slider Grab" : U8(u8"������ ��������"), (float*)&menufunctional::editcolormenu::SliderGrab, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Slider Active" : U8(u8"�������� ��������"), (float*)&menufunctional::editcolormenu::SliderActive, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Slider Hovered" : U8(u8"��������� ��������"), (float*)&menufunctional::editcolormenu::SliderHovered, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Text" : U8(u8"�����"), (float*)&menufunctional::editcolormenu::Text, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Text error" : U8(u8"������ ������"), (float*)&menufunctional::editcolormenu::TextError, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Window Backround" : U8(u8"��� ����"), (float*)&menufunctional::editcolormenu::WindowBg, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Delete" : U8(u8"������: �������"), (float*)&menufunctional::editcolormenu::Button_delete, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Delete - Hovered" : U8(u8"������: ������� - ���������"), (float*)&menufunctional::editcolormenu::Button_delete_Hovered, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Delete - Active" : U8(u8"������:������� - ��������"), (float*)&menufunctional::editcolormenu::Button_delete_Active, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Confirm Delete" : U8(u8"������: ����������� ��������"), (float*)&menufunctional::editcolormenu::Button_confirmdelete, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Confirm Delete - Hovered" : U8(u8"������: ����������� �������� - ���������"), (float*)&menufunctional::editcolormenu::Button_confirmdelete_Hovered, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Confirm Delete - Active" : U8(u8"������: ����������� �������� - ��������"), (float*)&menufunctional::editcolormenu::Button_confirmdelete_Active, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Cancle" : U8(u8"������: ������ "), (float*)&menufunctional::editcolormenu::Button_cancle, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Cancle - Hovered" : U8(u8"������: ������ - ��������� "), (float*)&menufunctional::editcolormenu::Button_cancle_Hovered, ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit3(menufunctional::settings::language == 0 ? "Button: Cancle - Active" : U8(u8"������: ������ - �������� "), (float*)&menufunctional::editcolormenu::Button_cancle_Active, ImGuiColorEditFlags_NoInputs);

    ImGui::PushItemWidth(150);
    ImGui::SliderFloat(menufunctional::settings::language == 0 ? "Menu rounding" : U8(u8"���������� ����"), &menufunctional::editcolormenu::FrameRounding, 0.0f, 10.0f);
    ImGui::SliderFloat(menufunctional::settings::language == 0 ? "Grab slider rounding" : U8(u8"���������� ������ ��������"), &menufunctional::editcolormenu::GrabRounding, 0.0f, 10.0f);
    ImGui::PopItemWidth();

    ImGui::EndChild();
    ImGui::End();
}

void Info(ImFont* icons_font)
{
    ImGui::SetNextWindowSize(ImVec2(300, 300));
    ImGui::Begin("Info", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking);
    ImGui::SetCursorPos(ImVec2(105, 0));
    ImGui::Text(menufunctional::settings::language == 0 ? "Information" : U8(u8"����������"));
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