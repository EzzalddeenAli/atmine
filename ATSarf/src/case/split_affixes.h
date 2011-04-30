#ifndef SPLIT_AFFIXES_H
#define SPLIT_AFFIXES_H

#include <QDialog>
#include <QComboBox>
//#include <QHBoxLayout>
//#include <QVBoxLayout>
#include <QHeaderView>
#include <QStringList>
#include <QPushButton>
#include <QGridLayout>
#include <QComboBox>
#include <QTableWidget>
#include <QTextBrowser>
#include "Retrieve_Template.h"
#include "Search_by_item.h"
#include "database_info_block.h"
#include "sql_queries.h"
#include "logger.h"
#include <iostream>

using namespace std;

class SplitDialog:public QDialog{
	Q_OBJECT
public:
	SplitDialog():QDialog() {
		errors=new QTextBrowser(this);
		errors->resize(errors->width(),50);
		errors_text=new QString();
		split=new QPushButton("&Split",this);
		affixType=new QComboBox(this);
		affixType->addItem("Prefix",PREFIX);
		affixType->addItem("Suffix",SUFFIX);
		originalAffixList=new QTableWidget(0,6,this);
		compatRulesList=new QTableWidget(0,3,this);
		grid=new QGridLayout(this);
		grid->addWidget(split,0,0);
		grid->addWidget(affixType,0,1);
		grid->addWidget(originalAffixList,1,0,1,2);
		grid->addWidget(compatRulesList,2,0,1,2);
		grid->addWidget(errors,3,0,1,2);
		connect(split,SIGNAL(clicked()),this,SLOT(split_clicked()));
		connect(affixType,SIGNAL(currentIndexChanged(int)),this,SLOT(affixType_currentIndexChanged(int)));
		displayed_error.setString(errors_text);
		loadAffixList();
		loadCompatibilityList();
		this->resize(700,700);
		errors->setText(*errors_text);
		source_id=-1;
	}
	void loadAffixList() {
		QString table=interpret_type((item_types)affixType->itemData(affixType->currentIndex()).toInt());
		Columns list;
		originalAffixList->clear();
		QStringList v;
		v<<table+" ID"<<"Affix"<<"Category"<<"Raw Data"<<"POS"<<"Description";
		originalAffixList->verticalHeader()->setHidden(true);
		originalAffixList->setHorizontalHeaderLabels(v);
		list.append(table+"_id");
		list.append("category_id");
		list.append("raw_data");
		list.append("POS");
		list.append("description_id");
		Retrieve_Template s(table+"_category",list,"1=1 ORDER BY "+table+"_id ASC");
		originalAffixList->setRowCount(s.size());
		originalAffixList->setEditTriggers(QAbstractItemView::NoEditTriggers);
		originalAffixList->setSelectionBehavior(QAbstractItemView::SelectRows);
		originalAffixList->setSelectionMode(QAbstractItemView::SingleSelection);
		int row=0;
		while (s.retrieve())
		{
			long id=s.get(0).toLongLong();
			originalAffixList->setItem(row,0,new QTableWidgetItem(tr("%1").arg(id)));
			originalAffixList->setItem(row,1,new QTableWidgetItem(getAffix(id)));
			originalAffixList->setItem(row,2,new QTableWidgetItem(database_info.comp_rules->getCategoryName(s.get(1).toLongLong())));
			originalAffixList->setItem(row,3,new QTableWidgetItem(s.get(2).toString()));
			originalAffixList->setItem(row,4,new QTableWidgetItem(s.get(3).toString()));
			long description_id=s.get(4).toLongLong();
			QString desc;
			if (description_id>0 && description_id<database_info.descriptions->size())
				desc= database_info.descriptions->at(description_id);
			else
				desc= QString::null;
			originalAffixList->setItem(row,5,new QTableWidgetItem(desc));
			row++;
		}
	}
	void loadCompatibilityList() {
		QStringList v;
		QString table=interpret_type((item_types)affixType->itemData(affixType->currentIndex()).toInt());
		v<<table+" Category 1"<<table+" Category 2"<<"Resulting Category";
		compatRulesList->setHorizontalHeaderLabels(v);
		compatRulesList->setEditTriggers(QAbstractItemView::NoEditTriggers);
		compatRulesList->setSelectionBehavior(QAbstractItemView::SelectRows);
		compatRulesList->setSelectionMode(QAbstractItemView::NoSelection);
		compatRulesList->setRowCount(0);
	}

public slots:
	void split_clicked() {
		split_action();
	}
	void affixType_currentIndexChanged(int)
	{
		loadAffixList();
		loadCompatibilityList();
		errors->clear();
	}

private:
	void split_action();

	QString getAffix(long id) {
		item_types t=(item_types)affixType->itemData(affixType->currentIndex()).toInt();
		QString table=interpret_type(t);
		Retrieve_Template s(table,"name",tr("id=%1").arg(id));
		if (s.retrieve())
			return s.get(0).toString();
		else
			return "--";
	}
	int getRow(const QString & affix,const QString & raw_data, const QString & pos, const QString description) const {
		if (affix.isEmpty())
			return -1;
		for (int i=0;i<originalAffixList->rowCount();i++) {
			//long affix_id1=originalAffixList->item(i,0)->text().toLongLong();
			QString affix1=originalAffixList->item(i,1)->text();
			//QString category2=originalAffixList->item(i,2)->text();
			QString raw_data1=originalAffixList->item(i,3)->text();
			QString pos1=originalAffixList->item(i,4)->text();
			QString description1=originalAffixList->item(i,5)->text();
			if (	affix1==affix &&
					raw_data1==raw_data &&
					pos1==pos &&
					description1==description) {
				/*if (description1!=description)
					warning<<"In ("<<affix<<","<<raw_data<<","<<pos<<") different descriptions found and used the first:"<<description<<" Versus "<<description1<<".\n";*/
				return i;
			}
		}
		return -1;
	}

public:
	int source_id;
	QTextEdit * affix;
	QPushButton * split;
	QComboBox * affixType;
	QTextBrowser * errors;
	QString * errors_text;
	QTableWidget * originalAffixList, * compatRulesList;
	QGridLayout * grid;

	~SplitDialog() {
		delete affix;
		delete split;
		delete affixType;
		delete errors;
		delete errors_text;
		delete originalAffixList;
		delete compatRulesList;
		delete grid;
	}
};


#endif // SPLIT_AFFIXES_H