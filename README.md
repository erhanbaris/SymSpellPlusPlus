# SymSpellPlusPlus

Code originally based on faroo.com's C# code.
Only you need include header file to your project and set words via CreateDictionaryEntry function after than you can check matches via Correct function.

Based on : [1000x Faster Spelling Correction algorithm](http://blog.faroo.com/2012/06/07/improved-edit-distance-based-spelling-correction/)<br>

google::dense_hash_set using for performance improvements. If you don't want to use it only delete #define USE_GOOGLE_DENSE_HASH_MAP from symspell.h<br>
msgpack using for saving and loading datas from disk. If you don't want to use it only delete #define IO_OPERATIONS from symspell.h<br>


##Usage
```c++
#include <iostream>
#include "symspell.h"

using namespace std;

int main(int argc, const char * argv[]) {
    SymSpell a;
    a.CreateDictionaryEntry("united kingdom");
    a.CreateDictionaryEntry("united state");
    
    vector<suggestItem> items = a.Correct("united stta");
    return 0;
}
```


##License

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License, 
version 3.0 (LGPL-3.0) as published by the Free Software Foundation.
[http://www.opensource.org/licenses/LGPL-3.0](http://www.opensource.org/licenses/LGPL-3.0)
