#include "../../include/Shapes/LineShape.h"
#include <QPainter>
#include <QDataStream>
#include <cmath>
#include <QDebug>

LineShape::LineShape(QPoint from, QPoint to) 
    : p1(from), p2(to) {}

void LineShape::draw(QPainter& painter) const {
    painter.save();
    QPen pen(color, penWidth);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    
    if (rotation_ != 0.0) {
        QPoint center = (p1 + p2) / 2;
        painter.translate(center);
        painter.rotate(rotation_);
        painter.translate(-center);
    }
    
    painter.drawLine(p1, p2);
    painter.restore();
}

bool LineShape::contains(const QPoint& pos) const {
    if (rotation_ == 0.0) {
        QLineF line(p1, p2);
        QLineF normal = line.normalVector();
        QPointF point(pos);
        
        qreal distance = std::abs((normal.dx()*(point.x()-p1.x()) + normal.dy()*(point.y()-p1.y())) / 
                         std::sqrt(normal.dx()*normal.dx() + normal.dy()*normal.dy()));
        
        return distance <= penWidth/2 + 3 &&
               point.x() >= std::min(p1.x(), p2.x()) - penWidth/2 &&
               point.x() <= std::max(p1.x(), p2.x()) + penWidth/2 &&
               point.y() >= std::min(p1.y(), p2.y()) - penWidth/2 &&
               point.y() <= std::max(p1.y(), p2.y()) + penWidth/2;
    } else {
        QPoint center = (p1 + p2) / 2;
        QPointF rotatedPos = pos;
        
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.rotate(-rotation_);
        transform.translate(-center.x(), -center.y());
        rotatedPos = transform.map(rotatedPos);
        
        QLineF line(p1, p2);
        QLineF normal = line.normalVector();
        
        qreal distance = std::abs((normal.dx()*(rotatedPos.x()-p1.x()) + 
                                 normal.dy()*(rotatedPos.y()-p1.y())) / 
                        std::sqrt(normal.dx()*normal.dx() + normal.dy()*normal.dy()));
        
        return distance <= penWidth/2 + 3 &&
               rotatedPos.x() >= std::min(p1.x(), p2.x()) - penWidth/2 &&
               rotatedPos.x() <= std::max(p1.x(), p2.x()) + penWidth/2 &&
               rotatedPos.y() >= std::min(p1.y(), p2.y()) - penWidth/2 &&
               rotatedPos.y() <= std::max(p1.y(), p2.y()) + penWidth/2;
    }
}

void LineShape::moveBy(int dx, int dy) {
    p1.rx() += dx; p1.ry() += dy;
    p2.rx() += dx; p2.ry() += dy;
}

void LineShape::resize(const QSize& size) {
    QPoint center = (p1 + p2) / 2;
    QPoint delta = p2 - p1;
    qreal length = std::sqrt(delta.x()*delta.x() + delta.y()*delta.y());
    
    if (length > 0) {
        qreal scaleX = size.width() / length;
        qreal scaleY = size.height() / length;
        qreal scale = std::max(scaleX, scaleY);
        
        delta = delta * scale;
        p1 = center - delta/2;
        p2 = center + delta/2;
    }
}

void LineShape::rotate(double angle) {
    rotation_ = angle;
}

void LineShape::update(const QPoint& toPoint) {
    p2 = toPoint;
}

QString LineShape::name() const {
    return "Line";
}

QRect LineShape::boundingRect() const {
    QPainterPath path;
    path.moveTo(p1);
    path.lineTo(p2);

    QPointF center = (p1 + p2) / 2;

    QTransform transform;
    transform.translate(center.x(), center.y());
    transform.rotate(rotation_);
    transform.translate(-center.x(), -center.y());

    path = transform.map(path);
    return path.boundingRect().adjusted(-penWidth / 2, -penWidth / 2, penWidth / 2, penWidth / 2).toRect();
}

QJsonObject LineShape::toJson() const {
    QJsonObject obj;
    obj["p1x"] = p1.x();
    obj["p1y"] = p1.y();
    obj["p2x"] = p2.x();
    obj["p2y"] = p2.y();
    obj["color"] = color.name();
    obj["width"] = penWidth;
    obj["rotation"] = rotation_;
    return obj;
}

void LineShape::fromJson(const QJsonObject& obj) {
    p1 = QPoint(obj["p1x"].toInt(), obj["p1y"].toInt());
    p2 = QPoint(obj["p2x"].toInt(), obj["p2y"].toInt());
    color = QColor(obj["color"].toString());
    penWidth = obj["width"].toInt();
    rotation_ = obj["rotation"].toDouble();
}

void LineShape::animateStep() {
    QPointF center = (p1 + p2) / 2.0;
    QTransform rotator;
    m_angle += 2.0;
    rotator.translate(center.x(), center.y());
    rotator.rotate(m_angle);
    rotator.translate(-center.x(), -center.y());

    QPointF newP1 = rotator.map(p1);
    QPointF newP2 = rotator.map(p2);

    p1 = newP1.toPoint();
    p2 = newP2.toPoint();
}
