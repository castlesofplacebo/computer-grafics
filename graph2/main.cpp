#include <iostream>
#include <cmath>
using namespace std;
typedef unsigned char unchar;

//Аргументы передаются через командную строку:
//program.exe <имя_входного_файла> <имя_выходного_файла> <яркость_линии> <толщина_линии> <x_начальный> <y_начальный> <x_конечный> <y_конечный> <гамма>
//где
//<яркость_линии>: целое число 0..255;
//<толщина_линии>: положительное дробное число;
//<x,y>: координаты внутри изображения, (0;0) соответствует левому верхнему углу, дробные числа (целые значения соответствуют центру пикселей).
//<гамма>: (optional)положительное вещественное число: гамма-коррекция с введенным значением в качестве гаммы. При его отсутствии используется sRGB.
//Частичное решение: <толщина линии>=1, <гамма>=2.0, координаты начала и конца – целые числа, чёрный фон вместо данных исходного файла (размеры берутся из исходного файла).
//Полное решение: всё работает (гамма + sRGB, толщина не только равная 1, фон из входного изображения) + корректно выделяется и освобождается память, закрываются файлы, есть обработка ошибок.

//полезная информация : https://docs.google.com/document/d/1UnHWoLKMTTT9fie2pwfhln7iBb-PbGEzqO3z5_q7GXA/edit

unchar *createMatrix(int dataSize)
{
    unchar *matrix;
    matrix = new unchar[dataSize];
    return matrix;
}

struct pixel
{
    pixel(double x, double y)
        : x(x), y(y)
    {
    }

    double x;
    double y;
};

void point(unchar *matrix, int x, int y, double opacity, int brightness, int wid, int hei)
{
    if ((y < 0) || (x < 0) || (y >= hei) || (x >= wid))
        return;
    opacity = max(min(opacity, 1.0), 0.0);

    double inputBright;

    double stanadrtPixel = matrix[wid * y + x] / 255.0;
    if (0.04045 >= stanadrtPixel)
        inputBright = stanadrtPixel / 12.92;
    else
        inputBright = pow((stanadrtPixel + 0.055) / 1.055, 2.4);

    double standartBrightness = brightness / 255.0;
    if (0.04045 >= standartBrightness)
        standartBrightness = standartBrightness / 12.92;
    else
        standartBrightness = pow((standartBrightness + 0.055) / 1.055, 2.4);

    double k = opacity * inputBright + (1 - opacity) * standartBrightness;

    double sRGB;
    if (0.0031308 >= k)
        sRGB = 12.92 * k;
    else
        sRGB = 1.055 * pow(k, 1 / 2.4) - 0.055;

    matrix[wid * y + x] = 255 * sRGB;
}

void point(unchar *matrix, int x, int y, double opacity, int brightness, double gammaCorrection, int wid, int hei)
{
    if (y < 0 || x < 0 || y >= hei || x >= wid)
        return;
    opacity = max(min(opacity, 1.0), 0.0);

    double linearBrightness = pow(brightness / 255.0, gammaCorrection);
    double linearPixel = pow(matrix[wid * y + x] / 255.0, gammaCorrection);
    double result = opacity * linearPixel + (1 - opacity) * linearBrightness;
    matrix[wid * y + x] = 255 * pow(result, 1.0 / gammaCorrection);
}

void drawPixel(unchar *matrix, int x, int y, double percent, bool steep, int brightness, double gammaCorrection, int wid, int hei)
{
    if (steep)
        point(matrix, y, x, 1.0 - percent, brightness, gammaCorrection, wid, hei);
    else
        point(matrix, x, y, 1.0 - percent, brightness, gammaCorrection, wid, hei);
}

void drawPixel(unchar *matrix, int x, int y, double percent, bool steep, int brightness, int wid, int hei)
{
    if (steep)
        point(matrix, y, x, 1.0 - percent, brightness, wid, hei);
    else
        point(matrix, x, y, 1.0 - percent, brightness, wid, hei);
}

int main(int argc, char *argv[])
{
    if (argc > 10 || argc < 9)
    {
        cerr << "WRONG AGUMENTS";
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
    fprintf(newFile, "P%c\n%i %i\n%i\n", number, wid, hei, maxColor);

    //reading console arguments
    int brightness = stoi(argv[3]);
    double lineThickness = stod(argv[4]);
    double x0 = stod(argv[5]);
    double y0 = stod(argv[6]);
    double x = stod(argv[7]);
    double y = stod(argv[8]);
    bool usingGamma = false;
    double gammaCorrection = 0.0;
    if (argc == 10)
    {
        gammaCorrection = stod(argv[9]);
        usingGamma = true;
    }

    //algorithm Wu
    pixel begin(x0, y0);
    pixel end(x, y);

    //1 case : line is a dot
    if (begin.x == end.x && begin.y == end.y && begin.x == end.y)
    {
        if (usingGamma)
            point(matrix, (int)(round(begin.x)), (int)(round(begin.y)), 0.0, brightness, gammaCorrection, wid, hei);
        else
            point(matrix, (int)(round(begin.x)), (int)(round(begin.y)), 0.0, brightness, wid, hei);

        fwrite(matrix, dataSize, 1, newFile);

        delete[] matrix;
        fclose(newFile);
        return 0;
    }

    //2 case : other cases
    bool steep = abs(end.y - begin.y) > abs(end.x - begin.x);

    if (steep)
    {
        swap(begin.x, begin.y);
        swap(end.x, end.y);
    }

    if (begin.x > end.x)
    {
        swap(begin.x, end.x);
        swap(begin.y, end.y);
    }

    double dx = end.x - begin.x;
    double dy = end.y - begin.y;
    double gradient;
    if (dx == 0.0)
        gradient = 1.0;
    else
        gradient = dy / dx;

    double intersect;

    //working with general part of line
    intersect = begin.y + gradient * (round(begin.x) - begin.x);

    begin.x = round(begin.x);
    begin.y = round(begin.y);
    end.x = round(end.x);
    end.y = round(end.y);

    //working with main part of line
    for (int i = (int)begin.x; i <= end.x; ++i)
    {
        for (int j = int(intersect - (lineThickness - 1.0) / 2.0);
             //учитываем отступ, чтобы по краям линии получались полупрозрачные пиксели
             j <= int(intersect + (lineThickness + 1.0) / 2.0); //intersect - (lThick - 1) / 2 + lThick
             ++j)
        {
            //вычисление отступа от самой линии для соседних пикселей
            double percent = (lineThickness + 1.0) / 2.0 - fabs(intersect - j);
            //учитываем, что значение в центре - в диапазоне 0 - 1
            if (usingGamma)
                drawPixel(matrix, i, j, min(1.0, percent), steep, brightness, gammaCorrection, wid, hei);
            else
                drawPixel(matrix, i, j, min(1.0, percent), steep, brightness, wid, hei);
        }
        intersect += gradient;
    }

    //working with start point
    for (int i = (int)(begin.x - lineThickness / 2.0);
         i < begin.x;
         ++i)
    {
        intersect = begin.y + gradient * (i - begin.x);
        for (int j = int(intersect - (lineThickness - 1.0) / 2.0);
             j <= int(intersect + (lineThickness + 1.0) / 2.0);
             ++j)
        {
            //вычисление отступа от конечной точки
            double distance = sqrt(pow(i - begin.x, 2) + pow(j - begin.y, 2));
            double percent = (lineThickness + 0.5) / 2.0 - distance;
            if (usingGamma)
                drawPixel(matrix, i, j, min(1.0, percent), steep, brightness, gammaCorrection, wid, hei);
            else
                drawPixel(matrix, i, j, min(1.0, percent), steep, brightness, wid, hei);
        }
    }

    //working with end point
    for (int i = (int)(end.x + 1.0);
         i <= end.x + lineThickness / 2.0;
         ++i)
    {
        intersect = begin.y + gradient * (i - begin.x);
        for (int j = int(intersect - (lineThickness - 1.0) / 2.0);
             j <= int(intersect + (lineThickness + 1.0) / 2.0);
             ++j)
        {
            //вычисление отступа от конечной точки
            double distance = sqrt(pow(i - end.x, 2) + pow(j - end.y, 2));
            double percent = (lineThickness + 0.5) / 2.0 - distance;
            if (usingGamma)
                drawPixel(matrix, i, j, min(1.0, percent), steep, brightness, gammaCorrection, wid, hei);
            else
                drawPixel(matrix, i, j, min(1.0, percent), steep, brightness, wid, hei);
        }
    }
    fwrite(matrix, dataSize, 1, newFile);

    delete[] matrix;
    fclose(newFile);

    return 0;
}