#include "../../include/Shapes/PolygonShape.h"
#include <QPainter>
#include <QJsonArray>

PolygonShape::PolygonShape(const QVector<QPoint>& points) {
    m_polygon = QPolygon(points);
}

void PolygonShape::draw(QPainter& painter) const {
    painter.save();
    QPen pen(color, penWidth);
    painter.setPen(pen);
    
    if (isFilled) {
        painter.setBrush(fillColor);
    } else {
        painter.setBrush(Qt::NoBrush);
    }
    
    if (rotation_ != 0.0) {
        QPoint center = boundingRect().center();
        painter.translate(center);
        painter.rotate(rotation_);
        painter.translate(-center);
    }
    
    painter.drawPolygon(m_polygon);

    if (m_polygon.isDetached()) {
        painter.setBrush(Qt::red);
        for (const QPoint& p : m_polygon) {
            painter.drawEllipse(p, 3, 3);
        }
    }
        
    //     if (!m_polygon.isEmpty()) {
    //         painter.setBrush(Qt::green);
    //         painter.drawEllipse(m_polygon.last(), 4, 4);
    //     }
    // }

    painter.restore();
}

bool PolygonShape::contains(const QPoint& pos) const {
    QPainterPath path;
    path.addPolygon(m_polygon);
    
    if (rotation_ != 0.0) {
        QTransform transform;
        QPoint center = boundingRect().center();
        transform.translate(center.x(), center.y());
        transform.rotate(-rotation_);
        transform.translate(-center.x(), -center.y());
        path = transform.map(path);
    }
    
    QPainterPathStroker stroker;
    stroker.setWidth(penWidth);
    QPainterPath outline = stroker.createStroke(path);
    
    return outline.contains(pos) || path.contains(pos);
}

void PolygonShape::moveBy(int32_t dx, int32_t dy) {
    m_polygon.translate(dx, dy);
}

void PolygonShape::resize(const QSize& size) {
    if (m_polygon.size() < 2) return;
    
    QRect oldRect = m_polygon.boundingRect();
    if (oldRect.width() == 0 || oldRect.height() == 0) return;
    
    qreal scaleX = static_cast<qreal>(size.width()) / oldRect.width();
    qreal scaleY = static_cast<qreal>(size.height()) / oldRect.height();
    
    for (int i = 0; i < m_polygon.size(); ++i) {
        QPoint p = m_polygon[i];
        p.setX(oldRect.left() + (p.x() - oldRect.left()) * scaleX);
        p.setY(oldRect.top() + (p.y() - oldRect.top()) * scaleY);
        m_polygon[i] = p;
    }
}

void PolygonShape::rotate(double angle) {
    rotation_ = angle;
}

void PolygonShape::update(const QPoint& toPoint) {
    if (m_polygon.isEmpty()) {
        m_polygon << toPoint << toPoint;
    } else {
        m_polygon.last() = toPoint;
    }
}

QRect PolygonShape::boundingRect() const {
    QPainterPath path;
    path.addPolygon(m_polygon);
    
    if (rotation_ != 0.0) {
        QTransform transform;
        QPoint center = m_polygon.boundingRect().center();
        transform.translate(center.x(), center.y());
        transform.rotate(rotation_);
        transform.translate(-center.x(), -center.y());
        path = transform.map(path);
    }
    
    return path.boundingRect().adjusted(-penWidth/2, -penWidth/2, penWidth/2, penWidth/2).toRect();
}

void PolygonShape::addPoint(const QPoint& point) {
    m_polygon << point;
}

void PolygonShape::finishShape() {
    if (m_polygon.size() > 2) {
        //if (m_polygon.size() > 2 && m_polygon.isDetached()) {
        m_polygon << m_polygon.first();
        
        // if (m_polygon.size() > 3 && m_polygon.first() == m_polygon.last()) {
        //     m_polygon.removeLast();
        // }
    }
}

QJsonObject PolygonShape::toJson() const {
    QJsonObject obj;
    obj["type"] = "Polygon";
    
    QJsonArray pointsArray;
    for (const QPoint& point : m_polygon) {
        QJsonObject pointObj;
        pointObj["x"] = point.x();
        pointObj["y"] = point.y();
        pointsArray.append(pointObj);
    }
    obj["points"] = pointsArray;
    obj["color"] = color.name();
    obj["penWidth"] = penWidth;
    obj["rotation"] = rotation_;
    obj["fillColor"] = fillColor.name();
    obj["isFilled"] = isFilled;
    
    return obj;
}

void PolygonShape::fromJson(const QJsonObject& obj) {
    m_polygon.clear();
    QJsonArray pointsArray = obj["points"].toArray();
    for (const QJsonValue& val : pointsArray) {
        QJsonObject pointObj = val.toObject();
        m_polygon << QPoint(pointObj["x"].toInt(), pointObj["y"].toInt());
    }
    
    color = QColor(obj["color"].toString());
    penWidth = obj["penWidth"].toInt();
    rotation_ = obj["rotation"].toDouble();
    fillColor = QColor(obj["fillColor"].toString());
    isFilled = obj["isFilled"].toBool();
}