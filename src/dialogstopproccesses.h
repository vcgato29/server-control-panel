#ifndef DIALOGSTOPPROCCESSES_H
#define DIALOGSTOPPROCCESSES_H

#include <QDialog>

namespace Ui {
class DialogStopProccesses;
}

class DialogStopProccesses : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogStopProccesses(QWidget *parent = 0);
    ~DialogStopProccesses();
    
private:
    Ui::DialogStopProccesses *ui;
};

#endif // DIALOGSTOPPROCCESSES_H
