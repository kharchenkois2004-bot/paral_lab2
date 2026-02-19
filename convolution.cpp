#include <cstdint>

class Convolution {
    private:

    public:
        static const int blurMatrix[3][3];
        static const int blurDiv = 9;

        static const int edgeMatrix[3][3];
        static const int edgeDiv = 1;

        static const int contrastMatrix[3][3];
        static const int contrastDiv = 1;
};

const int Convolution::blurMatrix[3][3] = {
            {1, 1, 1},
            {1, 1, 1},
            {1, 1, 1}
};

const int Convolution::edgeMatrix[3][3] = {
            {0, 1, 0},
            {1, -4, 1},
            {0, 1, 0}
};

const int Convolution::contrastMatrix[3][3] = {
            {0, -1, 0},
            {-1, 5, -1},
            {0, -1, 0}
};