#include "../include/ToolBar.h"
#include "../include/BrushWidthSpinBox.h"
#include <QIcon>
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QLabel>
#include <QStyleFactory>

ToolBar::ToolBar(QWidget *parent) : QToolBar("Tools", parent) {

    setStyle(QStyleFactory::create("Fusion"));
    setStyleSheet(R"(
        QToolBar {
            background: #f3f3f3;
            border: none;
            padding: 4px;
            spacing: 4px;
            color: #333333;
        }
        QToolButton {
            background: transparent;
            border: 1px solid transparent;
            border-radius: 4px;
            padding: 4px;
            color: #333333;
        }
        QToolButton:hover {
            background: #e5e5e5;
        }
        QToolButton:checked {
            background-color: #0078d7;
            color: white;
        }
        QLabel {
            color: #333333;
        }
    )");


    setIconSize(QSize(24, 24));
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    
    createActions();
    setupUi();
    
    m_selectAction->setChecked(true);
}

void ToolBar::createActions() {
    m_toolActionGroup = new QActionGroup(this);
    
    // Selection tool
    m_selectAction = new QAction(QIcon("../resources/icons/hand.svg"), tr("Select"), this);
    m_selectAction->setCheckable(true);
    m_selectAction->setData(SelectTool);
    m_toolActionGroup->addAction(m_selectAction);
    m_selectAction->setToolTip("Select one shape(press on the border)");
    
    // Line tool
    m_lineAction = new QAction(QIcon("../resources/icons/line.svg"), tr("Line"), this);
    m_lineAction->setCheckable(true);
    m_lineAction->setData(LineTool);
    m_toolActionGroup->addAction(m_lineAction);
    m_lineAction->setToolTip("Create line");
    
    // Circle tool
    m_circleAction = new QAction(QIcon("../resources/icons/circle.svg"), tr("Circle"), this);
    m_circleAction->setCheckable(true);
    m_circleAction->setData(CircleTool);
    m_toolActionGroup->addAction(m_circleAction);
    m_circleAction->setToolTip("Create circle");
    
    // Rectangle tool
    m_rectAction = new QAction(QIcon("../resources/icons/square.svg"), tr("Rectangle"), this);
    m_rectAction->setCheckable(true);
    m_rectAction->setData(RectangleTool);
    m_toolActionGroup->addAction(m_rectAction);
    m_rectAction->setToolTip("Create rectangle");
    
    // Freehand tool
    m_freehandAction = new QAction(QIcon("../resources/icons/color_pen.svg"), tr("Freehand"), this);
    m_freehandAction->setCheckable(true);
    m_freehandAction->setData(FreehandTool);
    m_toolActionGroup->addAction(m_freehandAction);
    m_freehandAction->setToolTip("Create free hand line");

    // Polygon tool
    m_polygonAction = new QAction(QIcon("../resources/icons/polygon.svg"), tr("Polygon"), this);
    m_polygonAction->setCheckable(true);
    m_polygonAction->setData(PolygonTool);
    m_toolActionGroup->addAction(m_polygonAction);
    m_polygonAction->setToolTip("Create polygon (LMB-add point, RMB-finish)");

    // Regular polygon tool
    m_regularPolygonAction = new QAction(QIcon("../resources/icons/hexagon.svg"), tr("Regular Polygon"), this);
    m_regularPolygonAction->setCheckable(true);
    m_regularPolygonAction->setData(RegularPolygonTool);
    m_toolActionGroup->addAction(m_regularPolygonAction);
    m_regularPolygonAction->setToolTip("Create regular polygon");
    
    // Other actions
    m_undoAction = new QAction(QIcon("../resources/icons/undo.svg"), tr("Undo"), this);
    m_redoAction = new QAction(QIcon("../resources/icons/redo.svg"), tr("Redo"), this);
    m_clearAction = new QAction(QIcon("../resources/icons/eraser.svg"), tr("Clear"), this);
    
    // Connect signals
    connect(m_toolActionGroup, &QActionGroup::triggered, this, &ToolBar::handleToolAction);
    connect(m_undoAction, &QAction::triggered, this, &ToolBar::undoRequested);
    connect(m_redoAction, &QAction::triggered, this, &ToolBar::redoRequested);
    connect(m_clearAction, &QAction::triggered, this, &ToolBar::clearCanvasRequested);
}

void ToolBar::setupUi() {
    // Add tools
    addAction(m_selectAction);
    addAction(m_lineAction);
    addAction(m_circleAction);
    addAction(m_rectAction);
    addAction(m_freehandAction);
    addAction(m_polygonAction);
    addAction(m_regularPolygonAction);
    
    addSeparator();
    
    // Undo/Redo
    addAction(m_undoAction);
    addAction(m_redoAction);
    
    addSeparator();
    
    // Color picker
    m_colorButton = new QPushButton(this);
    m_colorButton->setFixedSize(QSize(30, 30));
    m_colorButton->setMaximumSize(30, 30);
    connect(m_colorButton, &QPushButton::clicked, [this]() {
        QColor newColor = QColorDialog::getColor(m_currentColor, this, tr("Select Color"));
        if (newColor.isValid()) {
            m_currentColor = newColor;
            updateColorButton();
            emit colorChanged(m_currentColor);
        }
    });
    
    addWidget(m_colorButton);
    updateColorButton();

    // Fill color button
    m_fillColorButton = new QPushButton(this);
    m_fillColorButton->setFixedSize(QSize(30, 30));
    m_fillColorButton->setMaximumSize(30, 30);
    m_fillColorButton->setToolTip("Fill Color");
    m_fillColorButton->setCheckable(true);
    connect(m_fillColorButton, &QPushButton::clicked, [this](bool checked) {
        if (checked) {
            QColor newColor = QColorDialog::getColor(m_fillColor, this, tr("Select Fill Color"));
            if (newColor.isValid()) {
                m_fillColor = newColor;
                updateFillColorButton();
                emit fillColorChanged(m_fillColor, true);
            } else {
                m_fillColorButton->setChecked(false);
            }
        } else {
            emit fillColorChanged(Qt::transparent, false);
        }
    });
    addWidget(m_fillColorButton);
    updateFillColorButton();

    addSeparator();
    
    // Pen width
    m_penWidthSpinBox = new QSpinBox(this);
    m_penWidthSpinBox->setRange(1, 50);
    m_penWidthSpinBox->setValue(6);
    m_penWidthSpinBox->setSuffix(" px");
    connect(m_penWidthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ToolBar::penWidthChanged);
    addWidget(m_penWidthSpinBox);
    
    addSeparator();
    
    // Clear canvas
    addAction(m_clearAction);

    addSeparator();

    m_rotationSpinBox = new QSpinBox(this);
    m_rotationSpinBox->setRange(-360, 360);
    m_rotationSpinBox->setSuffix("°");
    connect(m_rotationSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ToolBar::rotationChanged);

    m_widthSpinBox = new QSpinBox(this);
    m_widthSpinBox->setRange(1, 1000);
    connect(m_widthSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int w) { emit resizeRequested(QSize(w, m_heightSpinBox->value())); });

    m_heightSpinBox = new QSpinBox(this);
    m_heightSpinBox->setRange(1, 1000);
    connect(m_heightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int h) { emit resizeRequested(QSize(m_widthSpinBox->value(), h)); });

    addWidget(new QLabel("Rotate:"));
    addWidget(m_rotationSpinBox);
    addWidget(new QLabel("Size:"));
    addWidget(m_widthSpinBox);
    addWidget(new QLabel("x"));
    addWidget(m_heightSpinBox);
    

    m_sidesSpinBox = new QSpinBox(this);
    m_sidesSpinBox->setRange(3, 20);
    m_sidesSpinBox->setValue(5);
    connect(m_sidesSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            [this](int sides) { emit polygonSidesRequested(sides); });

    addSeparator();
    addWidget(new QLabel("Sides:"));
    addWidget(m_sidesSpinBox);
}

void ToolBar::updateColorButton() {
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(m_currentColor);
    painter.setPen(QColor("#888"));
    painter.drawEllipse(0, 0, 23, 23);

    m_colorButton->setIcon(QIcon(pixmap));
    m_colorButton->setIconSize(QSize(24, 24));
    m_colorButton->setFixedSize(30, 30);
    m_colorButton->setToolTip(QString("Pen Color: %1").arg(m_currentColor.name()));
}

void ToolBar::updateFillColorButton() {
    QPixmap pixmap(24, 24);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(m_fillColor);
    painter.setPen(QColor("#888"));
    painter.drawEllipse(0, 0, 23, 23);

    m_fillColorButton->setIcon(QIcon(pixmap));
    m_fillColorButton->setIconSize(QSize(24, 24));
    m_fillColorButton->setFixedSize(30, 30);
    m_fillColorButton->setToolTip(QString("Fill Color: %1").arg(m_fillColor.name()));
}

void ToolBar::handleToolAction(QAction *action) {
    m_currentTool = static_cast<Tool>(action->data().toInt());
    emit toolChanged(m_currentTool);
}

void ToolBar::setRotation(double angle) {
    m_rotationSpinBox->blockSignals(true);
    m_rotationSpinBox->setValue(static_cast<int>(angle));
    m_rotationSpinBox->blockSignals(false);
}

void ToolBar::setSize(const QSize& size) {
    m_widthSpinBox->blockSignals(true);
    m_heightSpinBox->blockSignals(true);
    
    m_widthSpinBox->setValue(size.width());
    m_heightSpinBox->setValue(size.height());
    
    m_widthSpinBox->blockSignals(false);
    m_heightSpinBox->blockSignals(false);
}

void ToolBar::setPenColor(const QColor& color) {
    m_currentColor = color;
    updateColorButton();
}

void ToolBar::setPenWidth(int32_t penWidth) {
    m_penWidthSpinBox->blockSignals(true);
    m_penWidthSpinBox->setValue(static_cast<int>(penWidth));
    m_penWidthSpinBox->blockSignals(false);
}

void ToolBar::setFillParams(const QColor& fillColor, bool isFilled) {
    m_fillColor = fillColor;
    m_fillEnabled = isFilled;
    m_fillColorButton->setChecked(m_fillEnabled);
    updateFillColorButton();
}