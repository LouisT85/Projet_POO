#include <SFML/Graphics.hpp>
#include "GameOfLife.h"
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

// Fonction pour sauvegarder l'état de la grille dans un fichier
void saveGridToFile(const std::vector<std::vector<bool>>& grid, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erreur : Impossible d'ouvrir le fichier " << filename << std::endl;
        std::exit(1);
    }
    int rows = grid.size();
    int cols = grid[0].size();
    file << rows << " " << cols << std::endl;
    for (const auto& row : grid) {
        for (bool cell : row) {
            file << (cell ? 1 : 0) << " ";
        }
        file << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Erreur : Veuillez fournir un fichier d'entrée." << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    bool isConsoleMode = (argc == 3 && std::string(argv[2]) == "--console");
    bool isGraphicsMode = (argc == 3 && std::string(argv[2]) == "--graphics");

    try {
        // Lecture de la grille et des obstacles depuis un fichier
        GameOfLife game = GameOfLife::fromFile(filename);
        auto [rows, cols] = game.getDimensions();
        std::vector<std::vector<bool>> obstacles(rows, std::vector<bool>(cols, false));

        // Charger les obstacles marqués par "2" dans le fichier
        std::ifstream file(filename);
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                int cellValue;
                file >> cellValue;
                if (cellValue == 2) {
                    obstacles[r][c] = true;
                }
            }
        }

        if (isConsoleMode) {
            // Mode Console : Exécuter un nombre d'itérations fixé
            int maxIterations = 100;
            std::string outputDir = filename + "_out";

            // Créer le répertoire de sortie
            if (!fs::exists(outputDir)) {
                fs::create_directory(outputDir);
            }

            for (int iteration = 0; iteration < maxIterations; ++iteration) {
                // Sauvegarder la grille dans un fichier à chaque itération
                std::string iterationFile = outputDir + "/iteration_" + std::to_string(iteration) + ".txt";
                saveGridToFile(game.getGrid(), iterationFile);

                // Mise à jour de la grille
                game.update(true, obstacles);

                std::cout << "Itération " << iteration + 1 << "/" << maxIterations << " terminée." << std::endl;

                if (game.hasStabilized()) {
                    std::cout << "La simulation s'est stabilisée à l'itération " << iteration + 1 << "." << std::endl;
                    break;
                }
            }

            std::cout << "Simulation terminée. Les résultats ont été enregistrés dans " << outputDir << std::endl;
        } else if (isGraphicsMode) {
            // Mode Graphique : Afficher la grille à l'aide de SFML
            const int cellSize = 20;
            sf::RenderWindow window(sf::VideoMode(cols * cellSize, rows * cellSize), "Jeu de la Vie");
            sf::RectangleShape cellShape(sf::Vector2f(cellSize - 1, cellSize - 1));
            cellShape.setFillColor(sf::Color::White);

            sf::RectangleShape obstacleShape(sf::Vector2f(cellSize - 1, cellSize - 1));
            obstacleShape.setFillColor(sf::Color::Blue);

            int iterationDelay = 200;
            bool isPaused = false;
            
            
            while (window.isOpen()) {
            	sf::Event event;
                while (window.pollEvent(event)) {
                    if (event.type == sf::Event::Closed) {
                        window.close();
                    }
                    if (event.type == sf::Event::MouseButtonPressed) {
                        if (event.mouseButton.button == sf::Mouse::Left) {
                            // Convertir les coordonnées de la souris dans l'espace logique
                            sf::Vector2i pixelPos(event.mouseButton.x, event.mouseButton.y);
                            sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

                            // Calculer la position de la cellule correspondante
                            int row = static_cast<int>(worldPos.y / cellSize);
                            int col = static_cast<int>(worldPos.x / cellSize);

                            // Vérifier si les coordonnées sont dans les limites de la grille
                            if (row >= 0 && row < rows && col >= 0 && col < cols) {
                                obstacles[row][col] = !obstacles[row][col]; // Inverser l'état de l'obstacle
                            }
                        }
                    }
                }

		
                

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    isPaused = !isPaused;
                    while (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    }
                }

                if (!isPaused) {
                    game.update(true, obstacles);
                }

                window.clear(sf::Color::Black);
                const auto& grid = game.getGrid();
                for (int r = 0; r < rows; ++r) {
                    for (int c = 0; c < cols; ++c) {
                        if (grid[r][c]) {
                            cellShape.setPosition(c * cellSize, r * cellSize);
                            window.draw(cellShape);
                        } else if (obstacles[r][c]) {
                            obstacleShape.setPosition(c * cellSize, r * cellSize);
                            window.draw(obstacleShape);
                        }
                    }
                }
                window.display();
                
                
                                // Gestion des touches pour les actions
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
                    try {
                        game.insertPattern("glider", rows / 2 - 1, cols / 2 - 1);
                    } catch (const std::invalid_argument& e) {
                        std::cerr << "Erreur : " << e.what() << std::endl;
                    }
                    while (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
                        // Attendre que la touche soit relâchée
                    }
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                    iterationDelay = std::max(50, iterationDelay - 50); // Augmenter la vitesse
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                    iterationDelay += 50; // Réduire la vitesse
                }
                
                sf::sleep(sf::milliseconds(iterationDelay));
            }
        } else {
            std::cerr << "Erreur : Mode non reconnu. Utilisez '--console' ou '--graphics'." << std::endl;
            return 1;
        }

    } catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
