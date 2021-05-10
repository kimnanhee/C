#include "logger.h"
#include <stdio.h>

FILE* fp;

void log_begin(char* str)
{
    fp = fopen(str, "w");
}