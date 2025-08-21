#include "TimeSeries.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept> //the idea of this header from chatgpt, for helping manage runtime errors, like in this project, invalid string to double(change of datatype) conversions and could ensure it can recover and provide proper error message when something unexcepted happens


TimeSeries::TimeSeries() //TimeSeries class constructor
{
    size = 0;
    year = new int[100];      // Fixed-size allocation for years.
    data = new double[100];   // Fixed-size allocation for data values.
    seriesNext = nullptr;     // Initialize linked-list pointer.
}

TimeSeries::~TimeSeries() //TimeSeries class destructor
{
    delete[] year;
    delete[] data;
}


int Node::computeW(const std::string &countryCode) //to covert the countrycode to hash code
{
    int W = 0;
    for (int i = 0; i < 3 && i < countryCode.size(); i++) //loop through the characters of the country code
    {
        char c = countryCode[i];
        int digit = c - 'A';  // Convert the letters to 0-25.
        W = W * 26 + digit;
    }
    return W; //return the numerical value
}


int Node::primaryHash(const std::string &countryCode)//primary Hash function
{
    int W = computeW(countryCode); //compute the countryCode to a numerical value
    return W % TABLE_SIZE; //return the mod num to put it in hash table
}


int Node::secondaryHash(const std::string &countryCode)//secondary Hash function
{
    int W = computeW(countryCode);
    int h = (W / TABLE_SIZE) % TABLE_SIZE; //primary hash value
    if(h % 2 == 0)
    {
        h = h + 1;       
    }
    return h;   //return the secondary hash value
}


//helper function for the LOOKUP, INSERT, REMOVE function
int Node::findCountryIndex(const std::string &countryCode, int &steps) // the int &steps make steps a pass by reference, in this way, its updated value inside this helper function will be available when calling this function
{
    int h1 = primaryHash(countryCode); //calculate the first hash value, convert the country code
    int h2 = secondaryHash(countryCode);//compute the second hash value, this value will used a step size for collision resolution

    steps = 0;

    for(int i = 0; i < TABLE_SIZE; i++)
    {
        int idx =(h1 + i * h2) % TABLE_SIZE; //compute index using double hashing

        steps++;

        if(table[idx] == nullptr && !slotOccupied[idx])
        {
            // Empty slot and not previously occupied: country not in table.
            return -1; //check if slot is empty
        }

        if(table[idx] != nullptr && table[idx]->getCountryCode() == countryCode)
        {
            return idx; //return the index when find the input country code
        }

    }
    return -1;
}


int Node::insertCountry(TimeSeries* newCountry) //insert country into the hash table //used this insert country from chatgpt in project3
{
    std::string countryCode = newCountry->getCountryCode();

    int h1 = primaryHash(countryCode);
    int h2 = secondaryHash(countryCode);


    int slotIndex = -1; //initalize a number that will never gonna be the index

    for (int i = 0; i < TABLE_SIZE; i++)
    {
        int idx = (h1 + i * h2) % TABLE_SIZE;
        if (table[idx] == nullptr)
        {
            if (slotOccupied[idx])
            { // Record first available slot.
                if (slotIndex == -1)
                slotIndex = idx;
            }
            else
            {
                // Found an empty slot, if a occupied slot has seen earlier, then use it.
                if(slotIndex != -1) //when the slot is empty
                {
                    table[slotIndex] = newCountry; //insert the slot in the hash table with the newcountry
                    slotOccupied[slotIndex] = false;
                    countryCount++;
                    return slotIndex;
                }
                else
                {
                    table[idx] = newCountry;
                    slotOccupied[idx] = false;
                    countryCount++;
                    return idx;
                }
            }
        }

        else if(table[idx]->getCountryCode() == countryCode)
        {
            return idx; //if the country is already exist
        }
    }

    if(slotIndex != -1)
    {
        table[slotIndex] = newCountry;
        slotOccupied[slotIndex] = false;
        countryCount++;
        return slotIndex;
    }

    return -1;  // Table is full.
}

//learnt from chatgpt, used it for extract all occupied country entires from the hash table into a managemable form
//This function just for making clear which slots can I use for later on
int Node::getAllCountries(TimeSeries* countries[], int max)
{
    int count = 0;
    for(int i = 0; i < TABLE_SIZE; i++) //iteratese all slots in the table
    {
        if (table[i] != nullptr) 
        {
            countries[count++] = table[i]; //stores the pointer in the countries array and increment the counter

            if (count >= max) //iterate until exceed 512 countries
            {
                break;
            }
        }
    }
    return count; //return the num of country found
}


//Helper function for building the tree (from P3)

//Starting of the BUILD function:
bool Node::allSameMean(double* means, int start, int end, double tolerance) // This is a helper function checking if all means in means[] are within tolerance of the first
{
    for(int i = start + 1; i <= end; i++)
    {
        double difference = means[i] - means[start]; //to compare the mean[i] to mean[start]

        if(difference < 0)
        {
            difference = -difference; //convert the negative difference to positive
        }

        if(difference > tolerance) //if exceed tolarance, it shows that they are not same, then different mean
        {
            return false;
        }
    }
    return true; //if no difference exceed tolerance
}

// A helper function that recursively builds a subtree from arrays of countryNames and means. //use some idea from chatgpt in project3
TreeNode* Node::buildSubtree(std::string* countryNames, double* means, int start, int end, double globalMin, double globalMax)
{

    //if there's only one countries or they all having same mean
    if(start == end || allSameMean(means, start, end)) 
    {
        TreeNode* leaf = new TreeNode(means[start], globalMin, globalMax, 0); // Create a leaf node with the mean value of the first element, and set the range to the global values.

        int total = end - start + 1; //determine the total number of countries in this subset
        leaf->countryCount = total;

        for(int i = 0; i < total; i++)
        {
            leaf->countries[i] = countryNames[start + i]; //put the country name from the input array into the leaf's fixed size array
        }
        return leaf;
    }

    // otherwise, find the midpoint to do partition
    double midPoint = globalMin + (globalMax - globalMin) / 2.0;

    // partition the subset into left and right
    std::string leftCountries[512], rightCountries[512];

    //fized size for the left and right leaf
    double leftMeans[512], rightMeans[512];

    int leftCount = 0, rightCount = 0;


    for(int i = start; i <= end; i++) //iterates the subset and distrubute the country to left and right
    {
        if(means[i] < midPoint) //less than midpoint, put to left
        {
            leftCountries[leftCount] = countryNames[i];
            leftMeans[leftCount] = means[i];
            leftCount++;
        }
        else //greater than, right
        {
            rightCountries[rightCount] = countryNames[i];
            rightMeans[rightCount] = means[i];
            rightCount++;
        }
    }

    //for the left tree: to find the min and max mean
    double leftMin = 1e9, leftMax = -1e9; //start with initializing the left min biggest and max the smallest for later replacing

    for(int i = 0; i < leftCount; i++) 
    {
        if(leftMeans[i] < leftMin)
        {
            leftMin = leftMeans[i]; //replace the left min with the new left min
        }

        if(leftMeans[i] > leftMax)
        {
            leftMax = leftMeans[i]; //replace the left max with the new left max
        }
    }

    //for the right tree: to find the min and max mean
    double rightMin = 1e9, rightMax = -1e9; //start with initializing the right min with a big num and right max with a small num

    for(int i = 0; i < rightCount; i++)
    {
        if(rightMeans[i] < rightMin)
        {
            rightMin = rightMeans[i];
        }

        if(rightMeans[i] > rightMax)
        {
            rightMax = rightMeans[i];
        }
    }

    TreeNode* leftChild = nullptr; // Recursively build left subtree, (if there's country in the left
    if(leftCount > 0)
    {
        leftChild = buildSubtree(leftCountries, leftMeans, 0, leftCount - 1, leftMin, leftMax);
    }

    TreeNode* rightChild = nullptr; //recursively build right subtree, if there's country in the right

    if(rightCount > 0)
    {
        rightChild = buildSubtree(rightCountries, rightMeans, 0, rightCount - 1, rightMin, rightMax);
    }

    TreeNode* parent = new TreeNode(globalMax, globalMin, globalMax, 0); // Create the parent node to stores all countries in this given range
    
    int total = end - start + 1;
    parent->countryCount = total;
    
    for(int i = 0; i < total; i++)
    {
        parent->countries[i] = countryNames[start + i]; // Copy all country names from the original array (for this range) into the parent's array.
    }

    parent->left = leftChild; //attach left tree to the partent
    parent->right = rightChild; //attach right tree to the partent

    return parent; //return the parent, which is the tree of this range

}

// A helper function that recursively searches the binary tree for countries whose actual mean satisfies the given condition.
//for FIND function (P3)
void Node::findHelper(TreeNode* node, double targetMean, const std::string &operation, std::string* results, int &count) //idea of this function from chatgpt
{
    if(!node)
    {
        return; //if there's no node then exit the function
    }

    for(int i = 0; i < node->countryCount; i++) //iterate through all countries stored in the current tree node
    {
        std::string countryName = node->countries[i]; //get current country's name form the node
        double countryMean = getMeanForCountry(countryName); //get the mean value for the current country using this helper function

        bool match = false; //create this bool match to check if the function's mean satisfy the condition

        if(operation == "less") //check the operation type and compare the country's mean to the target mean value
        {
            if(countryMean < targetMean) //matches
            {
                match = true;
            }
        }

        else if(operation == "greater") //check the operation type and compare the country's mean to the target mean value
        {
            if(countryMean > targetMean) //matches
            {
                match = true;
            }
        }

        else if(operation == "equal") //check the operation type and compare the country's mean to the target mean value
        {
            double difference = countryMean - targetMean;
            if(difference < 0)
            {
                difference = -difference; //conver the differnece between  current country's mean and the target mean to positive
            }
            if(difference < 1e-3) //if the difference less than the tolerance
            {
                match = true;  //matches
            }
        }

        if(match)//country's mean matches with the input mean: match is true, then add it to the result
        {
            bool alreadyInside = false; //used this idea from chatgpt, shows that the country already in the result to avoid duplicate
            for(int j = 0; j < count; j++)
            {
                if(results[j] == countryName)
                {
                    alreadyInside = true;
                    break; //if found, exit the loop
                }
            }
            if(!alreadyInside && count < 512) //if country not in the result yet, then add it in if there's still space
            {
                results[count++] = countryName;
            }
        }
    }

    findHelper(node->left, targetMean, operation, results, count); //recursively search the left subtree
    findHelper(node->right, targetMean, operation, results, count); //recursively search the right subtree

}


//Helper function for the FIND function (P3)
// Node::getMeanForCountry - Looks up the country in our fixed-size array and
// computes the mean for the currently built series (using currentSeriesCode). 
double Node::getMeanForCountry(const std::string &countryName)
{
    for (int i = 0; i < TABLE_SIZE; i++) // loop through every slots in the hash table
    {
        if(table[i] != nullptr && table[i]->getCountryName() == countryName)
        {
            TimeSeries* s = table[i]->seriesNext; // if found the country; now search its series list.
            
            while(s)//traverse the series to find the series that matches with the current seris code
            {
                if(s->getSeriesCode() == currentSeriesCode)
                {
                    double sum = 0.0;
                    for(int j = 0; j < s->size; j++)
                    {
                        sum += s->data[j]; //if found, compute the sum of all data points
                    }
                    return (s->size > 0) ? (sum / s->size) : 0.0; //return the mean value of all these data points, otherwise, return 0.0
                }
                s = s->seriesNext; //move to the next series in the linked list
            }
            return 0.0; // If the country is found but no matching series code is found
        }
    }
    return 0.0; //when country not found in the hash table
}


//Starting of the DELETE function
//Tree Deletion Helper Functions(from Project 3) used this from chatgpt
bool removeCountryFromArray(TreeNode* node, const std::string &countryName)
{
    bool foundAny = false; //flag to check if any country were removed, add this for hash
    int i = 0;  // Initialize the index as zero for iterating through the countries array

    while(i < node->countryCount) //loop over countries in the node
    {
        if(node->countries[i] == countryName) //if current country matches with the given country name
        {
            for(int j = i; j < node->countryCount - 1; j++) //shift all country names one position to the left
            {
                node->countries[j] = node->countries[j + 1];
            }
            node->countryCount--; // Decrement the count of countries since one has been removed.
            foundAny = true; // Mark the flag when found and removed happens.

            // Do not increment 'i' here because after shifting, a new element now occupies index i. 
        }
        else
        {
            i++; //if no matches at the index
        }
    }
    return foundAny; //return true if >= 1 country removed, otherwise, return false
}

TreeNode* deleteCountryHelper(TreeNode* root, const std::string &countryName, bool &found)
{
    if (!root)
    {
        return nullptr; //if current node is null, exit the function
    }

    bool removedHere = removeCountryFromArray(root, countryName); //when >=1 country removed, use the previous helper function to chect this

    if(removedHere) // If the country was removed from this node, update the 'found' flag.
    {
        found = true;
    }

    root->left  = deleteCountryHelper(root->left, countryName, found); // Recursively process the left subtree and update the left pointer.
    root->right = deleteCountryHelper(root->right, countryName, found); // Recursively process the right subtree and update the right pointer.

    if(!root->left && !root->right && root->countryCount == 0)
    {
        delete root; //aviod memory leak
        return nullptr;
    }
    return root; //return the current node
}

//Node class constructor
Node::Node() : countryCount(0) //initialize the member variable countryCount to 0 before the constructor's body runs, means when node created, it will starts with zero countries stored in the hash table
{
    for(int i = 0; i < TABLE_SIZE; i++) //loop iterates every index in the hash table.
    {
        table[i] = nullptr; //each slots set to nullptr, means they are empty
        slotOccupied[i] = false; //means no slot has been used yet
    }
}

 //Node class destructor
Node::~Node()
{
    for(int i = 0; i < TABLE_SIZE; i++)
    {
        if(table[i] != nullptr)
        {
            TimeSeries* series = table[i]->seriesNext; //traverse the linked list
            
            while(series) //recursively deleting the tree
            {
                TimeSeries* temp = series;
                series = series->seriesNext;
                delete temp;
            }

            delete table[i];
        }
    }
    if(tree.root)
    {
        tree.deleteTree(tree.root); //indicating the tree no longer exist
        tree.root = nullptr;
    }
}


// LOOKUP: Given a country code, print the table index and number of hashing steps.
void Node::lookup(const std::string &countryCode)
{
    int steps;
    int index = findCountryIndex(countryCode, steps); //using helper function to find the index and steps

    if(index == -1) //if the countrycode not found in the list
    {
        std::cout << "failure" << std::endl;
    }
    else
    {
        std::cout << "index " << index << " searches " << steps << std::endl;
    }
}


// REMOVE: remove a country by given countryCode from both hash table and tree, and then free its memory.
void Node::remove(const std::string &countryCode)
{
    int steps;
    int index = findCountryIndex(countryCode, steps);

    if(index == -1) //country wasn't found in the hash table
    {
        std::cout << "failure" << std::endl; //do nothing if there's no country
    }
    else
    {
        std::string countryName = table[index]->getCountryName(); //gets the country name
        if(tree.root)//check if the binary tree exist, check tree.root is not nullptr
        {
            bool found = false; 
            tree.root = deleteCountryHelper(tree.root, countryName, found); //remove the country from the tree
        }

        TimeSeries* s = table[index]->seriesNext;

        while(s) //iterates though the timeseries
        {
            TimeSeries* temp = s;
            s = s->seriesNext;
            delete temp;  //free the memory for that timeseies
        }

        delete table[index];
        table[index] = nullptr; //show the slot is empty now
        slotOccupied[index] = true; //this flag tells the hash table that this slot was once used, ensuring that during future lookups the probing doesn’t stop prematurely
        countryCount--; //count of countries -1
        
        std::cout << "success" << std::endl;
    }
}

// INSERT: Given a country code and filename, find the country in the file and insert it into the hash table.
void Node::insert(const std::string &countryCode, const std::string &filename)
{
    int steps; //to count number of hash probes
    int index = findCountryIndex(countryCode, steps); //return the index of the country to where they were find, and give the updated steps value in the find country index function to the new int steps variable here

    if(index != -1) //country does not exist in the hash table
    {
        std::cout << "failure" << std::endl; // return failure if the country does not exist in the hash table
        return;
    }

    std::ifstream file(filename); //open the input file
    if(!file)
    {
        std::cout << "failure" << std::endl;
        return;
    }
    
    std::string line;
    bool foundLine = false; //having a boolean flag here to false, make it to true after finding the matches countryCode.

    while(std::getline(file, line)) //loop through the file
    {
        std::stringstream ss(line); //for parsing
        std::string countryName, cCode, seriesName, seriesCode;

        std::getline(ss, countryName, ',');
        std::getline(ss, cCode, ',');
        std::getline(ss, seriesName, ',');
        std::getline(ss, seriesCode, ',');

        if(cCode == countryCode) //find matches countryCode
        {
            foundLine = true; //mark this flag as true
            TimeSeries* newCountry = new TimeSeries();

            newCountry->setCountryName(countryName); //set the series name
            newCountry->setCountryCode(cCode); //set the series code
            newCountry->seriesNext = nullptr;

            TimeSeries* newSeries = new TimeSeries();
            newSeries->setSeriesName(seriesName);
            newSeries->setSeriesCode(seriesCode);
        
            newSeries->size = 0;
            newSeries->seriesNext = nullptr;
            newCountry->seriesNext = newSeries;


            int yearValue = 1960;
            std::string value;
            while(std::getline(ss, value, ','))
            {
                try //used this try and catch from chatgpt
                {
                    double num = std::stod(value); //using this std::stod to convert this std::string value to a double data type
                    if(num >= 0)
                    {
                        newSeries->year[newSeries->size] = yearValue;
                        newSeries->data[newSeries->size] = num;
                        newSeries->size++;
                    }
                }
                catch(std::invalid_argument&) // used this std::invalid_argument& from chatgpt, for when the std::stod receiveds argument that it cannot convert or process properly, by catching it with '&', it can aviod copying the exception object and can optionally inspect its detail//use this try and catch for: if std::stod fails, for example, when the value is invalud number, the excepting is cought and the data will be ignored. 
                {
                    // ignore invalid data.
                }
                yearValue++;
            }

            int insertedIndex = insertCountry(newCountry);

            if(insertedIndex == -1) //means hash table is full, no available slots.
            {
                std::cout << "failure" << std::endl;
            }
            else
            {
                std::cout << "success" << std::endl;
            }

            break;
        }
    }

    file.close();

    if(!foundLine) //means no matching found
    {
        std::cout << "failure" << std::endl;
    }
}


// some modified functions from project 3:

// LOAD: Read the file and insert each country using hashing.
void Node::load(std::string filename)
{
    std::ifstream file(filename);
    if(!file)
    {
        std::cout << "failure" << std::endl; //file cannot be opened
        return;
    }

    std::string line;

    while(std::getline(file, line)) //read the file line by line
    {
        std::stringstream ss(line);
        std::string countryName, countryCode, seriesName, seriesCode;

        std::getline(ss, countryName, ',');
        std::getline(ss, countryCode, ',');
        std::getline(ss, seriesName, ',');
        std::getline(ss, seriesCode, ',');


        int steps;
        int idx = findCountryIndex(countryCode, steps); //return the index of the country to where they were find, and give the updated steps value in the find country index function to the new int steps variable here

        if(idx == -1)
        {
            TimeSeries* newCountry = new TimeSeries();
            newCountry->setCountryName(countryName);
            newCountry->setCountryCode(countryCode);
            newCountry->seriesNext = nullptr;

            int insertedIndex = insertCountry(newCountry); //used insert country helper function to determine where to insert in the hash table.

            if(insertedIndex == -1)
            {
                continue;  //loop will move to the next line
            }

            idx = insertedIndex;
        }

        TimeSeries* newSeries = new TimeSeries();

        newSeries->setSeriesName(seriesName);
        newSeries->setSeriesCode(seriesCode);
        newSeries->size = 0;
        newSeries->seriesNext = nullptr;

        TimeSeries* head = table[idx]->seriesNext;

        if(head == nullptr)
        {
            table[idx]->seriesNext = newSeries;
        }

        else
        {
            while(head->seriesNext)
            {
                head = head->seriesNext;
            }

            head->seriesNext = newSeries;
        }

        int yearValue = 1960;
        std::string value;


        while(std::getline(ss, value, ','))
        {
            try
            {
                double num = std::stod(value);
                if(num >= 0)
                {
                    newSeries->year[newSeries->size] = yearValue;
                    newSeries->data[newSeries->size] = num;
                    newSeries->size++;
                }
            }
            catch(std::invalid_argument&)
            {
                // ignore invalid data.
            }

            yearValue++;
        }
    }

    file.close();
    std::cout << "success" << std::endl;

}

// LIST: Given a country name, search through the hash table and print its details.
void Node::list(std::string country)
{
    for(int i = 0; i < TABLE_SIZE; i++) //iterates through the hash table
    {
        if(table[i] != nullptr)
        {
            if(table[i]->getCountryName() == country) //if found matching country
            {
                std::cout << table[i]->getCountryName() << " " << table[i]->getCountryCode();
                TimeSeries* series = table[i]->seriesNext;
                
                while(series)
                {
                    std::cout << " " << series->getSeriesName();
                    series = series->seriesNext;
                }

                std::cout << std::endl;
                return;
            }
        }
    }
}

// RANGE: Compute the min and max mean (for a given series code) across all countries.
void Node::range(const std::string &seriesCode)
{
    TimeSeries* countries[512];
    int n = getAllCountries(countries, 512);

    if(n == 0)
    {
        std::cout << "failure" << std::endl;
        return;
    }

    double minMean = 1e9, maxMean = -1e9;
    bool found = false;
    for (int i = 0; i < n; i++)
    {
        TimeSeries* cur = countries[i];
        TimeSeries* s = cur->seriesNext;
        double sum = 0.0;
        int countNum = 0;
        while(s)
        {
            if(s->getSeriesCode() == seriesCode)
            {
                for(int j = 0; j < s->size; j++)
                {
                    sum += s->data[j];
                    countNum++;
                }
                break;
            }
            
            s = s->seriesNext;
        }

        if(countNum > 0)
        {
            double mean = sum / countNum;
            if(mean < minMean)
            {
                minMean = mean;
            }
            if(mean > maxMean)
            {
                maxMean = mean;
            }
            found = true;
        }
    }

    if(!found)
    {
        std::cout << "failure" << std::endl;
    }
    else
    {
        std::cout << minMean << " " << maxMean << std::endl;
    }
}

// BUILD: Build the binary tree based on the means of the series specified.
void Node::build(const std::string &seriesCode)
{
    TimeSeries* countries[512]; // Create an array, holding pointers to all occupied countries from the hash table
    int n = getAllCountries(countries, 512);
    std::string allCountries[512]; //array store country names

    double means[512];
    int count = 0; //to count number of data that compute a mean

    for (int i = 0; i < n; i++)
    {
        double sum = 0.0;
        int counting = 0;
        TimeSeries* s = countries[i]->seriesNext; //get the pointer to the linked list of the timeseries of this country

        while(s) //traverse of the linked list of series data of this country
        {
            if(s->getSeriesCode() == seriesCode) //check if the series code matches
            {
                for(int j = 0; j < s->size; j++) //if match then iterates it's data points
                {
                    sum += s->data[j]; //sum up the datas
                    counting++; //simultaneously, count the num of data 
                }
                break;
            }
            s = s->seriesNext; //move to the next series of this linked list
        }

        double mean = (counting > 0) ? (sum / counting) : 0.0;
        allCountries[count] = countries[i]->getCountryName();
        means[count] = mean;
        count++; //increase the amount of the proceeded country
    }

    double globalMin = 1e9, globalMax = -1e9; //initialize the gloval min with extremely large, gloval max with extremely small
    for(int i = 0; i < count; i++) //iterate all means to find global max and min
    {
        if(means[i] < globalMin)
        {
            globalMin = means[i];            
        }
        if(means[i] > globalMax)
        {
            globalMax = means[i];
        }
    }

    currentSeriesCode = seriesCode; //update the current series code for later

    if(tree.root)
    {
        tree.deleteTree(tree.root);
        tree.root = nullptr; //delete the tree to aviod memory leak
    }
    if(count > 0)
    {
        tree.root = buildSubtree(allCountries, means, 0, count - 1, globalMin, globalMax);
    }

    std::cout << "success" << std::endl;
}

// FIND: Using the binary tree, find all countries whose mean (for the current series) meets the given condition.
void Node::find(double meanValue, const std::string &operation)
{
    if(!tree.root) //if the tree hasn't been built before, return failue then exit
    {
        std::cout << "failure" << std::endl;
        return;
    }

    std::string results[512]; //array to store name of the matching countries
    int count = 0; //initialize the count of the num of matching countries to zero
    findHelper(tree.root, meanValue, operation, results, count); //use the given mean and operation to recursively search the tree starting from the root

    if(count == 0) //if no matching country found
    {
        std::cout << std::endl;
    }
    else
    {
        for(int i = 0; i < count; i++) //otherwise, if found, print all the matching country names
        {
            std::cout << results[i];
            if(i < count - 1)
                std::cout << " ";
        }

        std::cout << std::endl;
    }
}

// DELETE by country name (for backward compatibility).
void Node::deleteCountryByName(const std::string &countryName)
{
    for (int i = 0; i < TABLE_SIZE; i++)
    {
        if(table[i] != nullptr && table[i]->getCountryName() == countryName) //if slot occupied and the country name matches
        {
            std::string code = table[i]->getCountryCode(); //get that country code from hash table
            remove(code); //call the remove funciton with this country code
            return; //exist the function after removing country
        }
    }
    
    std::cout << "failure" << std::endl; //if no matching country found
}

// LIMITS: Traverse the binary tree to print countries at the lowest or highest leaf.
void Node::limits(const std::string &condition)
{
    if(!tree.root) //binary tree hasn't build before
    {
        std::cout << "failure" << std::endl;
        return;
    }

    TreeNode* current = tree.root; //start the traversal from the root of the tree

    if(condition == "lowest") // For "lowest", follow the left child pointers to reach the leftmost leaf.
    {
        while(current->left != nullptr)
        {
            current = current->left;
        }
    }

    else if(condition == "highest") // For "highest", follow the right child pointers to reach the rightmost leaf.
    {
        while(current->right != nullptr)
            current = current->right;
    }

    else
    {
        std::cout << "failure" << std::endl; //any invalid conditions
        return;
    }

    for(int i = 0; i < current->countryCount; i++)
    {
        std::cout << current->countries[i];
        if(i < current->countryCount - 1)
        {
            std::cout << " ";            
        }
    }

    std::cout << std::endl;
}
