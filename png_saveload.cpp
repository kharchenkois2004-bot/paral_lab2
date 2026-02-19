#include <iostream>
#include <vector>
#include <lodepng.h>

using namespace std;

/* Функция для загрузки PNG-изображения
- В параметр image сохраняется вектор каналов пикселей (R, G, B, A)
- В параметр width сохраняется значения ширины ихображения
- В параметр height сохраняется значение высоты изображения
- Параметр filename отвечает за путь до файла изображения
- Параметр verbose отвечает за вывод информации в консоль (по умолчанию true)*/
void loadPNG(vector<unsigned char> &image,
             unsigned int &width, unsigned int &height,
             char *filename,
             bool verbose = true) {
    // Декодируем изображение и возвращаем код ошибки (0 для успешного выполнения)
    unsigned load_error = lodepng::decode(image, width, height, filename);

    // Выводим сообщение об ошибке. Если код 0, то будет false. 
    // В противном случае, будет true, а значит что-то пошло не так
    if(load_error) {
        cout << "Decoder error " << load_error
             << ": " << lodepng_error_text(load_error) << endl;
    }

    // Выводим информацию об изображении
    if(verbose) {
        cout << "Image '" << filename << "' loaded"
             << endl << "Resolution: "
             << width << "x" << height
             << ", pixels: " << image.size() / 4
             << endl;
    }    
}

/* Функция для сохранения PNG-изображения
- Параметр filename отвечает за путь до файла изображения
- Параметр image передает вектор каналов пикселей (R, G, B, A)
- Параметр width передает значения ширины ихображения
- Параметр height передает значение высоты изображения
- Параметр verbose отвечает за вывод информации в консоль (по умолчанию true)*/
void savePNG(char *filename,
             vector<unsigned char> &image,
             unsigned int &width, unsigned int &height,
             bool verbose = true) {
    // Кодируем изображение и возвращаем код ошибки (0 для успешного выполнения)
    unsigned save_error = lodepng::encode(filename, image, width, height);

    // Выводим сообщение об ошибке. Если код 0, то будет false. 
    // В противном случае, будет true, а значит что-то пошло не так
    if(save_error) {
        cout << "Encoder error " << save_error
             << ": " << lodepng_error_text(save_error) << endl;
    }

    // Выводим информацию об изображении
    if (verbose) {
        cout << "Image saved as '" << filename << "'"
             << endl << "Resolution: "
             << width << "x" << height
             << ", pixels: " << image.size() / 4
             << endl;
    }
}