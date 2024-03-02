// Author: Tobias
// A function that calulates the length of any string
int stringLength(char *string)
{
    int i = 0;
    // Loop through the string
    while (*string)
    {
        i++;
        string++;
    }
    // Reset the string address
    string -= i;
    return i;
}
// End Author

// Author: Tobias
// A function that concatenate two strings
char *stringConcat(char *string1, char *string2)
{
    // Determine the lengths of the strings
    int string1Counter = stringLength(string1);
    int string2Counter = stringLength(string2);

    // Creates a string with the size of string 1 and 2 together + 1 for the null character
    char finalStr[string1Counter + string2Counter + 1];
    int i;
    for (i = 0; i < sizeof(finalStr); i++)
    {
        // Save String 1 first in the final string
        if (i < string1Counter)
        {
            finalStr[i] = string1[i];
        }
        // Then save string 2 after string 1
        else
        {
            finalStr[i] = string2[i - string1Counter];
        }
    }
    // Add the null character to the end
    finalStr[sizeof(finalStr) - 1] = 0x00;
    return (char *)finalStr;
}
// End Author