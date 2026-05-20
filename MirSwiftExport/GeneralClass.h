#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QDebug>;
#include <QFile>
#include <qsqlerror>
#include <QSqlQuery>
#include <iostream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <qtimer>
#include <QString>
#include <QPair>

class GeneralClass  : public QObject
{
	Q_OBJECT

public:
	GeneralClass(QObject *parent = nullptr);
	~GeneralClass();

	void connectToDb();
	void getAllDevice();
	void getMiddleId(QList<QPair<QString, QString>>tempArr);

	void getAllValueForEnergy();

	void getIdFromIdValues(QList<QPair<QString, QString>>tempArr);


private:

	QSqlDatabase mainConnection;
};

