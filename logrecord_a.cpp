#include "logging.cpp"

#pragma once

// Запись лога для программы A
struct LogRecord_a : public ILogRecord {  
    char *date;
    const char *imageName;

    unsigned int width;
    unsigned int height;    
    int threshold;
    int step;
    int threadCount;

    double milliseconds;

    LogRecord_a(const char *imageName,
                unsigned int width, unsigned int height,
                int threshold, int step,
                int threadCount, double milliseconds
               ) {
        time_t now = time(0);
        tm* local_time = std::localtime(&now);
        this->date = asctime(local_time);
        
        this->imageName = imageName;
        this->width = width;
        this->height = height;
        this->threshold = threshold;
        this->step = step;
        this->threadCount = threadCount;
        this->milliseconds = milliseconds;
    }

    // Функция для составления имени файла лога
    // Имя файла состоит из имени иозбражения и кол-ва потоков 
    char* logFilename() override {
        const int bufSize = 128;
        char *filename = new char[bufSize];
        sprintf_s(filename, bufSize,
                  "%s_%dthreads.%s",
                  imageName, threadCount, "txt");
        return filename;
    }

    // Функция для составления содержимого лога
    // Сохраняет в текстовом виде все поля структуры
    char* logContent() override {
        const int bufSize = 256;
        char *content = new char[bufSize];
        sprintf_s(content, bufSize,
                  "%sImage: \"%s\"; Resolution: %dx%d;\n"
                  "Threshold: %d; Erosion step: %d;\n"
                  "Threads: %d; Execution time: %.4f ms;",
                  date, imageName, width, height,
                  threshold, step,
                  threadCount, milliseconds);
        return content;
    }
};