#ifndef QT_DEBUGGER_HPP
#define QT_DEBUGGER_HPP

#include <QDialog>
#include <QLabel>
#include <QMap>
#include <QTranslator>

namespace Ui {
class Debugger;
}

class Debugger : public QDialog {
    Q_OBJECT

public:
    explicit Debugger(QWidget *parent = nullptr);
	
	QString formatFrequency(uint32_t rspeed);
	QString formatBytes(uint32_t mem_size);
	
    ~Debugger();
protected slots:
	void accept() override;
    void paintEvent(QPaintEvent *event) override;
private:
    Ui::Debugger *ui;
	QMap<QString, QLabel*> cpu_labels;
	QList<QLabel*> qlist_buf;
	
	friend class MainWindow;
	 
	void drawCPUInfo(QPaintEvent *event);
};

#endif // QT_DEBUGGER_HPP
