#ifndef TEST_H
#define TEST_H

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "symspell6.h"


TEST_CASE( "WordsWithSharedPrefixShouldRetainCounts", "WordsWithSharedPrefixShouldRetainCounts" ) {

    symspell::SymSpell symSpell(16, 1, 3);
    symSpell.CreateDictionaryEntry("pipe", 5);
    symSpell.CreateDictionaryEntry("pips", 10);
    vector<std::unique_ptr<symspell::SuggestItem>> result;

    symSpell.Lookup("pipe", symspell::Verbosity::All, 1, result);
    REQUIRE(2 == result.size());
    REQUIRE(strcmp("pipe", result[0]->term) == 0);
    REQUIRE(5 == result[0]->count);
    REQUIRE(strcmp("pips", result[1]->term) == 0);
    REQUIRE(10 == result[1]->count);
    result.clear();


    symSpell.Lookup("pips", symspell::Verbosity::All, 1, result);
    REQUIRE(2 == result.size());
    REQUIRE(strcmp("pips", result[0]->term) == 0);
    REQUIRE(10 == result[0]->count);
    REQUIRE(strcmp("pipe", result[1]->term) == 0);
    REQUIRE(5 == result[1]->count);
    result.clear();

    symSpell.Lookup("pip", symspell::Verbosity::All, 1, result);
    REQUIRE(2 == result.size());
    REQUIRE(strcmp("pips", result[0]->term) == 0);
    REQUIRE(10 == result[0]->count);
    REQUIRE(strcmp("pipe", result[1]->term) == 0);
    REQUIRE(5 == result[1]->count);
    result.clear();
}

TEST_CASE( "AddAdditionalCountsShouldNotAddWordAgain", "AddAdditionalCountsShouldNotAddWordAgain" ) {

    symspell::SymSpell symSpell;
    char* word = "hello";
    symSpell.CreateDictionaryEntry(word, 11);
    REQUIRE(1 ==  symSpell.WordCount());
    symSpell.CreateDictionaryEntry(word, 3);
    REQUIRE(1 == symSpell.WordCount());
}

TEST_CASE( "AddAdditionalCountsShouldIncreaseCount", "AddAdditionalCountsShouldIncreaseCount" ) {

    symspell::SymSpell symSpell;
    char* word = "hello";
    symSpell.CreateDictionaryEntry(word, 11);
    vector<std::unique_ptr<symspell::SuggestItem>> result;
    symSpell.Lookup(word, symspell::Verbosity::Top, result);
    long count = 0;
    if (result.size() == 1) count = result[0]->count;
    REQUIRE(11 == count);
    symSpell.CreateDictionaryEntry(word, 3);
    symSpell.Lookup(word, symspell::Verbosity::Top, result);
    count = 0;
    if (result.size() == 1) count = result[0]->count;
    REQUIRE(11 + 3 == count);
}

TEST_CASE( "AddAdditionalCountsShouldNotOverflow", "AddAdditionalCountsShouldNotOverflow" ) {
    int64_t m = (std::numeric_limits<int64_t>::max)();
    symspell::SymSpell symSpell;
    char* word = "hello";
    vector<std::unique_ptr<symspell::SuggestItem>> result;
    symSpell.CreateDictionaryEntry(word, m - 10);
    symSpell.Lookup(word, symspell::Verbosity::Top, result);
    long count = 0;
    if (result.size() == 1) count = result[0]->count;
    REQUIRE(m - 10 == count);
    symSpell.CreateDictionaryEntry(word, 11);
    symSpell.Lookup(word, symspell::Verbosity::Top, result);
    count = 0;
    if (result.size() == 1) count = result[0]->count;
    REQUIRE(m == count);
}

TEST_CASE( "VerbosityShouldControlLookupResults", "VerbosityShouldControlLookupResults" ) {
    symspell::SymSpell symSpell;
    symSpell.CreateDictionaryEntry("steam", 1);
    symSpell.CreateDictionaryEntry("steams", 2);
    symSpell.CreateDictionaryEntry("steem", 3);
    vector<std::unique_ptr<symspell::SuggestItem>> result;
    symSpell.Lookup("steems", symspell::Verbosity::Top, 2, result);
    REQUIRE(1 == result.size());
    symSpell.Lookup("steems", symspell::Verbosity::Closest, 2, result);
    REQUIRE(2 == result.size());
    symSpell.Lookup("steems", symspell::Verbosity::All, 2, result);
    REQUIRE(3 == result.size());
}

TEST_CASE( "LookupShouldReturnMostFrequent", "LookupShouldReturnMostFrequent" ) {
    symspell::SymSpell symSpell;
    symSpell.CreateDictionaryEntry("steama", 4);
    symSpell.CreateDictionaryEntry("steamb", 6);
    symSpell.CreateDictionaryEntry("steamc", 2);
    vector<std::unique_ptr<symspell::SuggestItem>> result;
    symSpell.Lookup("steam", symspell::Verbosity::Top, 2, result);
    REQUIRE(1 == result.size());
    REQUIRE(strcmp("steamb", result[0]->term) == 0);
    REQUIRE(6 == result[0]->count);
}

TEST_CASE( "LookupShouldFindExactMatch", "LookupShouldFindExactMatch" ) {
    symspell::SymSpell symSpell;
    symSpell.CreateDictionaryEntry("steama", 4);
    symSpell.CreateDictionaryEntry("steamb", 6);
    symSpell.CreateDictionaryEntry("steamc", 2);
    vector<std::unique_ptr<symspell::SuggestItem>> result;
    symSpell.Lookup("steama", symspell::Verbosity::Top, 2, result);
    REQUIRE(1 == result.size());
    REQUIRE(strcmp("steama", result[0]->term) == 0);
}

TEST_CASE( "LookupShouldNotReturnNonWordDelete", "LookupShouldFindExactMatch" ) {
    symspell::SymSpell symSpell(16, 2, 7, 10);
    symSpell.CreateDictionaryEntry("pawn", 10);
    vector<std::unique_ptr<symspell::SuggestItem>> result;
    symSpell.Lookup("paw", symspell::Verbosity::Top, 0, result);
    REQUIRE(0 == result.size());
    symSpell.Lookup("awn", symspell::Verbosity::Top, 0, result);
    REQUIRE(0 == result.size());
}

TEST_CASE( "LookupShouldNotReturnLowCountWord", "LookupShouldFindExactMatch" ) {
    symspell::SymSpell symSpell(16, 2, 7, 10);
    symSpell.CreateDictionaryEntry("pawn", 1);
    vector<std::unique_ptr<symspell::SuggestItem>> result;
    symSpell.Lookup("pawn", symspell::Verbosity::Top, 0, result);
    REQUIRE(0 == result.size());
}

TEST_CASE( "LookupShouldNotReturnLowCountWordThatsAlsoDeleteWord", "LookupShouldFindExactMatch" ) {
    symspell::SymSpell symSpell(16, 2, 7, 10);
    symSpell.CreateDictionaryEntry("flame", 20);
    symSpell.CreateDictionaryEntry("flam", 1);
    vector<std::unique_ptr<symspell::SuggestItem>> result;
    symSpell.Lookup("flam", symspell::Verbosity::Top, 0, result);
    REQUIRE(0 == result.size());
}
#endif // TEST_H
