// VS.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#ifdef _MSC_VER
#if( _SECURE_SCL != 0 )
#pragma message( "Warning: _SECURE_SCL != 0. You _will_ get either slowness or runtime errors." )
#endif

#if( _HAS_ITERATOR_DEBUGGING != 0 )
#pragma message( "Warning: _HAS_ITERATOR_DEBUGGING != 0. You _will_ get either slowness or runtime errors." )
#endif
#endif
#ifdef _MSC_VER
#   define _CRT_SECURE_NO_WARNINGS
#   define _SCL_SECURE_NO_WARNINGS
#   define _ITERATOR_DEBUG_LEVEL 0
#   define _SECURE_SCL 0

#endif

#include "../test.h"
#include <iostream>
#include <chrono> 

int main()
{
    //Catch::Session().run();
	auto start = std::chrono::high_resolution_clock::now();
	int index = 1000000;

	symspell::SymSpell symSpell;
	symSpell.CreateDictionaryEntry("erhan", 1);
	symSpell.CreateDictionaryEntry("orhan", 2);
	symSpell.CreateDictionaryEntry("ayhan", 3);
	char searchTerm[6] = "ozhan";
	vector< std::unique_ptr<symspell::SuggestItem>> items;
	while (index != 0)
	{
		symSpell.Lookup(searchTerm, symspell::Verbosity::Top, items);
		--index;
	}
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	std::cout << "Elapsed time: " << elapsed.count() << " s\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
