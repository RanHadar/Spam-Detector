#include <iostream>
#include "HashMap.hpp"
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
//**********************************************************************************************
using String = std::string;
using pair = std::pair<String, int>; //Represents a pair (string,int)
//**********************************************************************************************
const String SPAM = "SPAM\n";
const String NOT_SPAM = "NOT_SPAM\n";
const String INVALID_INPUT = "Invalid input\n";
const String USAGE = "Usage: SpamDetector <database path> <message path> <threshold>\n";
const int ARGS = 4;


bool isNumber(const std::string& s);
int keyScore(String &text, pair& p);

//********************************************************************************************
/**
 * Responsible for closing the program's files
 * @param dataBase
 * @param message
 */
void closeFiles(std::fstream &dataBase, std::fstream &message);
/**
 *
 * @param dataBase
 * @return
 */
int closeData(std::fstream &dataBase);
/**
 *
 * @param threshold
 * @param totalScore
 */
void scoreOutPut(int threshold, int totalScore);
//*********************************************************************************************
/**
 *Main method
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[])
{
    int threshold;
    if(argc != ARGS)
    {
        std::cerr << USAGE << std::endl;
        exit(EXIT_FAILURE);
    }

    std::fstream dataBase, message;
    dataBase.open(argv[1]);

    if(!dataBase.is_open())
    {
        std::cerr << INVALID_INPUT << std::endl;
        exit(EXIT_FAILURE);
    }
    if(dataBase.peek() == std::ifstream::traits_type::eof())
    {
        std::cout << NOT_SPAM;
        return 0;
    }
    if(!isNumber(argv[3]))
    {
        return closeData(dataBase);
    }
    threshold = std::stoi(argv[3]);
    if(threshold <= 0)
    {
        return closeData(dataBase);
    }

    String dataLine, scoreLine, key;
    char delimiter = ',';
    int score;
    HashMap<String, int> map;

    while(!dataBase.eof())
    {
        std::getline(dataBase, key, delimiter);
        std::getline(dataBase, scoreLine);
        if(key.empty() && dataBase.eof())
        {
            break;
        }
        if(dataBase.fail() || key.empty())
        {
            return closeData(dataBase);
        }

        if(!isNumber(scoreLine))
        {
            return closeData(dataBase);

        }
        score = std::stoi(scoreLine);
        std::transform(key.begin(), key.end(), key.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        map.insert(key, score);
    }

    message.open(argv[2]);
    if(!message.is_open())
    {
        return closeData(dataBase);
    }

    String emailLine;
    emailLine = dynamic_cast<std::ostringstream&>(std::ostringstream{} << message.rdbuf()).str();
    std::transform(emailLine.begin(), emailLine.end(), emailLine.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if(message.fail())
    {
        std::cerr << INVALID_INPUT;
        closeFiles(dataBase, message);
        exit(EXIT_FAILURE);
    }
    int totalScore = 0;
    for (auto &p : map)
    {
        totalScore += keyScore(emailLine, p);
    }
    scoreOutPut(threshold, totalScore);
    closeFiles(dataBase, message);
    return 0;
}
/**
 *
 * @param threshold
 * @param totalScore
 */
void scoreOutPut(int threshold, int totalScore)
{
    if(totalScore >= threshold)
    {
        std::cout << SPAM;
    }
    else
    {
        std::cout << NOT_SPAM;
    }
}
/**
 *
 * @param dataBase
 * @return
 */
int closeData(std::fstream &dataBase)
{
    std::cerr << INVALID_INPUT;
    dataBase.close();
    exit(EXIT_FAILURE);
}

/**
 *Responsible for closing the files
 * @param dataBase
 * @param message
 */
void closeFiles(std::fstream &dataBase, std::fstream &message)
{
    dataBase.close();
    message.close();
}

/**
 *Responsible for checking is a string represents a number
 * @param s
 * @return
 */
bool isNumber(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
    {
        ++it;
    }
    return !s.empty() && it == s.end();
}
/**
 *Calculates the key's score
 * @param text
 * @param p
 * @return
 */
int keyScore(String &text, pair& p)
{
    int occurrences = 0;
    String::size_type pos = 0;
    String key = p.first;
    int value = p.second;
    while((pos = text.find(key, pos)) != String::npos)
    {
        ++occurrences;
        pos += key.length();
    }
    return (occurrences*value);
}