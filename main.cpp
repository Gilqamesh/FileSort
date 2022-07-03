#include "FileSort.hpp"
#include "Log.hpp"

int main()
{
    try
    {
        FileSort fileSort(Megabytes(100), 20, 7);

        string inFilePath("test.txt");
        fileSort.Sort(inFilePath, "out" + inFilePath);
    }
    catch (exception &e)
    {
        LOG(e.what());
    }

    return (0);
}
