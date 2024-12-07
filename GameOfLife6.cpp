#include "GameOfLife6.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <thread>

using namespace std;

// Constructeur
GameOfLife::GameOfLife(int r, int c, const vector<vector<bool>>& initialGrid)
    : rows(r), cols(c), grid(initialGrid), previousGrid(initialGrid) {
    initializePatterns(); // Initialiser les motifs
}

// Initialiser les motifs préprogrammés
void GameOfLife::initializePatterns() {
    patterns["glider"] = {
        {0, 1, 0},
        {0, 0, 1},
        {1, 1, 1}
    };

}

// Compte les voisins vivants avec grille torique
int GameOfLife::countLivingNeighbors(int row, int col, bool toroidal) {
    int directions[8][2] = {
        {-1, -1}, {-1, 0}, {-1, 1},
        {0, -1},          {0, 1},
        {1, -1}, {1, 0}, {1, 1}
    };
    int count = 0;
    for (auto& dir : directions) {
        int newRow = row + dir[0];
        int newCol = col + dir[1];

        if (toroidal) {
            newRow = (newRow + rows) % rows;
            newCol = (newCol + cols) % cols;
        }

        if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
            count += grid[newRow][newCol];
        }
    }
    return count;
}

// Mise à jour de la grille
void GameOfLife::update(bool toroidal, const vector<vector<bool>>& obstacles) {
    vector<vector<bool>> newGrid(rows, vector<bool>(cols, false));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (obstacles[r][c]) {
                newGrid[r][c] = grid[r][c];
                continue;
            }

            int livingNeighbors = countLivingNeighbors(r, c, toroidal);
            if (grid[r][c]) {
                newGrid[r][c] = (livingNeighbors == 2 || livingNeighbors == 3);
            } else {
                newGrid[r][c] = (livingNeighbors == 3);
            }
        }
    }
    previousGrid = grid;
    grid = newGrid;
}

// Mise à jour parallèle de la grille
void GameOfLife::parallelUpdate(bool toroidal, const vector<vector<bool>>& obstacles, int numThreads) {
    vector<vector<bool>> newGrid(rows, vector<bool>(cols, false));
    auto worker = [&](int start, int end) {
        for (int r = start; r < end; ++r) {
            for (int c = 0; c < cols; ++c) {
                if (obstacles[r][c]) {
                    newGrid[r][c] = grid[r][c];
                    continue;
                }
                int livingNeighbors = countLivingNeighbors(r, c, toroidal);
                if (grid[r][c]) {
                    newGrid[r][c] = (livingNeighbors == 2 || livingNeighbors == 3);
                } else {
                    newGrid[r][c] = (livingNeighbors == 3);
                }
            }
        }
    };

    vector<thread> threads;
    int rowsPerThread = rows / numThreads;
    for (int i = 0; i < numThreads; ++i) {
        int start = i * rowsPerThread;
        int end = (i == numThreads - 1) ? rows : start + rowsPerThread;
        threads.emplace_back(worker, start, end);
    }

    for (auto& t : threads) {
        t.join();
    }

    previousGrid = grid;
    grid = newGrid;
}

// Vérifie si la grille est stable
bool GameOfLife::hasStabilized() const {
    return grid == previousGrid;
}

// Insérer un motif prédéfini
void GameOfLife::insertPattern(const string& name, int startRow, int startCol) {
    if (patterns.find(name) == patterns.end()) {
        throw invalid_argument("Motif non reconnu : " + name);
    }
    const auto& pattern = patterns.at(name);
    for (int r = 0; r < pattern.size(); ++r) {
        for (int c = 0; c < pattern[0].size(); ++c) {
            int newRow = startRow + r;
            int newCol = startCol + c;
            if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols) {
                grid[newRow][newCol] = pattern[r][c];
            }
        }
    }
}

// Obtenir la grille actuelle
const vector<vector<bool>>& GameOfLife::getGrid() const {
    return grid;
}

// Retourner les dimensions
pair<int, int> GameOfLife::getDimensions() const {
    return {rows, cols};
}

// Charger une grille depuis un fichier
GameOfLife GameOfLife::fromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Erreur : Impossible d'ouvrir le fichier " + filename);
    }

    int rows, cols;
    file >> rows >> cols;
    vector<vector<bool>> initialGrid(rows, vector<bool>(cols, false));

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int value;
            file >> value;
            initialGrid[r][c] = (value == 1);
        }
    }

    return GameOfLife(rows, cols, initialGrid);
}

// Écrire la grille dans un fichier
void GameOfLife::writeToFile(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Erreur : Impossible d'écrire dans le fichier " + filename);
    }

    file << rows << " " << cols << endl;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            file << (grid[r][c] ? 1 : 0) << " ";
        }
        file << endl;
    }
}

// Ajouter des obstacles
void GameOfLife::addObstacles(const vector<pair<int, int>>& obstaclePositions) {
    obstacles = vector<vector<bool>>(rows, vector<bool>(cols, false));
    for (auto& pos : obstaclePositions) {
        int r = pos.first, c = pos.second;
        if (r >= 0 && r < rows && c >= 0 && c < cols) {
            obstacles[r][c] = true;
        }
    }
}
