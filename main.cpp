#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

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
vector<uint8_t> readBMP(const string& filename, BMPInfoHeader& infoHeader) {
    ifstream file(filename, ios::binary);

    if (!file) {
        cerr << "Error opening the file." << endl;
        return vector<uint8_t>();
    }

    BMPHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(BMPInfoHeader));

    vector<uint8_t> data(infoHeader.dataSize);
    file.read(reinterpret_cast<char*>(data.data()), infoHeader.dataSize);

    return data;
}

// Function to write a BMP image
void writeBMP(const string& filename, const BMPHeader& header, const BMPInfoHeader& infoHeader, const vector<uint8_t>& data) {
    ofstream file(filename, ios::binary);

    if (!file) {
        cerr << "Error creating the file." << endl;
        return;
    }

    file.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(BMPInfoHeader));
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}

// Function to rotate a BMP image to the left (counterclockwise)
vector<uint8_t> rotateBMPLeft(const vector<uint8_t>& data, const BMPInfoHeader& infoHeader) {
    vector<uint8_t> rotatedData(infoHeader.dataSize, 0);

    for (int y = 0; y < infoHeader.height; y++) {
        for (int x = 0; x < infoHeader.width; x++) {
            int originalIndex = (y * infoHeader.width + x) * 3;
            int rotatedIndex = (x * infoHeader.height + (infoHeader.height - 1 - y)) * 3;

            for (int channel = 0; channel < 3; channel++) {
                rotatedData[rotatedIndex + channel] = data[originalIndex + channel];
            }
        }
    }

    return rotatedData;
}

// Function to rotate a BMP image to the right (clockwise)
vector<uint8_t> rotateBMPRight(const vector<uint8_t>& data, const BMPInfoHeader& infoHeader) {
    vector<uint8_t> rotatedData(infoHeader.dataSize, 0);

    for (int y = 0; y < infoHeader.height; y++) {
        for (int x = 0; x < infoHeader.width; x++) {
            int originalIndex = (y * infoHeader.width + x) * 3;
            int rotatedIndex = ((infoHeader.width - 1 - x) * infoHeader.height + y) * 3;

            for (int channel = 0; channel < 3; channel++) {
                rotatedData[rotatedIndex + channel] = data[originalIndex + channel];
            }
        }
    }

    return rotatedData;
}

// Function to apply Gaussian blur to an image
void applyGaussianBlur(vector<uint8_t>& image, int width, int height, int channels, double sigma) {
    // Add your Gaussian blur code here
    // You can use the existing applyGaussianBlur function or implement a new one
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

    vector<uint8_t> outputImage(image.size());

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            for (int c = 0; c < channels; c++) {
                double newValue = 0.0;
                for (int i = -halfKernel; i <= halfKernel; i++) {
                    for (int j = -halfKernel; j <= halfKernel; j++) {
                        int xOffset = x + j;
                        int yOffset = y + i;

                        if (xOffset >= 0 && xOffset < width && yOffset >= 0 && yOffset < height) {
                            int index = (yOffset * width + xOffset) * channels + c;
                            int kernelIndex = (i + halfKernel) * kernelSize + (j + halfKernel);
                            newValue += image[index] * kernel[kernelIndex];
                        }
                    }
                }
                int outputIndex = (y * width + x) * channels + c;
                outputImage[outputIndex] = static_cast<uint8_t>(newValue);
            }
        }
    }

    // Copy the filtered image back to the original image
    for (size_t i = 0; i < image.size(); i++) {
        image[i] = outputImage[i];
    }

    delete[] kernel;
}

int main() {
    BMPInfoHeader infoHeader;
    vector<uint8_t> imageData = readBMP("picture.bmp", infoHeader);

    if (imageData.empty()) {
        return 1;
    }

    BMPHeader header;
    ifstream("picture.bmp", ios::binary).read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));
    cout << "Amount of memory allocated for loading the image: " << infoHeader.dataSize << " bytes" << endl;

    // Rotate the image to the right
    vector<uint8_t> rotatedImageDataRight = rotateBMPRight(imageData, infoHeader);
    BMPInfoHeader rotatedInfoHeaderRight = infoHeader;
    rotatedInfoHeaderRight.width = infoHeader.height;
    rotatedInfoHeaderRight.height = infoHeader.width;
    rotatedInfoHeaderRight.dataSize = rotatedImageDataRight.size();
    writeBMP("rotated1_right.bmp", header, rotatedInfoHeaderRight, rotatedImageDataRight);
    cout << "Image rotated to the right and saved as 'rotated1_right.bmp'." << endl;

    // Rotate the image to the left
    vector<uint8_t> rotatedImageDataLeft = rotateBMPLeft(imageData, infoHeader);
    BMPInfoHeader rotatedInfoHeaderLeft = infoHeader;
    rotatedInfoHeaderLeft.width = infoHeader.height;
    rotatedInfoHeaderLeft.height = infoHeader.width;
    rotatedInfoHeaderLeft.dataSize = rotatedImageDataLeft.size();
    writeBMP("rotated2_left.bmp", header, rotatedInfoHeaderLeft, rotatedImageDataLeft);
    cout << "Image rotated to the left and saved as 'rotated2_left.bmp'." << endl;

    // Apply Gaussian blur to the 'rotated_left.bmp' image
    double sigma = 1.5; // Adjust the sigma value as needed
    applyGaussianBlur(rotatedImageDataLeft, rotatedInfoHeaderLeft.width, rotatedInfoHeaderLeft.height, 3, sigma);

    // Update the BMP header for the filtered image
    BMPInfoHeader filteredInfoHeader = rotatedInfoHeaderLeft;
    filteredInfoHeader.dataSize = rotatedImageDataLeft.size();

    // Save the filtered image as "rotated2_gaussian.bmp"
    writeBMP("rotated2_gaussian.bmp", header, filteredInfoHeader, rotatedImageDataLeft);
    cout << "Gaussian blur applied to 'rotated2_left.bmp' and saved as 'rotated2_gaussian.bmp'." << endl;

    return 0;
}
