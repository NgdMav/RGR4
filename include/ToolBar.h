#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QActionGroup>
#include <QSpinBox>
#include <QPushButton>
#include <QColorDialog>
#include "Shapes/Shape.h"

class ToolBar : public QToolBar {
    Q_OBJECT

public:
    enum Tool {
        SelectTool,
        LineTool,
        CircleTool,
        RectangleTool,
        FreehandTool,
        PolygonTool,
        RegularPolygonTool
    };

    explicit ToolBar(QWidget* parent = nullptr);
    
    QColor currentColor() const { return m_currentColor; }
    int32_t currentPenWidth() const { return m_penWidthSpinBox->value(); }
    Tool currentTool() const { return m_currentTool; }

    QAction* undoAction() const { return m_undoAction; }
    QAction* redoAction() const { return m_redoAction; }

    void setFillEnabled(bool enabled) { m_fillEnabled = enabled; }
    bool isFillEnabled() const { return m_fillEnabled; }
    QColor fillColor() const {return m_fillColor; }

    int32_t sidesCount() const { return m_sidesSpinBox->value(); }

signals:
    void toolChanged(Tool tool);
    void colorChanged(const QColor &color);
    void fillColorChanged(const QColor &color, bool);
    void penWidthChanged(int width);
    void undoRequested();
    void redoRequested();
    void clearCanvasRequested();
    void rotationChanged(double angle);
    void resizeRequested(const QSize& newSize);
    void polygonSidesRequested(int32_t sides);

public slots:
    void setRotation(double angle);
    void setSize(const QSize& size);
    void setPenColor(const QColor& color);
    void setPenWidth(int32_t penWidth);
    void setFillParams(const QColor& fillColor, bool isFilled);

private slots:
    void updateColorButton();
    void updateFillColorButton();
    void handleToolAction(QAction *action);

private:
    void createActions();
    void setupUi();

    Tool m_currentTool = SelectTool;
    QColor m_currentColor = Qt::black;
    QColor m_fillColor;
    bool m_fillEnabled;
    
    QActionGroup* m_toolActionGroup;
    
    // Tools
    QAction* m_selectAction;
    QAction* m_lineAction;
    QAction* m_circleAction;
    QAction* m_rectAction;
    QAction* m_freehandAction;
    QAction* m_polygonAction;
    QAction* m_regularPolygonAction;
    
    // Other actions
    QAction* m_undoAction;
    QAction* m_redoAction;
    QAction* m_clearAction;
    
    // Widgets
    QPushButton* m_colorButton;
    QPushButton* m_fillColorButton;
    QSpinBox* m_penWidthSpinBox;
    QSpinBox* m_rotationSpinBox;
    QSpinBox* m_widthSpinBox;
    QSpinBox* m_heightSpinBox;
    QSpinBox* m_sidesSpinBox;
};

#endif // TOOLBAR_H