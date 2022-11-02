#include <stdio.h>

//__attribute__((export_name("r2.add")))
extern int add (int a, int b);

__attribute__((export_name("main")))
int main() {
	// printf ("Hello World %d\n", add (1, 2));
	printf ("Hello World %d\n", 123);
}
