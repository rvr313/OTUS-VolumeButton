#include "volumebutton.h"
#include <array>
#include <QPainter>
#include <QStyleOptionSlider>
#include <QLinearGradient>
#include <QRect>
#include <QPoint>
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <QPainterPath>
#include <QFontMetrics>

VolumeButton::VolumeButton(QWidget *parent) : QDial(parent)
{
    setTracking(false);

    m_time_line.setDirection(QTimeLine::Forward);
    m_time_line.setDuration(300);
    m_time_line.setFrameRange(0,100);
    m_time_line.setEasingCurve(QEasingCurve::InQuad);

    connect(this, &QAbstractSlider::valueChanged,
        [this](int value)
        {
            m_prev_value = m_curr_value;
            m_curr_value = value;
            if (m_time_line.state() == QTimeLine::NotRunning) {
                m_time_line.start();
            }
        }
    );

    connect(&m_time_line, &QTimeLine::valueChanged, [this](qreal) { update(); });
}


void VolumeButton::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    constexpr double factor = 3.03030303030303;

    p.setRenderHint(QPainter::Antialiasing);

    QStyleOptionSlider opt;
    initStyleOption(&opt);

    const QColor &bc = opt.palette.color(QPalette::Window);

    int side = qMin(opt.rect.height(), opt.rect.width()) - 2 ;
    qreal iwidth = side / 24.0;
    qreal dw = iwidth / 2.0 + 1;
    qreal dh = iwidth / 2.0;
    qreal r = (side - iwidth) / 2.0;
    qreal hr = r * 0.5;
    qreal dr = hr * std::sqrt(3);

    QPoint topleft(static_cast<int>(dw) + ((opt.rect.width() > side) ? (opt.rect.width() - side) / 2 : 0),
                   static_cast<int>(dh) + ((opt.rect.height() > side) ? (opt.rect.height() - side) / 2 : 0));
    QRect rect(topleft, QSize(side - iwidth, side - iwidth));

    // paint termo-scale
    {
        p.save();
        QConicalGradient cg;
        cg.setCenter(rect.center());
        cg.setAngle(-120);
        cg.setColorAt(0, Qt::red);
        cg.setColorAt(0.1, Qt::red);
        cg.setColorAt(0.4, Qt::yellow);
        cg.setColorAt(1, Qt::green);
        p.setPen(QPen(QBrush(cg), iwidth + 2, Qt::SolidLine, Qt::PenCapStyle::FlatCap));
        p.drawArc(rect, -120 * 16, -300 * 16);
        p.restore();
    }

    // paint inactive part of scale in gray
    {
        int delta = (m_curr_value - m_prev_value) * m_time_line.currentValue();
        int posAngle =  static_cast<int>(factor * (m_prev_value + delta) + 0.5);

        p.save();
        p.setPen(QPen(bc, iwidth + 4, Qt::SolidLine, Qt::PenCapStyle::FlatCap));
        p.drawArc(rect, -60 * 16, (300 - posAngle) * 16);
        p.setPen(QPen(bc.darker(), iwidth / 3, Qt::SolidLine, Qt::PenCapStyle::FlatCap));
        p.drawArc(rect, -60 * 16, (300 - posAngle) * 16);
        p.restore();
    }

    // paint cicle under switcher
    {
        p.save();
        int sqside = rect.width() * 9 / 10;
        QRect drawingRect(-sqside/2, -sqside/2, sqside, sqside);
        drawingRect.translate(opt.rect.width()/2, opt.rect.height()/2);
        QConicalGradient gradient;
        gradient.setCenter(drawingRect.center());
        gradient.setAngle(270);
        gradient.setColorAt(0, bc.lighter());
        gradient.setColorAt(0.5, bc.darker());
        gradient.setColorAt(1, bc.lighter());
        p.setPen(Qt::NoPen);
        p.setBrush(gradient);
        p.drawEllipse(drawingRect);
        if (opt.state & QStyle::State_HasFocus) {
            p.setPen(QPen(Qt::darkCyan, 2));
            p.drawEllipse(drawingRect);
        }
        p.restore();
    }

    // paint min/max notches
    {
        p.save();
        QLine notch_line(-iwidth/2 - iwidth/10 - 1, 0, iwidth/2 + iwidth/10 + 1, 0);
        p.setPen(QPen(bc.darker(), 4, Qt::SolidLine, Qt::PenCapStyle::FlatCap));
        {
            QTransform transform;
            transform.translate(opt.rect.width() / 2 + hr + 1, opt.rect.height() / 2 + dr);
            transform.rotate(240);
            p.drawLine(transform.map(notch_line));
        }
        {
            QTransform transform;
            transform.translate(opt.rect.width() / 2 - hr + 1, opt.rect.height() / 2 + dr);
            transform.rotate(300);
            p.drawLine(transform.map(notch_line));
        }
        p.restore();
    }

    // paint position switcher
    int instantPos = static_cast<int>(factor * opt.sliderPosition + 0.5);
    {
        p.save();
        QTransform transform;
        transform.translate(opt.rect.width() / 2, opt.rect.height() / 2);
        transform.rotate((instantPos - 150) % 360);

        qreal y1 = topleft.y() + iwidth + 4;
        QPainterPath switcher(QPoint(topleft.x() + rect.width() / 2 - 1, topleft.y() - iwidth));
        switcher.lineTo(switcher.currentPosition().x() + 2, switcher.currentPosition().y());
        switcher.lineTo(switcher.currentPosition().x(), y1);
        switcher.lineTo(switcher.currentPosition().x() + iwidth, topleft.y() + 3 * rect.height() / 4);
        switcher.lineTo(switcher.currentPosition().x() - 2*iwidth - 2,  switcher.currentPosition().y());
        switcher.lineTo(switcher.currentPosition().x() + iwidth, y1);
        switcher.lineTo(topleft.x() + rect.width() / 2 - 1, topleft.y() - iwidth);
        switcher.translate(-opt.rect.width()/2, -opt.rect.height()/2);

        QRectF brect = switcher.boundingRect();
        QLinearGradient lg(brect.bottomLeft(), brect.bottomRight());
        lg.setSpread(QGradient::ReflectSpread);
        lg.setColorAt(0, bc.darker());
        lg.setColorAt(0.5, bc.lighter());
        lg.setColorAt(1, bc.darker());
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(lg));
        p.setTransform(transform);
        p.drawPath(switcher);
        p.restore();
    }
}
