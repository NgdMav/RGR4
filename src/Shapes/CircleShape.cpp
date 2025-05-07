#include "../../include/Shapes/CircleShape.h"
#include <QPainter>
#include <QPoint>
#include <QColor>
#include <QDataStream>
#include <cmath>
#include <QDebug>

CircleShape::CircleShape(const QPoint& topLeft, const QPoint& bottomRight) {
    m_rect = QRect(topLeft, bottomRight).normalized();
}

void CircleShape::draw(QPainter& painter) const {
    painter.save();
    QPen pen(color, penWidth);
    painter.setPen(pen);
    
    if (isFilled) {
        painter.setBrush(fillColor);
    } else {
        painter.setBrush(Qt::NoBrush);
    }
    
    if (rotation_ != 0.0) {
        QPoint center = m_rect.center();
        painter.translate(center);
        painter.rotate(rotation_);
        painter.translate(-center);
    }
    
    painter.drawEllipse(m_rect);
    painter.restore();
}

bool CircleShape::contains(const QPoint& pos) const {
    QRect normRect = m_rect.normalized();
    
    if (normRect.width() <= 1 && normRect.height() <= 1) {
        return normRect.contains(pos);
    }
    
    QPointF center = normRect.center();
    qreal a = normRect.width() / 2.0;
    qreal b = normRect.height() / 2.0;
    
    QPointF point = pos;
    
    if (rotation_ != 0.0) {
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.rotate(-rotation_);
        transform.translate(-center.x(), -center.y());
        point = transform.map(point);
    }
    
    qreal x = point.x() - center.x();
    qreal y = point.y() - center.y();
    
    qreal distance = (x*x)/(a*a) + (y*y)/(b*b);
    
    qreal penWidthFactor = penWidth / qMin(a, b);
    return distance <= (1.0 + penWidthFactor) * (1.0 + penWidthFactor) && 
           distance >= (1.0 - penWidthFactor) * (1.0 - penWidthFactor);
}

void CircleShape::moveBy(int32_t dx, int32_t dy) {
    m_rect.translate(dx, dy);
}

void CircleShape::resize(const QSize& size) {
    //QPoint center = m_rect.center();
    m_rect.setSize(size);
    //m_rect.moveCenter(center);
}

void CircleShape::rotate(double angle) {
    rotation_ = angle;
}

void CircleShape::update(const QPoint& toPoint) {
    QPoint fixedPoint = m_rect.topLeft();
    QRect newRect(fixedPoint, toPoint);
    m_rect = newRect;
}

QString CircleShape::name() const {
    return "Circle";
}

QRect CircleShape::boundingRect() const {
    QPainterPath path;
    path.addEllipse(m_rect);

    QTransform transform;
    transform.translate(m_rect.center().x(), m_rect.center().y());
    transform.rotate(rotation_);
    transform.translate(-m_rect.center().x(), -m_rect.center().y());

    path = transform.map(path);
    return path.boundingRect().adjusted(-penWidth / 2, -penWidth / 2, penWidth / 2, penWidth / 2).toRect();
}


QJsonObject CircleShape::toJson() const {
    QJsonObject obj;
    obj["type"] = "Circle";
    obj["x"] = m_rect.x();
    obj["y"] = m_rect.y();
    obj["width"] = m_rect.width();
    obj["height"] = m_rect.height();
    obj["color"] = color.name();
    obj["penWidth"] = penWidth;
    obj["rotation"] = rotation_;
    obj["fillColor"] = fillColor.name();
    obj["isFilled"] = isFilled;
    return obj;
}

void CircleShape::fromJson(const QJsonObject& obj) {
    int x = obj["x"].toInt();
    int y = obj["y"].toInt();
    int w = obj["width"].toInt();
    int h = obj["height"].toInt();
    m_rect = QRect(x, y, w, h);

    color = QColor(obj["color"].toString());
    penWidth = obj["penWidth"].toInt();
    rotation_ = obj["rotation"].toDouble();
    fillColor = QColor(obj["fillColor"].toString());
    isFilled = obj["isFilled"].toBool();
}

void CircleShape::animateStep() {
    m_rect.translate(m_velocity);

    const int32_t canvasWidth = 800;
    const int32_t canvasHeight = 600;

    if (m_rect.left() < 0) {
        m_rect.moveLeft(0);
        m_velocity.setX(-m_velocity.x());
    }
    if (m_rect.right() > canvasWidth) {
        m_rect.moveRight(canvasWidth);
        m_velocity.setX(-m_velocity.x());
    }

    if (m_rect.top() < 0) {
        m_rect.moveTop(0);
        m_velocity.setY(-m_velocity.y());
    }
    if (m_rect.bottom() > canvasHeight) {
        m_rect.moveBottom(canvasHeight);
        m_velocity.setY(-m_velocity.y());
    }
}
