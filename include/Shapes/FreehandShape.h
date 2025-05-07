#ifndef FREEHANDSHAPE_H
#define FREEHANDSHAPE_H

#include "Shape.h"
#include <QVector>

class FreehandShape : public Shape
{
public:
    FreehandShape() = default;
    
    void draw(QPainter& painter) const override;
    bool contains(const QPoint& pos) const override;
    void moveBy(int32_t dx, int32_t dy) override;
    void resize(const QSize& size) override;
    void rotate(double angle) override;
    void update(const QPoint& toPoint) override;
    QString name() const override;
    
    QRect boundingRect() const override;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& obj) override;

    void animateStep() override;

private:
    QVector<QPoint> m_points;
    QRect m_boundingRect;

    double m_angle = 0.0;
    int32_t m_hue = 0;
};

#endif // FREEHANDSHAPE_H