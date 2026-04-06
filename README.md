# Liu-kata-go-game-cpp
# KataGo Go Game - C++ Edition

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++17">
  <img src="https://img.shields.io/badge/Platform-Windows-green.svg" alt="Windows">
  <img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="License">
</p>

A production-ready Go (Weiqi/Baduk) game server with integrated KataGo AI engine, built entirely in C++ using the Win32 API. Play against a powerful AI opponent directly in your browser.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Technology Stack](#technology-stack)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Quick Start](#quick-start)
  - [Building from Source](#building-from-source)
- [Usage Guide](#usage-guide)
  - [Web Interface](#web-interface)
  - [API Endpoints](#api-endpoints)
  - [Difficulty Levels](#difficulty-levels)
- [Project Structure](#project-structure)
- [Testing](#testing)
- [Troubleshooting](#troubleshooting)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)
- [Contact](#contact)

---

## Overview

**KataGo Go Game** is a fully functional Go (Weiqi/Baduk) game server that allows players to play against an advanced AI opponent. The project is implemented in pure C++ using the Windows Win32 API, featuring:

- A complete HTTP server for web-based gameplay
- Integrated KataGo AI engine for intelligent move generation
- Full implementation of Go game rules including captures, ko detection, and scoring
- Interactive web interface for seamless gaming experience

Whether you're a beginner learning the game or an experienced player looking for a challenge, this server provides an engaging AI opponent with adjustable difficulty levels.

---

## Features

### Core Game Features

- **19×19 Standard Go Board** - Full-sized Go board with all standard rules
- **Complete Rule Implementation**
  - Stone placement validation
  - Liberty counting and group detection
  - Capture mechanics (removing stones with no liberties)
  - Ko rule prevention (no repeated board positions)
  - Automatic scoring calculation at game end
- **9 Star Points** - Standard 4-4 point handicap placement

### AI Integration

- **KataGo Engine** - State-of-the-art Go AI powered by neural networks
- **5 Difficulty Levels**
  - `easy` - Beginner level for learning
  - `medium` - Intermediate play
  - `hard` - Strong amateur level
  - `expert` - Near-professional strength
  - `professional` - Maximum AI strength
- **GTP Protocol** - Standard Go Text Protocol for AI communication
- **Fallback AI** - Built-in AI engine when KataGo is unavailable

### Server Features

- **HTTP Server** - Pure Win32 API implementation (no external dependencies)
- **RESTful API** - Complete JSON API for programmatic access
- **Real-time Gameplay** - Instant move processing and AI response
- **Multi-client Support** - Handle multiple simultaneous connections

### Web Interface

- **Interactive Board** - Click-to-place stones with visual feedback
- **Move History** - Track all moves in the current game
- **Score Display** - Live territory count during gameplay
- **Game Controls** - Pass, resign, and new game options
- **Difficulty Selector** - Change AI strength on the fly

---

## Technology Stack

| Component | Technology |
|-----------|------------|
| **Language** | C++17 |
| **HTTP Server** | Win32 API (Winsock2) |
| **AI Engine** | KataGo (via GTP) |
| **Protocol** | HTTP/1.1, JSON, GTP |
| **Frontend** | HTML5, CSS3, Vanilla JavaScript |
| **Build System** | Makefile, CMake, or build.bat |
| **Platform** | Windows (x64) |

---

## Getting Started

### Prerequisites

- **Operating System**: Windows 10/11 (x64)
- **Dependencies**: All required DLLs are included in the distribution

No additional software installation is required - the program is self-contained.

### Quick Start

1. **Download or Clone**
   ```bash
   git clone https://github.com/Liu200709/Liu-kata-go-game-cpp.git
   cd Liu-kata-go-game-cpp
   ```

2. **Run the Server**
   ```bash
   .\katago_game.exe
   ```

3. **Play the Game**
   Open your browser and navigate to:
   ```
   http://localhost:8000
   ```

That's it! The server will start and KataGo will initialize automatically.

### Building from Source

#### Option 1: Using Makefile (Recommended)

```bash
# Compile the project
make

# Or with explicit compiler
g++ -std=c++17 -O2 -o katago_game.exe main.cpp -lws2_32
```

#### Option 2: Using CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

#### Option 3: Using build.bat

```cmd
build.bat
```

---

## Usage Guide

### Web Interface

The web interface provides a complete gaming experience:

1. **Starting a Game**
   - The server starts with medium difficulty by default
   - Black (you) plays first
   - Click on the board to place a stone
   - AI responds automatically with white's move

2. **Game Controls**
   - **New Game**: Click "New Game" or send POST to `/reset`
   - **Pass**: Click "Pass" button
   - **Resign**: Click "Resign" button

3. **Viewing Score**
   - Score is calculated automatically
   - Click "Score" or send POST to `/score`

### API Endpoints

The server provides a comprehensive REST API:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Serve the web interface |
| `/reset` | POST | Reset the board to empty state |
| `/make_move` | POST | Place a stone at specified coordinates |
| `/score` | POST | Calculate and return current score |
| `/katago_status` | GET | Get KataGo initialization status |
| `/set_difficulty` | POST | Change AI difficulty level |

#### Example: Making a Move

```bash
# Reset the board
curl -X POST http://localhost:8000/reset

# Make a move at position (3, 3)
curl -X POST http://localhost:8000/make_move \
  -H "Content-Type: application/json" \
  -d '{"x": 3, "y": 3}'

# Response
{
  "success": true,
  "board": [[0,0,0,...], ...],
  "current_player": 1,
  "katago": true
}
```

#### Example: Changing Difficulty

```bash
# Set to expert level
curl -X POST http://localhost:8000/set_difficulty \
  -H "Content-Type: application/json" \
  -d '{"difficulty": "expert"}'

# Response
{
  "success": true,
  "difficulty": "expert"
}
```

### Difficulty Levels

| Level | Description | GTP Config |
|-------|-------------|------------|
| `easy` | Learning mode | 10 visits, fast |
| `medium` | Casual play | 100 visits |
| `hard` | Strong amateur | 500 visits |
| `expert` | Near-pro level | 1000 visits |
| `professional` | Maximum strength | 2000 visits |

---

## Project Structure

```
Project_seven/
├── main.cpp                 # Main server implementation
├── katago_game.exe          # Compiled executable
├── katago.exe              # KataGo AI engine
├── default_model.bin.gz    # Neural network model
├── default_gtp.cfg        # KataGo configuration
├── *.dll                  # Required dependencies (11 files)
├── templates/
│   └── index.html         # Web interface
├── KataGoData/
│   └── opencltuning/      # GPU optimization files
├── gtp_logs/             # GTP communication logs
├── test_*.ps1            # Test scripts
├── Makefile              # Build configuration
├── CMakeLists.txt        # CMake configuration
└── build.bat             # Windows build script
```

---

## Testing

The project includes comprehensive test scripts:

### Run All Tests

```powershell
powershell -ExecutionPolicy Bypass -File test_complete.ps1
```

### Individual Tests

```powershell
# Test KataGo integration
powershell -ExecutionPolicy Bypass -File test_katago.ps1

# Test API endpoints
powershell -ExecutionPolicy Bypass -File test_api.ps1

# Test turn system
powershell -ExecutionPolicy Bypass -File test_turn.ps1
```

### Test Coverage

| Test | Description |
|------|-------------|
| Server startup | HTTP server initialization |
| Board setup | 19×19 board creation |
| Move validation | Legal move detection |
| Capture logic | Stone capture mechanics |
| Ko detection | Prevention of illegal repeats |
| Scoring | Territory calculation |
| AI response | KataGo move generation |
| Difficulty levels | All 5 levels functional |

---

## Troubleshooting

### Common Issues

#### 1. Server Won't Start

**Symptom**: Port 8000 already in use

```bash
# Find and kill the process
netstat -ano | findstr :8000
taskkill /PID <process_id> /F
```

#### 2. KataGo Not Initializing

**Symptom**: AI not responding, fallback to built-in AI

**Solution**:
- Ensure `katago.exe` and `default_model.bin.gz` are in the same directory
- Check that all DLL files are present
- Verify GPU drivers are up to date

#### 3. Slow AI Response

**Symptom**: AI takes too long to respond

**Solution**:
- Lower the difficulty level
- Ensure `KataGoData/opencltuning/` contains GPU-specific tuning files
- Close other GPU-intensive applications

#### 4. Build Errors

**Symptom**: Compilation fails

**Solution**:
- Use a C++17 compatible compiler (GCC 9+, MSVC 2019+)
- Ensure WinSock2 development headers are available
- Try different build methods (Makefile, CMake, build.bat)

---

## Contributing

Contributions are welcome! Please follow these guidelines:

### How to Contribute

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### Code Style

- Follow standard C++ conventions
- Use meaningful variable and function names
- Add comments for complex logic
- Test thoroughly before submitting

### Reporting Issues

Please report bugs and suggest features via GitHub Issues. Include:
- Detailed description of the problem
- Steps to reproduce
- Expected vs. actual behavior
- System information (Windows version, GPU, etc.)

---

## License

This project is licensed under the **MIT License** - see the LICENSE file for details.

```
MIT License

Copyright (c) 2026 Liu200709

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## Acknowledgments

- **KataGo** - Open-source Go AI engine (https://github.com/lightvector/KataGo)
- **Go Game Rules** - Standard Chinese/Japanese rules implementation
- **Win32 API** - For providing a robust foundation for the HTTP server

---

## Contact

For questions, suggestions, or collaborations:

- **GitHub**: [Liu200709/Liu-kata-go-game-cpp](https://github.com/Liu200709/Liu-kata-go-game-cpp)
- **Issues**: [Open an Issue](https://github.com/Liu200709/Liu-kata-go-game-cpp/issues)

---

<p align="center">
  <sub>Built with ❤️ in C++ | Happy Go Playing! 🐱</sub>
</p>
