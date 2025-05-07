#ifndef SHAPE_H
#define SHAPE_H

#include <QPainter>
#include <QDataStream>
#include <QColor>
#include <QPoint>
#include <QRect>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QPainterPath>

class Shape {
public:
    virtual ~Shape() = default;

    virtual void draw(QPainter& painter) const = 0;
    virtual bool contains(const QPoint& pos) const = 0;
    virtual void moveBy(int32_t dx, int32_t dy) = 0;
    virtual void resize(const QSize& size) = 0;
    virtual void rotate(double angle) = 0;
    virtual void update(const QPoint& toPoint) = 0;
    virtual QString name() const = 0;

    virtual void setColor(const QColor& color) { this->color = color; }
    virtual void setPenWidth(int32_t width) { penWidth = width; }
    virtual void setRotation(double angle) { rotation_ = angle; }
    virtual void setAnimated(bool flag) { m_animated = flag; }
    virtual void setFillColor(const QColor& color) { Q_UNUSED(color); }
    virtual void setFilled(bool filled) { Q_UNUSED(filled); }

    virtual QColor getColor() const { return color; }
    virtual int32_t getPenWidth() const { return penWidth; }
    virtual double getRotation() const { return rotation_; }
    virtual bool isAnimated() const { return m_animated; }
    virtual double rotation() const {return rotation_;}
    virtual QColor getFillColor() const { return Qt::transparent; }
    virtual bool isShapeFilled() const { return false; }

    virtual QRect boundingRect() const = 0;

    virtual QJsonObject toJson() const = 0;
    virtual void fromJson(const QJsonObject& obj) = 0;

    virtual void animateStep() {}


protected:
    QColor color = Qt::black;
    int32_t penWidth = 2;
    double rotation_ = 0.0;
    bool m_animated = false;
};

#endif // SHAPE_H
