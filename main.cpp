#include <iostream>
#include <fstream>

using namespace std;

void applyGaussianBlur(unsigned char* image, int width, int height, int channels, double sigma) {
    int kernelSize = static_cast<int>(6 * sigma);
    if (kernelSize % 2 == 0) {
        // Ensure the kernel size is odd
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

    unsigned char* outputImage = new unsigned char[width * height * channels];

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
                outputImage[outputIndex] = static_cast<unsigned char>(newValue);
            }
        }
    }

    // Copy the filtered image back to the original image array
    for (int i = 0; i < width * height * channels; i++) {
        image[i] = outputImage[i];
    }

    delete[] outputImage;
    delete[] kernel;
}

int main() {

    ifstream is("picture.bmp", ifstream::binary);
    if (!is) {
        cerr << "Error opening the file." << endl; // Display an error message in the standard error stream
        return 1;
    }

    // Determine the file length correctly
    is.seekg(0, std::ios::end);
    std::streamoff length = is.tellg();
    is.seekg(0, std::ios::beg);

    cout << "Amount of memory allocated for loading the image: " << length << " bits" << endl;

    // Determine the size and color depth of the image
    is.seekg(18);  // Move the pointer to the 18th byte, where width and height data is located
    int width, height;
    is.read(reinterpret_cast<char*>(&width), sizeof(width));
    is.read(reinterpret_cast<char*>(&height), sizeof(height));

    // Read pixels from the original file and rotate the image clockwise
    int bufferSize = width * height * 3; // Each pixel has 3 bytes (RGB)
    char* buffer = new char[bufferSize];
    is.seekg(54);  // Move the pointer to the beginning of pixel data (byte 54)
    is.read(buffer, bufferSize);

    // Create a new buffer for the rotated image, with dimensions swapped
    int newWidth = height; // Swap width and height
    int newHeight = width;
    int newBufferSize = newWidth * newHeight * 3; // Each pixel has 3 bytes (RGB)
    char* newBuffer = new char[newBufferSize];

    // Rotate the image 90 degrees clockwise
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            int newIndex = j * newWidth + (newWidth - i - 1); // Correct rotation
            int oldIndex = (i * width + j) * 3; // Each pixel has 3 bytes (RGB)

            // Ensure newIndex and oldIndex are within bounds
            if (newIndex >= 0 && newIndex < newBufferSize && oldIndex >= 0 && oldIndex < bufferSize) {
                // Copy RGB values
                for (int k = 0; k < 3; ++k) {
                    newBuffer[newIndex * 3 + k] = buffer[oldIndex + k];
                }
            }
            else {
                cerr << "Error: Buffer index out of bounds." << endl;
                delete[] buffer;
                delete[] newBuffer;
                return 1;
            }
        }
    }

    // Close the original file
    is.close();

    // Create and save a file with the rotated image (clockwise)
    ofstream os("Rotated1.bmp", ofstream::binary);
    if (!os) {
        cerr << "Error creating a file for saving." << endl;
        delete[] buffer;
        delete[] newBuffer;
        return 1;
    }

    // Copy the BMP file header
    is.open("picture.bmp", ifstream::binary);
    os << is.rdbuf();
    is.close();

    // Update the width and height in the header with new values
    os.seekp(18);  // Move the pointer to the 18th byte
    os.write(reinterpret_cast<char*>(&newWidth), sizeof(newWidth));
    os.write(reinterpret_cast<char*>(&newHeight), sizeof(newHeight));

    // Write the rotated pixels to the file
    os.seekp(54);  // Move the pointer to the beginning of pixel data
    os.write(newBuffer, newBufferSize);

    // Close files and free memory
    os.close();

    // Create and save a file with the rotated image (counterclockwise)
    ofstream os2("Rotated2.bmp", ofstream::binary);
    if (!os2) {
        cerr << "Error creating a file for saving the counterclockwise rotated image." << endl;
        delete[] buffer;
        delete[] newBuffer;
        return 1;
    }

    // Copy the BMP file header
    is.open("Rotated1.bmp", ifstream::binary); // Open the previously saved image
    os2 << is.rdbuf();
    is.close();

    // Update the width and height in the header with new values
    os2.seekp(18);
    os2.write(reinterpret_cast<char*>(&newHeight), sizeof(newHeight));
    os2.write(reinterpret_cast<char*>(&newWidth), sizeof(newWidth));

    // Write the counterclockwise rotated pixels to the file
    os2.seekp(54);
    os2.write(buffer, bufferSize); // Use the original buffer data

    // Close files and free memory
    os2.close();
    delete[] buffer;
    delete[] newBuffer;

    cout << "The image has been successfully rotated 90 degrees clockwise and counterclockwise and saved to files Rotated1.bmp and Rotated2.bmp." << endl;

    ifstream is2("Rotated2.bmp", ifstream::binary);
    if (!is2) {
        cerr << "Error opening the file." << endl;
        return 1;
    }

    // Load the BMP header data
    char header2[54];
    is2.read(header2, 54);

    // Determine the image dimensions and color depth from the header
    int width2 = *(int*)&header2[18];
    int height2 = *(int*)&header2[22];
    int channels2 = 3; // Assuming RGB color format

    // Read the image data
    unsigned char* image2 = new unsigned char[width2 * height2 * channels2];
    is2.read(reinterpret_cast<char*>(image2), width2 * height2 * channels2);

    // Apply Gaussian blur
    double sigma2 = 1.0; // Adjust the value as needed
    applyGaussianBlur(image2, width2, height2, channels2, sigma2);

    // Create and save a file with the Gaussian filtered image
    ofstream os3("Gaussian.bmp", ofstream::binary);
    if (!os3) {
        cerr << "Error creating a file for saving." << endl;
        delete[] image2;
        return 1;
    }

    // Write the BMP header
    os3.write(header2, 54);

    // Write the filtered image data
    os3.write(reinterpret_cast<char*>(image2), width2 * height2 * channels2);

    // Close files and free memory
    os3.close();
    delete[] image2;

    cout << "Gaussian filter applied and saved to Gaussian.bmp." << endl;

    return 0;
}
