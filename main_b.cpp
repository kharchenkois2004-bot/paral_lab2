#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <cmath>
#include <chrono>
#include <direct.h>
#include <omp.h>

#include "convolution.cpp"
#include "png_saveload.cpp"
#include "logging.cpp"
#include "logrecord_b.cpp"

#pragma once

using namespace std;

// Функция для операции свертки в соответствие с выбранным фильтром
static uint8_t processPixel(vector<vector<uint8_t>> *imageChannel, int rowIdx, int colIdx, 
                            const int convMatrix[3][3], const int convDiv) {
    int targetValue = 0;

    int height = (*imageChannel).size();
    int width = (*imageChannel)[0].size();
    int radius = 1;

    int up = max(0, rowIdx - radius);
    int left = max(0, colIdx - radius);
    int down = min(height - 1, rowIdx + radius);
    int right = min(width - 1, colIdx + radius);

    for(int row = up; row <= down; row++) {
        for(int col = left; col <= right; col++) {
            targetValue += ((*imageChannel)[row][col] *
                            convMatrix[row - rowIdx + radius][col - colIdx + radius]);
        }
    }
    targetValue = targetValue / convDiv;
    return targetValue;
}

// Функция для нахождения делителей числа
static int* findDvisors(int number) {
    int div1 = trunc(sqrt(number));    
    while (number % div1 != 0 && div1 > 1) {
        div1--;
    }
    int div2 = number / div1;
    static int divisors[2] = {min(div1, div2), max(div1, div2)};
    return divisors;
}

// ||||||||||||||||||||||||||||||||||||
// ||||||||||||    MAIN    ||||||||||||
// ||||||||||||||||||||||||||||||||||||
// P.S. Я устал постоянно терять из виду главную функцию
int main() {
    // Постоянные
    const int bufSize = 64;                         // Размер буфера для файла изображения

    const char images[][bufSize] = {                // Список изображений
        "Spider",
        "BlueScreen",
        "Linux",
        "Game"
    };

    const char *imageName = images[3];              // Имя изображения

    const char *inputDir = "input";                 // Директория с изображения
    const char *outputDir = "output";               // Директория для результатов обработки
    const char *logDir = "logs";                    // Директория для логов

    const char *outputDir_b = "output/output_b";    // Директория с результатами обработки программы B
    const char *logDir_b = "logs/logs_b";           // Директория с логами программы B

    const int stepX = 10;                           // Шаг переноса по X
    const int stepY = 10;                           // Шаг переноса по Y

    const int borderColorR = 187;                   // Значение Red для граничного пикселя
    const int borderColorG = 38;                    // Значение Green для граничного пикселя
    const int borderColorB = 73;                    // Значение Blue для граничного пикселя
    
    const bool DEBUG = false;

    // Переменные
    int threadCount = 6;                            // Кол-во потоков

    vector<unsigned char> image;                    // Массив каналов, т.е.: [R, G, B, A, R, G, B, A, ...]
    unsigned int width;                             // Ширина изображения
    unsigned int height;                            // Высота изображения
  
    // Создаем необходимые директории
    _mkdir(inputDir);
    _mkdir(outputDir);
    _mkdir(outputDir_b);
    _mkdir(logDir);
    _mkdir(logDir_b);
    
    // Проверка доступности OpenMP
    #ifdef _OPENMP
        cout << "OpenMP is active" << endl;
        omp_set_num_threads(threadCount);
    #else
        cout << "OpenMP is not active" << endl;
        threadCount = 1;
    #endif

    // Выбор метода обработки изображения (размытие в данном случае)
    const char *processMethod = "blur";
    const int (*convMatrix)[3];
    convMatrix = Convolution::blurMatrix;
    const int convDiv = Convolution::blurDiv;
    
    // Получение пути до изображения
    char inputImage[bufSize];
    sprintf_s(inputImage, bufSize, "%s/%s.%s", inputDir, imageName, "png");

    loadPNG(image, width, height, inputImage);

    // Матрицы для хранения каналов изображения (R, G, B)
    vector<vector<uint8_t>> reds(height, vector<uint8_t>(width));
    vector<vector<uint8_t>> greens(height, vector<uint8_t>(width));
    vector<vector<uint8_t>> blues(height, vector<uint8_t>(width));

    // Вычисляем делители для числа потоков, чтобы разделить изображение между ними
    int *divisors = findDvisors(threadCount);
    int threadRows = divisors[0];
    int threadCols = divisors[1];
    if(DEBUG) {
        cout << threadRows << "x" << threadCols << endl;
    }

    // Ииндексы матрицы для каждого из потоков
    size_t *startRows = new size_t[threadCount];
    size_t *finishRows = new size_t[threadCount];
    size_t *startCols = new size_t[threadCount];
    size_t *finishCols = new size_t[threadCount];
    
    for (int thread = 0; thread < threadCount; thread++) {
        startRows[thread] = (thread / threadCols) * height / threadRows;
        finishRows[thread] = (thread / threadCols + 1) * height / threadRows;
        startCols[thread] = (thread % threadCols) * width / threadCols;
        finishCols[thread] = (thread % threadCols + 1) * width / threadCols;
    }

    auto timerStart = chrono::high_resolution_clock::now();

    // Параллельные операции
    # pragma omp parallel
    {
        // Получаем id потока
        int id_thread = omp_get_thread_num();

        // Индексы начальной и конечной строки для потока
        size_t startRow = startRows[id_thread];
        size_t finishRow = finishRows[id_thread];

        // Индексы начального и конечного столбца для потока
        size_t startCol = startCols[id_thread];
        size_t finishCol = finishCols[id_thread];

        if (DEBUG) {
            printf("Thread %d\n"
                "Cols: %dx%d\n"
                "Rows: %dx%d\n",
                id_thread,
                (int) startCol, (int) finishCol,
                (int) startRow, (int) finishRow);
        }

        // Получаем значение каналов изобржения (R, G, B), игнорируем Alpha
        for(size_t row = startRow; row < finishRow; row++) {
            for(size_t col = startCol; col < finishCol; col++) {
                size_t pixel = row * 4 * width + 4 * col;       // Индекс пикселя
                reds[row][col] = image[pixel];                  // R
                greens[row][col] = image[pixel + 1];            // G
                blues[row][col] = image[pixel + 2];             // B
            }
        }

        // Переносим пиксели изображения
        for(size_t row = startRow; row < finishRow; row++) {
            for(size_t col = startCol; col < finishCol; col++) {
                size_t pixel = row * 4 * width + 4 * col;       // Индекс пикселя

                // Проверяем индексы на выход за границы массива (учитывая отрицательный перенос)
                size_t rowIdx = max((size_t) startRow, row + stepY);
                rowIdx = min(rowIdx, (size_t) finishRow - 1);
                size_t colIdx = max((size_t) startCol, col + stepX);
                colIdx = min(colIdx, (size_t) finishCol - 1);

                reds[rowIdx][colIdx] = image[pixel];                  // R
                greens[rowIdx][colIdx] = image[pixel + 1];            // G
                blues[rowIdx][colIdx] = image[pixel + 2];             // B
            }
        }

        // Окрас перенесенных граничных пикселей
        // По вертикали
        if (stepX >= 0) {
            for(int row = startRow; row < finishRow; row++) {
                for(int col = startCol; col < startCol + stepX; col++) {
                    reds[row][col] = borderColorR;
                    greens[row][col] = borderColorG;
                    blues[row][col] = borderColorB;
                }
            }
        } else {
            for(int row = startRow; row < finishRow; row++) {
                for(int col = finishCol + stepX; col < finishCol; col++) {
                    reds[row][col] = borderColorR;
                    greens[row][col] = borderColorG;
                    blues[row][col] = borderColorB;
                }
            }
        }
        
        // По горизонтали
        if (stepY >= 0) {
            for(int row = startRow; row < startRow + stepY; row++) {
                for(int col = startCol; col < finishCol; col++) {
                    reds[row][col] = borderColorR;
                    greens[row][col] = borderColorG;
                    blues[row][col] = borderColorB;
                }
            }
        } else {
            for(int row = finishRow + stepY; row < finishRow; row++) {
                for(int col = startCol; col < finishCol; col++) {
                    reds[row][col] = borderColorR;
                    greens[row][col] = borderColorG;
                    blues[row][col] = borderColorB;
                }
            }
        }        

        // Применяем изменения изображния
        for(size_t row = startRow; row < finishRow; row++) {
            for(size_t col = startCol; col < finishCol; col++) {
                size_t pixel = row * 4 * width + 4 * col;       // Индекс пикселя
                image[pixel] = reds[row][col];                  // R
                image[pixel + 1] = greens[row][col];            // G
                image[pixel + 2] = blues[row][col];             // B
            }
        }
    }

    // Получение пути до обработанного изображения
    char outputImage[bufSize];
    sprintf_s(outputImage, bufSize, "%s/%s_transfer.%s", outputDir_b, imageName, "png");

    savePNG(outputImage, image, width, height, false);

    // Параллельные операции
    # pragma omp parallel
    {
        // Получаем id потока
        int id_thread = omp_get_thread_num();

        // Индексы начальной и конечной строки для потока
        size_t startRow = (id_thread / threadCols) * height / threadRows;
        size_t finishRow = (id_thread / threadCols + 1) * height / threadRows;

        // Индексы начального и конечного столбца для потока
        size_t startCol = (id_thread % threadCols) * width / threadCols;
        size_t finishCol = (id_thread % threadCols + 1) * width / threadCols;

        // Обработка изображений
        for(size_t row = startRow; row < finishRow; row++) {
            for(size_t col = startCol; col < finishCol; col++) {
                size_t pixel = row * 4 * width + 4 * col;
                image[pixel] = processPixel(&reds, row, col, convMatrix, convDiv);
                image[pixel + 1] = processPixel(&greens, row, col, convMatrix, convDiv);
                image[pixel + 2] = processPixel(&blues, row, col, convMatrix, convDiv);
            }
        }
    }
        
    auto timerFinish = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> timerDuration = timerFinish - timerStart;

    printf("Threads exectution time = %.4f milliseconds\n", timerDuration.count());

    LogRecord_b logRecord(imageName,
                          width, height,
                          stepX, stepY,
                          processMethod,
                          threadCount, timerDuration.count());

    saveLog(logRecord, logDir_b);;

    // Получение пути до обработанного изображения с эрозией
    sprintf_s(outputImage, bufSize, "%s/%s_transfer_%s.%s", outputDir_b, imageName, processMethod, "png");

    savePNG(outputImage, image, width, height, false);
    return 0;
}