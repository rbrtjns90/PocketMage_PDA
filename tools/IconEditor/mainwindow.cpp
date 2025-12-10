#include "mainwindow.h"
#include "pixelcanvas.h"

#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QColorDialog>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QFrame>
#include <QButtonGroup>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QComboBox>
#include <QtMath>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("PocketMage Icon Editor");
    resize(700, 600);
    
    // Create canvas
    m_canvas = new PixelCanvas(this);
    connect(m_canvas, &PixelCanvas::colorPicked, this, &MainWindow::onColorPicked);
    connect(m_canvas, &PixelCanvas::imageModified, this, &MainWindow::updateTitle);
    
    // Scroll area for canvas
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(m_canvas);
    scrollArea->setWidgetResizable(false);
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setStyleSheet("background-color: #404040;");
    
    setCentralWidget(scrollArea);
    
    createActions();
    createToolBar();
    createColorPalette();
    
    statusBar()->showMessage("Ready - 40x40 icon");
}

void MainWindow::createActions() {
    // File menu
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    QAction *newAction = fileMenu->addAction("&New", this, &MainWindow::newFile);
    newAction->setShortcut(QKeySequence::New);
    
    QAction *openAction = fileMenu->addAction("&Open...", this, &MainWindow::openFile);
    openAction->setShortcut(QKeySequence::Open);
    
    QAction *saveAction = fileMenu->addAction("&Save", this, &MainWindow::saveFile);
    saveAction->setShortcut(QKeySequence::Save);
    
    fileMenu->addAction("Save &As...", this, &MainWindow::saveFileAs);
    
    fileMenu->addSeparator();
    
    QAction *exportAction = fileMenu->addAction("&Export BMP (PocketMage)...", this, &MainWindow::exportBMP);
    exportAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    
    fileMenu->addAction("Export &Arduino Header (.h)...", this, &MainWindow::exportArduinoHeader);
    fileMenu->addAction("Export Raw &Binary (.bin)...", this, &MainWindow::exportRawBinary);
    
    fileMenu->addSeparator();
    
    QAction *importAction = fileMenu->addAction("&Import Image to Pixel Art...", this, &MainWindow::importImage);
    importAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_I));
    
    fileMenu->addSeparator();
    
    QAction *quitAction = fileMenu->addAction("&Quit", this, &QWidget::close);
    quitAction->setShortcut(QKeySequence::Quit);
    
    // Edit menu
    QMenu *editMenu = menuBar()->addMenu("&Edit");
    QAction *clearAction = editMenu->addAction("&Clear", m_canvas, &PixelCanvas::clear);
    clearAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Delete));
    
    QAction *invertAction = editMenu->addAction("&Invert Colors", this, &MainWindow::invertColors);
    invertAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
}

void MainWindow::createToolBar() {
    QToolBar *toolbar = addToolBar("Tools");
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));
    
    QButtonGroup *toolGroup = new QButtonGroup(this);
    
    auto addToolButton = [&](const QString &text, const QString &shortcut, int tool) {
        QToolButton *btn = new QToolButton(this);
        btn->setText(text);
        btn->setCheckable(true);
        btn->setShortcut(QKeySequence(shortcut));
        btn->setToolTip(text + " (" + shortcut + ")");
        toolbar->addWidget(btn);
        toolGroup->addButton(btn, tool);
        m_toolButtons.append(btn);
        return btn;
    };
    
    addToolButton("✏️ Pencil", "P", PixelCanvas::Pencil)->setChecked(true);
    addToolButton("🧹 Eraser", "E", PixelCanvas::Eraser);
    addToolButton("🪣 Fill", "F", PixelCanvas::Fill);
    addToolButton("📏 Line", "L", PixelCanvas::Line);
    addToolButton("⬜ Rect", "R", PixelCanvas::Rectangle);
    addToolButton("⚪ Ellipse", "O", PixelCanvas::Ellipse);
    addToolButton("💉 Picker", "I", PixelCanvas::Eyedropper);
    addToolButton("🔤 Text", "T", PixelCanvas::Text);
    addToolButton("✂️ Select", "S", PixelCanvas::Select);
    
    connect(toolGroup, &QButtonGroup::idClicked, this, &MainWindow::setTool);
    
    toolbar->addSeparator();
    
    // Color button
    m_colorButton = new QToolButton(this);
    m_colorButton->setText("Color");
    m_colorButton->setToolTip("Pick Color (C)");
    m_colorButton->setShortcut(QKeySequence("C"));
    updateColorButton();
    connect(m_colorButton, &QToolButton::clicked, this, &MainWindow::pickColor);
    toolbar->addWidget(m_colorButton);
    
    toolbar->addSeparator();
    
    // Size controls
    toolbar->addWidget(new QLabel(" Size: "));
    m_widthSpin = new QSpinBox(this);
    m_widthSpin->setRange(1, 128);
    m_widthSpin->setValue(40);
    toolbar->addWidget(m_widthSpin);
    
    toolbar->addWidget(new QLabel(" x "));
    
    m_heightSpin = new QSpinBox(this);
    m_heightSpin->setRange(1, 128);
    m_heightSpin->setValue(40);
    toolbar->addWidget(m_heightSpin);
    
    QToolButton *resizeBtn = new QToolButton(this);
    resizeBtn->setText("Resize");
    connect(resizeBtn, &QToolButton::clicked, this, [this]() {
        m_canvas->newImage(m_widthSpin->value(), m_heightSpin->value());
        statusBar()->showMessage(QString("New %1x%2 icon").arg(m_widthSpin->value()).arg(m_heightSpin->value()));
    });
    toolbar->addWidget(resizeBtn);
    
    toolbar->addSeparator();
    
    // Zoom controls
    QToolButton *zoomOutBtn = new QToolButton(this);
    zoomOutBtn->setText("🔍−");
    zoomOutBtn->setToolTip("Zoom Out (-)");
    zoomOutBtn->setShortcut(QKeySequence("-"));
    connect(zoomOutBtn, &QToolButton::clicked, this, [this]() {
        int current = m_canvas->pixelSize();
        int step = qMax(1, current / 4);  // 25% relative step
        m_canvas->setPixelSize(current - step);
        statusBar()->showMessage(QString("Zoom: %1x").arg(m_canvas->pixelSize()));
    });
    toolbar->addWidget(zoomOutBtn);
    
    QToolButton *zoomInBtn = new QToolButton(this);
    zoomInBtn->setText("🔍+");
    zoomInBtn->setToolTip("Zoom In (+)");
    zoomInBtn->setShortcut(QKeySequence("+"));
    connect(zoomInBtn, &QToolButton::clicked, this, [this]() {
        int current = m_canvas->pixelSize();
        int step = qMax(1, current / 4);  // 25% relative step
        m_canvas->setPixelSize(current + step);
        statusBar()->showMessage(QString("Zoom: %1x").arg(m_canvas->pixelSize()));
    });
    toolbar->addWidget(zoomInBtn);
    
    QToolButton *zoom1to1Btn = new QToolButton(this);
    zoom1to1Btn->setText("1:1");
    zoom1to1Btn->setToolTip("Actual Size - 1 screen pixel = 1 image pixel (1)");
    zoom1to1Btn->setShortcut(QKeySequence("1"));
    connect(zoom1to1Btn, &QToolButton::clicked, this, [this]() {
        m_canvas->setPixelSize(1);
        statusBar()->showMessage("Zoom: 1:1 (actual size)");
    });
    toolbar->addWidget(zoom1to1Btn);
    
    QToolButton *zoomFitBtn = new QToolButton(this);
    zoomFitBtn->setText("⊡ Fit");
    zoomFitBtn->setToolTip("Fit to Window (0)");
    zoomFitBtn->setShortcut(QKeySequence("0"));
    connect(zoomFitBtn, &QToolButton::clicked, this, [this]() {
        // Calculate size to fit in window
        int availW = width() - 100;
        int availH = height() - 200;
        int imgW = m_canvas->image().width();
        int imgH = m_canvas->image().height();
        int fitSize = qMin(availW / imgW, availH / imgH);
        fitSize = qMax(1, fitSize);
        m_canvas->setPixelSize(fitSize);
        statusBar()->showMessage(QString("Zoom: %1x (fit to window)").arg(fitSize));
    });
    toolbar->addWidget(zoomFitBtn);
}

void MainWindow::createColorPalette() {
    QToolBar *paletteBar = addToolBar("Palette");
    paletteBar->setMovable(false);
    
    // Common colors for pixel art
    QList<QColor> colors = {
        Qt::black, Qt::white, 
        QColor(128, 128, 128), QColor(192, 192, 192),
        Qt::red, QColor(255, 128, 128),
        QColor(255, 128, 0), QColor(255, 200, 100),
        Qt::yellow, QColor(255, 255, 128),
        Qt::green, QColor(128, 255, 128),
        QColor(0, 128, 0), QColor(0, 255, 128),
        Qt::cyan, QColor(128, 255, 255),
        Qt::blue, QColor(128, 128, 255),
        QColor(128, 0, 255), QColor(200, 128, 255),
        Qt::magenta, QColor(255, 128, 255),
        QColor(128, 64, 0), QColor(200, 150, 100),
    };
    
    for (const QColor &color : colors) {
        QToolButton *btn = new QToolButton(this);
        btn->setFixedSize(20, 20);
        btn->setStyleSheet(QString("background-color: %1; border: 1px solid #888;").arg(color.name()));
        connect(btn, &QToolButton::clicked, this, [this, color]() {
            m_canvas->setCurrentColor(color);
            updateColorButton();
        });
        paletteBar->addWidget(btn);
    }
}

void MainWindow::updateColorButton() {
    QColor c = m_canvas->currentColor();
    m_colorButton->setStyleSheet(QString("background-color: %1; color: %2; padding: 4px;")
        .arg(c.name())
        .arg(c.lightness() > 128 ? "black" : "white"));
}

void MainWindow::newFile() {
    m_canvas->newImage(m_widthSpin->value(), m_heightSpin->value());
    m_currentFile.clear();
    m_modified = false;
    updateTitle();
    statusBar()->showMessage(QString("New %1x%2 icon").arg(m_widthSpin->value()).arg(m_heightSpin->value()));
}

void MainWindow::openFile() {
    QString path = QFileDialog::getOpenFileName(this, "Open Image", QString(),
        "All Files (*);;All Supported (*.png *.bmp *.jpg *.jpeg *.gif *.bin);;Images (*.png *.bmp *.jpg *.jpeg *.gif);;Binary Icons (*.bin)");
    if (path.isEmpty()) return;
    
    // Check if it's a binary file
    if (path.endsWith(".bin", Qt::CaseInsensitive)) {
        if (m_canvas->loadRawBinary(path)) {
            m_currentFile = path;
            m_modified = false;
            m_widthSpin->setValue(m_canvas->image().width());
            m_heightSpin->setValue(m_canvas->image().height());
            updateTitle();
            statusBar()->showMessage("Opened binary icon: " + path);
        } else {
            QMessageBox::warning(this, "Error", "Could not open binary file: " + path);
        }
        return;
    }
    
    QImage source(path);
    if (source.isNull()) {
        QMessageBox::warning(this, "Error", "Could not open image: " + path);
        return;
    }
    
    // Ask user how to open the image
    QDialog dialog(this);
    dialog.setWindowTitle("Open Options");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    layout->addWidget(new QLabel(QString("Image: %1 x %2 pixels")
        .arg(source.width()).arg(source.height())));
    
    // Color mode
    QHBoxLayout *modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel("Color Mode:"));
    QComboBox *modeCombo = new QComboBox();
    modeCombo->addItem("Keep Original Colors");
    modeCombo->addItem("Monochrome (Black & White)");
    modeCombo->addItem("Grayscale (4 levels)");
    modeCombo->addItem("Grayscale (8 levels)");
    modeCombo->addItem("Quantize Colors");
    modeLayout->addWidget(modeCombo);
    layout->addLayout(modeLayout);
    
    // Threshold (for monochrome)
    QHBoxLayout *threshLayout = new QHBoxLayout();
    QLabel *threshLabel = new QLabel("Threshold:");
    threshLayout->addWidget(threshLabel);
    QSpinBox *threshSpin = new QSpinBox();
    threshSpin->setRange(0, 255);
    threshSpin->setValue(128);
    threshLayout->addWidget(threshSpin);
    layout->addLayout(threshLayout);
    
    // Color count
    QHBoxLayout *colorLayout = new QHBoxLayout();
    QLabel *colorLabel = new QLabel("Max Colors:");
    colorLayout->addWidget(colorLabel);
    QSpinBox *colorSpin = new QSpinBox();
    colorSpin->setRange(2, 256);
    colorSpin->setValue(16);
    colorLayout->addWidget(colorSpin);
    layout->addLayout(colorLayout);
    
    // Show/hide based on mode
    auto updateOptions = [&]() {
        int mode = modeCombo->currentIndex();
        threshLabel->setVisible(mode == 1);
        threshSpin->setVisible(mode == 1);
        colorLabel->setVisible(mode == 4);
        colorSpin->setVisible(mode == 4);
    };
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), updateOptions);
    updateOptions();
    
    QCheckBox *invertCheck = new QCheckBox("Invert colors");
    layout->addWidget(invertCheck);
    
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    
    if (dialog.exec() != QDialog::Accepted) return;
    
    int colorMode = modeCombo->currentIndex();
    int threshold = threshSpin->value();
    int maxColors = colorSpin->value();
    bool invert = invertCheck->isChecked();
    
    QImage result = source.convertToFormat(QImage::Format_RGB32);
    
    if (colorMode == 1) {
        // Monochrome
        for (int y = 0; y < result.height(); y++) {
            for (int x = 0; x < result.width(); x++) {
                int val = qGray(result.pixel(x, y));
                bool isBlack = val < threshold;
                if (invert) isBlack = !isBlack;
                result.setPixel(x, y, isBlack ? qRgb(0,0,0) : qRgb(255,255,255));
            }
        }
    } else if (colorMode == 2 || colorMode == 3) {
        // Grayscale with levels
        int levels = (colorMode == 2) ? 4 : 8;
        int step = 256 / levels;
        for (int y = 0; y < result.height(); y++) {
            for (int x = 0; x < result.width(); x++) {
                int val = qGray(result.pixel(x, y));
                val = (val / step) * step + step/2;
                val = qBound(0, val, 255);
                if (invert) val = 255 - val;
                result.setPixel(x, y, qRgb(val, val, val));
            }
        }
    } else if (colorMode == 4) {
        // Quantize colors
        int levels = qMax(2, (int)qPow(maxColors, 1.0/3.0));
        int step = 256 / levels;
        for (int y = 0; y < result.height(); y++) {
            for (int x = 0; x < result.width(); x++) {
                QColor c(result.pixel(x, y));
                int r = (c.red() / step) * step + step/2;
                int g = (c.green() / step) * step + step/2;
                int b = (c.blue() / step) * step + step/2;
                r = qBound(0, r, 255);
                g = qBound(0, g, 255);
                b = qBound(0, b, 255);
                if (invert) { r = 255 - r; g = 255 - g; b = 255 - b; }
                result.setPixel(x, y, qRgb(r, g, b));
            }
        }
    } else if (invert) {
        // Keep original but invert
        for (int y = 0; y < result.height(); y++) {
            for (int x = 0; x < result.width(); x++) {
                QColor c(result.pixel(x, y));
                result.setPixel(x, y, qRgb(255-c.red(), 255-c.green(), 255-c.blue()));
            }
        }
    }
    
    m_canvas->setImage(result);
    m_currentFile = path;
    m_modified = false;
    m_widthSpin->setValue(result.width());
    m_heightSpin->setValue(result.height());
    updateTitle();
    statusBar()->showMessage("Opened: " + path);
}

void MainWindow::saveFile() {
    if (m_currentFile.isEmpty()) {
        saveFileAs();
        return;
    }
    
    if (m_canvas->saveImage(m_currentFile)) {
        m_modified = false;
        updateTitle();
        statusBar()->showMessage("Saved: " + m_currentFile);
    } else {
        QMessageBox::warning(this, "Error", "Could not save image.");
    }
}

void MainWindow::saveFileAs() {
    QString path = QFileDialog::getSaveFileName(this, "Save Image", QString(),
        "PNG Image (*.png);;BMP Image (*.bmp);;All Files (*)");
    if (path.isEmpty()) return;
    
    m_currentFile = path;
    saveFile();
}

void MainWindow::exportBMP() {
    QString path = QFileDialog::getSaveFileName(this, "Export BMP for PocketMage", QString(),
        "BMP Image (*.bmp)");
    if (path.isEmpty()) return;
    
    // Ensure .bmp extension
    if (!path.endsWith(".bmp", Qt::CaseInsensitive)) {
        path += ".bmp";
    }
    
    if (m_canvas->saveBMP(path)) {
        statusBar()->showMessage("Exported: " + path);
        QMessageBox::information(this, "Export Complete", 
            "Icon exported!\n\nCopy to SD card /apps/ folder with matching .bin name.");
    } else {
        QMessageBox::warning(this, "Error", "Could not export BMP.");
    }
}

void MainWindow::exportArduinoHeader() {
    QString path = QFileDialog::getSaveFileName(this, "Export Arduino Header", QString(),
        "C++ Header (*.h)");
    if (path.isEmpty()) return;
    
    // Ensure .h extension
    if (!path.endsWith(".h", Qt::CaseInsensitive)) {
        path += ".h";
    }
    
    // Get variable name from filename
    QFileInfo fi(path);
    QString varName = fi.baseName();
    varName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
    if (varName.isEmpty() || varName[0].isDigit()) {
        varName = "icon_" + varName;
    }
    
    if (m_canvas->saveCppHeader(path, varName)) {
        statusBar()->showMessage("Exported: " + path);
        QMessageBox::information(this, "Export Complete", 
            QString("Arduino header exported!\n\nVariable name: %1\nUse with display.drawBitmap()").arg(varName));
    } else {
        QMessageBox::warning(this, "Error", "Could not export header.");
    }
}

void MainWindow::exportRawBinary() {
    QString path = QFileDialog::getSaveFileName(this, "Export Raw Binary", QString(),
        "Binary File (*.bin);;All Files (*)");
    if (path.isEmpty()) return;
    
    if (!path.endsWith(".bin", Qt::CaseInsensitive)) {
        path += ".bin";
    }
    
    if (m_canvas->saveRawBinary(path)) {
        statusBar()->showMessage("Exported: " + path);
        QMessageBox::information(this, "Export Complete", 
            QString("Raw binary exported!\n\nFormat: 1-bit packed pixels (no header)\nCompatible with PocketMage AppLauncher"));
    } else {
        QMessageBox::warning(this, "Error", "Could not export binary.");
    }
}

void MainWindow::invertColors() {
    m_canvas->invertColors();
    statusBar()->showMessage("Colors inverted");
}

void MainWindow::importImage() {
    QString path = QFileDialog::getOpenFileName(this, "Import Image", QString(),
        "Images (*.png *.jpg *.jpeg *.bmp *.gif);;All Files (*)");
    if (path.isEmpty()) return;
    
    QImage source(path);
    if (source.isNull()) {
        QMessageBox::warning(this, "Error", "Could not load image.");
        return;
    }
    
    // Create options dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Import Options");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    
    // Show source image info
    layout->addWidget(new QLabel(QString("Source: %1 x %2 pixels")
        .arg(source.width()).arg(source.height())));
    
    // Target size
    QHBoxLayout *sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("Target Size:"));
    QSpinBox *widthSpin = new QSpinBox();
    widthSpin->setRange(1, 512);
    widthSpin->setValue(40);
    sizeLayout->addWidget(widthSpin);
    sizeLayout->addWidget(new QLabel("x"));
    QSpinBox *heightSpin = new QSpinBox();
    heightSpin->setRange(1, 512);
    heightSpin->setValue(40);
    sizeLayout->addWidget(heightSpin);
    layout->addLayout(sizeLayout);
    
    // Color mode
    QHBoxLayout *modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel("Color Mode:"));
    QComboBox *modeCombo = new QComboBox();
    modeCombo->addItem("Monochrome (Black & White)");
    modeCombo->addItem("Preserve Colors");
    modeCombo->addItem("Grayscale (4 levels)");
    modeCombo->addItem("Grayscale (8 levels)");
    modeLayout->addWidget(modeCombo);
    layout->addLayout(modeLayout);
    
    // Threshold (for monochrome)
    QHBoxLayout *threshLayout = new QHBoxLayout();
    QLabel *threshLabel = new QLabel("Threshold (0-255):");
    threshLayout->addWidget(threshLabel);
    QSpinBox *threshSpin = new QSpinBox();
    threshSpin->setRange(0, 255);
    threshSpin->setValue(128);
    threshLayout->addWidget(threshSpin);
    layout->addLayout(threshLayout);
    
    // Color count (for color mode)
    QHBoxLayout *colorLayout = new QHBoxLayout();
    QLabel *colorLabel = new QLabel("Max Colors:");
    colorLayout->addWidget(colorLabel);
    QSpinBox *colorSpin = new QSpinBox();
    colorSpin->setRange(2, 256);
    colorSpin->setValue(16);
    colorLayout->addWidget(colorSpin);
    layout->addLayout(colorLayout);
    
    // Show/hide options based on mode
    auto updateOptions = [&]() {
        int mode = modeCombo->currentIndex();
        threshLabel->setVisible(mode == 0);
        threshSpin->setVisible(mode == 0);
        colorLabel->setVisible(mode == 1);
        colorSpin->setVisible(mode == 1);
    };
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), updateOptions);
    updateOptions();
    
    // Options
    QCheckBox *autoCropCheck = new QCheckBox("Auto-crop whitespace");
    autoCropCheck->setChecked(false);
    layout->addWidget(autoCropCheck);
    
    QCheckBox *invertCheck = new QCheckBox("Invert colors");
    invertCheck->setChecked(false);
    layout->addWidget(invertCheck);
    
    // Buttons
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    
    if (dialog.exec() != QDialog::Accepted) return;
    
    int targetW = widthSpin->value();
    int targetH = heightSpin->value();
    int colorMode = modeCombo->currentIndex();
    int threshold = threshSpin->value();
    int maxColors = colorSpin->value();
    bool autoCrop = autoCropCheck->isChecked();
    bool invert = invertCheck->isChecked();
    
    // Work with RGB image
    QImage work = source.convertToFormat(QImage::Format_RGB32);
    
    // Auto-crop if enabled (based on corner color)
    if (autoCrop) {
        QColor bgColor = QColor(work.pixel(0, 0));
        int minX = work.width(), minY = work.height(), maxX = 0, maxY = 0;
        for (int y = 0; y < work.height(); y++) {
            for (int x = 0; x < work.width(); x++) {
                QColor c(work.pixel(x, y));
                // Check if different from background
                int diff = qAbs(c.red() - bgColor.red()) + 
                           qAbs(c.green() - bgColor.green()) + 
                           qAbs(c.blue() - bgColor.blue());
                if (diff > 30) {
                    minX = qMin(minX, x);
                    minY = qMin(minY, y);
                    maxX = qMax(maxX, x);
                    maxY = qMax(maxY, y);
                }
            }
        }
        if (maxX >= minX && maxY >= minY) {
            work = work.copy(minX, minY, maxX - minX + 1, maxY - minY + 1);
        }
    }
    
    // Scale to target size
    QImage scaled = work.scaled(targetW, targetH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    
    // Create final image centered
    QImage result(targetW, targetH, QImage::Format_RGB32);
    result.fill(Qt::white);
    
    int offsetX = (targetW - scaled.width()) / 2;
    int offsetY = (targetH - scaled.height()) / 2;
    
    // Process based on color mode
    if (colorMode == 0) {
        // Monochrome
        for (int y = 0; y < scaled.height(); y++) {
            for (int x = 0; x < scaled.width(); x++) {
                int val = qGray(scaled.pixel(x, y));
                bool isBlack = val < threshold;
                if (invert) isBlack = !isBlack;
                result.setPixel(offsetX + x, offsetY + y, isBlack ? qRgb(0,0,0) : qRgb(255,255,255));
            }
        }
    } else if (colorMode == 1) {
        // Preserve colors - quantize to reduce palette
        for (int y = 0; y < scaled.height(); y++) {
            for (int x = 0; x < scaled.width(); x++) {
                QColor c(scaled.pixel(x, y));
                // Simple quantization: reduce color depth
                int levels = qMax(2, (int)qPow(maxColors, 1.0/3.0));
                int step = 256 / levels;
                int r = (c.red() / step) * step + step/2;
                int g = (c.green() / step) * step + step/2;
                int b = (c.blue() / step) * step + step/2;
                r = qBound(0, r, 255);
                g = qBound(0, g, 255);
                b = qBound(0, b, 255);
                if (invert) {
                    r = 255 - r; g = 255 - g; b = 255 - b;
                }
                result.setPixel(offsetX + x, offsetY + y, qRgb(r, g, b));
            }
        }
    } else {
        // Grayscale with levels
        int levels = (colorMode == 2) ? 4 : 8;
        int step = 256 / levels;
        for (int y = 0; y < scaled.height(); y++) {
            for (int x = 0; x < scaled.width(); x++) {
                int val = qGray(scaled.pixel(x, y));
                val = (val / step) * step + step/2;
                val = qBound(0, val, 255);
                if (invert) val = 255 - val;
                result.setPixel(offsetX + x, offsetY + y, qRgb(val, val, val));
            }
        }
    }
    
    m_canvas->setImage(result);
    m_widthSpin->setValue(targetW);
    m_heightSpin->setValue(targetH);
    
    QString modeNames[] = {"monochrome", "color", "4-level gray", "8-level gray"};
    statusBar()->showMessage(QString("Imported %1 as %2x%3 %4 pixel art")
        .arg(QFileInfo(path).fileName()).arg(targetW).arg(targetH).arg(modeNames[colorMode]));
}

void MainWindow::pickColor() {
    QColor color = QColorDialog::getColor(m_canvas->currentColor(), this, "Pick Color");
    if (color.isValid()) {
        m_canvas->setCurrentColor(color);
        updateColorButton();
    }
}

void MainWindow::onColorPicked(const QColor &color) {
    m_canvas->setCurrentColor(color);
    updateColorButton();
    m_canvas->setTool(PixelCanvas::Pencil);
    m_toolButtons[0]->setChecked(true);
    statusBar()->showMessage(QString("Picked color: %1").arg(color.name()));
}

void MainWindow::updateTitle() {
    QString title = "PocketMage Icon Editor";
    if (!m_currentFile.isEmpty()) {
        title += " - " + QFileInfo(m_currentFile).fileName();
    }
    m_modified = true;
    setWindowTitle(title + (m_modified ? " *" : ""));
}

void MainWindow::setTool(int tool) {
    m_canvas->setTool(static_cast<PixelCanvas::Tool>(tool));
    
    QStringList toolNames = {"Pencil", "Eraser", "Fill", "Line", "Rectangle", "Ellipse", "Eyedropper", "Text", "Select"};
    if (tool >= 0 && tool < toolNames.size()) {
        QString msg = "Tool: " + toolNames[tool];
        if (tool == PixelCanvas::Text) {
            msg += " - Click to place cursor, then type";
        } else if (tool == PixelCanvas::Select) {
            msg += " - Drag to select, then drag selection to move";
        }
        statusBar()->showMessage(msg);
    }
}
