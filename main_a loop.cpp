#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <cmath>
#include <chrono>
#include <direct.h>
#include <omp.h>

#include "colorMap.cpp"
#include "png_saveload.cpp"
#include "logging.cpp"
#include "logrecord_a.cpp"

#pragma once

using namespace std;

// Функция эрозии отдельного пикселя
static int erode(vector<vector<int>> *matrix, int rowIdx, int colIdx, int step) {
    // Получаем размеры матрицы
    int height = (*matrix).size();
    int width = (*matrix)[0].size();

    // Получаем индексы крайних соседей в округе radius
    int up = max(0, rowIdx - step);
    int left = max(0, colIdx - step);
    int down = min(height - 1, rowIdx + step);
    int right = min(width - 1, colIdx + step);

    int targetValue = (*matrix)[rowIdx][colIdx];

    for(int row = up; row <= down; row++) {
        for(int col = left; col <= right; col++) {
            targetValue &= (*matrix)[row][col];
        }
    }

    return targetValue;
}

// Функция эрозии отдельного пикселя
static int dilate(vector<vector<int>> *matrix, int rowIdx, int colIdx, int step) {
    // Получаем размеры матрицы
    int height = (*matrix).size();
    int width = (*matrix)[0].size();

    // Получаем индексы крайних соседей в округе radius
    int up = max(0, rowIdx - step);
    int left = max(0, colIdx - step);
    int down = min(height - 1, rowIdx + step);
    int right = min(width - 1, colIdx + step);

    int targetValue = (*matrix)[rowIdx][colIdx];

    for(int row = up; row <= down; row++) {
        for(int col = left; col <= right; col++) {
            targetValue |= (*matrix)[row][col];
        }
    }

    return targetValue;
}

// Функция для нахождения делителей числа
static int* findDvisors(int number) {
    int div1 = trunc(sqrt(number));    
    while (number % div1 != 0 && div1 > 1) {
        div1--;
    }
    int div2 = number / div1;
    int divisors[] = {min(div1, div2), max(div1, div2)};
    return divisors;
}

static void paintImage(vector<unsigned char> *image, unsigned int width,
                       size_t startRow, size_t finishRow,
                       size_t startCol, size_t finishCol,
                       uint8_t *threadColor) {
    for(size_t row = startRow; row < finishRow; row++) {
        for(size_t col = startCol; col < finishCol; col++) {
            size_t pixel = row * 4 * width + 4 * col;
            for (int i = 0; i < 3; i++) {
                int div = 2 - (*image)[pixel + i];
                (*image)[pixel + i] = threadColor[i] / div;
            }
        }
    }
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

    const char *imageName = images[2];                  // Имя изображения

    const char *inputDir = "input";                     // Директория с изображения
    const char *outputDir = "output";                   // Директория для результатов обработки
    const char *logDir = "logs";                        // Директория для логов

    const char *outputDir_a = "output/output_a_loop";   // Директория с результатами обработки программы A
    const char *logDir_a = "logs/logs_a_loop";          // Директория с логами программы A    
    const char *processMethod = "erosion";              // Способ обработки (для имени выходного файла)

    const int threshold = 128;                          // Пороговое значение для интенсивности
    const int step = 2;                                 // Шаг эрозии

    const bool DEBUG = false;

    // Переменные
    int threadCount = 16;                               // Кол-во потоков

    // Создаем необходимые директории
    _mkdir(inputDir);
    _mkdir(outputDir);
    _mkdir(outputDir_a);
    _mkdir(logDir);
    _mkdir(logDir_a);

    // Ииндексы матрицы для каждого из потоков
    

    for(int img = 0; img < 4; img++){
        imageName = images[img];
        for(int threads = 2; threads < 17; threads+=2) {
    
    threadCount = threads;    
    omp_set_num_threads(threadCount);
    size_t *startRows = new size_t[threadCount];
    size_t *finishRows = new size_t[threadCount];
    size_t *startCols = new size_t[threadCount];
    size_t *finishCols = new size_t[threadCount];        
    
    // Проверка доступности OpenMP
    #ifdef _OPENMP
        cout << "OpenMP is active" << endl;
    #else
        cout << "OpenMP is not active" << endl;
        threadCount = 1;
    #endif

    // Вычисляем делители для числа потоков, чтобы разделить изображение между ними
    int *divisors = findDvisors(threadCount);
    int threadRows = divisors[0];
    int threadCols = divisors[1];
    
    for(int i = 0; i < 5; i++) {

    vector<unsigned char> image;                    // Массив каналов изображеиня ([R, G, B, A, R, G, B, A, ...])
    unsigned int width;                             // Ширина изображения
    unsigned int height;                            // Высота изображения

    // Карта цветов для раскраски экрана
    ColorMap colorMap(threadCount);
    uint8_t **colors = colorMap.getColors();
    
    // Получение пути до изображения
    char inputImage[bufSize];
    sprintf_s(inputImage, bufSize, "%s/%s.%s", inputDir, imageName, "png");

    loadPNG(image, width, height, inputImage);

    vector<vector<int>> matrix(height, vector<int>(width));

    

    if (DEBUG) {
        cout << threadRows << "x" << threadCols << endl;
    }

    
    
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
        // ID потока
        int id_thread = omp_get_thread_num();

        uint8_t *threadColor = colors[id_thread];

        if (DEBUG) {
            printf("Thread %d\n"
                "Cols: %dx%d\n"
                "Rows: %dx%d\n",
                id_thread,
                (int) startCols[id_thread], (int) finishCols[id_thread],
                (int) startRows[id_thread], (int) finishRows[id_thread]);
        }

        // Обрабатываем изображение по отдельным каналам, игнорируем Alpha.
        // Вычисляем интенсивность
        for(size_t row = startRows[id_thread]; row < finishRows[id_thread]; row++) {
            for(size_t col = startCols[id_thread]; col < finishCols[id_thread]; col++) {
                size_t pixel = row * 4 * width + 4 * col;       // Индекс пикселя
                int intensity = (image[pixel] +                 // Red
                                image[pixel + 1] +              // Green
                                image[pixel + 2]) / 3;          // Blue
            
                // Заполняем матрицу: 0 для интенсивности меньше порога;
                // 1 для интенсивности большей или равной порогу
                matrix[row][col] = intensity >= threshold;

                for(int i = 0; i < 3; i++) {
                    image[pixel + i] = matrix[row][col];
                }
            }
        }

        // Окрашиваем изображение
        paintImage(&image, width,
                   startRows[id_thread], finishRows[id_thread],
                   startCols[id_thread], finishCols[id_thread],
                   colors[id_thread]);

    }

    // Получение пути до обработанного изображения
    char outputImage[bufSize];
    sprintf_s(outputImage, bufSize, "%s/%s_%dthreads_result.%s", outputDir_a, imageName, threadCount, "png");

    savePNG(outputImage, image, width, height, false);

    // Параллельные операции
    # pragma omp parallel
    {
        // Получаем id потока
        int id_thread = omp_get_thread_num();

        // Проводим операцию эрозии
        for(size_t row = startRows[id_thread]; row < finishRows[id_thread]; row++) {
            for(size_t col = startCols[id_thread]; col < finishCols[id_thread]; col++) {
                size_t pixel = row * 4 * width + 4 * col;
                int result = erode(&matrix, row, col, step);
                for(int i = 0; i < 3; i++) {
                    image[pixel + i] = result;
                }
            }
        }

        // Окрашиваем изображение
        paintImage(&image, width,
                   startRows[id_thread], finishRows[id_thread],
                   startCols[id_thread], finishCols[id_thread],
                   colors[id_thread]);

    }
        
    auto timerFinish = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> timerDuration = timerFinish - timerStart;
  
    printf("Threads exectution time = %.4f milliseconds\n", timerDuration.count());

    LogRecord_a logRecord(imageName,
                          width, height,
                          threshold, step,
                          threadCount, timerDuration.count());

    saveLog(logRecord, logDir_a);

    // Получение пути до обработанного изображения с эрозией
    sprintf_s(outputImage, bufSize,
              "%s/%s_%dthreads_result_%s.%s",
              outputDir_a, imageName, threadCount, processMethod, "png");

    savePNG(outputImage, image, width, height, false);
  
    // Удаление массивов, чтобы не занимали память
    image.clear();
    matrix.clear();

            }
    delete[] startRows, finishRows, startCols, finishCols;
    startRows = nullptr;
    finishRows = nullptr;
    startCols = nullptr;
    finishCols = nullptr;
    
        }
    }
    

    return 0;
}