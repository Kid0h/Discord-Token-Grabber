# Discord-Token-Grabber
A simple Discord token grabber.

## Disclaimer
This is just a proof of concept, I do not hold liability for any of your uses with this library.

## Description
This was just a small challenge I set for my self a while ago, so I cleaned it up a bit
and released it here, for other people to look at the implementation and how I went by doing it.

## Example
It really is simple:
```c++
#include <iostream>

#include "Grabber.hpp"

int main()
{
	std::cout << "Tokens found:\n" << grabber::grab_tokens();

	std::cin.get(); // Blocking until input
	return 0;
}
```

## Note
Please. for the love of god, don't be an asshole.