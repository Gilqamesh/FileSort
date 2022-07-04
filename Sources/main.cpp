#include "FileSort.hpp"
#include "Log.hpp"
#include <random>
#include <climits>

#define NUMBER_OF_FILES 20
#define LINES_PER_SEGMENT 20
#define LINE_SIZE_BYTES 7

static string generateFile(int numberOfLinesPerSegment, int lineSizeBytes)
{
    static const char chars[] = "0123456789ABCDEFGHIJKLM"
                                "NOPQRSTUVWXYZabcdefghij"
                                "klmnopqrstuvwxyz";

    int segmentSize = numberOfLinesPerSegment * lineSizeBytes;
    static random_device dev;
    static mt19937 rng(dev());
    uniform_int_distribution<mt19937::result_type> distSegments(1, 100);
    int numberOfSegments = distSegments(rng);
    vector<char> data(numberOfSegments * segmentSize);
    int dataIndex = 0;
    uniform_int_distribution<mt19937::result_type> distChar(0, sizeof(chars) - 2);
    for (int curSegmentIndex = 0; curSegmentIndex < numberOfSegments; ++curSegmentIndex)
    {
        for (int curLineIndex = 0; curLineIndex < numberOfLinesPerSegment; ++curLineIndex)
        {
            for (int curCharIndex = 0; curCharIndex < lineSizeBytes - 2; ++curCharIndex)
            {
                data[dataIndex++] = chars[distChar(rng)];
            }
            data[dataIndex++] = '\r';
            data[dataIndex++] = '\n';
        }
    }

    static int fileNameExtraLength = 20;
    string fileName("IN");
    for (int i = 0; i < fileNameExtraLength; ++i)
    {
        char randomChar = chars[distChar(rng)];
        fileName.push_back(randomChar);
    }

    FileManager fileManager;
    FileHandle fileHandle = fileManager.create(fileName);
    fileManager.write(fileHandle, data.data(), data.size());
    return (fileName);
}

int main()
{
    srand((unsigned int)time(NULL));
    try
    {
        FileSort fileSort(INT_MAX, LINES_PER_SEGMENT, LINE_SIZE_BYTES);

        vector<string> fileNames(NUMBER_OF_FILES);
        for (int fileIndex = 0; fileIndex < (int)fileNames.size(); ++fileIndex)
        {
            fileNames[fileIndex] = generateFile(LINES_PER_SEGMENT, LINE_SIZE_BYTES);
        }    
        fileSort.Sort(fileNames, "sortedOutfile");
    }
    catch (exception &e)
    {
        LOG(e.what());
    }

    return (0);
}
