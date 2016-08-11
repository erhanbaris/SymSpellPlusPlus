# SymSpellPlusPlus

Code originally based on faroo.com's C# code. It has got a some performence issue and i will fix it.
Only you need include header file to your project and set words via CreateDictionaryEntry function after than you can check matches via Correct function.

I will create Python implemantation over c++ code for performence.<br>

Based on : [1000x Faster Spelling Correction algorithm](http://blog.faroo.com/2012/06/07/improved-edit-distance-based-spelling-correction/)<br>



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
