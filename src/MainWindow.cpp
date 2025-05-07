#include "../include/MainWindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QLabel>
#include <QStatusBar>
#include <QMenuBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStyleFactory>
#include <QDockWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{

    setStyle(QStyleFactory::create("Fusion"));
    setStyleSheet(R"(
        QMainWindow {
            background: #f9f9f9;
        }
        QMenuBar {
            background: #f3f3f3;
            color: #333333;
        }
        QMenuBar::item {
            background: transparent;
            color: #333333;
        }
        QMenuBar::item:selected {
            background: #e5e5e5;
        }
        QMenu {
            background: white;
            color: #333333;
            border: 1px solid #e0e0e0;
        }
        QMenu::item {
            color: #333333;
        }
        QMenu::item:selected {
            background: #e0f0ff;
        }
        QListWidget {
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 4px;
            padding: 4px;
            color: #333333;
        }
        QListWidget::item {
            padding: 4px;
            color: #333333;
        }
        QListWidget::item:hover {
            background: #f0f0f0;
        }
        QListWidget::item:selected {
            background: #e0f0ff;
            color: black;
        }
        QPushButton {
            background-color: #f6f6f6;
            border: 1px solid #d3d3d3;
            border-radius: 4px;
            padding: 6px 12px;
            color: #333333;
        }
        QPushButton:hover {
            background-color: #e5e5e5;
        }
        QPushButton:pressed {
            background-color: #d5d5d5;
        }
        QPushButton:checked {
            background-color: #0078d7;
            color: white;
            border: 1px solid #005a9e;
        }
        QStatusBar {
            background: #f3f3f3;
            border-top: 1px solid #e0e0e0;
            color: #333333;
        }
    )");


    m_canvas = new CanvasWidget(this);
    setCentralWidget(m_canvas);

    m_toolBar = new ToolBar(this);
    addToolBar(Qt::TopToolBarArea, m_toolBar);

    createActions();
    createMenus();
    createStatusBar();
    createControls();

    setupConnections();

    setWindowTitle(tr("Inkscape-like Editor[*]"));
    resize(1000, 700);
    setMinimumSize(800, 600);
}

void MainWindow::createControls() {
    // Панель кіравання
    QWidget *controlPanel = new QWidget();
    QVBoxLayout *panelLayout = new QVBoxLayout(controlPanel);
    panelLayout->setContentsMargins(8, 8, 8, 8);
    panelLayout->setSpacing(8);

    // Кнопка выдалення
    m_deleteButton = new QPushButton(tr("Delete Selected"));
    //m_deleteButton->setIcon(QIcon("../resources/icons/delete.svg"));
    panelLayout->addWidget(m_deleteButton);

    //
    m_stopAnimationButton = new QPushButton(tr("Stop All Animations"));
    panelLayout->addWidget(m_stopAnimationButton);

    QHBoxLayout *zOrderLayout = new QHBoxLayout();
    m_moveUpButton = new QPushButton(tr("Move Up"));
    m_moveDownButton = new QPushButton(tr("Move Down"));
    zOrderLayout->addWidget(m_moveUpButton);
    zOrderLayout->addWidget(m_moveDownButton);
    panelLayout->addLayout(zOrderLayout);

    // Спіс фігур
    QLabel *shapesLabel = new QLabel(tr("Shapes:"));
    panelLayout->addWidget(shapesLabel);
    
    m_shapeListWidget = new QListWidget();
    m_shapeListWidget->setAlternatingRowColors(true);
    panelLayout->addWidget(m_shapeListWidget);

    // Дадаем панель у правую частку акна
    QDockWidget *dock = new QDockWidget(tr("Properties"), this);
    dock->setWidget(controlPanel);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    // Налады цэнтральнага віджэта
    QWidget *central = new QWidget();
    QHBoxLayout *centralLayout = new QHBoxLayout(central);
    centralLayout->addWidget(m_canvas);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    setCentralWidget(central);
}

void MainWindow::setupConnections() {
    // Злучэнні ToolBar
    connect(m_toolBar, &ToolBar::toolChanged, m_canvas, &CanvasWidget::setTool);
    connect(m_toolBar, &ToolBar::colorChanged, m_canvas, &CanvasWidget::setPenColor);
    connect(m_toolBar, &ToolBar::fillColorChanged, m_canvas, &CanvasWidget::setFillColor);
    connect(m_toolBar, &ToolBar::penWidthChanged, m_canvas, &CanvasWidget::setPenWidth);
    connect(m_toolBar, &ToolBar::undoRequested, m_canvas, &CanvasWidget::undo);
    connect(m_toolBar, &ToolBar::redoRequested, m_canvas, &CanvasWidget::redo);
    connect(m_toolBar, &ToolBar::clearCanvasRequested, m_canvas, &CanvasWidget::clear);

    // Іншыя злучэнні
    connect(m_canvas, &CanvasWidget::modificationChanged, 
            [this](bool changed) { setWindowModified(changed); });
    connect(m_canvas, &CanvasWidget::undoAvailable, m_undoAct, &QAction::setEnabled);
    connect(m_canvas, &CanvasWidget::redoAvailable, m_redoAct, &QAction::setEnabled);
    connect(m_canvas, &CanvasWidget::undoAvailable, m_toolBar->undoAction(), &QAction::setEnabled);
    connect(m_canvas, &CanvasWidget::redoAvailable, m_toolBar->redoAction(), &QAction::setEnabled);
    connect(m_canvas, &CanvasWidget::shapeSelected, 
            statusBar(), [this](const QString &msg) { statusBar()->showMessage(msg, 0); });
    connect(m_canvas, &CanvasWidget::updateShapeParameters,
            this, [this](const QColor& penColor, int32_t penWidth, const QColor& fillColor, bool isFilled, const QSize& size, double rotation) {
                m_toolBar->setRotation(rotation);
                m_toolBar->setSize(size);
                m_toolBar->setPenColor(penColor);
                m_toolBar->setPenWidth(penWidth);
                m_toolBar->setFillParams(fillColor, isFilled);
            });
    connect(m_toolBar, &ToolBar::rotationChanged, 
            m_canvas, &CanvasWidget::rotateSelectedShape);
    connect(m_toolBar, &ToolBar::resizeRequested,
            m_canvas, &CanvasWidget::resizeSelectedShape);
    connect(m_toolBar, &ToolBar::polygonSidesRequested,
            m_canvas, &CanvasWidget::resizePolygonSides);
    connect(m_deleteButton, &QPushButton::clicked, 
            m_canvas, &CanvasWidget::deleteSelectedShape);
    connect(m_stopAnimationButton, &QPushButton::clicked,
            m_canvas, &CanvasWidget::stopAllAnimations);
    connect(m_moveUpButton, &QPushButton::clicked, 
            m_canvas, &CanvasWidget::moveShapeUp);
    connect(m_moveDownButton, &QPushButton::clicked, 
            m_canvas, &CanvasWidget::moveShapeDown);
    connect(m_canvas, &CanvasWidget::shapeListChanged, 
            this, &MainWindow::updateShapeList);
    connect(m_shapeListWidget, &QListWidget::currentRowChanged, 
            m_canvas, &CanvasWidget::selectShapeFromList);
}


void MainWindow::updateStatusBar()
{
    QString status;
    switch (m_toolBar->currentTool()) {
        case ToolBar::SelectTool: status = "Select Tool"; break;
        case ToolBar::LineTool: status = "Line Tool"; break;
        case ToolBar::CircleTool: status = "Circle Tool"; break;
        case ToolBar::RectangleTool: status = "Rectangle Tool"; break;
        case ToolBar::FreehandTool: status = "Freehand Tool"; break;
        default: status = "Ready";
    }
    statusBar()->showMessage(status);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        m_canvas->clear();
        setCurrentFile("");
    }
}

void MainWindow::open()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Image"), "", tr("Image Files (*.drw)"));
        if (!fileName.isEmpty()) {
            loadFile(fileName);
        }
    }
}

bool MainWindow::save()
{
    if (m_currentFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(m_currentFile);
    }
}

bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Image"), "", tr("Image Files (*.drw)"));
    if (fileName.isEmpty()) {
        return false;
    }
    return saveFile(fileName);
}

void MainWindow::exportAsImage() {
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Export Image"), "", tr("PNG Image (*.png);;JPEG Image (*.jpg)"));
    if (!fileName.isEmpty()) {
        if (m_canvas->exportAsImage(fileName)) {
            statusBar()->showMessage(tr("Image exported successfully"), 2000);
        } else {
            statusBar()->showMessage(tr("Failed to export image"), 2000);
        }
    }
}

void MainWindow::importBackground() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Background Image"), "", tr("Image Files (*.png *.jpg *.jpeg *.bmp)"));
    if (!fileName.isEmpty()) {
        if (m_canvas->loadBackgroundImage(fileName)) {
            statusBar()->showMessage(tr("Background image loaded"), 2000);
        } else {
            statusBar()->showMessage(tr("Failed to load background image"), 2000);
        }
    }
}


void MainWindow::about()
{
    QMessageBox::about(this, tr("About Inkscape-like Editor"),
        tr("<h2>Inkscape-like Editor 1.0</h2>"
           "<p>A simple vector graphics editor inspired by Inkscape.</p>"));
}

void MainWindow::updateShapeList() {
    m_shapeListWidget->clear();
    m_shapeListWidget->addItems(m_canvas->getShapeList());
}

void MainWindow::createActions()
{
    // File actions
    m_newAct = new QAction(tr("&New"), this);
    m_newAct->setShortcut(QKeySequence::New);
    connect(m_newAct, &QAction::triggered, this, &MainWindow::newFile);

    m_openAct = new QAction(tr("&Open..."), this);
    m_openAct->setShortcut(QKeySequence::Open);
    connect(m_openAct, &QAction::triggered, this, &MainWindow::open);

    m_saveAct = new QAction(tr("&Save"), this);
    m_saveAct->setShortcut(QKeySequence::Save);
    connect(m_saveAct, &QAction::triggered, this, &MainWindow::save);

    m_saveAsAct = new QAction(tr("Save &As..."), this);
    m_saveAsAct->setShortcut(QKeySequence::SaveAs);
    connect(m_saveAsAct, &QAction::triggered, this, &MainWindow::saveAs);

    m_exportImageAct = new QAction(tr("Export as Image"), this);
    connect(m_exportImageAct, &QAction::triggered, this, &MainWindow::exportAsImage);

    m_importBackgroundAct = new QAction(tr("Import Background"), this);
    connect(m_importBackgroundAct, &QAction::triggered, this, &MainWindow::importBackground);

    m_exitAct = new QAction(tr("E&xit"), this);
    m_exitAct->setShortcut(QKeySequence::Quit);
    connect(m_exitAct, &QAction::triggered, this, &QWidget::close);

    // Edit actions
    m_undoAct = new QAction(tr("&Undo"), this);
    m_undoAct->setShortcut(QKeySequence::Undo);
    m_undoAct->setEnabled(false);
    connect(m_undoAct, &QAction::triggered, m_canvas, &CanvasWidget::undo);

    m_redoAct = new QAction(tr("&Redo"), this);
    m_redoAct->setShortcut(QKeySequence::Redo);
    m_redoAct->setEnabled(false);
    connect(m_redoAct, &QAction::triggered, m_canvas, &CanvasWidget::redo);

    m_clearAct = new QAction(tr("&Clear"), this);
    connect(m_clearAct, &QAction::triggered, m_canvas, &CanvasWidget::clear);

    // Help actions
    m_aboutAct = new QAction(tr("&About"), this);
    connect(m_aboutAct, &QAction::triggered, this, &MainWindow::about);
}

void MainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_newAct);
    m_fileMenu->addAction(m_openAct);
    m_fileMenu->addAction(m_saveAct);
    m_fileMenu->addAction(m_saveAsAct);
    m_fileMenu->addAction(m_exportImageAct);
    m_fileMenu->addAction(m_importBackgroundAct);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAct);

    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_undoAct);
    m_editMenu->addAction(m_redoAct);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_clearAct);

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAct);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

bool MainWindow::maybeSave()
{
    if (!m_canvas->isModified()) {
        return true;
    }

    const QMessageBox::StandardButton ret = QMessageBox::warning(this,
        tr("Application"), tr("The image has been modified.\nDo you want to save your changes?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

bool MainWindow::saveFile(const QString &fileName)
{
    if (m_canvas->saveToFile(fileName)) {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File saved"), 2000);
        return true;
    }
    statusBar()->showMessage(tr("Failed to save file"), 2000);
    return false;
}

void MainWindow::loadFile(const QString &fileName)
{
    if (m_canvas->loadFromFile(fileName)) {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File loaded"), 2000);
    } else {
        statusBar()->showMessage(tr("Failed to load file"), 2000);
    }
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    m_currentFile = fileName;
    setWindowModified(false);

    QString shownName = m_currentFile;
    if (m_currentFile.isEmpty()) {
        shownName = "untitled.drw";
    }
    setWindowFilePath(shownName);
}