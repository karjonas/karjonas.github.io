---
layout: post 
title: std::initializer_list
---

I was watching one of Bisqwit's awesome [videos](https://www.youtube.com/watch?v=rpLoS7B6T94) and saw a line of code in a for loop I have never seen before. It looked something like this:

``` C++
for(unsigned n: { 0xF999F,0x26227,0xF1F8F,0xF1244,
                  0xF9F9F,0xF9F1F,0xF9F99,0xF8F88 })
```

I wondered what type the list would be so I tried to compile this with clang and get the ast dumped but instead I got the following error:

``` C++
file.cpp:5:19: error: cannot deduce type of initializer 
list because std::initializer_list was
 not found; include <initializer_list>
```

Aha, so we know this is an ```initializer_list```. According to [cppreference.com](http://en.cppreference.com/w/cpp/utility/initializer_list): "An object of type ```std::initializer_list<T>``` is a lightweight proxy object that provides access to an array of objects of type ```const T```". When I include ```<initializer_list>``` the code compiles. Altough the motivational uses for initializer_list are quite different than to be used as temporaries in for loops it is still a nice litte hack. 
