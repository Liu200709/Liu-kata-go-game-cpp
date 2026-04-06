#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <stack>
#include <map>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include <cstring>
#include <WS2tcpip.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

const int BOARD_SIZE = 19;
const double KOMI = 7.5;

bool isGameOver();
bool makeMove(int x, int y, int player);
std::map<std::string, double> calculateWinner();
std::pair<int, int> generateAIMove();

std::vector<std::vector<int>> board(BOARD_SIZE, std::vector<int>(BOARD_SIZE, 0));
int currentPlayer = 1;
std::string difficulty = "medium";
std::vector<std::vector<int>> previousBoard;
bool gameOver = false;

const std::vector<std::pair<int, int>> STAR_POINTS = {
    {3, 3}, {3, 9}, {3, 15},
    {9, 3}, {9, 9}, {9, 15},
    {15, 3}, {15, 9}, {15, 15}
};

bool isValidPosition(int x, int y) {
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

std::vector<std::pair<int, int>> getNeighbors(int x, int y) {
    std::vector<std::pair<int, int>> neighbors;
    int dx[4] = {-1, 1, 0, 0};
    int dy[4] = {0, 0, -1, 1};
    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (isValidPosition(nx, ny)) {
            neighbors.push_back({nx, ny});
        }
    }
    return neighbors;
}

int getLiberties(int x, int y, const std::vector<std::vector<int>>& testBoard) {
    if (!isValidPosition(x, y) || testBoard[x][y] == 0) {
        return 0;
    }

    std::set<std::pair<int, int>> visited;
    std::stack<std::pair<int, int>> st;
    st.push({x, y});
    visited.insert({x, y});

    int liberties = 0;
    std::set<std::pair<int, int>> counted;

    while (!st.empty()) {
        auto [cx, cy] = st.top();
        st.pop();

        for (auto [nx, ny] : getNeighbors(cx, cy)) {
            if (visited.find({nx, ny}) != visited.end()) continue;

            if (testBoard[nx][ny] == 0) {
                if (counted.find({nx, ny}) == counted.end()) {
                    liberties++;
                    counted.insert({nx, ny});
                }
            } else if (testBoard[nx][ny] == testBoard[cx][cy]) {
                visited.insert({nx, ny});
                st.push({nx, ny});
            }
        }
    }

    return liberties;
}

std::vector<std::pair<int, int>> removeDeadStones(int player, std::vector<std::vector<int>>& testBoard) {
    std::vector<std::pair<int, int>> removed;
    std::set<std::pair<int, int>> visited;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (testBoard[i][j] == player && visited.find({i, j}) == visited.end()) {
                if (getLiberties(i, j, testBoard) == 0) {
                    std::stack<std::pair<int, int>> st;
                    st.push({i, j});
                    std::vector<std::pair<int, int>> group;

                    while (!st.empty()) {
                        auto [cx, cy] = st.top();
                        st.pop();

                        if (visited.find({cx, cy}) != visited.end()) continue;
                        if (testBoard[cx][cy] != player) continue;

                        visited.insert({cx, cy});
                        group.push_back({cx, cy});

                        for (auto [nx, ny] : getNeighbors(cx, cy)) {
                            if (testBoard[nx][ny] == player) {
                                st.push({nx, ny});
                            }
                        }
                    }

                    for (auto [cx, cy] : group) {
                        testBoard[cx][cy] = 0;
                        removed.push_back({cx, cy});
                    }
                }
            }
        }
    }

    return removed;
}

bool hasCaptures(const std::vector<std::vector<int>>& testBoard, int player) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (testBoard[i][j] == player) {
                if (getLiberties(i, j, testBoard) == 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool isValidMove(int x, int y, int player) {
    if (!isValidPosition(x, y) || board[x][y] != 0) {
        return false;
    }

    std::vector<std::vector<int>> testBoard = board;
    testBoard[x][y] = player;

    if (getLiberties(x, y, testBoard) > 0) {
        int opponent = (player == 1) ? 2 : 1;
        removeDeadStones(opponent, testBoard);

        if (!previousBoard.empty() && testBoard == previousBoard) {
            return false;
        }

        return true;
    } else {
        int opponent = (player == 1) ? 2 : 1;
        auto removed = removeDeadStones(opponent, testBoard);

        if (!removed.empty()) {
            return true;
        }

        return false;
    }
}

bool makeMove(int x, int y, int player) {
    if (!isValidMove(x, y, player)) {
        return false;
    }

    previousBoard = board;

    board[x][y] = player;

    int opponent = (player == 1) ? 2 : 1;
    removeDeadStones(opponent, board);

    if (isGameOver()) {
        gameOver = true;
    }

    return true;
}

int countTerritory(int player) {
    std::set<std::pair<int, int>> visited;
    int territory = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 0 && visited.find({i, j}) == visited.end()) {
                std::stack<std::pair<int, int>> st;
                st.push({i, j});
                std::set<std::pair<int, int>> groupVisited;
                groupVisited.insert({i, j});

                bool isBlackTerritory = true;
                bool isWhiteTerritory = true;

                while (!st.empty()) {
                    auto [cx, cy] = st.top();
                    st.pop();

                    for (auto [nx, ny] : getNeighbors(cx, cy)) {
                        if (groupVisited.find({nx, ny}) != groupVisited.end()) continue;

                        if (board[nx][ny] == 0) {
                            groupVisited.insert({nx, ny});
                            st.push({nx, ny});
                        } else if (board[nx][ny] == 1) {
                            isWhiteTerritory = false;
                        } else if (board[nx][ny] == 2) {
                            isBlackTerritory = false;
                        }
                    }
                }

                visited.insert(groupVisited.begin(), groupVisited.end());

                if (isBlackTerritory && !isWhiteTerritory) {
                    territory += groupVisited.size();
                } else if (isWhiteTerritory && !isBlackTerritory) {
                    territory += groupVisited.size();
                }
            }
        }
    }

    return territory;
}

bool isGameOver() {
    bool hasStones = false;
    for (int i = 0; i < BOARD_SIZE && !hasStones; i++) {
        for (int j = 0; j < BOARD_SIZE && !hasStones; j++) {
            if (board[i][j] != 0) {
                hasStones = true;
            }
        }
    }

    if (!hasStones) {
        return false;
    }

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 0) {
                if (isValidMove(i, j, 1)) {
                    return false;
                }
                if (isValidMove(i, j, 2)) {
                    return false;
                }
            }
        }
    }

    return true;
}

std::map<std::string, double> calculateWinner() {
    int blackStones = 0;
    int whiteStones = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 1) blackStones++;
            else if (board[i][j] == 2) whiteStones++;
        }
    }

    int blackTerritory = countTerritory(1);
    int whiteTerritory = countTerritory(2);

    double blackTotal = blackStones + blackTerritory;
    double whiteTotal = whiteStones + whiteTerritory + KOMI;

    std::map<std::string, double> result;

    if (blackTotal > 180.5) {
        result["winner"] = 1;
        result["black_total"] = blackTotal;
        result["white_total"] = whiteTotal;
        result["margin"] = blackTotal - 180.5;
    } else if (whiteTotal > 180.5) {
        result["winner"] = 2;
        result["black_total"] = blackTotal;
        result["white_total"] = whiteTotal;
        result["margin"] = whiteTotal - 180.5;
    } else {
        result["winner"] = 0;
        result["black_total"] = blackTotal;
        result["white_total"] = whiteTotal;
        result["margin"] = 0;
    }

    return result;
}

std::pair<int, int> generateAIMove() {
    static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 0) {
                std::vector<std::vector<int>> testBoard = board;
                testBoard[i][j] = 2;
                if (hasCaptures(testBoard, 1)) {
                    return {i, j};
                }
            }
        }
    }

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 0) {
                std::vector<std::vector<int>> testBoard = board;
                testBoard[i][j] = 1;
                if (hasCaptures(testBoard, 2)) {
                    return {i, j};
                }
            }
        }
    }

    if (difficulty == "easy") {
        while (true) {
            int x = rng() % BOARD_SIZE;
            int y = rng() % BOARD_SIZE;
            if (board[x][y] == 0) {
                return {x, y};
            }
        }
    } else if (difficulty == "medium") {
        for (auto [i, j] : STAR_POINTS) {
            if (board[i][j] == 0) {
                return {i, j};
            }
        }
        std::vector<std::pair<int, int>> edge;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if ((i < 3 || i > 15 || j < 3 || j > 15) && board[i][j] == 0) {
                    edge.push_back({i, j});
                }
            }
        }
        if (!edge.empty()) {
            return edge[rng() % edge.size()];
        }
        while (true) {
            int x = rng() % BOARD_SIZE;
            int y = rng() % BOARD_SIZE;
            if (board[x][y] == 0) {
                return {x, y};
            }
        }
    } else if (difficulty == "hard") {
        for (auto [i, j] : STAR_POINTS) {
            if (board[i][j] == 0) {
                return {i, j};
            }
        }
        for (int i = 6; i < 13; i++) {
            for (int j = 6; j < 13; j++) {
                if (board[i][j] == 0) {
                    return {i, j};
                }
            }
        }
        while (true) {
            int x = rng() % BOARD_SIZE;
            int y = rng() % BOARD_SIZE;
            if (board[x][y] == 0) {
                return {x, y};
            }
        }
    } else {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == 1) {
                    int dx[8] = {0, 1, 0, -1, 1, 1, -1, -1};
                    int dy[8] = {1, 0, -1, 0, 1, -1, 1, -1};
                    for (int k = 0; k < 8; k++) {
                        int ni = i + dx[k];
                        int nj = j + dy[k];
                        if (isValidPosition(ni, nj) && board[ni][nj] == 0) {
                            return {ni, nj};
                        }
                    }
                }
            }
        }
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == 2) {
                    int dx[8] = {0, 1, 0, -1, 1, 1, -1, -1};
                    int dy[8] = {1, 0, -1, 0, 1, -1, 1, -1};
                    for (int k = 0; k < 8; k++) {
                        int ni = i + dx[k];
                        int nj = j + dy[k];
                        if (isValidPosition(ni, nj) && board[ni][nj] == 0) {
                            return {ni, nj};
                        }
                    }
                }
            }
        }
        for (int i = 6; i < 13; i++) {
            for (int j = 6; j < 13; j++) {
                if (board[i][j] == 0) {
                    return {i, j};
                }
            }
        }
        while (true) {
            int x = rng() % BOARD_SIZE;
            int y = rng() % BOARD_SIZE;
            if (board[x][y] == 0) {
                return {x, y};
            }
        }
    }
}

std::string boardToJson() {
    std::string json = "[";
    for (int i = 0; i < BOARD_SIZE; i++) {
        json += "[";
        for (int j = 0; j < BOARD_SIZE; j++) {
            json += std::to_string(board[i][j]);
            if (j < BOARD_SIZE - 1) json += ",";
        }
        json += "]";
        if (i < BOARD_SIZE - 1) json += ",";
    }
    json += "]";
    return json;
}

void resetGame() {
    board = std::vector<std::vector<int>>(BOARD_SIZE, std::vector<int>(BOARD_SIZE, 0));
    currentPlayer = 1;
    previousBoard.clear();
    gameOver = false;
}

class HTTPServer {
private:
    SOCKET serverSocket;
    int port;

public:
    HTTPServer(int p) : port(p), serverSocket(INVALID_SOCKET) {}

    ~HTTPServer() {
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
        }
        WSACleanup();
    }

    bool start() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return false;
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        BOOL opt = TRUE;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            std::cerr << "Failed to bind to port " << port << std::endl;
            return false;
        }

        if (listen(serverSocket, 10) == SOCKET_ERROR) {
            std::cerr << "Failed to listen on port " << port << std::endl;
            return false;
        }

        return true;
    }

    void handleRequest(SOCKET clientSocket) {
        char buffer[8192] = {0};
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesRead <= 0) {
            closesocket(clientSocket);
            return;
        }

        std::string request(buffer);
        std::istringstream iss(request);
        std::string method, path;
        iss >> method >> path;

        std::string response;
        std::map<std::string, std::string> headers;
        std::string body;

        size_t bodyStart = request.find("\r\n\r\n");
        if (bodyStart != std::string::npos) {
            body = request.substr(bodyStart + 4);
        }

        if (path == "/" || path == "/index.html") {
            std::ifstream file("templates/index.html", std::ios::binary);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                body = buffer.str();
                headers["Content-Type"] = "text/html";
            } else {
                response = "HTTP/1.1 404 Not Found\r\n\r\n";
                send(clientSocket, response.c_str(), response.length(), 0);
                closesocket(clientSocket);
                return;
            }
        } else if (path == "/make_move" && method == "POST") {
            handleMakeMove(clientSocket, body);
            return;
        } else if (path == "/reset" && method == "POST") {
            handleReset(clientSocket);
            return;
        } else if (path == "/check_game_over" && method == "POST") {
            handleCheckGameOver(clientSocket);
            return;
        } else if (path == "/score" && method == "POST") {
            handleScore(clientSocket);
            return;
        } else if (path == "/set_difficulty" && method == "POST") {
            handleSetDifficulty(clientSocket, body);
            return;
        } else {
            response = "HTTP/1.1 404 Not Found\r\n\r\n";
            send(clientSocket, response.c_str(), response.length(), 0);
            closesocket(clientSocket);
            return;
        }

        std::string contentLength = std::to_string(body.length());
        response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: " + (headers.count("Content-Type") ? headers["Content-Type"] : "text/plain") + "\r\n";
        response += "Content-Length: " + contentLength + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += body;

        send(clientSocket, response.c_str(), response.length(), 0);
        closesocket(clientSocket);
    }

    void handleMakeMove(SOCKET clientSocket, const std::string& body) {
        int x = -1, y = -1;

        size_t xPos = body.find("\"x\":");
        if (xPos != std::string::npos) {
            size_t start = body.find(":", xPos) + 1;
            size_t end = body.find(",", start);
            if (end == std::string::npos) end = body.find("}", start);
            x = std::stoi(body.substr(start, end - start));
        }

        size_t yPos = body.find("\"y\":");
        if (yPos != std::string::npos) {
            size_t start = body.find(":", yPos) + 1;
            size_t end = body.find("}", start);
            y = std::stoi(body.substr(start, end - start));
        }

        std::string response;

        if (x >= 0 && y >= 0 && makeMove(x, y, currentPlayer)) {
            if (currentPlayer == 1) {
                auto [aiX, aiY] = generateAIMove();
                makeMove(aiX, aiY, 2);
            }

            currentPlayer = 1;

            std::string jsonBody = "{\"success\":true,\"board\":" + boardToJson() + ",\"current_player\":" + std::to_string(currentPlayer) + "}";

            response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: application/json\r\n";
            response += "Content-Length: " + std::to_string(jsonBody.length()) + "\r\n";
            response += "Connection: close\r\n";
            response += "\r\n";
            response += jsonBody;
        } else {
            std::string jsonBody = "{\"success\":false,\"message\":\"落子不合法\"}";

            response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: application/json\r\n";
            response += "Content-Length: " + std::to_string(jsonBody.length()) + "\r\n";
            response += "Connection: close\r\n";
            response += "\r\n";
            response += jsonBody;
        }

        send(clientSocket, response.c_str(), response.length(), 0);
        closesocket(clientSocket);
    }

    void handleReset(SOCKET clientSocket) {
        resetGame();

        std::string jsonBody = "{\"success\":true,\"board\":" + boardToJson() + ",\"current_player\":1}";

        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Content-Length: " + std::to_string(jsonBody.length()) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += jsonBody;

        send(clientSocket, response.c_str(), response.length(), 0);
        closesocket(clientSocket);
    }

    void handleCheckGameOver(SOCKET clientSocket) {
        if (!gameOver) {
            gameOver = isGameOver();
        }

        auto score = calculateWinner();

        std::string winnerStr;
        if (score["winner"] == 1) winnerStr = "\"black\"";
        else if (score["winner"] == 2) winnerStr = "\"white\"";
        else winnerStr = "null";

        std::string jsonBody = "{\"game_over\":" + std::string(gameOver ? "true" : "false") +
            ",\"winner\":" + winnerStr +
            ",\"black_total\":" + std::to_string(score["black_total"]) +
            ",\"white_total\":" + std::to_string(score["white_total"]) +
            ",\"margin\":" + std::to_string(score["margin"]) + "}";

        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Content-Length: " + std::to_string(jsonBody.length()) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += jsonBody;

        send(clientSocket, response.c_str(), response.length(), 0);
        closesocket(clientSocket);
    }

    void handleScore(SOCKET clientSocket) {
        auto score = calculateWinner();

        std::string winnerStr;
        if (score["winner"] == 1) winnerStr = "\"black\"";
        else if (score["winner"] == 2) winnerStr = "\"white\"";
        else winnerStr = "null";

        std::string jsonBody = "{\"winner\":" + winnerStr +
            ",\"black_total\":" + std::to_string(score["black_total"]) +
            ",\"white_total\":" + std::to_string(score["white_total"]) +
            ",\"margin\":" + std::to_string(score["margin"]) + "}";

        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Content-Length: " + std::to_string(jsonBody.length()) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += jsonBody;

        send(clientSocket, response.c_str(), response.length(), 0);
        closesocket(clientSocket);
    }

    void handleSetDifficulty(SOCKET clientSocket, const std::string& body) {
        size_t pos = body.find("\"difficulty\":");
        if (pos != std::string::npos) {
            size_t start = body.find(":", pos) + 2;
            size_t end = body.find("}", start);
            if (end == std::string::npos) end = body.find("]", start);
            std::string newDifficulty = body.substr(start, end - start - 1);
            difficulty = newDifficulty;
        }

        std::string jsonBody = "{\"success\":true}";

        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Content-Length: " + std::to_string(jsonBody.length()) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += jsonBody;

        send(clientSocket, response.c_str(), response.length(), 0);
        closesocket(clientSocket);
    }

    void run() {
        std::cout << "服务器启动在 http://localhost:" << port << std::endl;
        std::cout << "请在浏览器中打开上述地址开始对弈" << std::endl;

        while (true) {
            struct sockaddr_in clientAddr;
            int clientLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Failed to accept connection" << std::endl;
                continue;
            }

            handleRequest(clientSocket);
        }
    }
};

int main(int argc, char* argv[]) {
    int port = 8000;

    HTTPServer server(port);

    if (!server.start()) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }

    server.run();

    return 0;
}
