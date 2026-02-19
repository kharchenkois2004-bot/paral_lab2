#include <cstdint>

class ColorMap {
    private:
        const uint8_t maxShade = 255;

        // Выбор цветов
        const uint8_t rgbMask[13][3] = {
            {2, 0, 0},              // Red
            {0, 2, 0},              // Green
            {0, 0, 2},              // Blue
            {2, 2, 0},              // Yellow
            {2, 0, 2},              // Purple
            {0, 2, 2},              // Aqua
            {2, 1, 0},              // Orange
            {1, 2, 0},              // Lime
            {0, 1, 2},              // Sky
            {2, 0, 1},              // Pink
            {0, 2, 1},              // Cyan
            {1, 0, 2},              // Violet
            {2, 2, 2}               // White
        };

        int colorNum;
        uint8_t shade;
        uint8_t **colors;

        void calcNumber(int n_colors) {
            this->shade = (maxShade + 1) / 4;
        }

        void initColors(int size) {
            colors = new uint8_t*[size];

            for (int i = 0; i < size; i++) {
                colors[i] = new uint8_t[3];
                int grade = (i / 13) + 1;

                for (int j = 0; j < 3; j++) {
                    colors[i][j] = (rgbMask[i % 13][j] *
                    this->shade / grade) + (maxShade / 2);
                }
            }
        }

    public:
        ColorMap(int n_colors) {
            this->colorNum = n_colors;
            this->calcNumber(colorNum);
            this->initColors(colorNum);
        }

        ~ColorMap() {
            // Удаляем матрицу colors
            for (int i = 0; i < colorNum; i++) {
                delete[] colors[i];
            }
            delete[] colors;
            colors = nullptr;
        };

        uint8_t** getColors() {
            return colors;
        }
};

