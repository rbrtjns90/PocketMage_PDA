#pragma once

#include <QWidget>
#include <QImage>
#include <QColor>
#include <QPoint>
#include <QRect>

class PixelCanvas : public QWidget {
    Q_OBJECT

public:
    explicit PixelCanvas(QWidget *parent = nullptr);
    
    void setPixelSize(int size);
    int pixelSize() const { return m_pixelSize; }
    void setCurrentColor(const QColor &color);
    QColor currentColor() const { return m_currentColor; }
    
    void clear();
    void newImage(int width, int height);
    bool loadImage(const QString &path);
    bool loadRawBinary(const QString &path);
    bool saveImage(const QString &path);
    bool saveBMP(const QString &path);
    bool saveCppHeader(const QString &path, const QString &varName);
    bool saveRawBinary(const QString &path);
    void invertColors();
    
    QImage image() const { return m_image; }
    void setImage(const QImage &image);
    
    // Tools
    enum Tool { Pencil, Eraser, Fill, Line, Rectangle, Ellipse, Eyedropper, Text, Select };
    void setTool(Tool tool);
    Tool tool() const { return m_tool; }
    
    // Selection
    void clearSelection();
    
    // Text tool
    void setTextCursor(const QPoint &pixel);
    QPoint textCursor() const { return m_textCursor; }

signals:
    void colorPicked(const QColor &color);
    void imageModified();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    bool event(QEvent *event) override;  // For pinch gestures

private:
    QPoint pixelAt(const QPoint &pos) const;
    void drawPixel(const QPoint &pixel);
    void floodFill(const QPoint &start, const QColor &targetColor, const QColor &fillColor);
    void drawLine(const QPoint &start, const QPoint &end);
    void drawRect(const QPoint &start, const QPoint &end, bool filled);
    void drawEllipse(const QPoint &start, const QPoint &end, bool filled);
    
    void drawChar(const QPoint &pos, char c);
    
    QImage m_image;
    QImage m_previewImage;  // For shape preview
    int m_pixelSize = 12;
    QColor m_currentColor = Qt::black;
    Tool m_tool = Pencil;
    
    bool m_drawing = false;
    QPoint m_startPoint;
    QPoint m_lastPoint;
    QPoint m_textCursor = QPoint(0, 0);
    bool m_textMode = false;
    
    // Selection state
    QRect m_selection;
    QImage m_selectionContent;
    bool m_hasSelection = false;
    bool m_movingSelection = false;
    QPoint m_selectionOffset;
};
