#include "pixelcanvas.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QGestureEvent>
#include <QQueue>
#include <QFile>
#include <QTextStream>
#include <cmath>

// 5x7 pixel font for characters (compact bitmap font)
static const uint8_t FONT_5X7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // space
    {0x00,0x00,0x5F,0x00,0x00}, // !
    {0x00,0x07,0x00,0x07,0x00}, // "
    {0x14,0x7F,0x14,0x7F,0x14}, // #
    {0x24,0x2A,0x7F,0x2A,0x12}, // $
    {0x23,0x13,0x08,0x64,0x62}, // %
    {0x36,0x49,0x55,0x22,0x50}, // &
    {0x00,0x05,0x03,0x00,0x00}, // '
    {0x00,0x1C,0x22,0x41,0x00}, // (
    {0x00,0x41,0x22,0x1C,0x00}, // )
    {0x08,0x2A,0x1C,0x2A,0x08}, // *
    {0x08,0x08,0x3E,0x08,0x08}, // +
    {0x00,0x50,0x30,0x00,0x00}, // ,
    {0x08,0x08,0x08,0x08,0x08}, // -
    {0x00,0x60,0x60,0x00,0x00}, // .
    {0x20,0x10,0x08,0x04,0x02}, // /
    {0x3E,0x51,0x49,0x45,0x3E}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 6
    {0x01,0x71,0x09,0x05,0x03}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x06,0x49,0x49,0x29,0x1E}, // 9
    {0x00,0x36,0x36,0x00,0x00}, // :
    {0x00,0x56,0x36,0x00,0x00}, // ;
    {0x00,0x08,0x14,0x22,0x41}, // <
    {0x14,0x14,0x14,0x14,0x14}, // =
    {0x41,0x22,0x14,0x08,0x00}, // >
    {0x02,0x01,0x51,0x09,0x06}, // ?
    {0x32,0x49,0x79,0x41,0x3E}, // @
    {0x7E,0x11,0x11,0x11,0x7E}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x01,0x01}, // F
    {0x3E,0x41,0x41,0x51,0x32}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x04,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x7F,0x20,0x18,0x20,0x7F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x03,0x04,0x78,0x04,0x03}, // Y
    {0x61,0x51,0x49,0x45,0x43}, // Z
};

static const int FONT_WIDTH = 5;
static const int FONT_HEIGHT = 7;

PixelCanvas::PixelCanvas(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
    grabGesture(Qt::PinchGesture);
    setFocusPolicy(Qt::StrongFocus);
    newImage(40, 40);
    setMinimumSize(40 * m_pixelSize, 40 * m_pixelSize);
}

void PixelCanvas::setPixelSize(int size) {
    m_pixelSize = qMax(1, size);
    setMinimumSize(m_image.width() * m_pixelSize, m_image.height() * m_pixelSize);
    update();
}

void PixelCanvas::setCurrentColor(const QColor &color) {
    m_currentColor = color;
}

void PixelCanvas::setTool(Tool tool) {
    if (m_tool == Select && tool != Select && m_hasSelection) {
        // Commit selection when switching away from select tool
        clearSelection();
    }
    m_tool = tool;
    m_textMode = false;
    update();
}

void PixelCanvas::clearSelection() {
    if (m_hasSelection && !m_selectionContent.isNull()) {
        // Paste selection content at current position
        for (int y = 0; y < m_selectionContent.height(); ++y) {
            for (int x = 0; x < m_selectionContent.width(); ++x) {
                int px = m_selection.x() + x;
                int py = m_selection.y() + y;
                if (px >= 0 && px < m_image.width() && py >= 0 && py < m_image.height()) {
                    QColor c = m_selectionContent.pixelColor(x, y);
                    if (c.alpha() > 0) {
                        m_image.setPixelColor(px, py, c);
                    }
                }
            }
        }
        emit imageModified();
    }
    m_hasSelection = false;
    m_movingSelection = false;
    m_selection = QRect();
    m_selectionContent = QImage();
    update();
}

void PixelCanvas::clear() {
    m_image.fill(Qt::white);
    emit imageModified();
    update();
}

void PixelCanvas::newImage(int width, int height) {
    m_image = QImage(width, height, QImage::Format_ARGB32);
    m_image.fill(Qt::white);
    m_previewImage = m_image;
    setMinimumSize(width * m_pixelSize, height * m_pixelSize);
    resize(width * m_pixelSize, height * m_pixelSize);
    emit imageModified();
    update();
}

bool PixelCanvas::loadImage(const QString &path) {
    QImage img(path);
    if (img.isNull()) return false;
    
    m_image = img.convertToFormat(QImage::Format_ARGB32);
    m_previewImage = m_image;
    setMinimumSize(m_image.width() * m_pixelSize, m_image.height() * m_pixelSize);
    resize(m_image.width() * m_pixelSize, m_image.height() * m_pixelSize);
    emit imageModified();
    update();
    return true;
}

bool PixelCanvas::loadRawBinary(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file:" << path;
        return false;
    }
    
    // Format: 1-bit packed pixel data only (no header)
    // Determine dimensions from file size
    qint64 fileSize = file.size();
    
    // Try common icon sizes: 40x40 (200 bytes), 32x32 (128 bytes), 64x64 (512 bytes)
    int width = 40;
    int height = 40;
    
    if (fileSize == 200) {
        width = height = 40;
    } else if (fileSize == 128) {
        width = height = 32;
    } else if (fileSize == 512) {
        width = height = 64;
    } else {
        // Try to guess square dimensions
        int pixels = fileSize * 8;
        int side = (int)sqrt(pixels);
        if (side * side == pixels) {
            width = height = side;
        } else {
            qDebug() << "Cannot determine dimensions from file size:" << fileSize;
            file.close();
            return false;
        }
    }
    
    qDebug() << "Loading binary file as" << width << "x" << height << "(" << fileSize << "bytes)";
    
    // Create new image
    m_image = QImage(width, height, QImage::Format_ARGB32);
    m_image.fill(Qt::white);
    
    // Read pixel data (1-bit monochrome, packed)
    int bytesPerRow = (width + 7) / 8;
    
    for (int y = 0; y < height; ++y) {
        for (int byteX = 0; byteX < bytesPerRow; ++byteX) {
            uint8_t byte;
            if (file.read(reinterpret_cast<char*>(&byte), 1) != 1) {
                file.close();
                return false;
            }
            
            for (int bit = 0; bit < 8; ++bit) {
                int x = byteX * 8 + bit;
                if (x < width) {
                    // Black pixel = 1, white = 0
                    bool isBlack = (byte & (0x80 >> bit)) != 0;
                    m_image.setPixelColor(x, y, isBlack ? Qt::black : Qt::white);
                }
            }
        }
    }
    
    file.close();
    
    m_previewImage = m_image;
    setMinimumSize(m_image.width() * m_pixelSize, m_image.height() * m_pixelSize);
    resize(m_image.width() * m_pixelSize, m_image.height() * m_pixelSize);
    emit imageModified();
    update();
    return true;
}

bool PixelCanvas::saveImage(const QString &path) {
    return m_image.save(path);
}

bool PixelCanvas::saveBMP(const QString &path) {
    // Save as 24-bit BMP for PocketMage compatibility
    QImage bmp = m_image.convertToFormat(QImage::Format_RGB888);
    return bmp.save(path, "BMP");
}

bool PixelCanvas::saveCppHeader(const QString &path, const QString &varName) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    // Convert to 1-bit monochrome (black = 1, white = 0)
    int width = m_image.width();
    int height = m_image.height();
    int bytesPerRow = (width + 7) / 8;
    
    out << "// Generated by PocketMage Icon Editor\n";
    out << "// Size: " << width << "x" << height << " pixels\n";
    out << "#pragma once\n\n";
    out << "const unsigned char " << varName << "[] PROGMEM = {\n";
    
    for (int y = 0; y < height; ++y) {
        out << "    ";
        for (int byteX = 0; byteX < bytesPerRow; ++byteX) {
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; ++bit) {
                int x = byteX * 8 + bit;
                if (x < width) {
                    QColor c = m_image.pixelColor(x, y);
                    // Black pixel = 1, white = 0
                    if (c.lightness() < 128) {
                        byte |= (0x80 >> bit);
                    }
                }
            }
            out << "0x" << QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
            if (y < height - 1 || byteX < bytesPerRow - 1) {
                out << ", ";
            }
        }
        out << "\n";
    }
    
    out << "};\n\n";
    out << "const int " << varName << "_width = " << width << ";\n";
    out << "const int " << varName << "_height = " << height << ";\n";
    
    file.close();
    return true;
}

bool PixelCanvas::saveRawBinary(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    // Format: 1-bit packed pixel data only (no header)
    // This matches the format expected by PocketMage AppLauncher
    int width = m_image.width();
    int height = m_image.height();
    int bytesPerRow = (width + 7) / 8;
    
    // Write pixel data (1-bit monochrome, black = 1)
    for (int y = 0; y < height; ++y) {
        for (int byteX = 0; byteX < bytesPerRow; ++byteX) {
            uint8_t byte = 0;
            for (int bit = 0; bit < 8; ++bit) {
                int x = byteX * 8 + bit;
                if (x < width) {
                    QColor c = m_image.pixelColor(x, y);
                    if (c.lightness() < 128) {
                        byte |= (0x80 >> bit);
                    }
                }
            }
            file.write(reinterpret_cast<char*>(&byte), 1);
        }
    }
    
    file.close();
    return true;
}

void PixelCanvas::invertColors() {
    for (int y = 0; y < m_image.height(); ++y) {
        for (int x = 0; x < m_image.width(); ++x) {
            QColor c = m_image.pixelColor(x, y);
            m_image.setPixelColor(x, y, QColor(255 - c.red(), 255 - c.green(), 255 - c.blue()));
        }
    }
    emit imageModified();
    update();
}

void PixelCanvas::setImage(const QImage &image) {
    m_image = image.convertToFormat(QImage::Format_ARGB32);
    m_previewImage = m_image;
    emit imageModified();
    update();
}

void PixelCanvas::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    
    // Draw pixels - use preview image only for shape tools during drawing
    bool usePreview = m_drawing && (m_tool == Line || m_tool == Rectangle || m_tool == Ellipse);
    const QImage &drawImage = usePreview ? m_previewImage : m_image;
    for (int y = 0; y < drawImage.height(); ++y) {
        for (int x = 0; x < drawImage.width(); ++x) {
            QColor color = drawImage.pixelColor(x, y);
            painter.fillRect(x * m_pixelSize, y * m_pixelSize, 
                           m_pixelSize, m_pixelSize, color);
        }
    }
    
    // Draw grid
    painter.setPen(QColor(200, 200, 200));
    for (int x = 0; x <= drawImage.width(); ++x) {
        painter.drawLine(x * m_pixelSize, 0, x * m_pixelSize, drawImage.height() * m_pixelSize);
    }
    for (int y = 0; y <= drawImage.height(); ++y) {
        painter.drawLine(0, y * m_pixelSize, drawImage.width() * m_pixelSize, y * m_pixelSize);
    }
    
    // Draw text cursor
    if (m_tool == Text && m_textMode) {
        painter.setPen(QPen(Qt::red, 2));
        int cx = m_textCursor.x() * m_pixelSize;
        int cy = m_textCursor.y() * m_pixelSize;
        painter.drawRect(cx, cy, FONT_WIDTH * m_pixelSize, FONT_HEIGHT * m_pixelSize);
    }
    
    // Draw selection
    if (m_tool == Select && (m_hasSelection || (m_drawing && !m_selection.isEmpty()))) {
        // Draw selection content if moving
        if (m_hasSelection && !m_selectionContent.isNull()) {
            for (int y = 0; y < m_selectionContent.height(); ++y) {
                for (int x = 0; x < m_selectionContent.width(); ++x) {
                    QColor c = m_selectionContent.pixelColor(x, y);
                    if (c.alpha() > 0) {
                        int px = (m_selection.x() + x) * m_pixelSize;
                        int py = (m_selection.y() + y) * m_pixelSize;
                        painter.fillRect(px, py, m_pixelSize, m_pixelSize, c);
                    }
                }
            }
        }
        
        // Draw selection rectangle (marching ants style)
        painter.setPen(QPen(Qt::blue, 2, Qt::DashLine));
        painter.drawRect(m_selection.x() * m_pixelSize, m_selection.y() * m_pixelSize,
                        m_selection.width() * m_pixelSize, m_selection.height() * m_pixelSize);
    }
}

QPoint PixelCanvas::pixelAt(const QPoint &pos) const {
    return QPoint(pos.x() / m_pixelSize, pos.y() / m_pixelSize);
}

void PixelCanvas::drawPixel(const QPoint &pixel) {
    if (pixel.x() >= 0 && pixel.x() < m_image.width() &&
        pixel.y() >= 0 && pixel.y() < m_image.height()) {
        QColor color = (m_tool == Eraser) ? Qt::white : m_currentColor;
        m_image.setPixelColor(pixel, color);
        emit imageModified();
        update();
    }
}

void PixelCanvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton) return;
    
    QPoint pixel = pixelAt(event->pos());
    m_startPoint = pixel;
    m_lastPoint = pixel;
    m_drawing = true;
    
    if (m_tool == Eyedropper) {
        if (pixel.x() >= 0 && pixel.x() < m_image.width() &&
            pixel.y() >= 0 && pixel.y() < m_image.height()) {
            emit colorPicked(m_image.pixelColor(pixel));
        }
        m_drawing = false;
    }
    else if (m_tool == Fill) {
        if (pixel.x() >= 0 && pixel.x() < m_image.width() &&
            pixel.y() >= 0 && pixel.y() < m_image.height()) {
            QColor targetColor = m_image.pixelColor(pixel);
            if (targetColor != m_currentColor) {
                floodFill(pixel, targetColor, m_currentColor);
            }
        }
        m_drawing = false;
    }
    else if (m_tool == Text) {
        setTextCursor(pixel);
        m_drawing = false;
    }
    else if (m_tool == Select) {
        if (m_hasSelection && m_selection.contains(pixel)) {
            // Start moving existing selection
            m_movingSelection = true;
            m_selectionOffset = pixel - m_selection.topLeft();
        } else {
            // Commit any existing selection and start new one
            if (m_hasSelection) {
                clearSelection();
            }
            m_previewImage = m_image;
        }
    }
    else if (m_tool == Pencil || m_tool == Eraser) {
        drawPixel(pixel);
    }
    else {
        m_previewImage = m_image;
    }
}

void PixelCanvas::mouseMoveEvent(QMouseEvent *event) {
    if (!m_drawing) return;
    
    QPoint pixel = pixelAt(event->pos());
    
    if (m_tool == Pencil || m_tool == Eraser) {
        // Draw line from last point to current for smooth drawing
        drawLine(m_lastPoint, pixel);
        m_lastPoint = pixel;
    }
    else if (m_tool == Line) {
        m_previewImage = m_image;
        QPainter painter(&m_previewImage);
        painter.setPen(m_currentColor);
        painter.drawLine(m_startPoint, pixel);
        update();
    }
    else if (m_tool == Rectangle) {
        m_previewImage = m_image;
        int x = qMin(m_startPoint.x(), pixel.x());
        int y = qMin(m_startPoint.y(), pixel.y());
        int w = qAbs(pixel.x() - m_startPoint.x()) + 1;
        int h = qAbs(pixel.y() - m_startPoint.y()) + 1;
        for (int py = y; py < y + h && py < m_previewImage.height(); ++py) {
            for (int px = x; px < x + w && px < m_previewImage.width(); ++px) {
                if (py == y || py == y + h - 1 || px == x || px == x + w - 1) {
                    if (px >= 0 && py >= 0) {
                        m_previewImage.setPixelColor(px, py, m_currentColor);
                    }
                }
            }
        }
        update();
    }
    else if (m_tool == Ellipse) {
        m_previewImage = m_image;
        int x = qMin(m_startPoint.x(), pixel.x());
        int y = qMin(m_startPoint.y(), pixel.y());
        int w = qAbs(pixel.x() - m_startPoint.x()) + 1;
        int h = qAbs(pixel.y() - m_startPoint.y()) + 1;
        float cx = x + w / 2.0f;
        float cy = y + h / 2.0f;
        float rx = w / 2.0f;
        float ry = h / 2.0f;
        for (int py = y; py < y + h && py < m_previewImage.height(); ++py) {
            for (int px = x; px < x + w && px < m_previewImage.width(); ++px) {
                float dx = (px - cx + 0.5f) / rx;
                float dy = (py - cy + 0.5f) / ry;
                float dist = dx * dx + dy * dy;
                if (dist <= 1.0f && dist >= 0.6f) {
                    if (px >= 0 && py >= 0) {
                        m_previewImage.setPixelColor(px, py, m_currentColor);
                    }
                }
            }
        }
        update();
    }
    else if (m_tool == Select) {
        if (m_movingSelection && m_hasSelection) {
            // Move selection
            QPoint newPos = pixel - m_selectionOffset;
            m_selection.moveTopLeft(newPos);
            update();
        } else if (!m_hasSelection) {
            // Draw selection rectangle preview
            int x = qMin(m_startPoint.x(), pixel.x());
            int y = qMin(m_startPoint.y(), pixel.y());
            int w = qAbs(pixel.x() - m_startPoint.x()) + 1;
            int h = qAbs(pixel.y() - m_startPoint.y()) + 1;
            m_selection = QRect(x, y, w, h);
            update();
        }
    }
}

void PixelCanvas::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() != Qt::LeftButton || !m_drawing) return;
    
    if (m_tool == Line || m_tool == Rectangle || m_tool == Ellipse) {
        m_image = m_previewImage;
        emit imageModified();
    }
    else if (m_tool == Select) {
        if (m_movingSelection) {
            m_movingSelection = false;
        } else if (!m_hasSelection && m_selection.width() > 0 && m_selection.height() > 0) {
            // Capture selection content and clear from image
            m_selectionContent = QImage(m_selection.width(), m_selection.height(), QImage::Format_ARGB32);
            m_selectionContent.fill(Qt::transparent);
            
            for (int y = 0; y < m_selection.height(); ++y) {
                for (int x = 0; x < m_selection.width(); ++x) {
                    int px = m_selection.x() + x;
                    int py = m_selection.y() + y;
                    if (px >= 0 && px < m_image.width() && py >= 0 && py < m_image.height()) {
                        m_selectionContent.setPixelColor(x, y, m_image.pixelColor(px, py));
                        m_image.setPixelColor(px, py, Qt::white);  // Clear original
                    }
                }
            }
            m_hasSelection = true;
            emit imageModified();
        }
    }
    
    m_drawing = false;
    update();
}

void PixelCanvas::wheelEvent(QWheelEvent *event) {
    int delta = event->angleDelta().y() > 0 ? 1 : -1;
    int step = qMax(1, m_pixelSize / 4);  // 25% relative step
    setPixelSize(m_pixelSize + delta * step);
}

void PixelCanvas::floodFill(const QPoint &start, const QColor &targetColor, const QColor &fillColor) {
    if (targetColor == fillColor) return;
    
    QQueue<QPoint> queue;
    queue.enqueue(start);
    
    while (!queue.isEmpty()) {
        QPoint p = queue.dequeue();
        
        if (p.x() < 0 || p.x() >= m_image.width() ||
            p.y() < 0 || p.y() >= m_image.height()) continue;
        
        if (m_image.pixelColor(p) != targetColor) continue;
        
        m_image.setPixelColor(p, fillColor);
        
        queue.enqueue(QPoint(p.x() + 1, p.y()));
        queue.enqueue(QPoint(p.x() - 1, p.y()));
        queue.enqueue(QPoint(p.x(), p.y() + 1));
        queue.enqueue(QPoint(p.x(), p.y() - 1));
    }
    
    emit imageModified();
    update();
}

void PixelCanvas::drawLine(const QPoint &start, const QPoint &end) {
    // Bresenham's line algorithm
    int x0 = start.x(), y0 = start.y();
    int x1 = end.x(), y1 = end.y();
    int dx = qAbs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -qAbs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    
    while (true) {
        drawPixel(QPoint(x0, y0));
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void PixelCanvas::setTextCursor(const QPoint &pixel) {
    m_textCursor = pixel;
    m_textMode = true;
    setFocus();
    update();
}

void PixelCanvas::drawChar(const QPoint &pos, char c) {
    int index = -1;
    
    if (c == ' ') index = 0;
    else if (c >= '!' && c <= '@') index = c - '!' + 1;
    else if (c >= 'A' && c <= 'Z') index = c - 'A' + 33;
    else if (c >= 'a' && c <= 'z') index = c - 'a' + 33;  // lowercase -> uppercase
    else if (c >= '0' && c <= '9') index = c - '0' + 16;
    
    if (index < 0 || index >= 59) return;
    
    const uint8_t *glyph = FONT_5X7[index];
    
    for (int col = 0; col < FONT_WIDTH; ++col) {
        uint8_t colData = glyph[col];
        for (int row = 0; row < FONT_HEIGHT; ++row) {
            if (colData & (1 << row)) {
                int px = pos.x() + col;
                int py = pos.y() + row;
                if (px >= 0 && px < m_image.width() && py >= 0 && py < m_image.height()) {
                    m_image.setPixelColor(px, py, m_currentColor);
                }
            }
        }
    }
    
    emit imageModified();
    update();
}

void PixelCanvas::keyPressEvent(QKeyEvent *event) {
    if (m_tool != Text || !m_textMode) {
        QWidget::keyPressEvent(event);
        return;
    }
    
    // Handle special keys first
    if (event->key() == Qt::Key_Backspace) {
        m_textCursor.setX(qMax(0, m_textCursor.x() - (FONT_WIDTH + 1)));
        update();
        event->accept();
        return;
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        m_textCursor.setX(0);
        m_textCursor.setY(m_textCursor.y() + FONT_HEIGHT + 1);
        update();
        event->accept();
        return;
    } else if (event->key() == Qt::Key_Escape) {
        m_textMode = false;
        update();
        event->accept();
        return;
    }
    
    QString text = event->text();
    if (!text.isEmpty()) {
        char c = text.at(0).toLatin1();
        if (c >= ' ' && c <= 'z') {
            drawChar(m_textCursor, c);
            m_textCursor.setX(m_textCursor.x() + FONT_WIDTH + 1);
            
            // Wrap to next line if needed
            if (m_textCursor.x() + FONT_WIDTH > m_image.width()) {
                m_textCursor.setX(0);
                m_textCursor.setY(m_textCursor.y() + FONT_HEIGHT + 1);
            }
            event->accept();
            return;
        }
    }
    
    QWidget::keyPressEvent(event);
}

bool PixelCanvas::event(QEvent *event) {
    static int pinchStartSize = 12;  // Store size at pinch start
    
    if (event->type() == QEvent::Gesture) {
        QGestureEvent *gestureEvent = static_cast<QGestureEvent*>(event);
        if (QPinchGesture *pinch = static_cast<QPinchGesture*>(gestureEvent->gesture(Qt::PinchGesture))) {
            if (pinch->state() == Qt::GestureStarted) {
                pinchStartSize = m_pixelSize;  // Remember starting size
            }
            if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged) {
                qreal totalScale = pinch->totalScaleFactor();
                int newSize = static_cast<int>(pinchStartSize * totalScale);  // Scale from start size
                newSize = qBound(2, newSize, 80);  // Wider range
                if (newSize != m_pixelSize) {
                    setPixelSize(newSize);
                }
            }
            return true;
        }
    }
    return QWidget::event(event);
}
