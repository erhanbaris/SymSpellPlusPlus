#ifndef TEST_H
#define TEST_H

#define CATCH_CONFIG_RUNNER
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

#endif // TEST_H
