#include "../../include/Shapes/RectangleShape.h"
#include <QPainter>
#include <QDataStream>
#include <cmath>
#include <QDebug>

RectangleShape::RectangleShape(const QPoint& topLeft, const QPoint& bottomRight)
    : m_topLeft(topLeft), m_bottomRight(bottomRight) {}

void RectangleShape::draw(QPainter& painter) const
{
    painter.save();
    QPen pen(color, penWidth);
    painter.setPen(pen);
    
    if (isFilled) {
        painter.setBrush(fillColor);
    } else {
        painter.setBrush(Qt::NoBrush);
    }
    
    if (rotation_ != 0.0) {
        QPoint center = (m_topLeft + m_bottomRight) / 2;
        painter.translate(center);
        painter.rotate(rotation_);
        painter.translate(-center);
    }
    
    painter.drawRect(QRect(m_topLeft, m_bottomRight));
    painter.restore();
}

bool RectangleShape::contains(const QPoint& pos) const {
    QRect rect(m_topLeft, m_bottomRight);
    rect = rect.normalized();
    
    if (rotation_ == 0.0) {
        QRect outer = rect.adjusted(-penWidth/2, -penWidth/2, penWidth/2, penWidth/2);
        QRect inner = rect.adjusted(penWidth/2, penWidth/2, -penWidth/2, -penWidth/2);
        return outer.contains(pos) && !inner.contains(pos);
    } else {
        QPoint center = rect.center();
        QPointF rotatedPos = pos;
        
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.rotate(-rotation_);
        transform.translate(-center.x(), -center.y());
        rotatedPos = transform.map(rotatedPos);
        
        QRect outer = rect.adjusted(-penWidth/2, -penWidth/2, penWidth/2, penWidth/2);
        QRect inner = rect.adjusted(penWidth/2, penWidth/2, -penWidth/2, -penWidth/2);
        return outer.contains(rotatedPos.toPoint()) && !inner.contains(rotatedPos.toPoint());
    }
}

void RectangleShape::moveBy(int32_t dx, int32_t dy)
{
    m_topLeft.rx() += dx;
    m_topLeft.ry() += dy;
    m_bottomRight.rx() += dx;
    m_bottomRight.ry() += dy;
}

void RectangleShape::resize(const QSize& size) {
    //QPoint center = (m_topLeft + m_bottomRight) / 2;
    QPoint center = m_topLeft;
    QSize oldSize = QRect(m_topLeft, m_bottomRight).normalized().size();
    
    if (oldSize.width() == 0 || oldSize.height() == 0)
        return;
    
    qreal scaleX = (qreal)size.width() / oldSize.width();
    qreal scaleY = (qreal)size.height() / oldSize.height();
    
    QPointF topLeftOffset = m_topLeft - center;
    QPointF bottomRightOffset = m_bottomRight - center;
    
    //not need)
    m_topLeft = center + QPoint(
        static_cast<int>(round(topLeftOffset.x() * scaleX)),
        static_cast<int>(round(topLeftOffset.y() * scaleY))
    );
    
    m_bottomRight = center + QPoint(
        static_cast<int>(round(bottomRightOffset.x() * scaleX)),
        static_cast<int>(round(bottomRightOffset.y() * scaleY))
    );
}

void RectangleShape::rotate(double angle)
{
    rotation_ = angle;
}

void RectangleShape::update(const QPoint& toPoint) {
    m_bottomRight = toPoint;
}

QString RectangleShape::name() const {
    return "Rectangle";
}

QRect RectangleShape::boundingRect() const {
    QRect rect = QRect(m_topLeft, m_bottomRight).normalized();

    QPainterPath path;
    path.addRect(rect);

    QTransform transform;
    transform.translate(rect.center().x(), rect.center().y());
    transform.rotate(rotation_);
    transform.translate(-rect.center().x(), -rect.center().y());

    path = transform.map(path);
    return path.boundingRect().adjusted(-penWidth / 2, -penWidth / 2, penWidth / 2, penWidth / 2).toRect();
}
   

QJsonObject RectangleShape::toJson() const {
    QJsonObject obj;
    obj["x1"] = m_topLeft.x();
    obj["y1"] = m_topLeft.y();
    obj["x2"] = m_bottomRight.x();
    obj["y2"] = m_bottomRight.y();
    obj["color"] = color.name();
    obj["width"] = penWidth;
    obj["rotation"] = rotation_;
    obj["fillColor"] = fillColor.name();
    obj["isFilled"] = isFilled;
    return obj;
}

void RectangleShape::fromJson(const QJsonObject& obj) {
    m_topLeft = QPoint(obj["x1"].toInt(), obj["y1"].toInt());
    m_bottomRight = QPoint(obj["x2"].toInt(), obj["y2"].toInt());
    color = QColor(obj["color"].toString());
    penWidth = obj["width"].toInt();
    rotation_ = obj["rotation"].toDouble();
    fillColor = QColor(obj["fillColor"].toString());
    isFilled = obj["isFilled"].toBool();
}

void RectangleShape::animateStep() {
    int32_t dy = m_goingDown ? 3 : -3;
    m_topLeft.ry() += dy;
    m_bottomRight.ry() += dy;

    m_bounceProgress += std::abs(dy);
    if (m_bounceProgress >= m_bounceLimit) {
        m_bounceProgress = 0;
        m_goingDown = !m_goingDown;
    }
}
