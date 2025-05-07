#ifndef POLYGONSHAPE_H
#define POLYGONSHAPE_H

#include "../Shapes/Shape.h"
#include <QPolygon>

class PolygonShape : public Shape {
public:
    PolygonShape() = default;
    explicit PolygonShape(const QVector<QPoint>& points);

    void draw(QPainter& painter) const override;
    bool contains(const QPoint& pos) const override;
    void moveBy(int32_t dx, int32_t dy) override;
    void resize(const QSize& size) override;
    void rotate(double angle) override;
    void update(const QPoint& toPoint) override;
    QString name() const override { return "Polygon"; }

    void setFillColor(const QColor& color) override { fillColor = color; }
    void setFilled(bool filled) override { isFilled = filled; }
    QColor getFillColor() const override { return fillColor; }
    bool isShapeFilled() const override { return isFilled; }

    QRect boundingRect() const override;
    void addPoint(const QPoint& point);
    void finishShape();

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& obj) override;

private:
    QPolygon m_polygon;
    QColor fillColor = Qt::transparent;
    bool isFilled = false;
};

#endif // POLYGONSHAPE_H