#include <iostream>
typedef unsigned char unchar;
using namespace std;

enum operations
{
    inversion,
    horizontalMirrorImage,
    verticalMirrorImage,
    clockwiseRotation,
    counterclockwiseRotation
};

unchar *createMatrix(int dataSize)
{
    unchar *matrix;
    matrix = new unchar[dataSize];
    return matrix;
}

int main(int argc, char *argv[])
{
    FILE *oldFile;

    //readind console data
    if (argc != 4)
    {
        cout << "WRONG ARGUMENTS";
        return -1;
    }
    oldFile = fopen(argv[1], "rb");

    //if file is not open
    if (oldFile == nullptr)
    {
        cout << "ERROR : FILE IS NOT OPEN" << endl;
        fclose(oldFile);
        return -1;
    }

    //header reading
    int wid = 0;
    int hei = 0;
    int maxColor = 0;
    char number = '0';
    int count = fscanf(oldFile, "P%c\n%i %i\n%i\n", &number, &wid, &hei, &maxColor);

    if (count != 4)
    {
        cout << "READING HEADER IS NOT SUCCESSFULL";
        fclose(oldFile);
        return -1;
    }

    //is file P5 or P6 format?
    if (number != '5')
    {
        if (number != '6')
        {
            cout << "FILE IS NOT P5 OR P6 PNM-FORMAT" << endl;
            fclose(oldFile);
            return -1;
        }
    }

    if (number != '0')
    {
        cout << "READING FINISHED. THE FILE IS P " << number << endl;
    }

    //bytes for P5 or P6 format
    int dataSize = 1;
    if (number == '5')
        dataSize = dataSize * wid * hei;
    else
        dataSize = dataSize * 3 * wid * hei;

    //creating memory for data-matrix
    unchar *matrix;
    matrix = createMatrix(dataSize);
    if (matrix == nullptr)
    {
        cout << "MEMORY IS NOT ALLOCATE";
        delete[] matrix;
        fclose(oldFile);
        return -1;
    }

    //reading data-matrix
    int currentBytesInFormat;
    currentBytesInFormat = fread(matrix, 1, dataSize, oldFile);
    if (currentBytesInFormat >= dataSize)
    {
        cout << "READING DATA IS NOT SUCCESSFULL";
        delete[] matrix;
        fclose(oldFile);
        return -1;
    }

    fclose(oldFile);

    //opening new pnm-file and working with it
    FILE *newFile;
    newFile = fopen(argv[2], "wb");

    //if file is not open
    if (newFile == nullptr)
    {
        cout << "ERROR : FILE IS NOT OPEN" << endl;
        delete[] matrix;
        fclose(newFile);
        return -1;
    }

    char *iter = argv[3];
    int num = atoi(iter);
    if (num < 0 || num > 4)
    {
        cout << "ERROR : WRONG NUMBER OF OPERATION" << endl;
        delete[] matrix;
        fclose(newFile);
        return -1;
    }

    switch (num)
    {
    case inversion:
    {
        fprintf(newFile, "P%c\n%i %i\n%i\n", number, wid, hei, maxColor);

        for (int i = 0; i < dataSize; ++i)
            matrix[i] = maxColor - matrix[i];

        fwrite(matrix, dataSize, 1, newFile);
        break;
    }

    case horizontalMirrorImage:
    {
        fprintf(newFile, "P%c\n%i %i\n%i\n", number, wid, hei, maxColor);

        //creating memory for new data-matrix
        unchar *newmatrix;
        newmatrix = createMatrix(dataSize);
        if (newmatrix == nullptr)
        {
            cout << "MEMORY IS NOT ALLOCATE";
            delete[] matrix;
            fclose(newFile);
            return -1;
        }
        //if format is P5
        int t = 0;
        if (number == '5')
        {
            for (int i = 0; i < hei; ++i)
                for (int j = 0; j < wid; ++j)
                {
                    newmatrix[t] = matrix[i * wid + wid - 1 - j];
                    ++t;
                }
        }
        //if format is P6
        else
        {
            wid = wid * 3;
            for (int i = 0; i < hei; ++i)
                for (int j = 0; j < wid; j = j + 3)
                {
                    newmatrix[t] = matrix[i * wid + wid - 3 - j];
                    ++t;
                    newmatrix[t] = matrix[i * wid + wid - 2 - j];
                    ++t;
                    newmatrix[t] = matrix[i * wid + wid - 1 - j];
                    ++t;
                }
        }
        fwrite(newmatrix, dataSize, 1, newFile);
        delete[] newmatrix;
        break;
    }

    case verticalMirrorImage:
    {
        fprintf(newFile, "P%c\n%i %i\n%i\n", number, wid, hei, maxColor);

        //creating memory for new data-matrix
        unchar *newmatrix;
        newmatrix = createMatrix(dataSize);
        if (newmatrix == nullptr)
        {
            cout << "MEMORY IS NOT ALLOCATE";
            delete[] matrix;
            fclose(newFile);
            return -1;
        }

        //if format is P5
        int t = 0;
        if (number == '5')
        {
            for (int i = 0; i < hei; ++i)
                for (int j = 0; j < wid; ++j)
                {
                    newmatrix[t] = matrix[(hei - i - 1) * wid + j];
                    ++t;
                }
        }
        //if format is P6
        else
        {
            wid = wid * 3;
            for (int i = 0; i < hei; ++i)
                for (int j = 0; j < wid; j = j + 3)
                {
                    newmatrix[t] = matrix[(hei - i - 1) * wid + j];
                    ++t;
                    newmatrix[t] = matrix[(hei - i - 1) * wid + j + 1];
                    ++t;
                    newmatrix[t] = matrix[(hei - i - 1) * wid + j + 2];
                    ++t;
                }
        }

        fwrite(newmatrix, dataSize, 1, newFile);
        delete[] newmatrix;
        break;
    }
    case clockwiseRotation:
    {
        fprintf(newFile, "P%c\n%i %i\n%i\n", number, hei, wid, maxColor);

        //creating memory for new data-matrix
        unchar *newmatrix;
        newmatrix = createMatrix(dataSize);
        if (newmatrix == nullptr)
        {
            cout << "MEMORY IS NOT ALLOCATE";
            delete[] matrix;
            fclose(newFile);
            return -1;
        }

        //if format is P5
        int t = 0;
        if (number == '5')
        {
            for (int i = 0; i < wid; ++i)
                for (int j = hei - 1; j > -1; --j)
                {
                    newmatrix[t] = matrix[j * wid + i];
                    ++t;
                }
        }
        //if format is P6
        else
        {
            wid = wid * 3;
            for (int i = 0; i < wid; i = i + 3)
                for (int j = hei - 1; j > -1; --j)
                {
                    newmatrix[t] = matrix[j * wid + i];
                    ++t;
                    newmatrix[t] = matrix[j * wid + i + 1];
                    ++t;
                    newmatrix[t] = matrix[j * wid + i + 2];
                    ++t;
                }
        }

        fwrite(newmatrix, dataSize, 1, newFile);
        delete[] newmatrix;
        break;
    }

    case counterclockwiseRotation:
    {
        fprintf(newFile, "P%c\n%i %i\n%i\n", number, hei, wid, maxColor);

        //creating memory for new data-matrix
        unchar *newmatrix;
        newmatrix = createMatrix(dataSize);
        if (newmatrix == nullptr)
        {
            cout << "MEMORY IS NOT ALLOCATE";
            delete[] matrix;
            fclose(newFile);
            return -1;
        }

        //if format is P5
        int t = 0;
        if (number == '5')
        {
            for (int i = wid - 1; i > -1; --i)
                for (int j = 0; j < hei; ++j)
                {
                    newmatrix[t] = matrix[j * wid + i];
                    ++t;
                }
        }
        //if format is P6
        else
        {
            wid = wid * 3;
            for (int i = wid - 3; i > -1; i = i - 3)
                for (int j = 0; j < hei; ++j)
                {
                    newmatrix[t] = matrix[j * wid + i];
                    ++t;
                    newmatrix[t] = matrix[j * wid + i + 1];
                    ++t;
                    newmatrix[t] = matrix[j * wid + i + 2];
                    ++t;
                }
        }

        fwrite(newmatrix, dataSize, 1, newFile);
        delete[] newmatrix;
        break;
    }

    default:
        cout << "ERROR";
        delete[] matrix;
        fclose(newFile);
    }

    cout << "DONE" << endl;
    delete[] matrix;
    fclose(newFile);

    return 0;
}