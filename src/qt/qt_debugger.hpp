#ifndef QT_DEBUGGER_HPP
#define QT_DEBUGGER_HPP

#include <QDialog>
#include <QTranslator>

namespace Ui {
class Debugger;
}

class Debugger : public QDialog {
    Q_OBJECT

public:
    explicit Debugger(QWidget *parent = nullptr);
	
	QString formatFrequency(uint32_t rspeed);
    ~Debugger();
protected slots:
	void accept() override;
    void paintEvent(QPaintEvent *event) override;
private:
    Ui::Debugger *ui;
    friend class MainWindow;
};

#endif // QT_DEBUGGER_HPP
