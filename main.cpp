#include <iostream>
#include <string>
#include "TimeSeries.h"

int main()
{
    Node ts;
    std::string command;
    while(std::cin >> command)
    {
        if(command == "LOAD")
        {
            std::string filename;
            std::cin >> filename;
            ts.load(filename);
        }
        else if(command == "LIST")
        {
            std::string country;
            std::cin >> country;
            ts.list(country);
        }
        else if(command == "RANGE")
        {
            std::string seriesCode;
            std::cin >> seriesCode;
            ts.range(seriesCode);
        }
        else if(command == "BUILD")
        {
            std::string seriesCode;
            std::cin >> seriesCode;
            ts.build(seriesCode);
        }
        else if(command == "FIND")
        {
            double meanVal;
            std::string op;
            if(std::cin >> meanVal >> op)
                ts.find(meanVal, op);
            else
                std::cout << "failure" << std::endl;
        }
        else if(command == "DELETE")
        {  
            // For backward compatibility: DELETE using country name.
            std::string country;
            std::cin >> country;
            ts.deleteCountryByName(country);
        }
        else if(command == "LIMITS")
        {
            std::string condition;
            std::cin >> condition;
            ts.limits(condition);
        }
        else if(command == "LOOKUP")
        {
            std::string countryCode;
            std::cin >> countryCode;
            ts.lookup(countryCode);
        }
        else if(command == "REMOVE")
        {
            std::string countryCode;
            std::cin >> countryCode;
            ts.remove(countryCode);
        }
        else if(command == "INSERT")
        {
            std::string countryCode, filename;
            std::cin >> countryCode >> filename;
            ts.insert(countryCode, filename);
        }
        else if(command == "EXIT")
        {
            break;
        }
    }
    return 0;
}

