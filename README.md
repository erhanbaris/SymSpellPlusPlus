# SymSpellPlusPlus

Fast way to spell checking on C++.

If you define USE_SPARSEPP before the including header file, it will much more faster than before. But also you have to sparsepp include to your project if you use USE_SPARSEPP preprocessor.

```c++
#include <iostream>

#define USE_SPARSEPP // optional
#include "symspell6.h"

using namespace std;

int main(int argc, const char * argv[]) {
    symspell::SymSpell symSpell;
    symSpell.CreateDictionaryEntry("united kingdom", 1);
    symSpell.CreateDictionaryEntry("united state", 2);

    char* searchTerm = (char*)"united stta";
    vector< std::unique_ptr<symspell::SuggestItem>> items;
    symSpell.Lookup(searchTerm, symspell::Verbosity::Top, items);
    return 0;
}
```

For sparsepp : https://github.com/greg7mdp/sparsepp

For SymSpell : https://github.com/wolfgarbe/symspell

***Static linking exception.*** The copyright holders give you permission to link this library with independent modules to produce an executable, regardless of the license terms of these independent modules, and to copy and distribute the resulting executable under terms of your choice, provided that you also meet, for each linked independent module, the terms and conditions of the license of that module. An independent module is a module which is not derived from or based on this library. If you modify this library, you must extend this exception to your version of the library.
