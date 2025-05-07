#ifndef LINESHAPE_H
#define LINESHAPE_H

#include "Shape.h"

class LineShape : public Shape {
public:
    LineShape() = default;
    LineShape(QPoint from, QPoint to);

    void draw(QPainter& painter) const override;
    bool contains(const QPoint& pos) const override;
    void moveBy(int dx, int dy) override;
    void resize(const QSize& size) override;
    void rotate(double angle) override;
    void update(const QPoint& toPoint) override;
    QString name() const override;

    QRect boundingRect() const override;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& obj) override;

    void animateStep() override;

private:
    QPoint p1, p2;

    double m_angle = 0.0;
};

#endif // LINESHAPE_H
