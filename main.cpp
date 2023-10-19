#include <iostream>
#include <fstream>
#include <cmath>
#include <memory>

using namespace std;

// Define a BMP header structure (14 bytes)
#pragma pack(push, 1)
struct BMPHeader {
    char signature[2];
    int32_t fileSize;
    int16_t reserved1;
    int16_t reserved2;
    int32_t dataOffset;
};

// Define a BMP info header structure (40 bytes)
struct BMPInfoHeader {
    int32_t size;
    int32_t width;
    int32_t height;
    int16_t planes;
    int16_t bitsPerPixel;
    int32_t compression;
    int32_t dataSize;
    int32_t horizontalResolution;
    int32_t verticalResolution;
    int32_t colors;
    int32_t importantColors;
};
#pragma pack(pop)

// Function to read a BMP image
bool readBMP(const string& filename, BMPInfoHeader& infoHeader, unique_ptr<uint8_t[]>& imageData) {
    ifstream file(filename, ios::binary);

    if (!file) {
        cerr << "Error opening the file." << endl;
        return false;
    }

    BMPHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(BMPInfoHeader));

    if (infoHeader.bitsPerPixel != 24) {
        cerr << "Only 24-bit BMP images are supported." << endl;
        return false;
    }

    imageData = make_unique<uint8_t[]>(infoHeader.dataSize);
    file.read(reinterpret_cast<char*>(imageData.get()), infoHeader.dataSize);

    return true;
}

// Function to write a BMP image
void writeBMP(const string& filename, const BMPHeader& header, const BMPInfoHeader& infoHeader, const uint8_t* imageData) {
    ofstream file(filename, ios::binary);

    if (!file) {
        cerr << "Error creating the file." << endl;
        return;
    }

    file.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BMPInfoHeader));
    file.write(reinterpret_cast<const char*>(imageData), infoHeader.dataSize);
}

// Function to rotate a BMP image to the left (counterclockwise)
void rotateBMPLeft(const uint8_t* imageData, uint8_t* rotatedData, const BMPInfoHeader& infoHeader) {
    for (int y = 0; y < infoHeader.height; y++) {
        for (int x = 0; x < infoHeader.width; x++) {
            int originalIndex = (y * infoHeader.width + x) * 3;
            int rotatedIndex = (x * infoHeader.height + (infoHeader.height - 1 - y)) * 3;

            for (int channel = 0; channel < 3; channel++) {
                rotatedData[rotatedIndex + channel] = imageData[originalIndex + channel];
            }
        }
    }
}

// Function to rotate a BMP image to the right (clockwise)
void rotateBMPRight(const uint8_t* imageData, uint8_t* rotatedData, const BMPInfoHeader& infoHeader) {
    for (int y = 0; y < infoHeader.height; y++) {
        for (int x = 0; x < infoHeader.width; x++) {
            int originalIndex = (y * infoHeader.width + x) * 3;
            int rotatedIndex = ((infoHeader.width - 1 - x) * infoHeader.height + y) * 3;

            for (int channel = 0; channel < 3; channel++) {
                rotatedData[rotatedIndex + channel] = imageData[originalIndex + channel];
            }
        }
    }
}

// Function to apply Gaussian blur to an image
void applyGaussianBlur(uint8_t* image, int width, int height, double sigma) {
    int kernelSize = static_cast<int>(6 * sigma);
    if (kernelSize % 2 == 0) {
        kernelSize++;
    }

    int halfKernel = kernelSize / 2;
    double* kernel = new double[kernelSize * kernelSize];

    // Initialize the kernel with zeros
    for (int i = 0; i < kernelSize * kernelSize; i++) {
        kernel[i] = 0.0;
    }

    // Calculate Gaussian kernel
    double sum = 0.0;
    for (int i = -halfKernel; i <= halfKernel; i++) {
        for (int j = -halfKernel; j <= halfKernel; j++) {
            int index = (i + halfKernel) * kernelSize + (j + halfKernel);
            kernel[index] = exp(-(i * i + j * j) / (2.0 * sigma * sigma));
            sum += kernel[index];
        }
    }

    // Normalize the kernel
    for (int i = 0; i < kernelSize * kernelSize; i++) {
        kernel[i] /= sum;
    }
    uint8_t* outputImage = new uint8_t[width * height * 3];

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < 3; c++) {
                double newValue = 0.0;
                for (int i = -halfKernel; i <= halfKernel; i++) {
                    for (int j = -halfKernel; j <= halfKernel; j++) {
                        int xOffset = x + j;
                        int yOffset = y + i;

                        if (xOffset >= 0 && xOffset < width && yOffset >= 0 && yOffset < height) {
                            int index = (yOffset * width + xOffset) * 3 + c;
                            int kernelIndex = (i + halfKernel) * kernelSize + (j + halfKernel);
                            newValue += image[index] * kernel[kernelIndex];
                        }
                    }
                }
                int outputIndex = (y * width + x) * 3 + c;
                outputImage[outputIndex] = static_cast<uint8_t>(newValue);
            }
        }
    }

    // Copy the filtered image back to the original image
    for (int i = 0; i < width * height * 3; i++) {
        image[i] = outputImage[i];
    }

    delete[] kernel;
    delete[] outputImage;
}

int main() {
    BMPInfoHeader infoHeader;
    unique_ptr<uint8_t[]> imageData;
    unique_ptr<uint8_t[]> rotatedImageDataLeft;
    unique_ptr<uint8_t[]> rotatedImageDataRight;

    if (!readBMP("picture.bmp", infoHeader, imageData)) {
        return 1;
    }

    BMPHeader header;
    ifstream("picture.bmp", ios::binary).read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));
    cout << "Amount of memory allocated for loading the image: " << infoHeader.dataSize << " bytes" << endl;

    // Rotate the image to the left
    rotatedImageDataLeft = make_unique<uint8_t[]>(infoHeader.dataSize);
    rotateBMPLeft(imageData.get(), rotatedImageDataLeft.get(), infoHeader);
    BMPInfoHeader rotatedInfoHeaderLeft = infoHeader;
    rotatedInfoHeaderLeft.width = infoHeader.height;
    rotatedInfoHeaderLeft.height = infoHeader.width;
    writeBMP("rotated2_left.bmp", header, rotatedInfoHeaderLeft, rotatedImageDataLeft.get());
    cout << "Image rotated to the left and saved as 'rotated2_left.bmp'." << endl;

    // Rotate the image to the right
    rotatedImageDataRight = make_unique<uint8_t[]>(infoHeader.dataSize);
    rotateBMPRight(imageData.get(), rotatedImageDataRight.get(), infoHeader);
    BMPInfoHeader rotatedInfoHeaderRight = infoHeader;
    rotatedInfoHeaderRight.width = infoHeader.height;
    rotatedInfoHeaderRight.height = infoHeader.width;
    writeBMP("rotated1_right.bmp", header, rotatedInfoHeaderRight, rotatedImageDataRight.get());
    cout << "Image rotated to the right and saved as 'rotated1_right.bmp'." << endl;

    // Apply Gaussian blur to the 'rotated2_left.bmp' image
    double sigma = 1.5; // Adjust the sigma value as needed
    applyGaussianBlur(rotatedImageDataLeft.get(), rotatedInfoHeaderLeft.width, rotatedInfoHeaderLeft.height, sigma);

    // Update the BMP header for the filtered image
    BMPInfoHeader filteredInfoHeader = rotatedInfoHeaderLeft;
    filteredInfoHeader.dataSize = rotatedInfoHeaderLeft.dataSize;

    // Save the filtered image as "rotated2_gaussian.bmp"
    writeBMP("rotated2_gaussian.bmp", header, filteredInfoHeader, rotatedImageDataLeft.get());
    cout << "Gaussian blur applied to 'rotated2_left.bmp' and saved as 'rotated2_gaussian.bmp'." << endl;

    return 0;
}
