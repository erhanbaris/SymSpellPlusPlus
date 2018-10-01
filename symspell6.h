/*
 *
        NOT FINISHED YET
        NOT TESTED YET

        DO NOT USE THIS HEADER FILE UNTIL THIS MESSAGE REMOVED.


        Sample Code:
        symspell::SymSpell symSpell;
        symSpell.CreateDictionaryEntry("erhan", 1);
        symSpell.CreateDictionaryEntry("orhan", 2);
        symSpell.CreateDictionaryEntry("ayhan", 3);
        vector<symspell::SuggestItem*> items;
        symSpell.Lookup("ozhan", symspell::Verbosity::Top, items);
        std::cout << "Count : " << items.size() << std::endl;
        if (items.size() > 0)
        {
                        auto itemsEnd = items.end();

                        for(auto it = items.begin(); it != itemsEnd; ++it)
                                        std::cout << "Item : '" << (*it)->term << "' Count : " << std::to_string((*it)->count) << " Distance : " << std::to_string((*it)->distance) << std::endl;
        }

        Todo List:
        1- Change std::string to char*
        2- Change levenshtein functions
        3- Improve performance
 *
 */


#ifndef SYMSPELL6_H
#define SYMSPELL6_H

#define USE_GOOGLE_HASH_MAP


#include <stdint.h>
#include <vector>
#include <functional>
#include <string>
#include <cstring>
#include <exception>
#include <limits>
#include <stdio.h>
#include <algorithm>
#include <queue>
#include <mutex>
#include <tuple>
#include <iostream>
#include <fstream>
#include <sstream>

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS 1

#ifdef _MSC_VER
#   include <windows/port.h>
    //typedef __int8 int8_t;
typedef unsigned __int8 u_int8_t;

typedef __int32 int32_t;
typedef unsigned __int32 u_int32_t;

typedef __int64 int64_t;
typedef unsigned __int64 u_int64_t;

char *strndup(const char *s1, size_t n)
{
    char *copy = (char*)malloc(n + 1);
    memcpy(copy, s1, n);
    copy[n] = 0;
    return copy;
};
#else
#   define _strdup strdup
#endif

#ifdef USE_GOOGLE_HASH_MAP
#   include <sparsehash/dense_hash_map>
#   include <sparsehash/dense_hash_set>

#   define CUSTOM_MAP dense_hash_map
#   define CUSTOM_SET dense_hash_set
using google::dense_hash_map;
using google::dense_hash_set;
#else
#   define CUSTOM_MAP unordered_map
#   define CUSTOM_SET unordered_set
#   include <unordered_map>
#   include <unordered_set>
#endif

using namespace std;

namespace symspell {
#define defaultMaxEditDistance 2
#define defaultPrefixLength 7
#define defaultCountThreshold 1
#define defaultInitialCapacity 16
#define defaultCompactLevel 5
#define mini(a, b, c) (min(a, min(b, c)))

    namespace {
        int32_t findCharLocation(const char * text, char ch)
        {
            const char* finded = strchr(text, ch);
            if (finded == nullptr)
                return -1;

            return finded - text + 1;
        }
    }

    struct comp_c_string {
            bool operator()(const char *s1, const char *s2) const {
                return (s1 == s2) || (s1 && s2 && strcmp(s1, s2) == 0);
            }
    };

    struct hash_c_string {
            void hash_combine(size_t& seed, const char& v)
            {
                seed ^= v + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }

            std::size_t operator() (const char* p) const
            {
                size_t hash = 0;
                for (; *p; ++p)
                    hash ^= *p + 0x9e3779b9 + (hash << 6) + (hash >> 2);
                return hash;
            }
    };

    /*
 * Copied from https://github.com/PierreBoyeau/levenshtein_distance
 * ########## BEGIN ##########
 */

    int levenshtein_dist(char const* word1, char const* word2) {
        ///
        ///  Please use lower-case strings
        /// word1 : first word
        /// word2 : second word
        /// getPath : bool. If True, sequence of operations to do to go from
        ///           word1 to word2
        ///
        int size1 = strlen(word1) + 1, size2 = strlen(word2) + 1;
        int suppr_dist, insert_dist, subs_dist;
        int* dist = new int[(size1)*size2];

        for (int i = 0; i < size1; ++i)
            dist[size2*i] = i;
        for (int j = 0; j < size2; ++j)
            dist[j] = j;
        for (int i = 1; i < size1; ++i) {
            for (int j = 1; j < size2; ++j) {
                suppr_dist = dist[size2*(i - 1) + j] + 1;
                insert_dist = dist[size2*i + j - 1] + 1;
                subs_dist = dist[size2*(i - 1) + j - 1];
                if (word1[i - 1] != word2[j - 1]) {  // word indexes are implemented differently.
                    subs_dist += 1;
                }
                dist[size2*i + j] = mini(suppr_dist, insert_dist, subs_dist);
            }
        }

        // --------------------------------------------------------
        int res = dist[size1 * size2 - 1];
        delete[] dist;
        return(res);
    }

    int dl_dist(char const* word1, char const* word2) {
        /// Damerau-Levenshtein distance
        ///  Please use lower-case strings
        /// word1 : first word
        /// word2 : second word
        ///
        int size1 = strlen(word1) + 1, size2 = strlen(word2) + 1;
        int suppr_dist, insert_dist, subs_dist, val;
        int* dist = new int[size1*size2];

        for (int i = 0; i < size1; ++i)
            dist[size2*i] = i;
        for (int j = 0; j < size2; ++j)
            dist[j] = j;
        for (int i = 1; i < size1; ++i) {
            for (int j = 1; j < size2; ++j) {
                suppr_dist = dist[size2*(i - 1) + j] + 1;
                insert_dist = dist[size2*i + j - 1] + 1;
                subs_dist = dist[size2*(i - 1) + j - 1];
                if (word1[i - 1] != word2[j - 1])  // word indexes are implemented differently.
                    subs_dist += 1;
                val = mini(suppr_dist, insert_dist, subs_dist);
                if (((i >= 2) && (j >= 2)) && ((word1[i - 1] == word2[j - 2]) && (word1[i - 2] == word2[j - 1])))
                    val = min(dist[size2*(i - 2) + j - 2] + 1, val);
                dist[size2*i + j] = val;
            }
        }

        int res = dist[size1*size2 - 1];
        delete[] dist;
        return(res);
    }

    /*
         * ########## END ##########
         */

    class SuggestionStage;
    template<typename T> class ChunkArray;
    enum class Verbosity
    {
        /// <summary>Top suggestion with the highest term frequency of the suggestions of smallest edit distance found.</summary>
        Top,
        /// <summary>All suggestions of smallest edit distance found, suggestions ordered by term frequency.</summary>
        Closest,
        /// <summary>All suggestions within maxEditDistance, suggestions ordered by edit distance
        /// , then by term frequency (slower, no early termination).</summary>
        All
    };

    class EditDistance {
        public:
            /// <summary>Wrapper for third party edit distance algorithms.</summary>

            /// <summary>Supported edit distance algorithms.</summary>
            enum class DistanceAlgorithm {
                /// <summary>Levenshtein algorithm.</summary>
                Levenshtein,
                /// <summary>Damerau optimal string alignment algorithm.</summary>
                DamerauOSA
            };

            EditDistance(DistanceAlgorithm algorithm) {
                this->algorithm = algorithm;
                switch (algorithm) {
                    case DistanceAlgorithm::DamerauOSA: this->distanceComparer = dl_dist; break;
                    case DistanceAlgorithm::Levenshtein: this->distanceComparer = levenshtein_dist; break;
                    default: throw std::invalid_argument("Unknown distance algorithm.");
                }
            }

            int Compare(char const* string1, char const* string2, int maxDistance) {
                return this->distanceComparer(string1, string2); // todo: max distance
            }

        private:
            DistanceAlgorithm algorithm;
            int(*distanceComparer)(char const*, char const*);
    };

    class SuggestItem
    {
        public:
            /// <summary>The suggested correctly spelled word.</summary>
            const char* term;
            /// <summary>Edit distance between searched for word and suggestion.</summary>
            u_int8_t distance = 0;
            /// <summary>Frequency of suggestion in the dictionary (a measure of how common the word is).</summary>
            int64_t count = 0;

            SuggestItem() { }
            SuggestItem(const symspell::SuggestItem & p)
            {
                this->count = p.count;
                this->distance = p.distance;
                this->term = p.term;
            }

            SuggestItem(const char* term, int32_t distance, int64_t count)
            {
                this->term = term;
                this->distance = distance;
                this->count = count;
            }

            ~SuggestItem()
            {
                delete[] term;
            }

            bool CompareTo(SuggestItem const& other)
            {
                // order by distance ascending, then by frequency count descending
                if (this->distance == other.distance)
                {
                    if (other.count == this->count)
                        return false;
                    else if (other.count > this->count)
                        return true;
                    return false;
                }

                if (other.distance > this->distance)
                    return false;
                return true;
            }

            bool operator == (const SuggestItem &ref) const
            {
                return strcmp(this->term, ref.term) == 0;
            }

            std::size_t GetHashCode()
            {
                return std::hash<const char*>{}(term);
            }

            SuggestItem& ShallowCopy()
            {
                SuggestItem item;
                item.count = this->count;
                item.distance = this->distance;
                item.term = this->term;

                return item;
            }
    };

    class WordSegmentationItem
    {
        public:
            const char* segmentedString{nullptr};
            const char* correctedString{nullptr};
            u_int8_t distanceSum = 0;
            double probabilityLogSum = 0;

            WordSegmentationItem() { }
            WordSegmentationItem(const symspell::WordSegmentationItem & p)
            {
                this->segmentedString = p.segmentedString;
                this->correctedString = p.correctedString;
                this->distanceSum = p.distanceSum;
                this->probabilityLogSum = p.probabilityLogSum;
            }

            WordSegmentationItem& operator=(const WordSegmentationItem&) { return *this ; }
            WordSegmentationItem& operator=(WordSegmentationItem&&) { return *this ; }

            void set(const char* pSegmentedString, const char* pCorrectedString,  u_int8_t pDistanceSum, double pProbabilityLogSum)
            {
                this->segmentedString = pSegmentedString;
                this->correctedString = pCorrectedString;
                this->distanceSum = pDistanceSum;
                this->probabilityLogSum = pProbabilityLogSum;
            }

            ~WordSegmentationItem()
            {
                delete[] segmentedString;
                delete[] correctedString;
            }
    };

    template<typename T>
    class ChunkArray
    {
        public:
            vector<vector<T>> Values; //todo: use pointer array
            size_t Count;

            ChunkArray()
            {
                Count = 0;
            }

            void Reserve(size_t initialCapacity)
            {
                size_t chunks = (initialCapacity + ChunkSize - 1) / ChunkSize;
                Values.resize(chunks);
                for (size_t i = 0; i < chunks; ++i)
                {
                    Values[i].resize(ChunkSize);
                }
            }

            size_t Add(T & value)
            {
                if (Count == Capacity())
                {
                    Values.push_back(vector<T>());
                    Values[Values.size()-1].resize(ChunkSize);
                }

                int row = Row(Count);
                int col = Col(Count);

                Values[row][col] = value;
                return Count++;
            }

            void Clear()
            {
                Count = 0;
            }

            T& at(size_t index)
            {
                return Values[Row(index)][Col(index)];
            }

            void set(size_t index, T &value)
            {
                Values[Row(index)][Col(index)] = value;
            }

        private:
            const int32_t ChunkSize = 4096; //this must be a power of 2, otherwise can't optimize Row and Col functions
            const int32_t DivShift = 12; // number of bits to shift right to do division by ChunkSize (the bit position of ChunkSize)
            int Row(uint32_t index) { return index >> DivShift; } // same as index / ChunkSize
            int32_t Col(uint32_t index) { return index & (ChunkSize - 1); } //same as index % ChunkSize
            int32_t Capacity() { return Values.size() * ChunkSize; }
    };


    class SuggestionStage
    {
        public:
            class Node;
            class Entry;

            CUSTOM_MAP<size_t, Entry*> Deletes;
            CUSTOM_MAP<size_t, Entry*>::iterator DeletesEnd;

            ChunkArray<Node> Nodes;
            SuggestionStage(size_t initialCapacity)
            {
#ifdef USE_GOOGLE_HASH_MAP
                Deletes.set_empty_key(0);
                Deletes.resize(initialCapacity);
#else
                Deletes.reserve(initialCapacity);
#endif
                Nodes.Reserve(initialCapacity * 2);
            }
            size_t DeleteCount() { return Deletes.size(); }
            size_t NodeCount() { return Nodes.Count; }
            void Clear()
            {
                Deletes.clear();
                Nodes.Clear();

                DeletesEnd = Deletes.end();
            }

            void Add(size_t deleteHash, const char* suggestion)
            {
                auto deletesFinded = Deletes.find(deleteHash);
                Entry* entry = nullptr;
                if (deletesFinded == DeletesEnd) {
                    entry = new Entry;
                    entry->count = 0;
                    entry->first = -1;
                }
                else
                    entry = deletesFinded->second;

                int64_t next = entry->first;
                ++entry->count;
                entry->first = Nodes.Count;
                Deletes[deleteHash] = entry;
                Node item;
                item.next = next;
                item.suggestion = suggestion;
                Nodes.Add(item);
            }

            void CommitTo(CUSTOM_MAP<size_t, vector<const char*>> & permanentDeletes)
            {
                auto permanentDeletesEnd = permanentDeletes.end();
                for (auto it = Deletes.begin(); it != DeletesEnd; ++it)
                {
                    auto permanentDeletesFinded = permanentDeletes.find(it->first);
                    vector<const char*>* suggestions = nullptr;
                    size_t i;
                    if (permanentDeletesFinded != permanentDeletesEnd)
                    {
                        suggestions = &permanentDeletesFinded->second;
                        i = suggestions->size();
                        vector<const char*> newSuggestions;
                        newSuggestions.reserve(suggestions->size() + it->second->count);

                        std::copy(suggestions->begin(), suggestions->end(), back_inserter(newSuggestions));
                        permanentDeletes[it->first] = newSuggestions;
                    }
                    else
                    {
                        i = 0;
                        suggestions = new vector<const char*>;
                        int32_t count = it->second->count;
                        suggestions->reserve(count);
                        permanentDeletes[it->first] = *suggestions;
                    }

                    int next = it->second->first;
                    while (next >= 0)
                    {
                        auto node = Nodes.at(next);
                        (*suggestions)[i] = node.suggestion;
                        next = node.next;
                        ++i;
                    }
                }
            }

        public:
            class Node
            {
                public:
                    const char* suggestion;
                    int64_t next;
            };

            class Entry
            {
                public:
                    int64_t count;
                    int64_t first;
            };
    };


    class SymSpell {
        public:
            SymSpell(int32_t initialCapacity = defaultInitialCapacity, int32_t maxDictionaryEditDistance = defaultMaxEditDistance, int32_t prefixLength = defaultPrefixLength, int32_t countThreshold = defaultCountThreshold, int32_t compactLevel = defaultCompactLevel)
            {
                if (initialCapacity < 0) throw std::invalid_argument("initialCapacity");
                if (maxDictionaryEditDistance < 0) throw std::invalid_argument("maxDictionaryEditDistance");
                if (prefixLength < 1 || prefixLength <= maxDictionaryEditDistance) throw std::invalid_argument("prefixLength");
                if (countThreshold < 0) throw std::invalid_argument("countThreshold");
                if (compactLevel > 16) throw std::invalid_argument("compactLevel");
#ifdef USE_GOOGLE_HASH_MAP
                this->words.set_empty_key(nullptr);
                this->deletes.set_empty_key(0);
                this->hashset1.set_empty_key(0);
                this->hashset2.set_empty_key(0);
                this->belowThresholdWords.set_empty_key(nullptr);
                this->edits.set_empty_key(0);
#endif
#ifdef USE_GOOGLE_HASH_MAP
                this->words.resize(initialCapacity);
                this->deletes.resize(initialCapacity);
#else
                this->words.reserve(initialCapacity);
                this->deletes.reserve(initialCapacity);
#endif
                this->initialCapacity = initialCapacity;
                this->distanceComparer = new EditDistance(this->distanceAlgorithm);

                this->maxDictionaryEditDistance = maxDictionaryEditDistance;
                this->prefixLength = prefixLength;
                this->countThreshold = countThreshold;
                if (compactLevel > 16) compactLevel = 16;
                this->compactMask = ((std::numeric_limits<uint32_t>::max)() >> (3 + compactLevel)) << 2;
                this->deletesEnd = this->deletes.end();
                this->wordsEnd = this->words.end();
                this->belowThresholdWordsEnd = this->belowThresholdWords.end();
                this->candidates.reserve(32);
                //this->maxDictionaryWordLength = 0;
            }

            ~SymSpell()
            {
                vector<const char*>::iterator vecEnd;
                auto deletesEnd = this->deletes.end();
                for(auto it = this->deletes.begin(); it != deletesEnd; ++it)
                {
                    vecEnd = it->second.end();
                    for(auto vecIt = it->second.begin(); vecIt != vecEnd; ++vecIt)
                    {
                        delete[] *vecIt;
                    }
                }

                delete this->distanceComparer;
            }

            bool CreateDictionaryEntry(const char * key, int64_t count, SuggestionStage * staging = nullptr)
            {
                int keyLen = strlen(key);
                if (count <= 0)
                {
                    if (this->countThreshold > 0) return false; // no point doing anything if count is zero, as it can't change anything
                    count = 0;
                }

                int64_t countPrevious = -1;
                auto belowThresholdWordsFinded = belowThresholdWords.find(key);
                auto wordsFinded = words.find(key);

                // look first in below threshold words, update count, and allow promotion to correct spelling word if count reaches threshold
                // threshold must be >1 for there to be the possibility of low threshold words
                if (countThreshold > 1 && belowThresholdWordsFinded != belowThresholdWordsEnd)
                {
                    countPrevious = belowThresholdWordsFinded->second;
                    // calculate new count for below threshold word
                    count = ((std::numeric_limits<int64_t>::max)() - countPrevious > count) ? countPrevious + count : (std::numeric_limits<int64_t>::max)();
                    // has reached threshold - remove from below threshold collection (it will be added to correct words below)
                    if (count >= countThreshold)
                    {
                        belowThresholdWords.erase(key);
                        belowThresholdWordsEnd = belowThresholdWords.end();
                    }
                    else
                    {
                        belowThresholdWords[key] = count;
                        belowThresholdWordsEnd = belowThresholdWords.end();
                        return false;
                    }
                }
                else if (wordsFinded != wordsEnd)
                {
                    countPrevious = wordsFinded->second;
                    count = ((std::numeric_limits<int64_t>::max)() - countPrevious > count) ? countPrevious + count : (std::numeric_limits<int64_t>::max)();
                    words[key] = count;
                    return false;
                }
                else if (count < CountThreshold())
                {
                    belowThresholdWords[key] = count;
                    belowThresholdWordsEnd = belowThresholdWords.end();
                    return false;
                }

                words[key] = count;

                if (keyLen > maxDictionaryWordLength)
                    maxDictionaryWordLength = keyLen;

                EditsPrefix(key, edits);

                if (staging != nullptr)
                {
                    auto editsEnd = edits.end();
                    for (auto it = edits.begin(); it != editsEnd; ++it)
                    {
                        staging->Add(*it, _strdup(key));
                    }
                }
                else
                {
                    auto editsEnd = edits.end();
                    for (auto it = edits.begin(); it != editsEnd; ++it)
                    {
                        size_t deleteHash = *it;
                        auto deletesFinded = deletes.find(deleteHash);
                        if (deletesFinded != deletesEnd)
                        {
                            char* tmp = new char[keyLen + 1];
                            std::memcpy(tmp, key, keyLen);
                            tmp[keyLen] = '\0';
                            
                            //delete[] deletes[deleteHash][deletesFinded->second.size() - 1];
                            deletes[deleteHash].push_back(tmp);
                            deletesEnd = deletes.end();
                        }
                        else
                        {
                            char* tmp = new char[keyLen + 1];
                            std::memcpy(tmp, key, keyLen);
                            tmp[keyLen] = '\0';
                            
                            deletes[deleteHash] = vector<const char*>();
                            //deletes[deleteHash].resize(1);
                            deletes[deleteHash].push_back(tmp);
                            deletesEnd = deletes.end();
                        }
                    }
                }

                edits.clear();
                return true;
            }

            void EditsPrefix(const char* key, CUSTOM_SET<size_t>& hashSet)
            {
                size_t len = strlen(key);
                char* tmp = nullptr;
                /*if (len <= maxDictionaryEditDistance) //todo fix
                    hashSet.insert("");*/

                if (len > prefixLength)
                {
                    tmp = new char[prefixLength + 1];
                    std::memcpy(tmp, key, prefixLength);
                    tmp[prefixLength] = '\0';
                }
                else
                {
                    tmp = new char[len + 1];
                    std::memcpy(tmp, key, len);
                    tmp[len] = '\0';
                }

                hashSet.insert(stringHash(tmp));
                Edits(tmp, 0, hashSet);
            }

            void Edits(const char * word, int32_t editDistance, CUSTOM_SET<size_t> & deleteWords)
            {
                auto deleteWordsEnd = deleteWords.end();
                ++editDistance;
                size_t wordLen = strlen(word);
                if (wordLen > 1)
                {
                    for (size_t i = 0; i < wordLen; ++i)
                    {
                        char* tmp = new char[wordLen];
                        std::memcpy(tmp, word, i);
                        std::memcpy(tmp + i, word + i + 1, wordLen - 1 - i);
                        tmp[wordLen - 1] = '\0';

                        if (deleteWordsEnd == deleteWords.find(stringHash(tmp)))
                        {
                            deleteWords.insert(stringHash(tmp));
                            deleteWordsEnd = deleteWords.end();

                            //recursion, if maximum edit distance not yet reached
                            if (editDistance < maxDictionaryEditDistance && (wordLen-1) > 1)
                                Edits(tmp, editDistance, deleteWords);
                        }
                        else {
                            delete[] tmp;
                        }
                    }
                }
            }

            void PurgeBelowThresholdWords()
            {
                belowThresholdWords.clear();
                belowThresholdWordsEnd = belowThresholdWords.end();
            }

            void CommitStaged(SuggestionStage staging)
            {
                staging.CommitTo(deletes);
            }

            void Lookup(const char * input, Verbosity verbosity, vector<std::unique_ptr<symspell::SuggestItem>> & items)
            {
                this->Lookup(input, verbosity, this->maxDictionaryEditDistance, false, items);
            }

            void Lookup(const char * input, Verbosity verbosity, int32_t maxEditDistance, vector<std::unique_ptr<symspell::SuggestItem>> & items)
            {
                this->Lookup(input, verbosity, maxEditDistance, false, items);
            }

            void Lookup(const char * input, Verbosity verbosity, int32_t maxEditDistance, bool includeUnknown, vector<std::unique_ptr<symspell::SuggestItem>> & suggestions)
            {
                mtx.lock();
                suggestions.clear();
                edits.clear();

                //verbosity=Top: the suggestion with the highest term frequency of the suggestions of smallest edit distance found
                //verbosity=Closest: all suggestions of smallest edit distance found, the suggestions are ordered by term frequency
                //verbosity=All: all suggestions <= maxEditDistance, the suggestions are ordered by edit distance, then by term frequency (slower, no early termination)

                // maxEditDistance used in Lookup can't be bigger than the maxDictionaryEditDistance
                // used to construct the underlying dictionary structure.
                if (maxEditDistance > MaxDictionaryEditDistance())  throw std::invalid_argument("maxEditDistance");
                long suggestionCount = 0;
                size_t suggestionsLen = 0;
                auto wordsFinded = words.find(input);
                int inputLen = strlen(input);
                // early exit - word is too big to possibly match any words
                if (inputLen - maxEditDistance > maxDictionaryWordLength)
                {
                    if (includeUnknown && (suggestionsLen == 0))
                    {
                        std::unique_ptr<SuggestItem> unq(new SuggestItem(_strdup(input), maxEditDistance + 1, 0));
                        suggestions.push_back(std::move(unq));
                    }

                    mtx.unlock();
                    return;
                }

                // quick look for exact match

                if (wordsFinded != wordsEnd)
                {
                    suggestionCount = wordsFinded->second;

                    {
                        std::unique_ptr<SuggestItem> unq(new SuggestItem(_strdup(input), 0, suggestionCount));
                        suggestions.push_back(std::move(unq));
                    }

                    ++suggestionsLen;
                    // early exit - return exact match, unless caller wants all matches
                    if (verbosity != Verbosity::All)
                    {
                        if (includeUnknown && (suggestionsLen == 0))
                        {
                            std::unique_ptr<SuggestItem> unq(new SuggestItem(_strdup(input), maxEditDistance + 1, 0));
                            suggestions.push_back(std::move(unq));
                            ++suggestionsLen;
                        }

                        mtx.unlock();
                        return;
                    }
                }

                //early termination, if we only want to check if word in dictionary or get its frequency e.g. for word segmentation
                if (maxEditDistance == 0)
                {
                    if (includeUnknown && (suggestionsLen == 0))
                    {
                        std::unique_ptr<SuggestItem> unq(new SuggestItem(_strdup(input), maxEditDistance + 1, 0));
                        suggestions.push_back(std::move(unq));

                        ++suggestionsLen;
                    }

                    mtx.unlock();
                    return;
                }


                auto hashset1End = hashset1.end();

                hashset2.insert(stringHash(input));
                hashset2End = hashset2.end();

                int maxEditDistance2 = maxEditDistance;
                int candidatePointer = 0;

                //add original prefix
                int inputPrefixLen = inputLen;
                if (inputPrefixLen > prefixLength)
                {
                    inputPrefixLen = prefixLength;
                    candidates.push_back(strndup(input, inputPrefixLen));
                }
                else
                {
                    candidates.push_back(_strdup(input));
                }

                size_t candidatesLen = 1; // candidates.size();
                while (candidatePointer < candidatesLen)
                {
                    const char* candidate = candidates[candidatePointer++];
                    int candidateLen = strlen(candidate);
                    int lengthDiff = inputPrefixLen - candidateLen;

                    //save some time - early termination
                    //if canddate distance is already higher than suggestion distance, than there are no better suggestions to be expected
                    if (lengthDiff > maxEditDistance2)
                    {
                        // skip to next candidate if Verbosity.All, look no further if Verbosity.Top or Closest
                        // (candidates are ordered by delete distance, so none are closer than current)
                        if (verbosity == Verbosity::All) continue;
                        break;
                    }

                    auto deletesFinded = deletes.find(stringHash(candidate));
                    vector<const char*>* dictSuggestions = nullptr;

                    //read candidate entry from dictionary
                    if (deletesFinded != deletesEnd)
                    {
                        dictSuggestions = &deletesFinded->second;
                        size_t dictSuggestionsLen = dictSuggestions->size();
                        //iterate through suggestions (to other correct dictionary items) of delete item and add them to suggestion list
                        for (int i = 0; i < dictSuggestionsLen; ++i)
                        {
                            const char* suggestion = dictSuggestions->at(i);
                            int suggestionLen = strlen(suggestion);
                            if (strcmp(suggestion, input) == 0) continue;
                            if ((abs(suggestionLen - inputLen) > maxEditDistance2) // input and sugg lengths diff > allowed/current best distance
                                    || (suggestionLen < candidateLen) // sugg must be for a different delete string, in same bin only because of hash collision
                                    || (suggestionLen == candidateLen && strcmp(suggestion, candidate) != 0)) // if sugg len = delete len, then it either equals delete or is in same bin only because of hash collision
                                continue;
                            auto suggPrefixLen = min(suggestionLen, prefixLength);
                            if (suggPrefixLen > inputPrefixLen && (suggPrefixLen - candidateLen) > maxEditDistance2) continue;

                            //True Damerau-Levenshtein Edit Distance: adjust distance, if both distances>0
                            //We allow simultaneous edits (deletes) of maxEditDistance on on both the dictionary and the input term.
                            //For replaces and adjacent transposes the resulting edit distance stays <= maxEditDistance.
                            //For inserts and deletes the resulting edit distance might exceed maxEditDistance.
                            //To prevent suggestions of a higher edit distance, we need to calculate the resulting edit distance, if there are simultaneous edits on both sides.
                            //Example: (bank==bnak and bank==bink, but bank!=kanb and bank!=xban and bank!=baxn for maxEditDistance=1)
                            //Two deletes on each side of a pair makes them all equal, but the first two pairs have edit distance=1, the others edit distance=2.
                            int distance = 0;
                            int _min = 0;
                            if (candidateLen == 0)
                            {
                                //suggestions which have no common chars with input (inputLen<=maxEditDistance && suggestionLen<=maxEditDistance)
                                distance = max(inputLen, suggestionLen);
                                if (distance > maxEditDistance2)
                                    continue;

                                auto hashset2Finded = hashset2.find(stringHash(suggestion));
                                if (hashset2End == hashset2Finded)
                                {
                                    hashset2.insert(stringHash(suggestion));
                                    hashset2End = hashset2.end();
                                }
                                else
                                    continue;
                            }
                            else if (suggestionLen == 1)
                            {
                                if (findCharLocation(input, suggestion[0]) < 0) distance = inputLen; else distance = inputLen - 1;
                                distance = max(inputLen, suggestionLen);
                                if (distance > maxEditDistance2)
                                    continue;

                                auto hashset2Finded = hashset2.find(stringHash(suggestion));
                                if (hashset2End == hashset2Finded)
                                {
                                    hashset2.insert(stringHash(suggestion));
                                    hashset2End = hashset2.end();
                                }
                                else
                                    continue;
                            }
                            else
                                if ((prefixLength - maxEditDistance == candidateLen)
                                    && (((_min = min(inputLen, suggestionLen) - prefixLength) > 1)
                                        && (std::strncmp(input, suggestion, max(inputLen + 1 - _min, suggestionLen + 1 - _min)) != 0) /*(input.substr(inputLen + 1 - _min) != suggestion.substr(suggestionLen + 1 - _min))*/)
                                    || ((_min > 0) && (input[inputLen - _min] != suggestion[suggestionLen - _min])
                                        && ((input[inputLen - _min - 1] != suggestion[suggestionLen - _min])
                                            || (input[inputLen - _min] != suggestion[suggestionLen - _min - 1]))))
                                {
                                    continue;
                                }
                                else
                                {
                                    if (verbosity != Verbosity::All && !DeleteInSuggestionPrefix(candidate, candidateLen, suggestion, suggestionLen)) continue;

                                    auto hashset2Finded = hashset2.find(stringHash(suggestion));
                                    if (hashset2End == hashset2Finded)
                                    {
                                        hashset2.insert(stringHash(suggestion));
                                        hashset2End = hashset2.end();
                                    }
                                    else
                                        continue;

                                    distance = distanceComparer->Compare(input, suggestion, maxEditDistance2);
                                    if (distance < 0) continue;
                                }

                            if (distance <= maxEditDistance2)
                            {
                                suggestionCount = words[suggestion];

                                std::unique_ptr<SuggestItem> si(new SuggestItem(_strdup(suggestion), distance, suggestionCount));

                                if (suggestionsLen > 0)
                                {
                                    switch (verbosity)
                                    {
                                        case Verbosity::Closest:
                                        {
                                            //we will calculate DamLev distance only to the smallest found distance so far
                                            if (distance < maxEditDistance2)
                                            {
                                                suggestions.clear();
                                                suggestionsLen = 0;
                                            }
                                            break;
                                        }
                                        case Verbosity::Top:
                                        {
                                            if (distance < maxEditDistance2 || suggestionCount > suggestions[0]->count)
                                            {
                                                maxEditDistance2 = distance;
                                                suggestions[0] = std::move(si);
                                            }
                                            continue;
                                        }
                                        case Verbosity::All:
                                        {
                                            break;
                                        }
                                    }
                                }
                                if (verbosity != Verbosity::All) maxEditDistance2 = distance;
                                suggestions.push_back(std::move(si));
                                ++suggestionsLen;
                            }
                        }//end foreach
                    }//end if

                    //add edits
                    //derive edits (deletes) from candidate (input) and add them to candidates list
                    //this is a recursive process until the maximum edit distance has been reached
                    if ((lengthDiff < maxEditDistance) && (candidateLen <= prefixLength))
                    {
                        //save some time
                        //do not create edits with edit distance smaller than suggestions already found
                        if (verbosity != Verbosity::All && lengthDiff >= maxEditDistance2) continue;

                        for (int i = 0; i < candidateLen; ++i)
                        {
                            char* tmp = new char[candidateLen];
                            std::memcpy(tmp, candidate, i);
                            std::memcpy(tmp + i, candidate + i + 1, candidateLen - 1 - i);
                            tmp[candidateLen - 1] = '\0';

                            if (hashset1End == hashset1.find(stringHash(tmp)))
                            {
                                hashset1.insert(stringHash(tmp));
                                hashset1End = hashset1.end();
                                candidates.push_back(tmp);
                                ++candidatesLen;
                            }
                            else
                                delete[] tmp;
                        }
                    }
                }//end while

                //sort by ascending edit distance, then by descending word frequency
                if (suggestionsLen > 1)
                    std::sort(suggestions.begin(), suggestions.end(), [](std::unique_ptr<symspell::SuggestItem> &l, std::unique_ptr<symspell::SuggestItem> & r)
                    {
                        return r->CompareTo(*l);
                    });


                //cleaning

                //std::cout << hashset2.size() << std::endl;

                auto candidatesEnd = candidates.end();
                for (auto it = candidates.begin(); it != candidatesEnd; ++it)
                    delete[] * it;

                candidates.clear();
                hashset1.clear();
                hashset2.clear();

                mtx.unlock();

            }//end if

            bool LoadDictionary(char* corpus, int termIndex, int countIndex)
            {
                ifstream stream;
                stream.open(corpus);
                if (!stream.is_open())
                    return false;

                char a, b, c;
                a = stream.get();
                b = stream.get();
                c = stream.get();
                if (a != (char)0xEF || b != (char)0xBB || c != (char)0xBF) {
                    stream.seekg(0);
                }

                SuggestionStage staging(16384);

                string line;
                while ( getline (stream,line) )
                {
                    vector<const char*> lineParts;
                    std::stringstream ss(line);
                    std::string token;
                    while (std::getline(ss, token, ' ')) {
                        size_t len = token.size();
                        char* tmp = new char[len + 1];
                        std::memcpy(tmp, token.c_str(), len);
                        tmp[len] = '\0';
                        lineParts.push_back(tmp);
                    }

                    if (lineParts.size() >= 2)
                    {
                        int64_t count = stoll(lineParts[countIndex]);
                        CreateDictionaryEntry(lineParts[termIndex], count/*, &staging*/);
                    }

                    auto linePartsEnd = lineParts.end();
                    for(auto it = lineParts.begin(); it != linePartsEnd; ++it)
                        delete[] *it;
                }

                stream.close();



                //CommitStaged(staging);
                return true;
            }

            void rempaceSpaces(char* source)
            {
                char* i = source;
                char* j = source;

                do
                {
                    *i = *j;
                    if(*i != ' ')
                        ++i;
                } while(*j++ != 0);
            }

            shared_ptr<WordSegmentationItem> WordSegmentation(const char* input)
            {
                return WordSegmentation(input, this->maxDictionaryEditDistance, this->maxDictionaryWordLength);
            }

            shared_ptr<WordSegmentationItem> WordSegmentation(const char* input, size_t maxEditDistance)
            {
                return WordSegmentation(input, maxEditDistance, this->maxDictionaryWordLength);
            }
        
            shared_ptr<WordSegmentationItem> WordSegmentation(const char* input, size_t maxEditDistance, size_t maxSegmentationWordLength)
            {
                size_t inputLen = strlen(input);
                int arraySize = min(maxSegmentationWordLength, strlen(input));
                std::vector<shared_ptr<WordSegmentationItem>> compositions;
                compositions.resize(arraySize);

                for(size_t i = 0; i < arraySize; ++i)
                {
                    std::shared_ptr<WordSegmentationItem> unq(new WordSegmentationItem());
                    compositions[i] = std::move(unq);
                }

                int circularIndex = -1;

                for (int j = 0; j < inputLen; ++j)
                {
                    //inner loop (row): all possible part lengths (from start position): part can't be bigger than longest word in dictionary (other than long unknown word)
                    int imax = min(inputLen - j, maxSegmentationWordLength);
                    for (int i = 1; i <= imax; ++i)
                    {
                        char* part = new char[i + 1];
                        std::memcpy(part, input + j, i);
                        part[i] = '\0';

                        int separatorLength = 0;
                        int topEd = 0;
                        double topProbabilityLog = 0;
                        char* topResult = nullptr;

                        if (isspace(part[0]))
                        {
                            size_t partLen = strlen(part);
                            char* tmp = new char[partLen];
                            std::memcpy(tmp, part + 1, partLen - 1);
                            tmp[(i - j)] = '\0';

                            delete[] part;
                            part = tmp;
                        }
                        else
                        {
                            //add ed+1: space did not exist, had to be inserted
                            separatorLength = 1;
                        }

                        //remove space from part1, add number of removed spaces to topEd
                        topEd += strlen(part);
                        //remove space
                        rempaceSpaces(part);
                        //add number of removed spaces to ed
                        topEd -= strlen(part);
                        vector<std::unique_ptr<symspell::SuggestItem>> results;
                        Lookup(part, symspell::Verbosity::Top, maxEditDistance, results);
                        if (results.size() > 0)
                        {
                            size_t termLen = strlen(results[0]->term);
                            topResult = new char[termLen + 1];
                            std::memcpy(topResult, results[0]->term, termLen);
                            topResult[termLen] = '\0';

                            topEd += results[0]->distance;
                            //Naive Bayes Rule
                            //we assume the word probabilities of two words to be independent
                            //therefore the resulting probability of the word combination is the product of the two word probabilities

                            //instead of computing the product of probabilities we are computing the sum of the logarithm of probabilities
                            //because the probabilities of words are about 10^-10, the product of many such small numbers could exceed (underflow) the floating number range and become zero
                            //log(ab)=log(a)+log(b)
                            topProbabilityLog = (double)log10((double)results[0]->count / (double)N);
                        }
                        else
                        {
                            delete[] topResult;
                            topResult = part;
                            //default, if word not found
                            //otherwise long input text would win as long unknown word (with ed=edmax+1 ), although there there should many spaces inserted
                            topEd += strlen(part);
                            topProbabilityLog = (double)log10(10.0 / (N * pow(10.0, strlen(part))));
                        }

                        int destinationIndex = ((i + circularIndex) % arraySize);

                        //set values in first loop
                        if (j == 0)
                        {
                            compositions[destinationIndex]->set(_strdup(part), _strdup(topResult), topEd, topProbabilityLog);
                        }
                        else if ((i == maxSegmentationWordLength)
                                 //replace values if better probabilityLogSum, if same edit distance OR one space difference
                                 || (((compositions[circularIndex]->distanceSum + topEd == compositions[destinationIndex]->distanceSum) || (compositions[circularIndex]->distanceSum + separatorLength + topEd == compositions[destinationIndex]->distanceSum)) && (compositions[destinationIndex]->probabilityLogSum < compositions[circularIndex]->probabilityLogSum + topProbabilityLog))
                                 //replace values if smaller edit distance
                                 || (compositions[circularIndex]->distanceSum + separatorLength + topEd < compositions[destinationIndex]->distanceSum))
                        {
                            const char* segmented =  compositions[circularIndex]->segmentedString;
                            const char* corrected =  compositions[circularIndex]->correctedString;

                            size_t segmentedLen = strlen(segmented);
                            size_t correctedLen = strlen(corrected);
                            size_t partLen = strlen(part);
                            size_t topResultLen = strlen(topResult);

                            char* segmentedTmp =  new char[segmentedLen + partLen + 2];
                            char* correctedTmp =  new char[correctedLen + topResultLen + 2];

                            std::memcpy(segmentedTmp, segmented, segmentedLen);
                            std::memcpy(segmentedTmp + segmentedLen, " ", 1);
                            std::memcpy(segmentedTmp + segmentedLen + 1, part, partLen);
                            segmentedTmp[segmentedLen + partLen + 1] = '\0';

                            std::memcpy(correctedTmp, corrected, correctedLen);
                            std::memcpy(correctedTmp + correctedLen, " ", 1);
                            std::memcpy(correctedTmp + correctedLen + 1, topResult, topResultLen);
                            correctedTmp[correctedLen + topResultLen + 1] = '\0';

                            if (strlen(compositions[destinationIndex]->segmentedString) > 0)
                                delete[] compositions[destinationIndex]->segmentedString;

                            if (strlen(compositions[destinationIndex]->correctedString) > 0)
                                delete[] compositions[destinationIndex]->correctedString;

                            compositions[destinationIndex]->set(segmentedTmp,
                                                                 correctedTmp,
                                                                 compositions[circularIndex]->distanceSum + separatorLength + topEd,
                                                                 compositions[circularIndex]->probabilityLogSum + topProbabilityLog);
                        }
                    }
                    ++circularIndex; if (circularIndex == arraySize) circularIndex = 0;
                }
                return compositions[circularIndex];
            }

            /// <summary>Maximum edit distance for dictionary precalculation.</summary>
            size_t MaxDictionaryEditDistance() { return this->maxDictionaryEditDistance; }

            /// <summary>Length of prefix, from which deletes are generated.</summary>
            size_t PrefixLength() { return this->prefixLength; }

            /// <summary>Length of longest word in the dictionary.</summary>
            size_t MaxLength() { return this->maxDictionaryWordLength; }

            /// <summary>Count threshold for a word to be considered a valid word for spelling correction.</summary>
            int64_t CountThreshold() { return this->countThreshold; }

            /// <summary>Number of unique words in the dictionary.</summary>
            size_t WordCount() { return this->words.size(); }

            /// <summary>Number of word prefixes and intermediate word deletes encoded in the dictionary.</summary>
            size_t EntryCount() { return this->deletes.size(); }

        private:
            int32_t initialCapacity;
            int32_t maxDictionaryEditDistance;
            int32_t prefixLength; //prefix length  5..7
            int64_t countThreshold; //a treshold might be specifid, when a term occurs so frequently in the corpus that it is considered a valid word for spelling correction
            uint32_t compactMask;
            EditDistance::DistanceAlgorithm distanceAlgorithm = EditDistance::DistanceAlgorithm::DamerauOSA;
            size_t maxDictionaryWordLength; //maximum dictionary term length
            std::mutex mtx;

            vector<const char*> candidates;

            EditDistance* distanceComparer{ nullptr };
            CUSTOM_SET<size_t> edits;
            CUSTOM_SET<size_t> hashset1; //TODO: use CUSTOM_SET<size_t> hashset1;
            CUSTOM_SET<size_t> hashset2; //TODO: use CUSTOM_SET<size_t> hashset1;
            CUSTOM_SET<size_t>::iterator hashset2End;  //TODO: use CUSTOM_SET<size_t>::iterator hashset2End; 
            hash<const char*> stringHash;
            long N = 1024908267229;

            CUSTOM_MAP<size_t, vector<const char*>> deletes;
            CUSTOM_MAP<size_t, vector<const char*>>::iterator deletesEnd;

            // Dictionary of unique correct spelling words, and the frequency count for each word.
            CUSTOM_MAP<const char*, int64_t, hash<const char*>, comp_c_string> words;
            CUSTOM_MAP<const char*, int64_t, hash<const char*>, comp_c_string>::iterator wordsEnd;

            // Dictionary of unique words that are below the count threshold for being considered correct spellings.
            CUSTOM_MAP<const char*, int64_t, hash<const char*>, comp_c_string> belowThresholdWords;
            CUSTOM_MAP<const char*, int64_t, hash<const char*>, comp_c_string>::iterator belowThresholdWordsEnd;

            bool DeleteInSuggestionPrefix(char const* del, int deleteLen, char const* suggestion, int suggestionLen)
            {
                if (deleteLen == 0) return true;
                if (prefixLength < suggestionLen) suggestionLen = prefixLength;
                int j = 0;
                for (int i = 0; i < deleteLen; ++i)
                {
                    char delChar = del[i];
                    while (j < suggestionLen && delChar != suggestion[j]) ++j;
                    if (j == suggestionLen) return false;
                }
                return true;
            }
    };
}

#endif // SYMSPELL6_H
