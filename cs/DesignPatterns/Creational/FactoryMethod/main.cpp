#include "image.hpp"

int main()
{
    Image img;

    std::string filename = "test.jpeg";

    img.openImageFile(filename);
    img.drawLine();
    img.brushRect();

    return 0;
}