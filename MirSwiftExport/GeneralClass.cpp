#include "GeneralClass.h"

GeneralClass::GeneralClass(QObject* parent)
	: QObject(parent)
{
	if (connectToDb())
	{
		getAllDevice();
		getAllValueForEnergy();


		/*
		for (int counter = 0, counterValue = 0; counter < idMiddleSerialFinal.length(); ++counter, ++counterValue)
		{
			QString temp;
			QString dayValue = finalArrIdAndValueFinal[counterValue].second.second;

			if (dayValue.length() > 3)
				dayValue.insert(dayValue.length() - 3, ",");
			else
				dayValue.push_front("0,");

			temp += idMiddleSerialFinal[counter].second + " " + dayValue;
			++counterValue;

			QString nightValue = finalArrIdAndValueFinal[counterValue].second.second;

			if (nightValue.length() > 3)
				nightValue.insert(nightValue.length() - 3, ",");
			else
				nightValue.push_front("0,");

			temp += " " + nightValue;

			if (idMiddleSerialFinal[counter].second == "") continue;

			qDebug() << temp;
		}
		*/

		importFromXlsFunc();
		importInConfiguration();

		mainConnection.close();
	}
}



GeneralClass::~GeneralClass()
{
	if (mainConnection.isOpen())
		mainConnection.close();

	CoUninitialize();  // Освобождение COM
}



bool GeneralClass::connectToDb()
{
	QFile tempForCheckDb;
	//tempForCheckDb.setFileName("C://Users//admin//source//repos//AddressSpaceFULL.db"); // рихтануть метод в плане указания базы

	std::string dbString = "importBase.db";

	/*
	do {
		std::cout << "Enter name of your dataBase in app directory: ";
		std::cin >> dbString;

		if (std::cin.fail() || dbString.length() < 0 || dbString.length() > 100) {
			std::cin.clear();
			std::cin.ignore(36501, '\n');  // Очищаем буфер
			std::cout << "Incorrect name. Try again.\n";
		}
	} while (std::cin.fail() || dbString.length() < 0 || dbString.length() > 100);
	*/
	tempForCheckDb.setFileName(QString::fromStdString(dbString).trimmed()); // рихтануть метод в плане указания базы

	if (!tempForCheckDb.exists())
	{
		qDebug() << "NOT found your dataBase";
		return false;
	}

	mainConnection = QSqlDatabase::addDatabase("QSQLITE", "mirSwiftDb");
	mainConnection.setDatabaseName(QString::fromStdString(dbString).trimmed());

	if (!mainConnection.open())
	{
		if (mainConnection.lastError().isValid())
		{
			qDebug() << "Error in connectToDb::connectToDb() when try to create/open nameDb. Error:\n" << mainConnection.lastError().text();
		}
		else
			qDebug() << "NOT OPEN nameDb";

		return false;
	}
	else
	{
		return true;
	}
}



void GeneralClass::getAllDevice()
{
	QSqlQuery queryMain(mainConnection);
	QSqlQuery queryCheck(mainConnection);

	QString queryString = "select object_owner_id, property_value, time_stamp from properties where property_type_id = 987 and property_value != '0'";

	QList<QPair<QString, QString>>idSerialList;

	if (!queryMain.exec(queryString) || !queryMain.next())
	{
		if (queryMain.lastError().isValid())
		{
			qDebug() << "Error in getAllData() when try to first read. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
		}
		else
			qDebug() << "NOT doing first read";

		return;
	}
	else
	{
		// Делаем первичную проверку на предмет того что это та запись с серийником которая нам требуется

		//std::cout << "First serial in func: " << queryMain.value(0).toString().toStdString() << "   " << queryMain.value(1).toString().toStdString() << std::endl;

		QString queryCheckString = QString("select * from objects where object_id = %1").arg(queryMain.value(0).toString());

		if (!queryCheck.exec(queryCheckString) || !queryCheck.next())
		{
			if (queryCheck.lastError().isValid())
			{
				qDebug() << "Error in getAllData() when try to check first read. Query:\n" << queryCheck.lastQuery() << "\nError text:\n" << queryCheck.lastError().text();
			}
			else
				qDebug() << "NOT check first read in start";

			return;
		}
		else
		{
			//std::cout << "First check in func: " << queryCheck.value(0).toString().toStdString() << "   " << queryCheck.value(1).toString().toStdString() << "   " << queryCheck.value(2).toString().toStdString() << std::endl;

			if (queryCheck.value(2).toString() == "Параметры устройства")
			{
				idSerialList.push_back(qMakePair(queryMain.value(0).toString(), queryMain.value(1).toString()));
			}
		}

		// Делаем последующие проверки на предмет того что это те записи с серийником которые нам требуется

		while (queryMain.next())
		{
			//std::cout << "Next serial in func: " << queryMain.value(0).toString().toStdString() << "   " << queryMain.value(1).toString().toStdString() << std::endl;

			QString queryCheckString = QString("select * from objects where object_id = %1").arg(queryMain.value(0).toString());

			if (!queryCheck.exec(queryCheckString) || !queryCheck.next())
			{
				if (queryCheck.lastError().isValid())
				{
					qDebug() << "Error in getAllData() when try to check first read. Query:\n" << queryCheck.lastQuery() << "\nError text:\n" << queryCheck.lastError().text();
				}
				else
					qDebug() << "NOT check first read";

				return;
			}
			else
			{
				//std::cout << "Next check in func: " << queryCheck.value(0).toString().toStdString() << "   " << queryCheck.value(1).toString().toStdString() << "   " << queryCheck.value(2).toString().toStdString() << std::endl;

				if (queryCheck.value(2).toString() == "Параметры устройства")
				{
					idSerialList.push_back(qMakePair(queryMain.value(0).toString(), queryMain.value(1).toString()));
				}
			}
		}
	}

	idMiddleSerialFinal = getMiddleId(idSerialList);
}



QList<QPair<QString, QString>> GeneralClass::getMiddleId(QList<QPair<QString, QString>>tempArr)
{
	QSqlQuery queryMain(mainConnection);
	QList<QPair<QString, QString>>idMiddleSerial;

	for (auto& val : tempArr)
	{
		QString queryString = QString("SELECT link_id, object_from_id, object_to_id FROM links where object_to_id = %1").arg(val.first);

		if (!queryMain.exec(queryString) || !queryMain.next())
		{
			if (queryMain.lastError().isValid())
			{
				qDebug() << "Error in getMiddleId() when try to get middle id. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
			}
			else
				qDebug() << "NOT doing get middle id";

			break;
		}
		else
		{
			queryString = QString("SELECT link_id, object_from_id, object_to_id FROM links where object_to_id = %1").arg(queryMain.value(1).toString());

			if (!queryMain.exec(queryString) || !queryMain.next())
			{
				if (queryMain.lastError().isValid())
				{
					qDebug() << "Error in getMiddleId() when try to get next middle id. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
				}
				else
					qDebug() << "NOT doing get next middle id";

				break;
			}
			else
			{
				idMiddleSerial.push_back(qMakePair(queryMain.value(1).toString(), val.second));
			}
		}
	}

	return idMiddleSerial;
}



void GeneralClass::getAllValueForEnergy()
{
	QString queryString = "select object_id, object_type_id, object_name from objects where object_type_id = 500105 OR object_type_id = 500106";

	QSqlQuery queryMain(mainConnection);
	QSqlQuery querySecond(mainConnection);

	QList<QPair<QString, QString>>idAndEnergy;

	if (!queryMain.exec(queryString) || !queryMain.next())
	{
		if (queryMain.lastError().isValid())
		{
			qDebug() << "Error in getAllValueForEnergy() when try to get all value id. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
		}
		else
			qDebug() << "NOT get all value";

		return;
	}
	else
	{
		QString queryString = QString("SELECT object_owner_id, property_value, time_stamp FROM properties where object_owner_id = %1").arg(queryMain.value(0).toString());

		if (!querySecond.exec(queryString) || !querySecond.next())
		{
			if (querySecond.lastError().isValid())
			{
				qDebug() << "Error in getAllValueForEnergy() when try to get first querySecond. Query:\n" << querySecond.lastQuery() << "\nError text:\n" << querySecond.lastError().text();
			}
			else
				qDebug() << "NOT get all value";

			return;
		}
		else
		{
			idAndEnergy.push_back(qMakePair(querySecond.value(0).toString(), querySecond.value(1).toString()));
		}

		while (queryMain.next())
		{
			QString queryString = QString("SELECT object_owner_id, property_value, time_stamp FROM properties where object_owner_id = %1").arg(queryMain.value(0).toString());

			if (!querySecond.exec(queryString) || !querySecond.next())
			{
				if (querySecond.lastError().isValid())
				{
					qDebug() << "Error in getAllValueForEnergy() when try to get first querySecond. Query:\n" << querySecond.lastQuery() << "\nError text:\n" << querySecond.lastError().text();
				}
				else
					qDebug() << "NOT get all value";

				return;
			}
			else
			{
				idAndEnergy.push_back(qMakePair(querySecond.value(0).toString(), querySecond.value(1).toString()));
			}
		}
	}

	finalArrIdAndValueFinal = getIdFromIdValues(idAndEnergy);
}



QList<QPair<QString, QPair<QString, QString>>> GeneralClass::getIdFromIdValues(QList<QPair<QString, QString>>tempArr)
{
	QSqlQuery queryMain(mainConnection);

	QList<QPair<QString, QPair<QString, QString>>>finalArrIdAndValue;


	for (auto& val : tempArr)
	{
		QString queryString = QString("SELECT link_id, object_from_id, object_to_id FROM links where object_to_id = %1").arg(val.first);

		if (!queryMain.exec(queryString) || !queryMain.next())
		{
			if (queryMain.lastError().isValid())
			{
				qDebug() << "Error in getMiddleId() when try to get middle id. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
			}
			else
				qDebug() << "NOT doing get middle id";

			break;
		}
		else
		{
			queryString = QString("SELECT link_id, object_from_id, object_to_id FROM links where object_to_id = %1").arg(queryMain.value(1).toString());

			QString tempId = queryMain.value(2).toString();

			if (!queryMain.exec(queryString) || !queryMain.next())
			{
				if (queryMain.lastError().isValid())
				{
					qDebug() << "Error in getMiddleId() when try to get next middle id. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
				}
				else
					qDebug() << "NOT doing get next middle id";

				break;
			}
			else
			{
				finalArrIdAndValue.push_back(qMakePair(queryMain.value(1).toString(), qMakePair(val.first, val.second)));
			}
		}
	}

	return finalArrIdAndValue;
}



void GeneralClass::importFromXlsFunc()
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
		qDebug() << "CoInitializeEx failed:" << hr;
		return;
	}

	QString addFileDonor = QCoreApplication::applicationDirPath() + "/123.xlsx";

	QFile tempForCheckFile(addFileDonor);

	if (!tempForCheckFile.exists()) 
	{
		qDebug() << "Файл не найден:" << addFileDonor;
		CoUninitialize();
		return;
	}

	QString pKey, pValue;

	QSharedPointer<QAxObject>excelDonor(new QAxObject("Excel.Application", 0));
	QSharedPointer<QAxObject>workbooksDonor(excelDonor->querySubObject("Workbooks"));
	QSharedPointer<QAxObject>workbookDonor(workbooksDonor->querySubObject("Open(const QString&)", addFileDonor));
	QSharedPointer<QAxObject>sheetsDonor(workbookDonor->querySubObject("Worksheets"));

	QSharedPointer<QAxObject>sheetDonor(sheetsDonor->querySubObject("Item(int)", 1));
	QSharedPointer<QAxObject>usedRangeDonor(sheetDonor->querySubObject("UsedRange"));
	QSharedPointer<QAxObject>rowsDonor(usedRangeDonor->querySubObject("Rows"));
	int countRowsDonor = rowsDonor->property("Count").toInt();
	QSharedPointer<QAxObject>usedRangeColDonor(sheetDonor->querySubObject("UsedRange"));
	QSharedPointer<QAxObject>columnsDonor(usedRangeColDonor->querySubObject("Columns"));
	int countColsDonor = columnsDonor->property("Count").toInt();

	for (int row = 1; row <= countRowsDonor; ++row)
	{
		for (int column = 1; column <= 2; ++column)
		{
			QSharedPointer<QAxObject>cell(sheetDonor.data()->querySubObject("Cells(int,int)", row, column)); // так указываем с какой ячейкой работать
			column == 1 ? pKey = cell->property("Value").toString().trimmed() : pValue = cell->property("Value").toString().trimmed();
		}

		xlsArrayForImport.push_back(qMakePair(pKey, pValue));
	}

	for (auto val : xlsArrayForImport)
		std::cout << val.first.toStdString() << "   " << val.second.toStdString() << "\n";

	workbookDonor->dynamicCall("Close()");
	excelDonor->dynamicCall("Quit()");

	CoUninitialize();
}



void GeneralClass::importInConfiguration()
{
	QString queryString = "SELECT object_id, object_name FROM objects where object_type_id like '1041%'";

	QSqlQuery queryMain(mainConnection);
	QSqlQuery queryChangeNameAndValue(mainConnection);

	if (!queryMain.exec(queryString) || !queryMain.next())
	{
		if (queryMain.lastError().isValid())
		{
			qDebug() << "Error in importInConfiguration() when try to get all tree object. Query:\n" << queryMain.lastQuery() << "\nError text:\n" << queryMain.lastError().text();
		}
		else
			qDebug() << "NOT get tree object";

		return;
	}
	else
	{
		qDebug() << "\nStart";/////////////////

		int counterArray = 0;

		do
		{
			QString queryChange = QString("UPDATE objects SET object_name = '%1' WHERE object_id = '%2'")
				.arg(counterArray >= xlsArrayForImport.length() ? "empty" : xlsArrayForImport[counterArray].first)
				.arg(queryMain.value(0).toString());

			if (!queryChangeNameAndValue.exec(queryChange))
			{
				if (queryChangeNameAndValue.lastError().isValid())
				{
					qDebug() << "Error in importInConfiguration() when try to update name of object tree. Query:\n" << queryChangeNameAndValue.lastQuery() << "\nError text:\n" << queryChangeNameAndValue.lastError().text();
					std::cout << "\n" << (counterArray >= xlsArrayForImport.length() ? "" : xlsArrayForImport[counterArray].first.toStdString()) << "  " << queryMain.value(0).toString().toStdString() << "\n" << queryChange.toStdString();
				}
				else
					qDebug() << "NOT update name";
			}
			else
			{
				 queryChange = QString("UPDATE properties SET property_value = '%1' WHERE object_owner_id = '%2' AND property_type_id = '987'")
					 .arg(counterArray >= xlsArrayForImport.length() ? "00000000" : xlsArrayForImport[counterArray].second)
					 .arg(queryMain.value(0).toString());

				 std::cout << "\n" << (counterArray >= xlsArrayForImport.length() ? "" : xlsArrayForImport[counterArray].second.toStdString()) << "  " << queryMain.value(0).toString().toStdString() << "\n" << queryChange.toStdString();


				if (!queryChangeNameAndValue.exec(queryChange))
				{
					if (queryChangeNameAndValue.lastError().isValid())
					{
						qDebug() << "Error in importInConfiguration() when try to update number of object tree. Query:\n" << queryChangeNameAndValue.lastQuery() << "\nError text:\n" << queryChangeNameAndValue.lastError().text();
						std::cout << "\n" << (counterArray >= xlsArrayForImport.length() ? "" : xlsArrayForImport[counterArray].first.toStdString()) << "  " << queryMain.value(0).toString().toStdString() << "\n" << queryChange.toStdString();
					}
					else
						qDebug() << "NOT update number";
				}
			}

			std::cout << counterArray << " is done";

			++counterArray;

		} while (queryMain.next());

		qDebug() << "\nFINISH";//////////////////////
	}
}