#include "rgbimage.h"
#include "color.h"
#include "assert.h"

RGBImage::RGBImage(unsigned int Width, unsigned int Height)
    : m_Width(Width), m_Height(Height) {
    m_Image = new Color[Width * Height];
}

RGBImage::~RGBImage() {
    delete[] m_Image;
}

void RGBImage::setPixelColor(unsigned int x, unsigned int y, const Color &c) {
    if (x >= m_Width || y >= m_Height) {
        return; // Ungültige Koordinaten
    }
    m_Image[y * m_Width + x] = c;
}
RGBImage& RGBImage::SobelFilter(RGBImage& dst, const RGBImage& src, float factor) {
    assert(dst.m_Width == src.m_Width && dst.m_Height == src.m_Height);

    // Sobel-Kernel für X-Richtung (horizontaler Gradient)
    float Gx[3][3] = {
        { 1,  0, -1 },
        { 2,  0, -2 },
        { 1,  0, -1 }
    };

    // Sobel-Kernel für Y-Richtung (vertikaler Gradient)
    float Gy[3][3] = {
        {  1,  2,  1 },
        {  0,  0,  0 },
        { -1, -2, -1 }
    };

    // Über jedes Pixel im Bild laufen
    for (int y = 1; y < src.m_Height - 1; y++) {
        for (int x = 1; x < src.m_Width - 1; x++) {
            Color sumX, sumY;

            // 3x3 Nachbarschaft anwenden
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    Color c = src.getPixelColor(x + i, y + j);

                    sumX += c * Gx[j + 1][i + 1];  // X-Richtung
                    sumY += c * Gy[j + 1][i + 1];  // Y-Richtung
                }
            }

            // Betrag des Gradienten berechnen (Länge)
            Color gradient;
            gradient.R = sqrt(sumX.R * sumX.R + sumY.R * sumY.R);
            gradient.G = sqrt(sumX.G * sumX.G + sumY.G * sumY.G);
            gradient.B = sqrt(sumX.B * sumX.B + sumY.B * sumY.B);

            // Ergebnis mit Faktor multiplizieren (Verstärkung des Kontrasts)
            dst.setPixelColor(x, y, gradient * factor);
        }
    }

    return dst;
}

const Color &RGBImage::getPixelColor(unsigned int x, unsigned int y) const {
    assert(x < m_Width && y < m_Height); // Abbruch bei ungültigen Koordinaten
    return m_Image[y * m_Width + x];
}

unsigned int RGBImage::width() const {
    return m_Width;
}

unsigned int RGBImage::height() const {
    return m_Height;
}

unsigned char RGBImage::convertColorChannel(float f) {
    if (f < 0.0f) {
        f = 0.0f;
    } else if (f > 1.0f) {
        f = 1.0f;
    }
    return static_cast<unsigned char>(f * 255.0f);
}

bool RGBImage::saveToDisk(const char *Filename) {
    FILE *file = fopen(Filename, "wb");
    if (!file) {
        return false;
    }

    // BMP Header erstellen
    unsigned int padding = (4 - (m_Width * 3) % 4) % 4;
    // Jede Zeile muss auf ein Vielfaches von 4 Bytes gepolstert werden
    unsigned int fileSize = 54 + (3 * m_Width + padding) * m_Height;

    unsigned char bmpHeader[54] = {
        'B', 'M',           // Magic Number
        0, 0, 0, 0,         // Dateigröße (wird später gesetzt)
        0, 0, 0, 0,         // Reserviert
        54, 0, 0, 0,        // Offset, ab dem die Bilddaten beginnen (54 Bytes)
        40, 0, 0, 0,        // Headergröße (40 Bytes)
        0, 0, 0, 0,         // Breite des Bildes (wird später gesetzt)
        0, 0, 0, 0,         // Höhe des Bildes (wird später gesetzt)
        1, 0,               // Anzahl der Farbebenen
        24, 0,              // Bits pro Pixel (24 = RGB)
        0, 0, 0, 0,         // Kompression (keine Kompression)
        0, 0, 0, 0,         // Bildgröße (kann 0 sein, da keine Kompression)
        0x13, 0x0B, 0, 0,   // Horizontale Auflösung (2835 pixels/meter)
        0x13, 0x0B, 0, 0,   // Vertikale Auflösung (2835 pixels/meter)
        0, 0, 0, 0,         // Anzahl der Farben in der Farbpalette (Standard: 0)
        0, 0, 0, 0          // Wichtige Farben (Standard: 0)
    };

    // Dateigröße in den Header setzten
    bmpHeader[2] = (unsigned char) (fileSize);
    bmpHeader[3] = (unsigned char) (fileSize >> 8);
    bmpHeader[4] = (unsigned char) (fileSize >> 16);
    bmpHeader[5] = (unsigned char) (fileSize >> 24);

    // Breite in den Header setzten
    bmpHeader[18] = (unsigned char) (m_Width);
    bmpHeader[19] = (unsigned char) (m_Width >> 8);
    bmpHeader[20] = (unsigned char) (m_Width >> 16);
    bmpHeader[21] = (unsigned char) (m_Width >> 24);

    // Höhe in den Header setzten
    bmpHeader[22] = (unsigned char) (m_Height);
    bmpHeader[23] = (unsigned char) (m_Height >> 8);
    bmpHeader[24] = (unsigned char) (m_Height >> 16);
    bmpHeader[25] = (unsigned char) (m_Height >> 24);

    // Header schreiben
    fwrite(bmpHeader, 1, 54, file);

    // Schreibe Pixeldaten (BGR-Format, Zeilen von Unten -> Oben gespeichert)
    unsigned char pad[3] = {0, 0, 0};
    for (int y = m_Height - 1; y >= 0; y--) {
        for (unsigned int x = 0; x < m_Width; x++) {
            const Color &c = getPixelColor(x, y);
            unsigned char r = convertColorChannel(c.R);
            unsigned char g = convertColorChannel(c.G);
            unsigned char b = convertColorChannel(c.B);

            fwrite(&b, 1, 1, file);
            fwrite(&g, 1, 1, file);
            fwrite(&r, 1, 1, file);
        }
        fwrite(pad, 1, padding, file); // Padding am Ende jeder Zeile
    }

    fclose(file);
    return true;
}
