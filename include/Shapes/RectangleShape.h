#ifndef RECTANGLESHAPE_H
#define RECTANGLESHAPE_H

#include "Shape.h"

class RectangleShape : public Shape
{
public:
    RectangleShape() = default;
    RectangleShape(const QPoint& topLeft, const QPoint& bottomRight);
    
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
    QPoint m_topLeft;
    QPoint m_bottomRight;
    QColor fillColor;
    bool isFilled;


    int m_velocityY = -3;
    int m_bounceLimit = 20;
    int m_bounceProgress = 0;
    bool m_goingDown = false;
};

#endif // RECTANGLESHAPE_H