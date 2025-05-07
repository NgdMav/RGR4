#include "../../include/Shapes/RegularPolygonShape.h"
#include <QPainter>
#include <QtMath>

RegularPolygonShape::RegularPolygonShape(const QPoint& center, int radius, int sides) 
    : m_center(center), m_radius(radius), m_sides(sides) {
    updatePolygon();
}

void RegularPolygonShape::draw(QPainter& painter) const {
    painter.save();
    QPen pen(color, penWidth);
    painter.setPen(pen);
    
    if (isFilled) {
        painter.setBrush(fillColor);
    } else {
        painter.setBrush(Qt::NoBrush);
    }
    
    if (rotation_ != 0.0) {
        painter.translate(m_center);
        painter.rotate(rotation_);
        painter.translate(-m_center);
    }
    
    painter.drawPolygon(m_polygon);
    painter.restore();
}

bool RegularPolygonShape::contains(const QPoint& pos) const {
    QPainterPath path;
    path.addPolygon(m_polygon);
    
    if (rotation_ != 0.0) {
        QTransform transform;
        transform.translate(m_center.x(), m_center.y());
        transform.rotate(-rotation_);
        transform.translate(-m_center.x(), -m_center.y());
        path = transform.map(path);
    }
    
    QPainterPathStroker stroker;
    stroker.setWidth(penWidth);
    QPainterPath outline = stroker.createStroke(path);
    
    return outline.contains(pos) || path.contains(pos);
}

void RegularPolygonShape::moveBy(int32_t dx, int32_t dy) {
    m_center += QPoint(dx, dy);
    updatePolygon();
}

void RegularPolygonShape::resize(const QSize& size) {
    m_radius = qMin(size.width(), size.height()) / 2;
    updatePolygon();
}

void RegularPolygonShape::rotate(double angle) {
    rotation_ = angle;
}

void RegularPolygonShape::update(const QPoint& toPoint) {
    m_radius = qMax(10, static_cast<int>(QLineF(m_center, toPoint).length()));
    updatePolygon();
}

QRect RegularPolygonShape::boundingRect() const {
    QPainterPath path;
    path.addPolygon(m_polygon);
    
    if (rotation_ != 0.0) {
        QTransform transform;
        transform.translate(m_center.x(), m_center.y());
        transform.rotate(-rotation_);
        transform.translate(-m_center.x(), -m_center.y());
        path = transform.map(path);
    }
    
    return path.boundingRect().adjusted(-penWidth/2, -penWidth/2, penWidth/2, penWidth/2).toRect();
}

void RegularPolygonShape::setSides(int32_t sides) {
    m_sides = qMax(3, sides);
    updatePolygon();
}

void RegularPolygonShape::updatePolygon() {
    m_polygon.clear();
    double angleStep = 2 * M_PI / m_sides;
    
    for (int i = 0; i < m_sides; ++i) {
        double angle = i * angleStep;
        int x = m_center.x() + m_radius * cos(angle);
        int y = m_center.y() + m_radius * sin(angle);
        m_polygon << QPoint(x, y);
    }
}

QJsonObject RegularPolygonShape::toJson() const {
    QJsonObject obj;
    obj["type"] = "RegularPolygon";
    obj["centerX"] = m_center.x();
    obj["centerY"] = m_center.y();
    obj["radius"] = m_radius;
    obj["sides"] = m_sides;
    obj["color"] = color.name();
    obj["penWidth"] = penWidth;
    obj["rotation"] = rotation_;
    obj["fillColor"] = fillColor.name();
    obj["isFilled"] = isFilled;
    
    return obj;
}

void RegularPolygonShape::fromJson(const QJsonObject& obj) {
    m_center = QPoint(obj["centerX"].toInt(), obj["centerY"].toInt());
    m_radius = obj["radius"].toInt();
    m_sides = obj["sides"].toInt();
    
    color = QColor(obj["color"].toString());
    penWidth = obj["penWidth"].toInt();
    rotation_ = obj["rotation"].toDouble();
    fillColor = QColor(obj["fillColor"].toString());
    isFilled = obj["isFilled"].toBool();
    
    updatePolygon();
}