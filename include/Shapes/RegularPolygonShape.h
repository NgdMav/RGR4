#ifndef REGULARPOLYGONSHAPE_H
#define REGULARPOLYGONSHAPE_H

#include "../Shapes/Shape.h"
#include <QPolygon>

class RegularPolygonShape : public Shape {
public:
    RegularPolygonShape() = default;
    RegularPolygonShape(const QPoint& center, int radius, int sides);

    void draw(QPainter& painter) const override;
    bool contains(const QPoint& pos) const override;
    void moveBy(int32_t dx, int32_t dy) override;
    void resize(const QSize& size) override;
    void rotate(double angle) override;
    void update(const QPoint& toPoint) override;
    QString name() const override { return "RegularPolygon"; }

    void setFillColor(const QColor& color) override { fillColor = color; }
    void setFilled(bool filled) override { isFilled = filled; }
    QColor getFillColor() const override { return fillColor; }
    bool isShapeFilled() const override { return isFilled; }

    void setSides(int32_t sides);
    int getSides() const { return m_sides; }

    QRect boundingRect() const override;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& obj) override;

private:
    void updatePolygon();

    QPoint m_center;
    int32_t m_radius = 0;
    int32_t m_sides = 5;
    QPolygon m_polygon;
    QColor fillColor = Qt::transparent;
    bool isFilled = false;
};

#endif // REGULARPOLYGONSHAPE_H