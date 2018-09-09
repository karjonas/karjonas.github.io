---
layout: post 
title: Command line suggestions with Boost
---

While working on [Brayns](https://github.com/BlueBrain/Brayns) I got tired of mistyping and forgetting command line arguments. I therefore wanted to add suggestions for unrecognized options similar to what git is doing. I could not find any examples of doing this with boost program options so therefore I provide a minimal example below in the hope it will be useful. The algorithm uses the [Levenshtein distance](https://en.wikipedia.org/wiki/Levenshtein_distance) to determine the similarity between strings. It also tries to match arguments which are a substring of a known option. Download [here](/assets/program_options_suggestion_example.cpp).

{% highlight c++ %}
{% include_relative /assets/program_options_suggestion_example.cpp %}
{% endhighlight %}