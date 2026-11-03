#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <limits>
#include <cctype>

using namespace std;

// Read PGM header tokens and skip comments
bool readToken(ifstream &file, string &token)
{
    token.clear();
    char ch;

    while (file.get(ch))
    {
        if (isspace(static_cast<unsigned char>(ch)))
            continue;

        if (ch == '#')
        {
            string comment;
            getline(file, comment);
            continue;
        }

        token += ch;
        break;
    }

    if (token.empty())
        return false;

    while (file.get(ch))
    {
        if (isspace(static_cast<unsigned char>(ch)))
            break;

        if (ch == '#')
        {
            string comment;
            getline(file, comment);
            break;
        }

        token += ch;
    }

    return true;
}

int main()
{
    cout << "====================================\n";
    cout << "        MY ASCII PHOTO ART\n";
    cout << "====================================\n\n";

    string filename;

    cout << "Enter photo filename (e.g. photo.pgm): ";
    cin >> filename;

    ifstream file(filename.c_str(), ios::binary);

    if (!file.is_open())
    {
        cout << "\nError: Photo file not found!\n";
        cout << "Place photo.pgm in the program folder.\n";
        return 1;
    }

    string format, token;

    if (!readToken(file, format))
    {
        cout << "Error: Empty file.\n";
        return 1;
    }

    if (format != "P2" && format != "P5")
    {
        cout << "Error: Only P2 and P5 PGM files are supported.\n";
        return 1;
    }

    int width, height, maxValue;

    if (!readToken(file, token))
        return 1;
    width = atoi(token.c_str());

    if (!readToken(file, token))
        return 1;
    height = atoi(token.c_str());

    if (!readToken(file, token))
        return 1;
    maxValue = atoi(token.c_str());

    if (width <= 0 || height <= 0 ||
        maxValue <= 0 || maxValue > 65535 ||
        1LL * width * height > 50000000)
    {
        cout << "Error: Invalid image dimensions or pixel range.\n";
        return 1;
    }

    vector<int> pixels(width * height);

    if (format == "P2")
    {
        for (int i = 0; i < width * height; i++)
        {
            if (!readToken(file, token))
            {
                cout << "Error: Incomplete P2 image data.\n";
                return 1;
            }

            pixels[i] = atoi(token.c_str());
        }
    }
    else
    {
        // P5 stores pixel values as binary data.
        // Values below 256 use one byte; larger values use two bytes.
        int bytesPerPixel = (maxValue < 256) ? 1 : 2;

        for (int i = 0; i < width * height; i++)
        {
            unsigned char firstByte, secondByte;

            if (!file.read(reinterpret_cast<char *>(&firstByte), 1))
            {
                cout << "Error: Incomplete P5 image data.\n";
                return 1;
            }

            int value = firstByte;

            if (bytesPerPixel == 2)
            {
                if (!file.read(reinterpret_cast<char *>(&secondByte), 1))
                {
                    cout << "Error: Incomplete 16-bit image data.\n";
                    return 1;
                }

                value = firstByte * 256 + secondByte;
            }

            pixels[i] = value;
        }
    }

    file.close();

    // Characters from darkest to brightest
    string ascii = "@%#*+=-:. ";

    // Limit output width for terminal readability
    int outputWidth = 100;
    int step = max(1, width / outputWidth);

    ofstream output("portrait.txt");

    if (!output.is_open())
    {
        cout << "Error: Cannot create portrait.txt\n";
        return 1;
    }

    cout << "\nConverting image to ASCII...\n\n";

    for (int y = 0; y < height; y += step)
    {
        string line;

        for (int x = 0; x < width; x += step)
        {
            int brightness = pixels[y * width + x];

            brightness = max(0, min(maxValue, brightness));

            // Normalize brightness to the range 0-255
            int normalized = brightness * 255 / maxValue;

            int index = normalized * (ascii.size() - 1) / 255;

            line += ascii[index];
        }

        output << line << '\n';
        cout << line << '\n';
    }

    output.close();

    cout << "\n====================================\n";
    cout << "ASCII portrait created successfully!\n";
    cout << "Saved as: portrait.txt\n";
    cout << "====================================\n";

    return 0;
}
