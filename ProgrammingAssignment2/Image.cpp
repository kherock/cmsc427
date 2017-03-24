#include "Image.hpp"

#include <stdio.h>
#include <string.h>
#include <fstream>

using namespace std;

Image::Image()
: npixels(0), width(0), height(0)
{}

Image::Image(const char *filename)
    : npixels(0), width(0), height(0)
{
    if (!Read(filename)){
        printf("Image not created");
    }
}

Image::~Image() {
    //delete image_data;
}

bool Image::Read(const char *filename)
{
    // load image file
    QImage image(QString(filename), "JPG");
    if (image.isNull()) {
        return IMAGE_RETURN_FAILURE;
    }
    // convert to 32-bit image where each pixel is a QRgb
    image_data = image.convertToFormat(QImage::Format_RGB32);
    if (image_data.isNull()) {
        return IMAGE_RETURN_FAILURE;
    }

    width = image_data.width();
    height = image_data.height();
    npixels = width * height;

    QImage new_data(500,1000, QImage::Format_RGB32);
    QColor color(50,50,50);
    new_data.setPixel(50, 87, color.rgb());

    return IMAGE_RETURN_SUCCESS;
}


bool Image::Write(const char *filename)
{
    if (image_data.save(QString(filename), "JPG")) {
        return IMAGE_RETURN_SUCCESS;
    } else {
        return IMAGE_RETURN_FAILURE;
    }
}


void Image::BilateralFilter(double rangesigma, double domainsigma)
{
    printf("Must implement BilateralFilter()\n");
}


void Image::BlackAndWhite()
{
    Saturation(0);  // Black & White is just Saturation 0
}


void Image::Brightness(double factor)
{
    if (factor < 0 || 2 < factor) {
        fputs("Brightness alpha factor must be in the range [0.0, 2.0]\n", stderr);
        exit(-1);
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            QColor color = QColor(image_data.pixel(x, y));
            color.setRed(qMin(qRound(color.red() * factor), 255));
            color.setGreen(qMin(qRound(color.green() * factor), 255));
            color.setBlue(qMin(qRound(color.blue() * factor), 255));
            image_data.setPixel(x, y, color.rgb());
        }
    }
}


void Image::ChannelExtract(int channel)
{
    if (channel < 0 || 4 < channel) {
        fputs("Channel must be one of 0=red, 1=green, 2=blue, 3=alpha\n", stderr);
        exit(-1);
    }
    // Create a mask for each pixel depending on the channel number
    // Do channel++ since alpha is stored first in QRgb
    if (channel++ == 3) channel = 0;
    QRgb mask = 0xff << (3 - channel) * 8;
    mask |= 0xff000000;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            QRgb rgb = image_data.pixel(x, y);
            image_data.setPixel(x, y, rgb & mask);
        }
    }
}


void Image::Composite()
{
    printf("Must implement Composite()\n");
}


void Image::Contrast(double factor)
{
    if (factor < -1 || 2 < factor) {
        fputs("Contrast alpha factor must be in the range [-1.0, 2.0]\n", stderr);
        exit(-1);
    }
    double averageLum = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            QColor color = QColor(image_data.pixel(x, y));
            double lum = 0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue();
            // Recompute the average
            averageLum = (averageLum * (y * x + x) + lum) / (y * x + x + 1);
        }
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            QColor color = QColor(image_data.pixel(x, y));
            // Do a linear transform on each channel based on the difference from the average luminance
            color.setRed(qBound(0, qRound(averageLum + (color.red() - averageLum) * factor), 255));
            color.setGreen(qBound(0, qRound(averageLum + (color.green() - averageLum) * factor), 255));
            color.setBlue(qBound(0, qRound(averageLum + (color.blue() - averageLum) * factor), 255));
            image_data.setPixel(x, y, color.rgb());
        }
    }
}


void Image::Crop(int top_left_x, int top_left_y, int crop_width, int crop_height)
{
    if (crop_width < 0 || crop_height < 0) {
        fputs("Width and height must be nonnegative\n", stderr);
        exit(-1);
    }
    QImage cropped = QImage(crop_width, crop_height, QImage::Format_RGB32);
    for (int y = 0; y < crop_height; y++) {
        for (int x = 0; x < crop_width; x++) {
            QRgb color = top_left_x + x < 0 || width <= top_left_x + x
                    || top_left_y + y < 0 || height <= top_left_y + y
                ? 0
                : image_data.pixel(top_left_x + x, top_left_y + y);
            cropped.setPixel(x, y, color);
        }
    }
    image_data = cropped;
    width = crop_width;
    height = crop_height;
}


void Image::Fun(int sampling_method)
{
    printf("Must implement Fun()\n");
}


void Image::Gamma(double factor)
{
    printf("Must implement Gamma()\n");
}


void Image::GaussianBlur(double sigma)
{
    printf("Must implement GaussianBlur()\n");
}


void Image::MedianFilter(int filter_width)
{
    printf("Must implement MedianFilter()\n");
}


void Image::MotionBlur(double sigma)
{
    if (sigma <= 0) {
        fputs("Blur factor must be a positive real value\n", stderr);
        exit(-1);
    }
    int radius = qCeil(3 * sigma);
    double variance2 = sigma * sigma * 2.0;
    // Use the left half of a one-dimensional Gaussian kernel
    double *transform = (double *)malloc(radius * sizeof(double));
    for (int i = 0; i < radius; i++) {
        double term = radius - 0.5 - i;
        transform[i] = 2 * exp(-(term * term) / variance2) / sqrt(M_PI * variance2);
    }
    QImage blurred = QImage(width, height, QImage::Format_RGB32);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            QColor newColor = QColor(0, 0, 0);
            for (int i = 0; i < radius; i++) {
                QColor color = QColor(image_data.pixel(qMax(0, x-radius+i+1), y));
                newColor.setRed(qRound(newColor.red() + color.red() * transform[i]));
                newColor.setGreen(qRound(newColor.green() + color.green() * transform[i]));
                newColor.setBlue(qRound(newColor.blue() + color.blue() * transform[i]));
            }
            blurred.setPixel(x, y, newColor.rgb());
        }
    }
    free(transform);
    image_data = blurred;
}


void Image::Nonphotorealism()
{
    printf("Must implement Nonphotorealism()\n");
}


void Image::Rotate(double angle, int sampling_method)
{
    if (angle < 0 || 360 < angle) {
        fputs("Rotation angle must be in the range [0, 360]\n", stderr);
        exit(-1);
    }
    QImage rotated = QImage(width, height, QImage::Format_RGB32);
    double dTheta = angle / 180 * M_PI;
    double cx = (width - 1) / 2,
           cy = (height - 1) / 2;
    switch (sampling_method) {
    case 0: // Point sampling
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                double r = sqrt((x - cx)*(x - cx) + (y - cy)*(y - cy));
                double theta = atan2(y - cy, x - cx);
                // Find the nearest pixel rotated -dTheta degrees in the original image
                int tx = qRound(r * qCos(theta - dTheta) + cx),
                    ty = qRound(r * qSin(theta - dTheta) + cy);
                QRgb rgb = tx < 0 || width <= tx || ty < 0 || height <= ty
                    ? 0 : image_data.pixel(tx, ty);
                rotated.setPixel(x, y, rgb);
            }
        }
        break;
    case 1: // Bilinear sampling
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                double r = sqrt((x - cx)*(x - cx) + (y - cy)*(y - cy));
                double theta = atan2(y - cy, x - cx);
                // Find the nearest pixel rotated -dTheta degrees in the original image
                double tx = r * qCos(theta - dTheta) + cx,
                       ty = r * qSin(theta - dTheta) + cy;
                QColor color = QColor();
                QColor q11 = QColor(qFloor(tx) < 0 || width <= qFloor(tx) || qFloor(ty) < 0 || height <= qFloor(ty)
                           ? 0 : image_data.pixel(qFloor(tx), qFloor(ty))),
                       q12 = QColor(qFloor(tx) < 0 || width <= qFloor(tx) || qCeil(ty) < 0 || height <= qCeil(ty)
                           ? 0 : image_data.pixel(qFloor(tx), qCeil(ty))),
                       q21 = QColor(qCeil(tx) < 0 || width <= qCeil(tx) || qFloor(ty) < 0 || height <= qFloor(ty)
                           ? 0 : image_data.pixel(qCeil(tx),  qFloor(ty))),
                       q22 = QColor(qCeil(tx) < 0 || width <= qCeil(tx) || qCeil(ty) < 0 || height <= qCeil(ty)
                           ? 0 : image_data.pixel(qCeil(tx),  qCeil(ty)));
                // [x2 - x, x - x1]
                double m1[2] = { qFloor(tx) + 1 - tx, tx - qFloor(tx) };
                // [y2 - y; y - y1]
                double m2[2] = { qFloor(ty) + 1 - ty, ty - qFloor(ty) };
                // https://en.wikipedia.org/wiki/Bilinear_interpolation
                color.setRed(
                    m1[0]*(q11.red() * m2[0] + q12.red() * m2[1]) +
                    m1[1]*(q21.red() * m2[0] + q22.red() * m2[1])
                );
                color.setGreen(
                    m1[0]*(q11.green() * m2[0] + q12.green() * m2[1]) +
                    m1[1]*(q21.green() * m2[0] + q22.green() * m2[1])
                );
                color.setBlue(
                    m1[0]*(q11.blue() * m2[0] + q12.blue() * m2[1]) +
                    m1[1]*(q21.blue() * m2[0] + q22.blue() * m2[1])
                );
                rotated.setPixel(x, y, color.rgb());
            }
        }
        break;
    case 2: // Gaussian sampling
        fputs("Must implement Gaussian sampling\n", stderr);
        break;
    default:
        fputs("Sampling method must be one of 0=point [default], 1=bilinear, 2=gaussian\n", stderr);
        exit(-1);
    }
    image_data = rotated;
    width = rotated.width();
    height = rotated.height();
}


void Image::Saturation(double factor)
{
    if (factor < -1 || 2.5 < factor) {
        fputs("Saturation factor must be in the range [-1.0, 2.5]\n", stderr);
        exit(-1);
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            QColor color = QColor(image_data.pixel(x, y));
            double lum = 0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue();
            color.setRed(qBound(0, qRound(lum + (color.red() - lum) * factor), 255));
            color.setGreen(qBound(0, qRound(lum + (color.green() - lum) * factor), 255));
            color.setBlue(qBound(0, qRound(lum + (color.blue() - lum) * factor), 255));
            image_data.setPixel(x, y, color.rgb());
        }
    }
}


void Image::Scale(double sx, double sy, int sampling_method)
{
    if (sx < 0.05 || 20 < sx || sy < 0.05 || 20 < sy) {
        fputs("Scaling factors must be in the range [0.05, 20]\n", stderr);
        exit(-1);
    }
    QImage resized = QImage(qRound(sx * width), qRound(sy * height), QImage::Format_RGB32);
    switch (sampling_method) {
    case 0: // Point sampling
        for (int y = 0; y < resized.height(); y++) {
            for (int x = 0; x < resized.width(); x++) {
                QRgb rgb = image_data.pixel(qMin(qRound(x / sx), width-1), qMin(qRound(y / sy), height-1));
                resized.setPixel(x, y, rgb);
            }
        }
        break;
    case 1: // Bilinear sampling
        for (int y = 0; y < resized.height(); y++) {
            for (int x = 0; x < resized.width(); x++) {
                QColor color = QColor();
                QColor q11 = QColor(image_data.pixel(qFloor(x / sx),               qFloor(y / sy))),
                       q12 = QColor(image_data.pixel(qFloor(x / sx),               qMin(qCeil(y / sy), height-1))),
                       q21 = QColor(image_data.pixel(qMin(qCeil(x / sx), width-1), qFloor(y / sy))),
                       q22 = QColor(image_data.pixel(qMin(qCeil(x / sx), width-1), qMin(qCeil(y / sy), height-1)));
                // [x2 - x, x - x1]
                double m1[2] = { qFloor(x / sx) + 1 - x / sx, x / sx - qFloor(x / sx) };
                // [y2 - y; y - y1]
                double m2[2] = { qFloor(y / sy) + 1 - y / sy, y / sy - qFloor(y / sy) };
                // https://en.wikipedia.org/wiki/Bilinear_interpolation
                color.setRed(
                    m1[0]*(q11.red() * m2[0] + q12.red() * m2[1]) +
                    m1[1]*(q21.red() * m2[0] + q22.red() * m2[1])
                );
                color.setGreen(
                    m1[0]*(q11.green() * m2[0] + q12.green() * m2[1]) +
                    m1[1]*(q21.green() * m2[0] + q22.green() * m2[1])
                );
                color.setBlue(
                    m1[0]*(q11.blue() * m2[0] + q12.blue() * m2[1]) +
                    m1[1]*(q21.blue() * m2[0] + q22.blue() * m2[1])
                );
                resized.setPixel(x, y, color.rgb());
            }
        }
        break;
    case 2: // Gaussian sampling
        fputs("Must implement Gaussian sampling\n", stderr);
        break;
    default:
        fputs("Sampling method must be one of 0=point [default], 1=bilinear, 2=gaussian\n", stderr);
        exit(-1);
    }
    image_data = resized;
    width = resized.width();
    height = resized.height();
}


void Image::Sharpen()
{
    QImage sharpened = QImage(width, height, QImage::Format_RGB32);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            QColor color = QColor();
            // Get pixels used by the kernel
            QColor q11 = QColor(image_data.pixel(qMax(0, x-1),       qMax(0, y-1))),
                   q12 = QColor(image_data.pixel(qMax(0, x-1),       y)),
                   q13 = QColor(image_data.pixel(qMax(0, x-1),       qMin(y+1, height-1))),
                   q21 = QColor(image_data.pixel(x,                  qMax(0, y-1))),
                   q22 = QColor(image_data.pixel(x,                  y)),
                   q23 = QColor(image_data.pixel(x,                  qMin(y+1, height-1))),
                   q31 = QColor(image_data.pixel(qMin(x+1, width-1), qMax(0, y-1))),
                   q32 = QColor(image_data.pixel(qMin(x+1, width-1), y)),
                   q33 = QColor(image_data.pixel(qMin(x+1, width-1), qMin(y+1, height-1)));
            color.setRed(qBound(0,
                -1 * q11.red() + -1 * q12.red() + -1 * q13.red() +
                -1 * q21.red() +  9 * q22.red() + -1 * q23.red() +
                -1 * q31.red() + -1 * q32.red() + -1 * q33.red(),
            255));
            color.setGreen(qBound(0,
                -1 * q11.green() + -1 * q12.green() + -1 * q13.green() +
                -1 * q21.green() +  9 * q22.green() + -1 * q23.green() +
                -1 * q31.green() + -1 * q32.green() + -1 * q33.green(),
            255));
            color.setBlue(qBound(0,
                -1 * q11.blue() + -1 * q12.blue() + -1 * q13.blue() +
                -1 * q21.blue() +  9 * q22.blue() + -1 * q23.blue() +
                -1 * q31.blue() + -1 * q32.blue() + -1 * q33.blue(),
            255));
            sharpened.setPixel(x, y, color.rgb());
        }
    }
    image_data = sharpened;
}
