#include <iostream>
typedef unsigned char unchar;
using namespace std;

struct channels
{
    unchar first;
    unchar second;
    unchar third;
};

struct autoExtremums
{
    int dark;
    int bright;
};

channels *createMatrix(int dataSize)
{
    channels *matrix;
    matrix = new channels[dataSize];
    return matrix;
}

unchar *createMatrix1(int dataSize)
{
    unchar *matrix;
    matrix = new unchar[dataSize];
    return matrix;
}

enum convertions
{
    RGBGet,
    YCbCrGet,
    RGBAuto,
    YCbCrAuto,
    RGBAuto39,
    YCbCrAuto39
};

int correctAns(int ans)
{
    if (ans < 0)
        return 0;
    else if (ans > 255)
        return 255;
    return ans;
}

void changeRGB(channels *matrix, char number, int dataSize, int offset, double multiplier)
{
    for (int i = 0; i < dataSize; ++i)
    {
        if (number == '6')
        {
            int ans = ((double)matrix[i].first - offset) * multiplier;
            matrix[i].first = correctAns(ans);

            ans = ((double)matrix[i].second - offset) * multiplier;
            matrix[i].second = correctAns(ans);

            ans = ((double)matrix[i].third - offset) * multiplier;
            matrix[i].third = correctAns(ans);
        }
        else
        {
            int ans = ((double)matrix[i].first - offset) * multiplier;
            ans = correctAns(ans);
            matrix[i].first = ans;
            matrix[i].second = ans;
            matrix[i].third = ans;
        }
    }
}

void changeYCbCr(channels *matrix, int number, int dataSize, int offset, double multiplier)
{
    for (int i = 0; i < dataSize; ++i)
    {
        if (number == '6')
        {
            int ans = ((double)matrix[i].first - offset) * multiplier;
            matrix[i].first = correctAns(ans);
        }
        else
        {
            int ans = ((double)matrix[i].first - offset) * multiplier;
            ans = correctAns(ans);
            matrix[i].first = ans;
            matrix[i].second = ans;
            matrix[i].third = ans;
        }
    }
}

void countColors(channels *matrix, int dataSize, char number, int convertion, int *colorsCount)
{
    for (int i = 0; i < dataSize; ++i)
    {
        if (number == '6' && (convertion % 2 == 0))
        {
            ++colorsCount[matrix[i].first];
            ++colorsCount[matrix[i].second];
            ++colorsCount[matrix[i].third];
        }
        else
            ++colorsCount[matrix[i].first];
    }
}

autoExtremums getExtremums(const int *colorsCount)
{
    autoExtremums current{};
    for (int i = 0; i < 256; ++i)
    {
        if (colorsCount[i] > 0)
        {
            current.dark = i;
            break;
        }
    }

    for (int i = 255; i > -1; --i)
    {
        if (colorsCount[i] > 0)
        {
            current.bright = i;
            break;
        }
    }
    return current;
}

int main(int argc, char *argv[])
{
    FILE *oldFile;

    //readind console data
    if (argc != 6 && argc != 4)
    {
        cerr << "WRONG ARGUMENTS";
        return 1;
    }

    int convertion = stoi(argv[3]);
    if (convertion < 0 || convertion > 5)
    {
        cerr << "ERROR : WRONG MEAN OF CONVERSATION" << endl;
        return 1;
    }

    if (argc != 4 && !(convertion == 0 || convertion == 1))
    {
        cerr << "ERROR : WRONG MEAN OF CONVERSATION AND ARGUMENTS" << endl;
        return 1;
    }

    int offset = 0;
    double multiplier = 0;
    if (argc == 6)
    {
        offset = stoi(argv[4]);
        multiplier = stod(argv[5]);
        if (offset < -255 || offset > 255 || multiplier < (1.0 / 255.0) || multiplier > 255.0)
        {
            cerr << "ERROR : WRONG MEAN OF OFFSET OR MULTIPLIER" << endl;
            return 1;
        }
    }

    oldFile = fopen(argv[1], "rb");

    //if file is not open
    if (oldFile == nullptr)
    {
        cerr << "ERROR : FILE IS NOT OPEN" << endl;
        fclose(oldFile);
        return 1;
    }

    //header reading
    int wid = 0;
    int hei = 0;
    int maxColor = 0;
    char number = '0';
    int count = fscanf(oldFile, "P%c\n%i %i\n%i\n", &number, &wid, &hei, &maxColor);

    if (count != 4)
    {
        cerr << "READING HEADER IS NOT SUCCESSFULL";
        fclose(oldFile);
        return 1;
    }

    //is file P5 or P6 format?
    if (number != '5')
    {
        if (number != '6')
        {
            cerr << "FILE IS NOT P5 OR P6 PNM-FORMAT" << endl;
            fclose(oldFile);
            return 1;
        }
    }
    
    int dataSize = wid * hei;

    //creating memory for data-matrix
    channels *matrix;
    matrix = createMatrix(dataSize);
    if (matrix == nullptr)
    {
        cerr << "MEMORY IS NOT ALLOCATE";
        delete[] matrix;
        fclose(oldFile);
        return 1;
    }

    unchar array[3];
    for (int i = 0; i < dataSize; ++i)
    {
        if (number == '6')
        {
            fread(array, 1, 3, oldFile);

            matrix[i].first = array[0];
            matrix[i].second = array[1];
            matrix[i].third = array[2];
        }
        else
        {
            fread(array, 1, 1, oldFile);

            matrix[i].first = array[0];
            matrix[i].second = array[0];
            matrix[i].third = array[0];
        }
    }

    fclose(oldFile);

    switch (convertion)
    {
    case RGBGet:
    {
        changeRGB(matrix, number, dataSize, offset, multiplier);
        break;
    }
    case YCbCrGet:
    {
        changeYCbCr(matrix, number, dataSize, offset, multiplier);
        break;
    }
    case RGBAuto:
    {
        int colorsCount[256];
        for (int &i : colorsCount)
            i = 0;

        countColors(matrix, dataSize, number, convertion, colorsCount);
        autoExtremums current = getExtremums(colorsCount);

        offset = current.dark;
        multiplier = 255.0 / ((double)current.bright - current.dark);
        cout << "OFFSET : " << offset << " ; MULTIPLIER : " << multiplier << '\n';
        changeRGB(matrix, number, dataSize, offset, multiplier);

        break;
    }
    case YCbCrAuto:
    {
        int colorsCount[256];
        for (int &i : colorsCount)
            i = 0;

        countColors(matrix, dataSize, number, convertion, colorsCount);
        autoExtremums current = getExtremums(colorsCount);

        offset = current.dark;
        multiplier = 255.0 / ((double)current.bright - current.dark);
        cout << "OFFSET : " << offset << " ; MULTIPLIER : " << multiplier << '\n';
        changeYCbCr(matrix, number, dataSize, offset, multiplier);
        break;
    }
    case RGBAuto39:
    {
        int colorsCount[256];
        for (int &i : colorsCount)
            i = 0;

        countColors(matrix, dataSize, number, convertion, colorsCount);

        int allDelete = 0;
        if (number == '6' && (convertion % 2 == 0))
            allDelete = (double)wid * hei * 3 * 0.0039;
        else
            allDelete = (double)wid * hei * 0.0039;

        int deleted = 0, positionMin = 0, positionMax = 255;

        while (deleted < allDelete)
        {
            if (deleted % 2 == 0)
            {
                while (colorsCount[positionMin] == 0)
                    ++positionMin;

                --colorsCount[positionMin];
                ++deleted;
            }
            else
            {
                while (colorsCount[positionMax] == 0)
                    --positionMax;

                --colorsCount[positionMax];
                ++deleted;
            }
        }
        autoExtremums current = getExtremums(colorsCount);

        offset = current.dark;
        multiplier = 255.0 / ((double)current.bright - current.dark);
        cout << "OFFSET : " << offset << " ; MULTIPLIER : " << multiplier << '\n';
        changeRGB(matrix, number, dataSize, offset, multiplier);

        break;
    }
    case YCbCrAuto39:
    {
        int colorsCount[256];
        for (int &i : colorsCount)
            i = 0;

        countColors(matrix, dataSize, number, convertion, colorsCount);

        int allDelete = 0;
        if (number == '6' && (convertion % 2 == 0))
            allDelete = (double)wid * hei * 3 * 0.0039;
        else
            allDelete = (double)wid * hei * 0.0039;

        int deleted = 0, positionMin = 0, positionMax = 255;
        while (deleted < allDelete)
        {
            if (deleted % 2 == 0)
            {
                while (colorsCount[positionMin] == 0)
                    ++positionMin;

                --colorsCount[positionMin];
                ++deleted;
            }
            else
            {
                while (colorsCount[positionMax] == 0)
                    --positionMax;

                --colorsCount[positionMax];
                ++deleted;
            }
        }
        autoExtremums current = getExtremums(colorsCount);

        offset = current.dark;
        multiplier = 255.0 / ((double)current.bright - current.dark);
        cout << "OFFSET : " << offset << " ; MULTIPLIER : " << multiplier << '\n';
        changeYCbCr(matrix, number, dataSize, offset, multiplier);

        break;
    }
    default:
        break;
    }

    //opening new pnm-file and working with it
    FILE *newFile;
    newFile = fopen(argv[2], "wb");

    //if file is not open
    if (newFile == nullptr)
    {
        cerr << "ERROR : FILE IS NOT OPEN" << endl;
        delete[] matrix;
        fclose(newFile);
        return 1;
    }

    fprintf(newFile, "P%c\n%i %i\n%i\n", number, wid, hei, maxColor);

    //writing in newFile
    int currentBytesInFormat;

    if (number == '6')
    {
        currentBytesInFormat = fwrite(matrix, dataSize * 3, 1, newFile);
    }
    else
    {
        unchar *newmatrix;
        newmatrix = createMatrix1(dataSize);
        if (newmatrix == nullptr)
        {
            cerr << "MEMORY IS NOT ALLOCATE";
            delete[] matrix;
            delete[] newmatrix;
            fclose(newFile);
            return 1;
        }

        for (int i = 0; i < dataSize; ++i)
            newmatrix[i] = matrix[i].first;
        currentBytesInFormat = fwrite(newmatrix, dataSize, 1, newFile);

        delete[] newmatrix;
    }

    if (currentBytesInFormat != 1)
    {
        cerr << "WRITING DATA IS NOT SUCCESSFULL";
        delete[] matrix;
        fclose(oldFile);
        return 1;
    }

    delete[] matrix;
    fclose(newFile);

    return 0;
}