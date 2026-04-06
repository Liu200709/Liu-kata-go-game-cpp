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
#include <process.h>
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

bool katagoInitialized = false;
bool useKatago = true;

PROCESS_INFORMATION katagoProcessInfo;
HANDLE katagoStdinWrite = NULL;
HANDLE katagoStdoutRead = NULL;
HANDLE katagoStderrRead = NULL;

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

std::string boardToGtp(int x, int y) {
    char gtpX = 'A' + y;
    int gtpY = 19 - x;
    std::string result;
    result += gtpX;
    result += std::to_string(gtpY);
    return result;
}

std::pair<int, int> gtpToBoard(const std::string& gtpCoord) {
    if (gtpCoord.length() < 2) {
        return {-1, -1};
    }

    char gtpX = toupper(gtpCoord[0]);
    std::string gtpYStr = gtpCoord.substr(1);

    int y = gtpX - 'A';
    int x = 19 - std::stoi(gtpYStr);

    if (x < 0 || x >= 19 || y < 0 || y >= 19) {
        return {-1, -1};
    }

    return {x, y};
}

bool initKataGo() {
    char cwd[256];
    GetCurrentDirectoryA(256, cwd);
    std::cerr << "[KataGo] Working directory: " << cwd << std::endl;
    
    std::string katagoPath = "katago.exe";
    std::string modelPath = "default_model.bin.gz";
    std::string configPath = "default_gtp.cfg";
    
    std::cerr << "[KataGo] Testing file access: " << katagoPath << std::endl;
    
    std::ifstream testKatago(katagoPath, std::ios::binary);
    if (!testKatago.is_open()) {
        std::cerr << "[KataGo] katago.exe not found, using built-in AI" << std::endl;
        useKatago = false;
        return false;
    }
    testKatago.close();

    std::ifstream testModel(modelPath);
    if (!testModel.is_open()) {
        std::cerr << "[KataGo] " << modelPath << " not found, using built-in AI" << std::endl;
        useKatago = false;
        return false;
    }
    testModel.close();

    std::ifstream testConfig(configPath);
    if (!testConfig.is_open()) {
        std::cerr << "[KataGo] " << configPath << " not found, using built-in AI" << std::endl;
        useKatago = false;
        return false;
    }
    testConfig.close();

    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;

    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(sa));
    sa.bInheritHandle = TRUE;

    if (!CreatePipe(&katagoStdoutRead, &si.hStdOutput, &sa, 0)) {
        std::cerr << "[KataGo] Failed to create stdout pipe" << std::endl;
        useKatago = false;
        return false;
    }

    if (!CreatePipe(&si.hStdInput, &katagoStdinWrite, &sa, 0)) {
        std::cerr << "[KataGo] Failed to create stdin pipe" << std::endl;
        useKatago = false;
        CloseHandle(katagoStdoutRead);
        return false;
    }

    if (!CreatePipe(&katagoStderrRead, &si.hStdError, &sa, 0)) {
        std::cerr << "[KataGo] Failed to create stderr pipe" << std::endl;
        useKatago = false;
        CloseHandle(katagoStdoutRead);
        CloseHandle(katagoStdinWrite);
        return false;
    }

    SetHandleInformation(katagoStdoutRead, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(katagoStdinWrite, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(katagoStderrRead, HANDLE_FLAG_INHERIT, 0);

    std::string searchParams = "";
    if (difficulty == "easy") {
        searchParams = "-override-config searchFactor=0.5";
    } else if (difficulty == "medium") {
        searchParams = "-override-config searchFactor=1.0";
    } else if (difficulty == "hard") {
        searchParams = "-override-config searchFactor=1.5";
    } else if (difficulty == "expert") {
        searchParams = "-override-config searchFactor=2.0";
    } else if (difficulty == "professional") {
        searchParams = "-override-config searchFactor=3.0";
    }

    std::string cmdLine = katagoPath + " gtp -model " + modelPath + " -config " + configPath;
    if (!searchParams.empty()) {
        cmdLine += " " + searchParams;
    }

    ZeroMemory(&katagoProcessInfo, sizeof(katagoProcessInfo));

    if (!CreateProcessA(NULL, (LPSTR)cmdLine.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &katagoProcessInfo)) {
        std::cerr << "[KataGo] Failed to start katago.exe: " << GetLastError() << std::endl;
        useKatago = false;
        CloseHandle(katagoStdoutRead);
        CloseHandle(katagoStdinWrite);
        CloseHandle(katagoStderrRead);
        return false;
    }

    Sleep(5000);

    DWORD bytesRead;
    char buffer[256];
    PeekNamedPipe(katagoStdoutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL, NULL);
    if (bytesRead > 0) {
        ReadFile(katagoStdoutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        buffer[bytesRead] = '\0';
        std::cout << "[KataGo] Response: " << buffer << std::endl;
    }

    katagoInitialized = true;
    std::cout << "[KataGo] Initialized successfully (difficulty: " << difficulty << ")" << std::endl;
    return true;
}

bool sendGtpCommand(const std::string& command, std::string& response, int timeoutMs = 30000) {
    if (!katagoInitialized || !useKatago) {
        return false;
    }

    std::string fullCommand = command + "\n";

    DWORD bytesWritten;
    if (!WriteFile(katagoStdinWrite, fullCommand.c_str(), fullCommand.length(), &bytesWritten, NULL)) {
        std::cerr << "[KataGo] Failed to write command: " << GetLastError() << std::endl;
        return false;
    }

    auto startTime = std::chrono::steady_clock::now();
    response = "";

    while (true) {
        DWORD bytesAvailable = 0;
        PeekNamedPipe(katagoStdoutRead, NULL, 0, NULL, &bytesAvailable, NULL);

        if (bytesAvailable > 0) {
            char buffer[4096];
            DWORD bytesRead = 0;
            if (ReadFile(katagoStdoutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                response += buffer;

                if (response.find('\n') != std::string::npos) {
                    size_t eqPos = response.find("= ");
                    size_t qPos = response.find("?");
                    if ((eqPos != std::string::npos && eqPos < response.find('\n')) || 
                        (qPos != std::string::npos && qPos < response.find('\n'))) {
                        break;
                    }
                }
            }
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count();

        if (elapsed > timeoutMs) {
            std::cerr << "[KataGo] Command timeout: " << command << std::endl;
            return false;
        }

        Sleep(50);
    }

    return true;
}

bool sendGtpCommandWithRetry(const std::string& command, std::string& response, int maxRetries = 3) {
    for (int attempt = 0; attempt < maxRetries; attempt++) {
        if (sendGtpCommand(command, response, 30000)) {
            if (response.find("?") == 0) {
                std::cerr << "[KataGo] GTP error response: " << response << std::endl;
                if (attempt < maxRetries - 1) {
                    Sleep(500);
                    continue;
                }
                return false;
            }
            return true;
        }
        std::cerr << "[KataGo] Attempt " << (attempt + 1) << " failed, retrying..." << std::endl;
        Sleep(1000);
    }
    return false;
}

void cleanupKataGo() {
    if (katagoProcessInfo.hProcess) {
        TerminateProcess(katagoProcessInfo.hProcess, 0);
        CloseHandle(katagoProcessInfo.hProcess);
        CloseHandle(katagoProcessInfo.hThread);
    }
    if (katagoStdinWrite) CloseHandle(katagoStdinWrite);
    if (katagoStdoutRead) CloseHandle(katagoStdoutRead);
    if (katagoStderrRead) CloseHandle(katagoStderrRead);

    katagoStdinWrite = NULL;
    katagoStdoutRead = NULL;
    katagoStderrRead = NULL;
    katagoProcessInfo.hProcess = NULL;

    katagoInitialized = false;
}

void restartKataGo() {
    cleanupKataGo();
    useKatago = true;
    initKataGo();
}

std::pair<int, int> generateKataGoMove() {
    std::string response;
    if (sendGtpCommandWithRetry("genmove W", response)) {
        size_t eqPos = response.find("= ");
        if (eqPos != std::string::npos) {
            std::string moveStr = response.substr(eqPos + 2);
            moveStr = moveStr.substr(0, moveStr.find('\n'));
            moveStr = moveStr.substr(0, moveStr.find('\r'));
            moveStr.erase(remove_if(moveStr.begin(), moveStr.end(), ::isspace), moveStr.end());

            if (moveStr == "pass" || moveStr.empty()) {
                std::cerr << "[KataGo] Got pass or empty move, using fallback" << std::endl;
            } else {
                auto [x, y] = gtpToBoard(moveStr);
                if (isValidPosition(x, y) && board[x][y] == 0) {
                    std::cout << "[KataGo] Generated move: " << moveStr << " (" << x << ", " << y << ")" << std::endl;
                    return {x, y};
                }
            }
        }
    }

    std::cerr << "[KataGo] Failed to get valid move, using fallback algorithm" << std::endl;
    return {-1, -1};
}

void playMoveToKataGo(int x, int y, int player) {
    if (!katagoInitialized || !useKatago) return;

    std::string color = (player == 1) ? "B" : "W";
    std::string gtpMove = boardToGtp(x, y);

    std::string response;
    std::string command = "play " + color + " " + gtpMove;
    sendGtpCommand(command, response, 2000);
}

void clearBoardKataGo() {
    if (!katagoInitialized || !useKatago) return;

    std::string response;
    sendGtpCommand("clear_board", response, 2000);
}

std::pair<int, int> generateAIMove() {
    if (katagoInitialized && useKatago) {
        auto [kx, ky] = generateKataGoMove();
        if (kx >= 0 && ky >= 0) {
            return {kx, ky};
        }
    }

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

    if (katagoInitialized && useKatago) {
        clearBoardKataGo();
    }
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
        char buffer[16384] = {0};
        int bytesRead = 0;
        int totalRead = 0;
        int requestContentLength = 0;
        size_t headerEndPos = 0;
        
        while (totalRead < sizeof(buffer) - 1) {
            bytesRead = recv(clientSocket, buffer + totalRead, sizeof(buffer) - 1 - totalRead, 0);
            if (bytesRead <= 0) {
                break;
            }
            
            totalRead += bytesRead;
            buffer[totalRead] = '\0';
            
            if (requestContentLength == 0) {
                size_t clPos = std::string(buffer).find("Content-Length:");
                if (clPos != std::string::npos) {
                    size_t valStart = clPos + 15;
                    size_t valEnd = std::string(buffer).find("\r\n", valStart);
                    if (valEnd == std::string::npos) valEnd = std::string(buffer).find("\n", valStart);
                    std::string clStr = std::string(buffer).substr(valStart, valEnd - valStart);
                    requestContentLength = std::stoi(clStr);
                }
            }
            
            size_t headerEnd = std::string(buffer).find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                headerEndPos = headerEnd + 4;
                int bodyReceived = totalRead - headerEndPos;
                int expectedTotal = headerEndPos + requestContentLength;
                
                if (totalRead >= expectedTotal) {
                    break;
                }
            }
        }

        std::string request(buffer);
        std::string requestBody;
        
        if (headerEndPos > 0 && totalRead > headerEndPos) {
            requestBody = request.substr(headerEndPos);
        }
        
        std::istringstream iss(request);
        std::string method, path;
        iss >> method >> path;

        std::string response;
        std::map<std::string, std::string> headers;
        std::string body = requestBody; // Use the body parsed from recv loop

        if (body.empty()) {
            size_t bodyStart = request.find("\r\n\r\n");
            if (bodyStart != std::string::npos) {
                body = request.substr(bodyStart + 4);
            } else {
                bodyStart = request.find("\n\n");
                if (bodyStart != std::string::npos) {
                    body = request.substr(bodyStart + 2);
                }
            }
        }
        
        std::cout << "[BODY] Length: " << body.length() << ", Content: '" << body << "'" << std::endl;
        std::cout.flush();

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
        } else if (path == "/katago_status" && method == "GET") {
            handleKataGoStatus(clientSocket);
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
            while (start < body.length() && !isdigit(body[start]) && body[start] != '-') start++;
            size_t end = start;
            while (end < body.length() && (isdigit(body[end]) || body[end] == '-')) end++;
            if (end > start) x = std::stoi(body.substr(start, end - start));
        }

        size_t yPos = body.find("\"y\":");
        if (yPos != std::string::npos) {
            size_t start = body.find(":", yPos) + 1;
            while (start < body.length() && !isdigit(body[start]) && body[start] != '-') start++;
            size_t end = start;
            while (end < body.length() && (isdigit(body[end]) || body[end] == '-')) end++;
            if (end > start) {
                y = std::stoi(body.substr(start, end - start));
            }
        }

        std::cerr << "[DEBUG] Parsed: x=" << x << ", y=" << y << std::endl;

        std::string response;

        if (x >= 0 && y >= 0 && makeMove(x, y, currentPlayer)) {
            playMoveToKataGo(x, y, currentPlayer);

            int playerWhoJustMoved = currentPlayer;

            if (playerWhoJustMoved == 1) {
                auto [aiX, aiY] = generateAIMove();
                if (aiX >= 0 && aiY >= 0) {
                    makeMove(aiX, aiY, 2);
                    playMoveToKataGo(aiX, aiY, 2);
                    currentPlayer = 1;
                } else {
                    currentPlayer = 2;
                }
            } else {
                currentPlayer = 1;
            }

            std::string jsonBody = "{\"success\":true,\"board\":" + boardToJson() + ",\"current_player\":" + std::to_string(currentPlayer) + ",\"katago\":" + (useKatago ? "true" : "false") + "}";

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

        std::string jsonBody = "{\"success\":true,\"board\":" + boardToJson() + ",\"current_player\":1,\"katago\":" + (useKatago ? "true" : "false") + "}";

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
            ",\"margin\":" + std::to_string(score["margin"]) +
            ",\"katago\":" + (useKatago ? "true" : "false") + "}";

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
            ",\"margin\":" + std::to_string(score["margin"]) +
            ",\"katago\":" + (useKatago ? "true" : "false") + "}";

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

            if (katagoInitialized && useKatago) {
                restartKataGo();
            }
        }

        std::string katagoStr = useKatago ? "true" : "false";
        std::string jsonBody = "{\"success\":true,\"katago\":" + katagoStr + "}";

        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Content-Length: " + std::to_string(jsonBody.length()) + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n";
        response += jsonBody;

        send(clientSocket, response.c_str(), response.length(), 0);
        closesocket(clientSocket);
    }

    void handleKataGoStatus(SOCKET clientSocket) {
        std::string jsonBody = "{\"initialized\":" + std::string(katagoInitialized ? "true" : "false") +
            ",\"use_katago\":" + (useKatago ? "true" : "false") +
            ",\"difficulty\":\"" + difficulty + "\"}";

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
        std::cout << "========================================" << std::endl;
        std::cout << "KataGo围棋人机对弈服务器 (C++ Version)" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "服务器启动在 http://localhost:" << port << std::endl;
        std::cout << "请在浏览器中打开上述地址开始对弈" << std::endl;
        std::cout << std::endl;

        initKataGo();

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

    cleanupKataGo();

    return 0;
}
