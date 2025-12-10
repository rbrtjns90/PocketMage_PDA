#pragma once

#include <QMainWindow>
#include <QColor>

class PixelCanvas;
class QToolButton;
class QLabel;
class QSpinBox;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void exportBMP();
    void exportArduinoHeader();
    void exportRawBinary();
    void importImage();
    void invertColors();
    void pickColor();
    void onColorPicked(const QColor &color);
    void updateTitle();
    void setTool(int tool);

private:
    void createActions();
    void createToolBar();
    void createColorPalette();
    void updateColorButton();
    
    PixelCanvas *m_canvas;
    QToolButton *m_colorButton;
    QLabel *m_sizeLabel;
    QSpinBox *m_widthSpin;
    QSpinBox *m_heightSpin;
    
    QString m_currentFile;
    bool m_modified = false;
    
    // Tool buttons for highlighting
    QList<QToolButton*> m_toolButtons;
};
