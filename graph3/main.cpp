#include <iostream>
#include <cmath>
#include <vector>

using namespace std;
typedef unsigned char unchar;

/*Цель работы: изучить алгоритмы и реализовать программу, применяющий алгоритм дизеринга
к изображению в формате PGM (P5) с учетом гамма-коррекции sRGB.

Аргументы передаются через командную строку:
program.exe <имя_входного_файла> <имя_выходного_файла> <градиент> <дизеринг> <битность> <гамма>*/

//https://imagemagick.org/source/thresholds.xml
//https://docs.google.com/document/d/1h8c5CUP9d0wpbYDe9jGUukHes-LLGX-ZyAL7WXz43G8/edit#heading=h.pptfpf42jky5
//https://habr.com/ru/post/326936/

enum dithering
{
    none,
    ordered,
    random,
    floydSteinberg,
    jarvisJudiceNinke,
    sierra,
    atkinson,
    halftone
};

double orderedMatrix[8][8] = {
    {0.0, 48.0 / 64.0, 12.0 / 64.0, 60.0 / 64.0, 3.0 / 64.0, 51.0 / 64.0, 15.0 / 64.0, 63.0 / 64.0},
    {32.0 / 64.0, 16.0 / 64.0, 44.0 / 64.0, 28.0 / 64.0, 35.0 / 64.0, 19.0 / 64.0, 47.0 / 64.0, 31.0 / 64.0},
    {8.0 / 64.0, 56.0 / 64.0, 4.0 / 64.0, 52.0 / 64.0, 11.0 / 64.0, 59.0 / 64.0, 7.0 / 64.0, 55.0 / 64.0},
    {40.0 / 64.0, 24.0 / 64.0, 36.0 / 64.0, 20.0 / 64.0, 43.0 / 64.0, 27.0 / 64.0, 39.0 / 64.0, 23.0 / 64.0},
    {2.0 / 64.0, 50.0 / 64.0, 14.0 / 64.0, 62.0 / 64.0, 1.0 / 64.0, 49.0 / 64.0, 13.0 / 64.0, 61.0 / 64.0},
    {34.0 / 64.0, 18.0 / 64.0, 46.0 / 64.0, 30.0 / 64.0, 33.0 / 64.0, 17.0 / 64.0, 45.0 / 64.0, 29.0 / 64.0},
    {10.0 / 64.0, 58.0 / 64.0, 6.0 / 64.0, 54.0 / 64.0, 9.0 / 64.0, 57.0 / 64.0, 5.0 / 64.0, 53.0 / 64.0},
    {42.0 / 64.0, 26.0 / 64.0, 38.0 / 64.0, 22.0 / 64.0, 41.0 / 64.0, 25.0 / 64.0, 37.0 / 64.0, 21.0 / 64.0}};

double halfToneMatrix[4][4] = {
    {7.0 / 17.0, 13.0 / 17.0, 11.0 / 17.0, 4.0 / 17.0},
    {12.0 / 17.0, 16.0 / 17.0, 14.0 / 17.0, 8.0 / 17.0},
    {10.0 / 17.0, 15.0 / 17.0, 6.0 / 17.0, 2.0 / 17.0},
    {5.0 / 17.0, 9.0 / 17.0, 3.0 / 17.0, 1.0 / 17.0}};

double jarvisJudiceNinkeMatrix[3][5] = {
    {0.0, 0.0, 0.0, 7.0 / 48.0, 5.0 / 48.0},
    {3.0 / 48.0, 5.0 / 48.0, 7.0 / 48.0, 5.0 / 48.0, 3.0 / 48.0},
    {1.0 / 48.0, 3.0 / 48.0, 5.0 / 48.0, 3.0 / 48.0, 1.0 / 48.0}};

double sierraMatrix[3][5] = {
    {0.0, 0.0, 0.0, 5.0 / 32.0, 3.0 / 32.0},
    {2.0 / 32.0, 4.0 / 32.0, 5.0 / 32.0, 4.0 / 32.0, 2.0 / 32.0},
    {0.0, 2.0 / 32.0, 3.0 / 32.0, 2.0 / 32.0, 0.0}};

double atkinsonMatrix[3][5] = {
    {0.0, 0.0, 0.0, 1.0 / 8.0, 1.0 / 8.0},
    {0.0, 1.0 / 8.0, 1.0 / 8.0, 1.0 / 8.0, 0.0},
    {0.0, 0.0, 1.0 / 8.0, 0.0, 0.0}};

unchar *createMatrix(int dataSize)
{
    unchar *matrix;
    matrix = new unchar[dataSize];
    return matrix;
}

double toGamma(double gammaCorrection, double pixel)
{
    if (gammaCorrection == 0)
    { //sRGB
        if (pixel <= 0.0031308)
            return 12.92 * pixel;
        else
            return 1.055 * pow(pixel, 1 / 2.4) - 0.055;
    }
    else
    {
        return pow(pixel, 1.0 / gammaCorrection);
    }
}

double fromGamma(double gammaCorrection, double pixel)
{
    if (gammaCorrection == 0)
    { //sRGB
        if (pixel <= 0.04045)
            return pixel / 12.92;
        else
            return pow((pixel + 0.055) / 1.055, 2.4);
    }
    else
    {
        return pow(pixel, gammaCorrection);
    }
}

int main(int argc, char *argv[])
{
    if (argc > 7 || argc < 6)
    {
        cerr << "WRONG AGUMENTS";
        return 1;
    }

    //readind console data
    int dither = stoi(argv[4]);
    if (dither < 0 || dither > 7)
    {
        cerr << "ERROR : WRONG MEAN OF DITHER" << endl;
        return 1;
    }

    FILE *oldFile;
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

    //is file P5 format?
    if (number != '5')
    {
        cerr << "FILE IS NOT P5 PNM-FORMAT" << endl;
        fclose(oldFile);
        return 1;
    }

    //bytes for P5 or P6 format
    int dataSize = 1;
    dataSize = dataSize * wid * hei;

    //creating memory for data-matrix
    unchar *matrix;
    matrix = createMatrix(dataSize);
    if (matrix == nullptr)
    {
        cerr << "MEMORY IS NOT ALLOCATE";
        delete[] matrix;
        fclose(oldFile);
        return 1;
    }

    //reading data-matrix
    int currentBytesInFormat;
    currentBytesInFormat = fread(matrix, 1, dataSize, oldFile);

    if (currentBytesInFormat != dataSize)
    {
        cerr << "READING DATA IS NOT SUCCESSFULL";
        delete[] matrix;
        fclose(oldFile);
        return 1;
    }

    fclose(oldFile);

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

    //creating new file
    int printInFile = fprintf(newFile, "P%c\n%i %i\n%i\n", number, wid, hei, maxColor);
    if (printInFile != 6 + 3 + (int(log10(hei) + 1)) + (int(log10(wid) + 1)))
    { //6 - P5 + spaces + '\n' ; 3 - maxColor; 4 + 4 - wid and hei
        cerr << "PRINT HEADER IN FILE IS NOT SUCCESSFULL";
        delete[] matrix;
        fclose(oldFile);
        return 1;
    }

    //still reading console data
    bool gradient = stoi(argv[3]);
    int byte = stoi(argv[5]);
    double gammaCorrection = stod(argv[6]);

    //horizontal gradient
    if (gradient)
    {
        for (int i = 0; i < hei; ++i)
        {
            for (int j = 0; j < wid; ++j)
            {
                matrix[wid * i + j] = 255 * toGamma(gammaCorrection, (double)j / (wid - 1));
            }
        }
    }

    switch (dither)
    {
    case none:
    {
        for (int i = 0; i < hei; ++i)
        {
            for (int j = 0; j < wid; ++j)
            {
                double linePixel = fromGamma(gammaCorrection, (double)matrix[wid * i + j] / 255.0);
                int color = pow(2, byte) - 1;

                linePixel = linePixel * color;
                linePixel = round(linePixel);

                matrix[wid * i + j] = round(255 * toGamma(gammaCorrection, linePixel / color));
            }
        }
        break;
    }
    case ordered:
    {
        for (int i = 0; i < hei; ++i)
        {
            for (int j = 0; j < wid; ++j)
            {
                double linePixel = fromGamma(gammaCorrection, (double)matrix[wid * i + j] / 255.0);
                linePixel += (orderedMatrix[i % 8][j % 8] - 0.5) / byte;

                int color = pow(2, byte) - 1;
                linePixel = linePixel * color;
                linePixel = round(linePixel);

                matrix[wid * i + j] = round(255 * toGamma(gammaCorrection, linePixel / color));
            }
        }
        break;
    }
    case random:
    {
        for (int i = 0; i < hei; ++i)
        {
            for (int j = 0; j < wid; ++j)
            {
                double linePixel = fromGamma(gammaCorrection, (double)matrix[wid * i + j] / 255.0);
                linePixel += ((double)(rand()) / RAND_MAX) / byte;

                int color = pow(2, byte) - 1;
                linePixel = linePixel * color;
                linePixel = round(linePixel);

                matrix[wid * i + j] = round(255 * toGamma(gammaCorrection, linePixel / color));
            }
        }
        break;
    }
    case floydSteinberg:
    {
        vector<int> error(dataSize);
        error.assign(dataSize, 0);

        for (int i = 0; i < hei; ++i)
        {
            for (int j = 0; j < wid; ++j)
            {
                double linePixel = fromGamma(gammaCorrection, (double)matrix[wid * i + j] / 255.0);
                linePixel = linePixel + error[wid * i + j] / 255.0;

                int color = pow(2, byte) - 1;
                linePixel = linePixel * color;
                linePixel = round(linePixel);
                double err = matrix[wid * i + j] + error[wid * i + j] - 255 * linePixel / color;

                matrix[wid * i + j] = round(255 * toGamma(gammaCorrection, linePixel / color));

                if (j + 1 < wid)
                    error[wid * i + j + 1] += err * 7.0 / 16.0;
                if (i + 1 < hei && j + 1 < wid)
                    error[wid * (i + 1) + j + 1] += err * 1.0 / 16.0;
                if (i + 1 < hei)
                    error[wid * (i + 1) + j] += err * 5.0 / 16.0;
                if (i + 1 < hei && j - 1 >= 0)
                    error[wid * (i + 1) + j - 1] += err * 3.0 / 16.0;
            }
        }
        break;
    }
    case jarvisJudiceNinke:
    {
        vector<int> error(dataSize);
        error.assign(dataSize, 0);

        for (int i = 0; i < hei; ++i)
        {
            for (int j = 0; j < wid; ++j)
            {
                double linePixel = fromGamma(gammaCorrection, (double)matrix[wid * i + j] / 255.0);
                linePixel = linePixel + error[wid * i + j] / 255.0;

                int color = pow(2, byte) - 1;
                linePixel = linePixel * color;
                linePixel = round(linePixel);
                double err = matrix[wid * i + j] + error[wid * i + j] - 255 * linePixel / color;

                matrix[wid * i + j] = round(255 * toGamma(gammaCorrection, linePixel / color));

                for (int iCur = 0; iCur < 3; ++iCur)
                {
                    for (int jCur = 0; jCur < 5; ++jCur)
                    {
                        if (i + iCur >= hei || j + (jCur - 2) >= wid || j + (jCur - 2) < 0 || (iCur == 0 && jCur < 3))
                            continue;

                        error[wid * (i + iCur) + j + (jCur - 2)] += err * jarvisJudiceNinkeMatrix[iCur][jCur];
                    }
                }
            }
        }
        break;
    }
    case sierra:
    {
        vector<int> error(dataSize);
        error.assign(dataSize, 0);

        for (int i = 0; i < hei; ++i)
        {
            for (int j = 0; j < wid; ++j)
            {
                double linePixel = fromGamma(gammaCorrection, (double)matrix[wid * i + j] / 255.0);
                linePixel = linePixel + error[wid * i + j] / 255.0;

                int color = pow(2, byte) - 1;
                linePixel = linePixel * color;
                linePixel = round(linePixel);
                double err = matrix[wid * i + j] + error[wid * i + j] - 255 * linePixel / color;

                matrix[wid * i + j] = round(255 * toGamma(gammaCorrection, linePixel / color));

                for (int iCur = 0; iCur < 3; ++iCur)
                {
                    for (int jCur = 0; jCur < 5; ++jCur)
                    {
                        if (i + iCur >= hei || j + (jCur - 2) >= wid || j + (jCur - 2) < 0 || (iCur == 0 && jCur < 3))
                            continue;

                        error[wid * (i + iCur) + j + (jCur - 2)] += err * sierraMatrix[iCur][jCur];
                    }
                }
            }
        }
        break;
    }
    case atkinson:
    {
        vector<int> error(dataSize);
        error.assign(dataSize, 0);

        for (int i = 0; i < hei; ++i)
        {
            for (int j = 0; j < wid; ++j)
            {
                double linePixel = fromGamma(gammaCorrection, (double)matrix[wid * i + j] / 255.0);
                linePixel = linePixel + error[wid * i + j] / 255.0;

                int color = pow(2, byte) - 1;
                linePixel = linePixel * color;
                linePixel = round(linePixel);
                double err = matrix[wid * i + j] + error[wid * i + j] - 255 * linePixel / color;

                matrix[wid * i + j] = round(255 * toGamma(gammaCorrection, linePixel / color));

                for (int iCur = 0; iCur < 3; ++iCur)
                {
                    for (int jCur = 0; jCur < 5; ++jCur)
                    {
                        if (i + iCur >= hei || j + (jCur - 2) >= wid || j + (jCur - 2) < 0 || (iCur == 0 && jCur < 3))
                            continue;

                        error[wid * (i + iCur) + j + (jCur - 2)] += err * atkinsonMatrix[iCur][jCur];
                    }
                }
            }
        }
        break;
    }
    case halftone:
    {
        for (int i = 0; i < hei; ++i)
        {
            for (int j = 0; j < wid; ++j)
            {
                double linePixel = fromGamma(gammaCorrection, (double)matrix[wid * i + j] / 255.0);
                linePixel += (halfToneMatrix[i % 4][j % 4] - 0.5) / byte;

                int color = pow(2, byte) - 1;
                linePixel = linePixel * color;
                linePixel = round(linePixel);

                matrix[wid * i + j] = round(255 * toGamma(gammaCorrection, linePixel / color));
            }
        }
        break;
    }
    default:
    {
    }
    }

    currentBytesInFormat = fwrite(matrix, dataSize, 1, newFile);
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