#ifndef CIRCLESHAPE_H
#define CIRCLESHAPE_H

#include "Shape.h"

class CircleShape : public Shape {
public:
    CircleShape() = default;
    CircleShape(const QPoint& topLeft, const QPoint& bottomRight);
    
    void draw(QPainter& painter) const override;
    bool contains(const QPoint& pos) const override;
    void moveBy(int32_t dx, int32_t dy) override;
    void resize(const QSize& size) override;
    void rotate(double angle) override;
    void update(const QPoint& toPoint) override;
    QString name() const override;

    void setFillColor(const QColor& color) override { fillColor = color; }
    void setFilled(bool filled) override { isFilled = filled; }
    QColor getFillColor() const override { return fillColor; }
    bool isShapeFilled() const override { return isFilled; }
    
    QRect boundingRect() const override;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& obj) override;

    void animateStep() override;

private:
    QRect m_rect;
    QColor fillColor;
    bool isFilled;

    QPoint m_velocity = {3, 3};
};

#endif // CIRCLESHAPE_H