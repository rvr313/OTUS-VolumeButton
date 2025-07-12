#ifndef VOLUMEBUTTON_H
#define VOLUMEBUTTON_H

#include <QDial>
#include <QTimeLine>

class VolumeButton : public QDial
{
    Q_OBJECT

    QTimeLine m_time_line;
    int m_prev_value{0};
    int m_curr_value{0};

public:
    explicit VolumeButton(QWidget *parent = nullptr);



    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // VOLUMEBUTTON_H
