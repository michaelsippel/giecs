#include <stdio.h>

#include <context.h>

int main(int argc, char** argv)
{
    printf("Hello World!\n");

    Context* context = new Context(512);

    delete context;

    return 0;
}

