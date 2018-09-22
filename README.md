# SymSpellPlusPlus

Code originally based on faroo.com's C# code.
Only you need include header file to your project and set words via CreateDictionaryEntry function after than you can check matches via Correct function.

Based on : [1000x Faster Spelling Correction algorithm](http://blog.faroo.com/2012/06/07/improved-edit-distance-based-spelling-correction/)

google::dense_hash_set using for performance improvements. If you don't want to use it only delete #define USE_GOOGLE_DENSE_HASH_MAP from symspell.h

msgpack using for saving and loading datas from disk. If you don't want to use it only delete #define IO_OPERATIONS from symspell.h


```c++
#include <iostream>
#include "symspell.h"

using namespace std;

int main(int argc, const char * argv[]) {
    SymSpell a;
    a.CreateDictionaryEntry("united kingdom");
    a.CreateDictionaryEntry("united state");

    vector<suggestItem> items;
    a.Correct("united stta", items);
    return 0;
}
```

Static linking exception. The copyright holders give you permission to link this library with independent modules to produce an executable, regardless of the license terms of these independent modules, and to copy and distribute the resulting executable under terms of your choice, provided that you also meet, for each linked independent module, the terms and conditions of the license of that module. An independent module is a module which is not derived from or based on this library. If you modify this library, you must extend this exception to your version of the library.
