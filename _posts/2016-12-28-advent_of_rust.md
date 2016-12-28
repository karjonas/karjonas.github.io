---
layout: post 
title: Advent of Rust 
---

This year I decided to participate in the [Advent of Code](http://adventofcode.com/) event which is "a series of small programming puzzles for a variety of skill levels". In total it was 25 puzzles with two parts for each. Since I have been wanting to learn Rust for a long time but haven't got around to it except for creating one very short [program](http://github.com/karjonas/rimgdiff) I decided to solve all puzzles using only Rust. In this post I will share my initial thoughts on Rust and its ecosystem. 

First off, if you do not know what [Rust](https://www.rust-lang.org/) is it is a "systems programming language that runs blazingly fast, prevents segfaults, and guarantees thread safety". As a professional C++ programmer this makes Rust a very interesting language since segfaults and thread issues is a huge annoyance for us.

## Setup
Before starting I had to choose my development setup. I found the [areweideyet](https://areweideyet.com/) website and decided to use [Visual Studio Code](https://code.visualstudio.com/). VS Code worked quite well for the most part but I did not manage to get the debug working so I had to rely on printf-debugging. This is not a huge issue for debugging these simpler problems but is really needed for more serious development.

## Cargo
When developing rust projects the idiomatic way is to use [cargo](http://doc.crates.io/) which is a package manager for rust projects. Using cargo you can easily setup dependencies on external projects available at the [crates.io](https://crates.io/) site. When solving the puzzles I had to rely on a few third-party crates (md5, regex, bitflags and skiplist) and including these was very simple, you only needed to add them as a dependency in your Cargo.toml like this:
```
[dependencies]
md5 = "*"
```

The Rust language developers seem to prefer to provide only the basic data structures and functions in its standard library and keep the rest available through third-party libs. Since installing third-party libs is quite easy I think this is a smart move that removes a lot of maintenance work from the Rust language developers while still easily providing many libraries when using cargo.

## Borrow Checker
That Rust is a language that "runs blazingly fast and prevents segfaults" is quite an extraordinary claim that needs to be followed by extraordinary proof. The proof is the [ownership model](https://doc.rust-lang.org/book/ownership.html) and the [borrow checker](https://doc.rust-lang.org/book/references-and-borrowing.html). Without going into too much detail the ownership and borrowing makes sure that the compiler knows at compile-time the lifetimes and owners of any resource at any given time. This is where the main strength of Rust lies. It will prevent you from even compiling programs that may create certain types of bugs. Of course, making sure the borrow checker can compile your program is where much of your time developing will be spent, at least in the beginning. Luckily the compiler does at least an okay job when explaining why your code does not pass the borrow checker.

## Higher order functions
Rust has support for higher order functions and lazy iterators which at least in theory is a good thing. I managed to use some of these constructs but found that many times it was just easier to create a for-loop instead of fighting with the compiler. I also found that the type checker had a hard time deducing what type some of these functions would return when collecting the iterators. Maybe this is something that will feel more natural after some more practice though.

## Enums and macros
One thing I really liked in Rust that I am missing in C++ is the ability to have [enums](https://doc.rust-lang.org/book/enums.html) with additonal data which you can do pattern matching on. I also like that it is possible to use macros for automatically deriving formatting, hashing and cloning functions. By adding ```#[derive(Debug)]``` before your enum or struct defintion the compiler will generate a format function for the construct. This is quite helpful when doing print debugging or just logging. If you want to use your enum or struct as a key in a hashmap you add ```#[derive(Hash)]``` and a hash function will be generated.

## Conclusion
I think Rust is a very promising language that seems to have really learned a lot from other programming languages mistakes. It removes many of the small annoying things programmers should not need to waste their time working on while still providing really great performance and most importantly guaranteed safety. It also provide an easy way to include third-party code and makes it easy to target multiple platforms. Even though I did only touch upon a subset of the language (I did not do any traits or concurrent programming) the parts I used were well designed. I think the biggest obstacle when learning Rust is that you have to have some patience and not get discouraged when problems occur. Knowing that your code is correct is not enough, you also have to convince the compiler of the fact. 

P.S. You can find my solutions to the puzzles [here](https://github.com/karjonas/aoc2016).
