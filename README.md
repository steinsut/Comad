# Comad
Comad is a command handling library.

Do you not want your main function to look like a huge if-else if block?
Are you tired of writing boilerplate code for handling commands?

```cpp
#include <string_view>

int main(int argc, char** argv) {
	if(argv[1] == std::string_view{"foo"}) {
		if(argc > 3)//gotta check argument count
		{
			//this is a string argument
			std::string_view bar1{argv[2]}

			//this is an integer argument
			int bar2;
			//need an int from those characters
			std::string_view bar2_str{argv[3]};
			auto result = std::from_chars(arg.data(), arg.data() + arg.size(), bar2);
			
			//check if conversion was successful
			if(result.ec == std::errc{}) return -1

			//parse the rest of the arguments
			//parse options/flags

			//now we can actually execute our command...
			}
	}
	else if(argv[1] == std::string_view{"baz"}) {
		//more of the same...
	}

	//more commands...
}
```


With Comad, you can do it like this:

```cpp
#include <string_view>

#include "Comad.h"

int main(int argc, char** argv) {
	using namespace comad;
	using namespace comad::command;
	using namespace comad::literals;
	using namespace comad::utils;

	CommandHandler handler{};

	//define command named foo with arguments
	(handler.GetCommandNode() >> "foo")("bar1"_ai, "bar2"_as) = 
		[](const ExecutionContext& ctx) {
			//execute the command
		};

	//define command named baz
	handler.GetCommandNode() >> "baz" = ...

	more commands...
}
```

The goal of Comad is to make sure developers spend time worrying about
how their program actually works, instead of writing tens of dozens of
conditional chains and/or figuring out a way of streamlining this process by 
writing their own miniature command handling library.

Comad tries to do this with a syntax that feels natural to use
with the help of operator overloads and literals. If desired, command chains can be defined
using the short syntax like in the example. The classes actually used in defining and
using command chains have public methods that can be used to interact with them.

## What else before 1.0 ?
- [ ] Preprocessing Command Input (e.g. combining strings between quotes into a single element)
- [ ] Alternative methods for operator overloads
- [ ] Logging
- [ ] Documentation
- [ ] More Tests
- [ ] CMake Packing
- [ ] GitHub Workflow

**NOTE: THERE MAY BE ANY NUMBER OF BREAKING CHANGES UNTIL 1.0**

## Installation
Download from the releases tab, or build from source using CMake.

## Usage
The documentation will be available on a later version.

## Contribution
Pull requests are appreciated.

## License

[MIT](https://choosealicense.com/licenses/mit/)
