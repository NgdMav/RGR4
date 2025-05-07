#ifndef CANVASWIDGET_H
#define CANVASWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QPoint>
#include <QList>
#include <QStack>
#include <QColor>
#include <QTimer>
#include <QPixmap>
#include <memory>
#include "Shapes/Shape.h"
#include "ToolBar.h"

class CanvasWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CanvasWidget(QWidget *parent = nullptr);
    
    void setTool(ToolBar::Tool tool);
    void setPenColor(const QColor &color);
    void setPenWidth(int32_t width);
    
    bool saveToFile(const QString &fileName);
    bool exportAsImage(const QString& filePath);
    bool loadFromFile(const QString &fileName);
    bool loadBackgroundImage(const QString& filePath);
    
    bool isModified() const { return m_modified; }

public slots:
    void undo();
    void redo();
    void clear();
    void rotateSelectedShape(double angle);
    void resizeSelectedShape(const QSize& newSize);
    void resizePolygonSides(int32_t sides);
    void deleteSelectedShape();
    QList<QString> getShapeList() const;
    void selectShapeFromList(size_t index);
    void stopAllAnimations();
    void moveShapeUp();
    void moveShapeDown();
    void setFillColor(const QColor& color, bool enabled);

signals:
    void modificationChanged(bool modified);
    void undoAvailable(bool available);
    void redoAvailable(bool available);
    void shapeSelected(QString shapeName);
    void shapeListChanged();
    void updateShapeParameters(const QColor& penColor, int32_t penWidth, const QColor& fillColor, bool isFilled, const QSize& size, double rotation);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    struct CanvasState {
        QList<std::shared_ptr<Shape>> shapes;
    };

    ToolBar::Tool m_currentTool = ToolBar::SelectTool;
    QColor m_penColor = Qt::black;
    int32_t m_penWidth = 6;
    bool m_modified = false;

    QSize m_originalSize = QSize(800, 600);
    qreal m_scaleFactor = 1.0;
    
    QList<std::shared_ptr<Shape>> m_shapes;
    QStack<CanvasState> m_undoStack;
    QStack<CanvasState> m_redoStack;
    
    std::shared_ptr<Shape> m_currentShape = nullptr;
    std::shared_ptr<Shape> m_selectedShape = nullptr;
    QPoint m_lastPoint;
    bool m_isDrawing = false;
    
    void pushUndoState();
    void updateModification(bool modified);
    void checkUndoRedo();
    
    std::shared_ptr<Shape> createShape(ToolBar::Tool tool, const QPoint &startPoint);
    void selectShapeAt(const QPoint &pos);
    void moveSelectedShape(const QPoint &delta);
    void scaleShapes();

    QTimer m_animationTimer;

    enum DragMode { NoDrag, MoveDrag, ResizeDrag, RotateDrag };
    DragMode m_dragMode = NoDrag;

    QPixmap m_backgroundImage;
};

#endif // CANVASWIDGET_H