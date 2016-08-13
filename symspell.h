#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <math.h>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <fstream>
#include <sparsehash/dense_hash_map>
#include <sparsehash/dense_hash_set>

#define set google::dense_hash_set
#define map google::dense_hash_map

#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define max(a, b)  (((a) > (b)) ? (a) : (b))

using namespace std;


class dictionaryItem {
public:
    vector<size_t> suggestions;
    size_t count = 0;
    
    dictionaryItem(size_t c)
    {
        count = c;
    }
    
    dictionaryItem()
    {
        count = 0;
    }
    ~dictionaryItem()
    {
        suggestions.clear();
    }
};

class dictionaryItemContainer
{
public:
    dictionaryItemContainer(void) {
        dictValue = NULL;
    }
    
    enum type { NONE, ITEM, INTEGER };
    type itemType;
    int intValue;
    dictionaryItem* dictValue;
};

class suggestItem
{
public:
    string term;
    int distance = 0;
    size_t count;
    
    bool operator== (const suggestItem & item) const
    {
        return term.compare(item.term) == 0;
    }
    
    size_t HastCode() {
        return hash<string>()(term);
    }
};

class SymSpell {
public:
    int verbose = 0;
    
    SymSpell()
    {
        setlocale(LC_ALL, "");
        dictionary.set_empty_key("");
    }
    
    void CreateDictionary(string corpus)
    {
        std::ifstream sr(corpus);
        
        if (!sr.good())
        {
            cout << "File not found: " << corpus;
            return;
        }
        
        cout << "Creating dictionary ..." << endl;
        
        long wordCount = 0;
        
        for (std::string line; std::getline(sr, line); ) {
            
            for(const string & key: parseWords(line))
            {
                if (CreateDictionaryEntry(key)) wordCount++;
            }
        }
        
        sr.close();
    }
    
    bool CreateDictionaryEntry(string key)
    {
        bool result = false;
        dictionaryItemContainer value;
        
        auto valueo = dictionary.find(key);
        if (valueo != dictionary.end())
        {
            value = valueo->second;
            
            if (valueo->second.itemType == dictionaryItemContainer::INTEGER)
            {
                value.itemType = dictionaryItemContainer::ITEM;
                value.dictValue = new dictionaryItem();
                value.dictValue->suggestions.push_back(valueo->second.intValue);
            }
            else
                value = valueo->second;
            
            if (value.dictValue->count < INT_MAX)
                ++(value.dictValue->count);
        }
        else if (wordlist.size() < INT_MAX)
        {
            value.itemType = dictionaryItemContainer::ITEM;
            value.dictValue = new dictionaryItem();
            ++(value.dictValue->count);
            string mapKey = key;
            dictionary.insert(pair<string, dictionaryItemContainer>(mapKey, value));
            
            if (key.size() > maxlength)
                maxlength = key.size();
        }
        
        if (value.dictValue->count == 1)
        {
            wordlist.push_back(key);
            size_t keyint = wordlist.size() - 1;
            
            result = true;
            
            auto deleted = set<string>();
            deleted.set_empty_key("");
            
            Edits(key, 0, deleted);
            
            for (string del : deleted)
            {
                auto value2 = dictionary.find(del);
                if (value2 != dictionary.end())
                {
                    if (value2->second.itemType == dictionaryItemContainer::INTEGER)
                    {
                        value2->second.itemType = dictionaryItemContainer::ITEM;
                        value2->second.dictValue = new dictionaryItem();
                        value2->second.dictValue->suggestions.push_back(value2->second.intValue);
                        dictionary[del].dictValue = value2->second.dictValue;
                        
                        if (std::find(value2->second.dictValue->suggestions.begin(), value2->second.dictValue->suggestions.end(), keyint) == value2->second.dictValue->suggestions.end())
                            AddLowestDistance(value2->second.dictValue, key, keyint, del);
                    }
                    else if (std::find(value2->second.dictValue->suggestions.begin(), value2->second.dictValue->suggestions.end(), keyint) == value2->second.dictValue->suggestions.end())
                        AddLowestDistance(value2->second.dictValue, key, keyint, del);
                }
                else
                {
                    dictionaryItemContainer tmp;
                    tmp.itemType = dictionaryItemContainer::INTEGER;
                    tmp.intValue = keyint;
                    
                    string mapKey = del;
                    dictionary.insert(pair<string, dictionaryItemContainer>(mapKey, tmp));
                }
                
            }
        }
        return result;
    }
    
    vector<suggestItem> Correct(string input)
    {
        vector<suggestItem> suggestions;
        suggestions = Lookup(input, editDistanceMax);
        return suggestions;
        
    }
    
    
private:
    size_t maxlength = 0;
    int editDistanceMax = 2;
    map<string, dictionaryItemContainer> dictionary;
    vector<string> wordlist;
    
    vector<string> parseWords(string text)
    {
        vector<string> returnData;
        
        std::transform(text.begin(), text.end(), text.begin(), ::tolower);
        std::regex word_regex("[^\\W\\d_]+");
        auto words_begin = std::sregex_iterator(text.begin(), text.end(), word_regex);
        auto words_end = std::sregex_iterator();
        
        for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
            std::smatch match = *i;
            returnData.push_back(match.str());
        }
        
        return returnData;
    }
    
    void AddLowestDistance(dictionaryItem * item, string suggestion, int suggestionint, string del)
    {
        if ((verbose < 2) && (item->suggestions.size() > 0) && (wordlist[item->suggestions[0]].size() - del.size() > suggestion.size() - del.size()))
            item->suggestions.clear();
        
        if ((verbose == 2) || (item->suggestions.size() == 0) || (wordlist[item->suggestions[0]].size() - del.size() >= suggestion.size() - del.size()))
            item->suggestions.push_back(suggestionint);
    }
    
    
    set<string> Edits(string word, int editDistance, set<string> & deletes)
    {
        ++editDistance;
        if (word.size() > 1)
        {
            for (size_t i = 0; i < word.size(); i++)
            {
                string wordClone = word;
                string del = wordClone.erase(i, 1);
                if (!deletes.count(del))
                {
                    deletes.insert(del);
                    
                    if (editDistance < editDistanceMax)
                        Edits(del, editDistance, deletes);
                }
            }
        }
        
        return deletes;
    }
    
    vector<suggestItem> Lookup(string input, size_t editDistanceMax)
    {
        if (input.size() - editDistanceMax > maxlength)
            return vector<suggestItem>();
        
        vector<string> candidates;
        set<string> hashset1;
        hashset1.set_empty_key("");
        
        vector<suggestItem> suggestions;
        set<string> hashset2;
        hashset2.set_empty_key("");
        
        //object valueo;
        
        candidates.push_back(input);
        
        while (candidates.size() > 0)
        {
            string candidate = candidates[0];
            candidates.erase(candidates.begin());
            
            if ((verbose < 2) && (suggestions.size() > 0) && (input.size() - candidate.size() > suggestions[0].distance))
                goto sort;
            
            auto valueo = dictionary.find(candidate);
            
            //read candidate entry from dictionary
            if (valueo != dictionary.end())
            {
                if (valueo->second.itemType == dictionaryItemContainer::INTEGER)
                {
                    valueo->second.itemType = dictionaryItemContainer::ITEM;
                    valueo->second.dictValue = new dictionaryItem();
                    valueo->second.dictValue->suggestions.push_back(valueo->second.intValue);
                }
                
                
                if ((valueo->second.dictValue->count > 0) && hashset2.insert(candidate).second)
                {
                    //add correct dictionary term term to suggestion list
                    suggestItem si;
                    si.term = candidate;
                    si.count = valueo->second.dictValue->count;
                    si.distance = input.size() - candidate.size();
                    suggestions.push_back(si);
                    //early termination
                    if ((verbose < 2) && (input.size() - candidate.size() == 0))
                        goto sort;
                }
                
                for (int suggestionint : valueo->second.dictValue->suggestions)
                {
                    //save some time
                    //skipping double items early: different deletes of the input term can lead to the same suggestion
                    //index2word
                    string suggestion = wordlist[suggestionint];
                    if (hashset2.insert(suggestion).second)
                    {
                        int distance = 0;
                        if (suggestion != input)
                        {
                            if (suggestion.size() == candidate.size()) distance = input.size() - candidate.size();
                            else if (input.size() == candidate.size()) distance = suggestion.size() - candidate.size();
                            else
                            {
                                int ii = 0;
                                int jj = 0;
                                while ((ii < suggestion.size()) && (ii < input.size()) && (suggestion[ii] == input[ii]))
                                    ii++;
                                
                                while ((jj < suggestion.size() - ii) && (jj < input.size() - ii) && (suggestion[suggestion.size() - jj - 1] == input[input.size() - jj - 1]))
                                    jj++;
                                if ((ii > 0) || (jj > 0))
                                {
                                    distance = DamerauLevenshteinDistance(suggestion.substr(ii, suggestion.size() - ii - jj), input.substr(ii, input.size() - ii - jj));
                                }
                                else
                                    distance = DamerauLevenshteinDistance(suggestion, input);
                                
                            }
                        }
                        
                        if ((verbose < 2) && (suggestions.size() > 0) && (suggestions[0].distance > distance))
                            suggestions.clear();
                        
                        if ((verbose < 2) && (suggestions.size() > 0) && (distance > suggestions[0].distance))
                            continue;
                        
                        if (distance <= editDistanceMax)
                        {
                            auto value2 = dictionary.find(suggestion);
                            
                            if (value2 != dictionary.end())
                            {
                                suggestItem si;
                                si.term = suggestion;
                                si.count = value2->second.dictValue->count;
                                si.distance = distance;
                                suggestions.push_back(si);
                            }
                        }
                    }
                }
            }
            
            
            if (input.size() - candidate.size() < editDistanceMax)
            {
                if ((verbose < 2) && (suggestions.size() > 0) && (input.size() - candidate.size() >= suggestions[0].distance))
                    continue;
                
                for (int i = 0; i < candidate.size(); i++)
                {
                    string wordClone = candidate;
                    string del = wordClone.erase(i, 1);
                    if (hashset1.insert(del).second)
                        candidates.push_back(del);
                }
            }
        }//end while
        
        //sort by ascending edit distance, then by descending word frequency
    sort:
        if (verbose < 2)
            sort(suggestions.begin(), suggestions.end(), Xgreater1());
        else
            sort(suggestions.begin(), suggestions.end(), Xgreater2());
        
        if ((verbose == 0) && (suggestions.size() > 1))
            return vector<suggestItem>(suggestions.begin(), suggestions.begin() + 1);
        else
            return suggestions;
    }
    
    struct Xgreater1
    {
        bool operator()(const suggestItem& lx, const suggestItem& rx) const {
            return lx.count > rx.count;
        }
    };
    
    struct Xgreater2
    {
        bool operator()(const suggestItem& lx, const suggestItem& rx) const {
            return 2 * (lx.distance - rx.distance) > (lx.count - rx.count);
        }
    };
    
    
    static size_t DamerauLevenshteinDistance(const std::string &s1, const std::string &s2)
    {
        const size_t m(s1.size());
        const size_t n(s2.size());
        
        if (m == 0) return n;
        if (n == 0) return m;
        
        size_t *costs = new size_t[n + 1];
        
        for (size_t k = 0; k <= n; k++) costs[k] = k;
        
        size_t i = 0;
        for (std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i)
        {
            costs[0] = i + 1;
            size_t corner = i;
            
            size_t j = 0;
            for (std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j)
            {
                size_t upper = costs[j + 1];
                if (*it1 == *it2)
                {
                    costs[j + 1] = corner;
                }
                else
                {
                    size_t t(upper < corner ? upper : corner);
                    costs[j + 1] = (costs[j] < t ? costs[j] : t) + 1;
                }
                
                corner = upper;
            }
        }
        
        size_t result = costs[n];
        delete[] costs;
        
        return result;
    }
};
