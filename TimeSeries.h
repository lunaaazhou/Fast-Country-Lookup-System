#ifndef TIMESERIES_H
#define TIMESERIES_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

// TimeSeries Class Declaration
class TimeSeries
{
    private:
        std::string countryName; 
        std::string countryCode; 
        std::string seriesName;
        std::string seriesCode;  

    public:

        TimeSeries* seriesNext; // Linked list pointer to hold multiple series for one country.
        int* year; 
        double* data;
        int size;

        TimeSeries();
        ~TimeSeries();

        // Getter functions for getting variables from private.
        std::string getCountryName() const { return countryName; }
        std::string getCountryCode() const { return countryCode; }
        std::string getSeriesName() const { return seriesName; }
        std::string getSeriesCode() const { return seriesCode; }

        // Setter functions
        void setCountryName(std::string name) { countryName = name; } //This setter function assigns the provided name to the countryName member variable.
        void setCountryCode(std::string code) { countryCode = code; }
        void setSeriesName(std::string name) { seriesName = name; }
        void setSeriesCode(std::string code) { seriesCode = code; }
};

// TreeNode for the binary tree (used in BUILD, FIND, LIMITS functions)
class TreeNode
{
    public:
        double value;                
        double minRange, maxRange;   //range of the mean values 
        int index;                //for debugging, used to track the position of the node   
        TreeNode* left;            //pointer to the left child node
        TreeNode* right;         //pointer to the right child node

        std::string countries[512];  // Fixed-size array for storing country names from the fixed array.
        int countryCount;  //count the number of country stored in the countries array
        TreeNode(double val, double minR, double maxR, int i): value(val), minRange(minR), maxRange(maxR), index(i), left(nullptr), right(nullptr), countryCount(0) {}
};

// Tree container
class Tree
{
    public:
        TreeNode* root;
        Tree() : root(nullptr) {}
        void deleteTree(TreeNode* node)
        {
            if (!node) return;
            deleteTree(node->left);
            deleteTree(node->right);
            delete node;
        }
        ~Tree() //destructor for the tree
        {
            deleteTree(root);
        }
};

// Node Class Declaration (Modified for Hashing)
class Node
{
    private:
        static const int TABLE_SIZE = 512;
        TimeSeries* table[TABLE_SIZE];   // for hash table, each slot will point to a country.
        bool slotOccupied[TABLE_SIZE];      // use this to mark the slots once occupied (for proper probing during deletions)
        int countryCount;                // To count the number of countries in the table
        std::string currentSeriesCode;    // Current series code (used in tree building and FIND)
        Tree tree;                       // Binary tree for grouping and searching countries.

        // Helper Functions for hash
        int computeW(const std::string &countryCode);
        int primaryHash(const std::string &countryCode);
        int secondaryHash(const std::string &countryCode);
        int findCountryIndex(const std::string &countryCode, int &steps);
        int insertCountry(TimeSeries* newCountry);
        int getAllCountries(TimeSeries* countries[], int max);
        
        // helper functions for tree
        bool allSameMean(double* means, int start, int end, double tolerance = 1e-3);
        TreeNode* buildSubtree(std::string* countryNames, double* means, int start, int end, double globalMin, double globalMax);
        void findHelper(TreeNode* node, double targetMean, const std::string &operation, std::string* results, int &count);
        double getMeanForCountry(const std::string &countryName);
        
        // Friend functions for tree deletion (from Project 3) //used this from chatgpt
        friend bool removeCountryFromArray(TreeNode* node, const std::string &countryName);
        friend TreeNode* deleteCountryHelper(TreeNode* root, const std::string &countryName, bool &found);

    public:

        Node(); //constructor
        ~Node(); //destructor

        void load(std::string filename);
        void list(std::string country);
        void range(const std::string &seriesCode);
        void build(const std::string &seriesCode);
        void find(double meanValue, const std::string &operation);

        void lookup(const std::string &countryCode);      
        void remove(const std::string &countryCode);          
        void insert(const std::string &countryCode, const std::string &filename); 
        void limits(const std::string &condition);
        void deleteCountryByName(const std::string &countryName);  
};

#endif
