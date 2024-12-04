#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H
 
#include <vector>
#include <string>
#include <utility>
#include <thread>
#include <functional>
#include <unordered_map>
 
class GameOfLife {
private:
    int rows, cols;
    std::vector<std::vector<bool>> grid;
    std::vector<std::vector<bool>> previousGrid;
    std::vector<std::vector<bool>> obstacles;  // Pour les cellules obstacles
    std::unordered_map<std::string, std::vector<std::vector<bool>>> patterns; // Patterns prédéfinis
    bool isStable = false; // Flag for detecting stable state
 
    // Compte les voisins vivants d'une cellule (avec gestion de la grille torique)
    int countLivingNeighbors(int row, int col, bool toroidal);

    // Initialiser les motifs préprogrammés
    void initializePatterns();
 
public:
    // Constructeur
    GameOfLife(int r, int c, const std::vector<std::vector<bool>>& initialGrid);
 
    // Mise à jour de la grille (gestion de la grille torique et des obstacles)
    void update(bool toroidal, const std::vector<std::vector<bool>>& obstacles);
    void parallelUpdate(bool toroidal, const std::vector<std::vector<bool>>& obstacles, int numThreads);
 
    // Obtenir la grille actuelle
    const std::vector<std::vector<bool>>& getGrid() const;
 
    // Retourner les dimensions
    std::pair<int, int> getDimensions() const;

    // Vérifier la stabilisation de la grille
    bool hasStabilized() const;
 
    // Charger une grille depuis un fichier
    static GameOfLife fromFile(const std::string& filename);
    
    // Écrire la grille dans un fichier
    void writeToFile(const std::string& filename) const;

    // Ajouter des obstacles
    void addObstacles(const std::vector<std::pair<int, int>>& obstaclePositions);

    // Insérer un motif prédéfini
    void insertPattern(const std::string& name, int startRow, int startCol);
};
 
#endif // GAME_OF_LIFE_H
