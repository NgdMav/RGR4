#include "../include/CanvasWidget.h"
#include "../include/Shapes/LineShape.h"
#include "../include/Shapes/CircleShape.h"
#include "../include/Shapes/RectangleShape.h"
#include "../include/Shapes/FreehandShape.h"
#include "../include/Shapes/PolygonShape.h"
#include "../include/Shapes/RegularPolygonShape.h"
#include <QPainter>
#include <QMouseEvent>
#include <QFile>
#include <QDataStream>
#include <QMessageBox>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    m_originalSize = QSize(800, 600);
    resize(m_originalSize);

    connect(&m_animationTimer, &QTimer::timeout, this, [this]() {
        bool anyAnimated = false;
        for (const auto& shape : m_shapes) {
            if (shape->isAnimated()) {
                shape->animateStep();
                anyAnimated = true;
            }
        }
        if (anyAnimated) update();
    });
    m_animationTimer.start(30);
}

void CanvasWidget::setTool(ToolBar::Tool tool)
{
    m_currentTool = tool;
    if (tool != ToolBar::SelectTool){
        m_selectedShape = nullptr;
    }
    update();
}

void CanvasWidget::setPenColor(const QColor &color)
{
    m_penColor = color;
    if (m_selectedShape) {
        m_selectedShape->setColor(color);
        updateModification(true);
        update();
    }
}

void CanvasWidget::setPenWidth(int32_t width)
{
    m_penWidth = width;
    if (m_selectedShape) {
        m_selectedShape->setPenWidth(width);
        updateModification(true);
        update();
    }
}

void CanvasWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);

    painter.save();
    painter.scale(m_scaleFactor, m_scaleFactor);

    if (!m_backgroundImage.isNull()) {
        painter.drawPixmap(0, 0, m_backgroundImage.scaled(size() / m_scaleFactor));
    }

    for (const auto &shape : m_shapes) {
        shape->draw(painter);
        
        if (shape == m_selectedShape) {
            QRect rect = shape->boundingRect();
            QPen pen(Qt::DashLine);
            pen.setColor(Qt::blue);
            pen.setWidth(1);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(rect.adjusted(-2, -2, 2, 2));
        
            // Resize handle (bottom-right)
            painter.setBrush(Qt::blue);
            painter.drawRect(QRect(rect.bottomRight() - QPoint(5, 5), QSize(10, 10)));
        
            // Rotate handle (top-center)
            QPoint topCenter(rect.center().x(), rect.top() - 20);
            painter.setBrush(Qt::red);
            painter.drawEllipse(topCenter, 5, 5);
        }
        
    }
    painter.restore();

    if (m_currentShape) {
        painter.save();
        painter.scale(m_scaleFactor, m_scaleFactor);
        m_currentShape->draw(painter);
        painter.restore();
    }
}

void CanvasWidget::mousePressEvent(QMouseEvent *event)
{
    m_lastPoint = event->pos() / m_scaleFactor;
    if (event->button() == Qt::LeftButton && m_currentTool != ToolBar::PolygonTool) {
        
        if (m_currentTool == ToolBar::SelectTool) {
            if (m_selectedShape) {
                QRect rect = m_selectedShape->boundingRect();
                QRect resizeHandle(rect.bottomRight() - QPoint(5, 5), QSize(10, 10));
                QPoint rotateHandleCenter(rect.center().x(), rect.top() - 20);
                QRect rotateHandle(rotateHandleCenter - QPoint(5, 5), QSize(10, 10));

                if (resizeHandle.contains(m_lastPoint)) {
                    m_dragMode = ResizeDrag;
                    return;
                } else if (rotateHandle.contains(m_lastPoint)) {
                    m_dragMode = RotateDrag;
                    return;
                } else if (rect.contains(m_lastPoint)) {
                    m_dragMode = MoveDrag;
                    return;
                }
            }

            selectShapeAt(m_lastPoint);
            if (m_selectedShape)
                emit shapeSelected(m_selectedShape->name());
            else
                emit shapeSelected("");

            m_dragMode = NoDrag;
        } else {
            m_isDrawing = true;
            m_currentShape = createShape(m_currentTool, m_lastPoint);
        }
    }
    else if (event->button() == Qt::RightButton) {
        QPoint pos = event->pos() / m_scaleFactor;
        for (auto it = m_shapes.rbegin(); it != m_shapes.rend(); ++it) {
            if ((*it)->contains(pos)) {
                (*it)->setAnimated(true);
                return;
            }
        }
    }

    if (m_currentTool == ToolBar::PolygonTool) {
        if (event->button() == Qt::LeftButton) {
            if (!m_isDrawing) {
                m_isDrawing = true;
                m_currentShape = createShape(m_currentTool, m_lastPoint);
            } else {
                auto polygon = dynamic_cast<PolygonShape*>(m_currentShape.get());
                if (polygon) {
                    polygon->addPoint(m_lastPoint);
                }
            }
        } else if (event->button() == Qt::RightButton && m_isDrawing) {
            auto polygon = dynamic_cast<PolygonShape*>(m_currentShape.get());
            if (polygon && polygon->boundingRect().width() > 10) {
                polygon->finishShape();
                pushUndoState();
                m_shapes.append(m_currentShape);
                emit shapeListChanged();
                updateModification(true);
                m_currentShape = nullptr;
                m_isDrawing = false;
            }
        }
        update();
        return;
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint currentPos = event->pos() / m_scaleFactor;

    if (m_isDrawing && m_currentShape) {
        m_currentShape->update(currentPos);
        m_selectedShape = m_currentShape;
        emit updateShapeParameters(m_selectedShape->getColor(), m_selectedShape->getPenWidth(),
         m_selectedShape->getFillColor(), m_selectedShape->isShapeFilled(), m_selectedShape->boundingRect().size(), m_selectedShape->rotation());
        update();
    } 
    else if (m_currentTool == ToolBar::SelectTool && m_selectedShape &&
             (event->buttons() & Qt::LeftButton))
    {
        switch (m_dragMode) {
        case MoveDrag: {
            QPoint delta = currentPos - m_lastPoint;
            moveSelectedShape(delta);
            m_lastPoint = currentPos;
            break;
        }

        case ResizeDrag: {
            QRect rect = m_selectedShape->boundingRect();
            QPoint topLeft = rect.topLeft();
            QPoint delta = currentPos - topLeft;
            QSize newSize(delta.x(), delta.y());
            newSize.setWidth(qMax(10, newSize.width()));
            newSize.setHeight(qMax(10, newSize.height()));
            resizeSelectedShape(newSize);
            emit updateShapeParameters(m_selectedShape->getColor(), m_selectedShape->getPenWidth(),
            m_selectedShape->getFillColor(), m_selectedShape->isShapeFilled(), m_selectedShape->boundingRect().size(), m_selectedShape->rotation());
            break;
        }

        case RotateDrag: {
            QRect rect = m_selectedShape->boundingRect();
            QPointF center = rect.center();

            QPointF vecCurrent = currentPos - center;

            qreal angleRad = std::atan2(vecCurrent.y(), vecCurrent.x()) - std::atan2(center.y(), center.x());
            qreal angleDeg = angleRad * 180.0 / M_PI;

            rotateSelectedShape(angleDeg);
            emit updateShapeParameters(m_selectedShape->getColor(), m_selectedShape->getPenWidth(),
            m_selectedShape->getFillColor(), m_selectedShape->isShapeFilled(), m_selectedShape->boundingRect().size(), m_selectedShape->rotation());

            m_lastPoint = currentPos;
            break;
        }

        default:
            break;
        }

        update();
    }
}


void CanvasWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_isDrawing && m_currentShape) {
            QPoint currentPos = event->pos() / m_scaleFactor;
            m_currentShape->update(currentPos);

            if (m_currentTool == ToolBar::PolygonTool){
                return;
            }

            if (m_currentShape->boundingRect().width() > 5 || 
                m_currentShape->boundingRect().height() > 5) {
                pushUndoState();
                m_shapes.append(m_currentShape);
                emit shapeListChanged();
                updateModification(true);
            }

            m_currentShape = nullptr;
            m_isDrawing = false;
            update();
        }

        m_dragMode = NoDrag;
    }
}

void CanvasWidget::resizeEvent(QResizeEvent *event)
{
    QSize newSize = event->size();
    qreal scaleX = static_cast<qreal>(newSize.width()) / m_originalSize.width();
    qreal scaleY = static_cast<qreal>(newSize.height()) / m_originalSize.height();
    m_scaleFactor = qMin(scaleX, scaleY);
    update();
}

void CanvasWidget::undo()
{
    if (!m_undoStack.isEmpty()) {
        m_redoStack.push({m_shapes});
        m_shapes = m_undoStack.pop().shapes;
        emit shapeListChanged();
        updateModification(true);
        checkUndoRedo();
        update();
    }
}

void CanvasWidget::redo()
{
    if (!m_redoStack.isEmpty()) {
        m_undoStack.push({m_shapes});
        m_shapes = m_redoStack.pop().shapes;
        emit shapeListChanged();
        updateModification(true);
        checkUndoRedo();
        update();
    }
}

void CanvasWidget::clear()
{
    if (!m_shapes.isEmpty()) {
        pushUndoState();
        m_shapes.clear();
        m_selectedShape = nullptr;
        emit shapeListChanged();
        updateModification(true);
        update();
    }
}

bool CanvasWidget::saveToFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) return false;

    QJsonObject root;
    root["version"] = 1;
    root["penColor"] = m_penColor.name();
    root["penWidth"] = m_penWidth;

    QJsonArray shapeArray;
    for (const auto& shape : m_shapes) {
        QJsonObject obj = shape->toJson();
        obj["type"] = shape->name();
        shapeArray.append(obj);
    }

    root["shapes"] = shapeArray;

    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();

    updateModification(false);
    return true;
}

bool CanvasWidget::exportAsImage(const QString& filePath) {
    QImage image(m_originalSize, QImage::Format_ARGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.scale(m_scaleFactor, m_scaleFactor);
    if (!m_backgroundImage.isNull()) {
        painter.drawPixmap(0, 0, m_backgroundImage.scaled(m_originalSize));
    }
    for (const auto &shape : m_shapes) {
        shape->draw(painter);
    }
    painter.end();

    return image.save(filePath);
}


bool CanvasWidget::loadFromFile(const QString &fileName) {
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return false;

    QJsonObject root = doc.object();
    if (!root.contains("shapes")) return false;

    m_shapes.clear();
    QJsonArray shapeArray = root["shapes"].toArray();
    for (const QJsonValue &val : shapeArray) {
        QJsonObject obj = val.toObject();
        QString type = obj["type"].toString();
        std::shared_ptr<Shape> shape;

        if (type == "Line") shape = std::make_shared<LineShape>();
        else if (type == "Circle") shape = std::make_shared<CircleShape>();
        else if (type == "Rectangle") shape = std::make_shared<RectangleShape>();
        else if (type == "Freehand") shape = std::make_shared<FreehandShape>();
        else if (type == "Polygon") shape = std::make_shared<PolygonShape>();
        else if (type == "RegularPolygon") shape = std::make_shared<RegularPolygonShape>();

        if (shape) {
            shape->fromJson(obj);
            m_shapes.append(shape);
        }
    }

    updateModification(false);
    emit shapeListChanged();
    update();
    return true;
}

bool CanvasWidget::loadBackgroundImage(const QString& filePath) {
    QPixmap img;
    if (!img.load(filePath)) return false;

    m_backgroundImage = img;
    update();
    return true;
}


// Private methods implementation
void CanvasWidget::pushUndoState()
{
    m_undoStack.push({m_shapes});
    m_redoStack.clear();
    checkUndoRedo();
}

void CanvasWidget::updateModification(bool modified)
{
    m_modified = modified;
    emit modificationChanged(modified);
}

void CanvasWidget::checkUndoRedo()
{
    emit undoAvailable(!m_undoStack.isEmpty());
    emit redoAvailable(!m_redoStack.isEmpty());
}

std::shared_ptr<Shape> CanvasWidget::createShape(ToolBar::Tool tool, const QPoint &startPoint)
{
    std::shared_ptr<Shape> shape;
    
    switch (tool) {
    case ToolBar::LineTool:
        shape = std::make_shared<LineShape>(startPoint, startPoint);
        break;
    case ToolBar::CircleTool:
        shape = std::make_shared<CircleShape>(startPoint, startPoint);
        break;
    case ToolBar::RectangleTool:
        shape = std::make_shared<RectangleShape>(startPoint, startPoint);
        break;
    case ToolBar::FreehandTool:
        shape = std::make_shared<FreehandShape>();
        shape->update(startPoint);
        break;
    case ToolBar::PolygonTool:
        shape = std::make_shared<PolygonShape>();
        shape->update(startPoint);
        break;
    case ToolBar::RegularPolygonTool:
        shape = std::make_shared<RegularPolygonShape>(startPoint, 10, 5);
        break;
    default:
        return nullptr;
    }
    
    shape->setColor(m_penColor);
    shape->setPenWidth(m_penWidth);
    return shape;
}

void CanvasWidget::selectShapeAt(const QPoint &pos) {
    m_selectedShape = nullptr;
    
    for (auto it = m_shapes.rbegin(); it != m_shapes.rend(); ++it) {
        if ((*it)->contains(pos)) {
            m_selectedShape = *it;
            emit shapeSelected(m_selectedShape->name());
            
            emit updateShapeParameters(m_selectedShape->getColor(), m_selectedShape->getPenWidth(),
            m_selectedShape->getFillColor(), m_selectedShape->isShapeFilled(), m_selectedShape->boundingRect().size(), m_selectedShape->rotation());
            break;
        }
    }
    
    update();
}

void CanvasWidget::moveSelectedShape(const QPoint &delta)
{
    if (m_selectedShape) {
        m_selectedShape->moveBy(delta.x(), delta.y());
        updateModification(true);
        update();
    }
}

void CanvasWidget::rotateSelectedShape(double angle) {
    if (m_selectedShape) {
        m_selectedShape->rotate(angle);
        updateModification(true);
        update();
    }
}

void CanvasWidget::resizeSelectedShape(const QSize& newSize) {
    if (m_selectedShape) {
        m_selectedShape->resize(newSize);
        updateModification(true);
        update();
    }
}

void CanvasWidget::resizePolygonSides(int32_t sides){
    auto polygon = dynamic_cast<RegularPolygonShape*>(m_selectedShape.get());
    if (m_selectedShape && polygon != nullptr) {
        polygon->setSides(sides);
        updateModification(true);
        update();
    }
}

void CanvasWidget::deleteSelectedShape() {
    if (m_selectedShape) {
        auto it = std::find(m_shapes.begin(), m_shapes.end(), m_selectedShape);
        if (it != m_shapes.end()) {
            pushUndoState();
            m_shapes.erase(it);
            m_selectedShape = nullptr;
            updateModification(true);
            update();
            emit shapeListChanged();
        }
    }
}

QList<QString> CanvasWidget::getShapeList() const {
    QList<QString> list;
    int32_t index = 0;
    for (const auto& shape : m_shapes) {
        list.append(shape->name() + " " + QString::number(index++));
    }
    return list;
}

void CanvasWidget::selectShapeFromList(size_t index) {
    if (index >= 0 && index < m_shapes.size()) {
        m_selectedShape = m_shapes[index];
        emit shapeSelected(m_selectedShape->name());

        emit updateShapeParameters(m_selectedShape->getColor(), m_selectedShape->getPenWidth(),
        m_selectedShape->getFillColor(), m_selectedShape->isShapeFilled(), m_selectedShape->boundingRect().size(), m_selectedShape->rotation());

        update();
    }
}

void CanvasWidget::stopAllAnimations() {
    for (const auto& shape : m_shapes) {
        shape->setAnimated(false);
    }
    update();
}

void CanvasWidget::moveShapeUp() {
    if (!m_selectedShape) return;
    
    auto it = std::find(m_shapes.begin(), m_shapes.end(), m_selectedShape);
    if (it != m_shapes.end() && it + 1 != m_shapes.end()) {
        pushUndoState();
        std::iter_swap(it, it + 1);
        updateModification(true);
        emit shapeListChanged();
        update();
    }
}

void CanvasWidget::moveShapeDown() {
    if (!m_selectedShape) return;
    
    auto it = std::find(m_shapes.begin(), m_shapes.end(), m_selectedShape);
    if (it != m_shapes.end() && it != m_shapes.begin()) {
        pushUndoState();
        std::iter_swap(it, it - 1);
        updateModification(true);
        emit shapeListChanged();
        update();
    }
}

void CanvasWidget::setFillColor(const QColor& color, bool enabled) {
    if (!m_selectedShape) return;

    m_selectedShape->setFillColor(color);
    m_selectedShape->setFilled(enabled);
    updateModification(true);
    update();
}

void CanvasWidget::scaleShapes()
{
    if (qFuzzyCompare(m_scaleFactor, 1.0)) {
        return;
    }
    update();
}