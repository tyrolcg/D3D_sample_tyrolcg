#include "app.h"

int main(int argc, wchar_t** argv, wchar_t** evnp) {
	App app(960, 540);
	app.Run();
	return 0;
}