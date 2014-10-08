#include "pch.h"

class StyleExample { //upper camel case class names
public: //public on top
	StyleExample() { //egyptian style curly brackets

	}

	void doIt() { //lower camel case method and function names
		int i = 1;
		switch (i) {
		case 1: //case in same column as switch
			playSfx(2);
			break;
		}
	}

	virtual void playSfx(int soundId) { //lower camel case for parameters and locals, camel case is used for akronyms, too

	}
private:
	int mySoundId; //members in lower camel case starting with "my"
};

class StyleExample2 : public StyleExample {
public:
	StyleExample2() : myExample(nullptr) { //use the C++0x nullptr keyword - already supported in VS2010, defined to NULL for all other compilers

	}

	virtual void playSfx(int soundId) override { //use the C++0x override keyword - already supported in VS2010, defined to nothing for all other compilers

	}
private:
	StyleExample* myExample;
};