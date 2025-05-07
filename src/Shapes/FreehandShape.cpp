#include "../../include/Shapes/FreehandShape.h"
#include <QPainter>
#include <QDataStream>
#include <algorithm>
#include <QDebug>

void FreehandShape::draw(QPainter& painter) const
{
    if (m_points.size() < 2) return;
    
    painter.save();
    QPen pen(color, penWidth);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    
    if (rotation_ != 0.0) {
        QPoint center = m_boundingRect.center();
        painter.translate(center);
        painter.rotate(rotation_);
        painter.translate(-center);
    }
    
    painter.drawPolyline(m_points.constData(), m_points.size());
    painter.restore();
}

bool FreehandShape::contains(const QPoint& pos) const {
    if (m_points.size() < 2) return false;
    
    QPointF point = pos;
    
    if (rotation_ != 0.0) {
        QPoint center = m_boundingRect.center();
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.rotate(-rotation_);
        transform.translate(-center.x(), -center.y());
        point = transform.map(point);
    }
    
    QRect bounds = boundingRect();
    if (!bounds.contains(point.toPoint())) {
        return false;
    }
    
    const qreal maxDistance = penWidth/2 + 5;
    
    for (size_t i = 1; i < m_points.size(); ++i) {
        QLineF line(m_points[i-1], m_points[i]);
        QLineF normal = line.normalVector();
        
        qreal distance = std::abs((normal.dx()*(point.x()-line.p1().x()) + 
                                 normal.dy()*(point.y()-line.p1().y())) / 
                        std::sqrt(normal.dx()*normal.dx() + normal.dy()*normal.dy()));
        
        bool withinSegmentBounds = 
            point.x() >= std::min(line.p1().x(), line.p2().x()) - maxDistance &&
            point.x() <= std::max(line.p1().x(), line.p2().x()) + maxDistance &&
            point.y() >= std::min(line.p1().y(), line.p2().y()) - maxDistance &&
            point.y() <= std::max(line.p1().y(), line.p2().y()) + maxDistance;
        
        if (distance <= maxDistance && withinSegmentBounds) {
            return true;
        }
    }
    return false;
}

void FreehandShape::moveBy(int32_t dx, int32_t dy)
{
    for (QPoint& point : m_points) {
        point.rx() += dx;
        point.ry() += dy;
    }
    m_boundingRect.translate(dx, dy);
}

void FreehandShape::resize(const QSize& size) {
    if (m_points.isEmpty() || m_boundingRect.width() == 0 || m_boundingRect.height() == 0) 
        return;
    
    QPointF center = m_boundingRect.center();
    qreal scaleX = size.width() / (qreal)m_boundingRect.width();
    qreal scaleY = size.height() / (qreal)m_boundingRect.height();
    
    for (QPoint& point : m_points) {
        point.rx() = center.x() + (point.x() - center.x()) * scaleX;
        point.ry() = center.y() + (point.y() - center.y()) * scaleY;
    }
    
    if (!m_points.isEmpty()) {
        m_boundingRect = QRect(m_points.first(), QSize(1, 1));
        for (const QPoint& point : m_points) {
            m_boundingRect.setRight(std::max(m_boundingRect.right(), point.x()));
            m_boundingRect.setLeft(std::min(m_boundingRect.left(), point.x()));
            m_boundingRect.setBottom(std::max(m_boundingRect.bottom(), point.y()));
            m_boundingRect.setTop(std::min(m_boundingRect.top(), point.y()));
        }
    }
}

void FreehandShape::rotate(double angle)
{
    rotation_ = angle;
}

void FreehandShape::update(const QPoint& newPoint)
{
    if (m_points.isEmpty()) {
        m_boundingRect = QRect(newPoint, QSize(1, 1));
    } else {
        m_boundingRect.setRight(std::max(m_boundingRect.right(), newPoint.x()));
        m_boundingRect.setLeft(std::min(m_boundingRect.left(), newPoint.x()));
        m_boundingRect.setBottom(std::max(m_boundingRect.bottom(), newPoint.y()));
        m_boundingRect.setTop(std::min(m_boundingRect.top(), newPoint.y()));
    }
    m_points.append(newPoint);
}

QString FreehandShape::name() const {
    return "Freehand";
}

QRect FreehandShape::boundingRect() const {
    if (m_points.isEmpty()) return QRect();

    QPainterPath path;
    path.moveTo(m_points.first());
    for (const QPoint& pt : m_points) {
        path.lineTo(pt);
    }

    QTransform transform;
    transform.translate(m_boundingRect.center().x(), m_boundingRect.center().y());
    transform.rotate(rotation_);
    transform.translate(-m_boundingRect.center().x(), -m_boundingRect.center().y());

    path = transform.map(path);
    return path.boundingRect().adjusted(-penWidth / 2, -penWidth / 2, penWidth / 2, penWidth / 2).toRect();
}

QJsonObject FreehandShape::toJson() const {
    QJsonObject obj;
    QJsonArray pointArray;
    for (const QPoint& pt : m_points) {
        QJsonObject pObj;
        pObj["x"] = pt.x();
        pObj["y"] = pt.y();
        pointArray.append(pObj);
    }

    obj["points"] = pointArray;
    obj["color"] = color.name();
    obj["width"] = penWidth;
    obj["rotation"] = rotation_;
    return obj;
}

void FreehandShape::fromJson(const QJsonObject& obj) {
    m_points.clear();
    QJsonArray pointArray = obj["points"].toArray();
    for (const QJsonValue& val : pointArray) {
        QJsonObject pObj = val.toObject();
        m_points.append(QPoint(pObj["x"].toInt(), pObj["y"].toInt()));
    }

    color = QColor(obj["color"].toString());
    penWidth = obj["width"].toInt();
    rotation_ = obj["rotation"].toDouble();

    if (!m_points.isEmpty()) {
        m_boundingRect = QRect(m_points.first(), QSize(1, 1));
        for (const QPoint& pt : m_points) {
            m_boundingRect.setLeft(std::min(m_boundingRect.left(), pt.x()));
            m_boundingRect.setRight(std::max(m_boundingRect.right(), pt.x()));
            m_boundingRect.setTop(std::min(m_boundingRect.top(), pt.y()));
            m_boundingRect.setBottom(std::max(m_boundingRect.bottom(), pt.y()));
        }
    }
}

void FreehandShape::animateStep() {
    if (m_points.isEmpty()) return;

    QRect rect = boundingRect();
    QPointF center = rect.center();
    QTransform tr;
    m_angle += 0.1;
    tr.translate(center.x(), center.y());
    tr.rotate(m_angle);
    tr.translate(-center.x(), -center.y());

    QVector<QPoint> rotated;
    for (const auto& pt : m_points)
        rotated.append(tr.map(pt));
    m_points = rotated;

    m_hue = (m_hue + 5) % 360;
    color.setHsv(m_hue, 255, 255);
}
