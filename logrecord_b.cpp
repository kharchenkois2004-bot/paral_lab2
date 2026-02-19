#include "logging.cpp"

#pragma once

// Запись лога для программы B
struct LogRecord_b : public ILogRecord {  
    char *date;
    const char *processMethod;
    const char *imageName;

    unsigned int width;
    unsigned int height;    
    int transferX;
    int transferY;
    int threadCount;

    double milliseconds;

    LogRecord_b(const char *imageName,
                unsigned int width, unsigned int height,
                int transferX, int transferY,
                const char *processMethod,
                int threadCount, double milliseconds
               ) {
        time_t now = time(0);
        tm* local_time = std::localtime(&now);
        this->date = asctime(local_time);
        
        this->imageName = imageName;
        this->width = width;
        this->height = height;
        this->transferX = transferX;
        this->transferY = transferY;
        this->processMethod = processMethod;
        this->threadCount = threadCount;
        this->milliseconds = milliseconds;
    }

    // Функция для составления имени файла лога
    // Имя файла состоит из имени иозбражения и кол-ва потоков 
    char* logFilename() override {
        const int bufSize = 128;
        char *filename = new char[bufSize];
        sprintf_s(filename, bufSize,
                  "%s_%s_%dthreads.%s",
                  imageName, processMethod, threadCount, "txt");
        return filename;
    }

    // Функция для составления содержимого лога
    // Сохраняет в текстовом виде все поля структуры
    char* logContent() override {
        const int bufSize = 256;
        char *content = new char[bufSize];
        sprintf_s(content, bufSize,
                  "%sImage: \"%s\"; Resolution: %dx%d;\n"
                  "Transfer: [X = %d, Y = %d]; Method: \"%s\";\n"
                  "Threads: %d; Execution time: %.4f ms;",
                  date, imageName, width, height,
                  transferX, transferY,
                  processMethod,
                  threadCount, milliseconds);
        return content;
    }
};