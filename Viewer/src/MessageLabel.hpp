// Copyright 2014 ECMWF.

#ifndef MESSAGELABEL_HPP_
#define MESSAGELABEL_HPP_

#include <QWidget>

class QLabel;

class MessageLabel : public QWidget
{
public:
	explicit MessageLabel(QWidget *parent=0);

	enum Type {NoType,InfoType,WarningType,ErrorType};

	void showInfo(QString);
	void showWarning(QString);
	void showError(QString);
	void startLoadLabel();
	void stopLoadLabel();

private:
	void showMessage(const Type&,QString);

	Type currentType_;
	QLabel *pixLabel_;
	QLabel* msgLabel_;
	QLabel* loadLabel_;

};

#endif

