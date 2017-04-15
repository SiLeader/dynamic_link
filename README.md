Dynamic Link Support Library
==========================
Copyright (C) 2017 SiLeader. All rights reserved.  
Licensed under the Mozilla Public License 2.0 (https://opensource.org/licenses/MPL-2.0)

## Overview
Multi-platform dynamic link support.
+ One header file
+ Support RAII
+ Confirm the existence of library class from function class

## Usage
```cpp
// link libdy.so, libdy.dylib or dy.dll
dylib::library dy("dy");

// get function
auto func=dy.get<void()>("func");

// call function
func();
```

## Supporting OS
+ Unix-like
+ Windows
