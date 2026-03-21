#include <string>
#include "types.h"
#include "uci.h"

#include "tracy/Tracy.hpp"

int main()
{
	ZoneScoped;
	Uci uci{};
	uci.run();
}
