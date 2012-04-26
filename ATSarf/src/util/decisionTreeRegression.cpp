#include <assert.h>
#include "decisionTreeRegression.h"
#include "distinguishingLargeFileIterator.h"
#include "iterativeMathFunctions.h"


void DecisionTreeRegression::initialize(QString fileName, QList<int> & featureColumns, int targetColumn, int weightColumn){
	this->fileName=fileName;
	this->featureColumns=featureColumns;
	this->weightColumn=weightColumn;
	this->targetColumn=targetColumn;
}

DecisionTreeRegression::DecisionTreeRegression(QString fileName, QList<int> featureColumns, int targetColumn, int weightColumn){
	initialize(fileName,featureColumns,weightColumn,targetColumn);
}

DecisionTreeRegression::DecisionTreeRegression(QString fileName, QList<int> featureColumns, int targetColumn, int weightColumn, TerminationRule rule){
	initialize(fileName,featureColumns,weightColumn,targetColumn);
	this->terminationRule=rule;
}


RegressionNode * DecisionTreeRegression::buildTreeNode(QList<int> workingfeatureColumns,ConditionMap map,ATMProgressIFC * prg) {
	if (workingfeatureColumns.size()>0) {
		int indexBest=0;
		double bestReduction=0;
		QStringList bestValues;
		ItDevList bestDevs;
		for (int i=0;i<workingfeatureColumns.size();i++) {
			DistinguishingLargeFileIterator itr(fileName,targetColumn,weightColumn,map,workingfeatureColumns[i],prg);
			if (columnNames.size()==0)
				columnNames=itr.getColumnNames();
			QStringList values;
			ItDevList devs;
			ItDev totalDev;
			double reduction=IterativeMathFunctions::weightedStandardDeviationReduction(itr,values,devs,totalDev);
			if (workingfeatureColumns.size()==featureColumns.size() && i==0) //update once in first call
				terminationDevThreshold=terminationRule.minStandardDevPercentage*(totalDev.getValue());
			if (reduction>bestReduction) {
				bestDevs=devs;
				bestValues=values;
				indexBest=i;
				bestReduction=reduction;
			}
		}
		if (bestValues.size()>0) {
			int bestCol=workingfeatureColumns[indexBest];
			workingfeatureColumns.removeAt(indexBest);
			QString bestColumnName=columnNames[bestCol];
			RegressionNode * node=new RegressionNode(bestColumnName);
			for (int i=0;i<bestValues.size();i++) {
				QString val=bestValues[i];
				map[bestCol]=val;
				RegressionNode * child=NULL;
				double stDev=bestDevs[i].getValue();
				double numInstances=bestDevs[i].getInstancesNum();
				if (!(stDev<terminationDevThreshold || workingfeatureColumns.size()==0 || numInstances<terminationRule.minInstances)) {
					child=buildTreeNode(workingfeatureColumns,map,prg);
				}
				if (child==NULL) {
					double mean=bestDevs[i].getWeightedMean();
					child=new RegressionNode(QString("%1").arg(mean));
				}
				node->addNode(val,child);
			}
			return node;
		} else {
			return NULL;
		}
	} else {
		assert(false);
		return NULL; //must not reach here
	}
}

RegressionTree * DecisionTreeRegression::buildTree(ATMProgressIFC * prg) {
	columnNames.clear();
	ConditionMap map;
	QList<int> workingfeatureColumns=featureColumns;
	RegressionNode * node=buildTreeNode(workingfeatureColumns,map,prg);
	return new RegressionTree(node);
}