
for (int j=0; j < i; j++)
{
    argLength = strlen(arguments[j]);
    strcpy(buffer, arguments[j]);
    for (int x=0; x < argLength; x++)
    {
        printf("%s ", arguments[j]);
        printf("%s\n", buffer[x]);
    }
}